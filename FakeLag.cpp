#include "FakeLag.h"
#include <arpa/inet.h>
#include <algorithm>

namespace GhostSystems {

// Ponteiros para funcoes originais
ssize_t (*FakeLag::orig_sendto)(int, const void*, size_t, int, const struct sockaddr*, socklen_t) = nullptr;
ssize_t (*FakeLag::orig_recvfrom)(int, void*, size_t, int, struct sockaddr*, socklen_t*) = nullptr;

FakeLag::~FakeLag() {
    stopThread = true;
    if (workerThread.joinable()) {
        workerThread.join();
    }
    
    // Limpar fila
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!packetQueue.empty()) {
        DelayedPacket& pkt = packetQueue.front();
        if (pkt.buf) free(pkt.buf);
        packetQueue.pop();
    }
}

void FakeLag::init() {
    // IPs conhecidos de servidores Free Fire (atualizar conforme necessario)
    knownGameServers = {
        "119.28.",     // Tencent Asia
        "182.254.",    // Tencent China
        "13.107.",     // Azure/Microsoft
        "52.94.",      // AWS
        "18.188.",     // AWS
        "3.0.",        // AWS
        "35.155.",     // GCP
        "104.155.",    // GCP
        "34.95.",      // GCP
    };
    
    // Iniciar thread de processamento
    stopThread = false;
    workerThread = std::thread(&FakeLag::processLoop, this);
    
    LOGI("[FakeLag] Inicializado com delay de %dms", delayMs);
}

void FakeLag::setDelay(int ms) {
    if (ms >= 0 && ms <= 2000) {
        delayMs = ms;
        LOGI("[FakeLag] Delay ajustado para %dms", delayMs);
    }
}

ssize_t FakeLag::hook_sendto(int sockfd, const void* buf, size_t len, int flags,
                             const struct sockaddr* dest_addr, socklen_t addrlen) {
    FakeLag& instance = getInstance();
    
    if (!instance.isEnabled || !buf || len == 0) {
        return orig_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    }
    
    // Verificar se eh pacote de telemetria
    if (instance.isTelemetryPacket(buf, len, dest_addr)) {
        // Criar copia do pacote
        void* bufCopy = malloc(len);
        if (!bufCopy) {
            return orig_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
        }
        memcpy(bufCopy, buf, len);
        
        // Criar entrada na fila
        DelayedPacket pkt;
        pkt.sockfd = sockfd;
        pkt.buf = bufCopy;
        pkt.len = len;
        pkt.flags = flags;
        memcpy(&pkt.dest_addr, dest_addr, addrlen);
        pkt.addrlen = addrlen;
        pkt.sendTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(instance.delayMs);
        pkt.isTelemetry = true;
        
        {
            std::lock_guard<std::mutex> lock(instance.queueMutex);
            instance.packetQueue.push(pkt);
        }
        
        instance.totalDelayedPackets++;
        LOGI("[FakeLag] Pacote de telemetria atrasado (delay: %dms, tamanho: %zu)", instance.delayMs, len);
        
        // Retornar sucesso imediatamente (simular envio)
        return len;
    }
    
    // Nao eh telemetria, enviar normalmente
    return orig_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

ssize_t FakeLag::hook_recvfrom(int sockfd, void* buf, size_t len, int flags,
                               struct sockaddr* src_addr, socklen_t* addrlen) {
    // Para recvfrom, podemos adicionar delay tambem se necessario
    // Por enquanto, apenas passar para a funcao original
    return orig_recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
}

void FakeLag::processLoop() {
    LOGI("[FakeLag] Thread de processamento iniciada");
    
    while (!stopThread) {
        auto now = std::chrono::steady_clock::now();
        
        std::vector<DelayedPacket> packetsToSend;
        
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            
            while (!packetQueue.empty()) {
                DelayedPacket& pkt = packetQueue.front();
                
                if (now >= pkt.sendTime) {
                    packetsToSend.push_back(pkt);
                    packetQueue.pop();
                } else {
                    break; // Os proximos ainda nao estao prontos
                }
            }
        }
        
        // Enviar pacotes prontos
        for (auto& pkt : packetsToSend) {
            if (orig_sendto) {
                orig_sendto(pkt.sockfd, pkt.buf, pkt.len, pkt.flags, 
                           (struct sockaddr*)&pkt.dest_addr, pkt.addrlen);
            }
            
            if (pkt.buf) {
                free(pkt.buf);
            }
            
            LOGI("[FakeLag] Pacote enviado apos delay");
        }
        
        // Dormir 1ms para nao consumir CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    LOGI("[FakeLag] Thread de processamento encerrada");
}

bool FakeLag::isTelemetryPacket(const void* buf, size_t len, const struct sockaddr* addr) {
    if (!buf || len < 4) return false;
    
    // Verificar se eh endereco do servidor do jogo
    if (!isGameServer(addr)) {
        return false;
    }
    
    // Heuristicas para detectar pacotes de telemetria
    const unsigned char* data = (const unsigned char*)buf;
    
    // Padroes comuns em pacotes de telemetria/anti-cheat
    // 1. Tamanho pequeno (telemetria eh geralmente < 1KB)
    if (len > 2048) return false;
    
    // 2. Primeiros bytes indicam protocolo de telemetria
    // Exemplo: 0x01 0x00 0x00 0x00 (comum em protocolos de jogos)
    if (data[0] == 0x01 && data[1] == 0x00) {
        return true;
    }
    
    // 3. Pacotes com strings JSON (logs de eventos)
    if (len > 10) {
        if (memcmp(data, "{\"", 2) == 0 || memcmp(data, "[\x00", 2) == 0) {
            return true;
        }
    }
    
    // 4. Portas comuns de telemetria (443, 80, 8080, 10000-20000)
    if (addr->sa_family == AF_INET) {
        const struct sockaddr_in* sin = (const struct sockaddr_in*)addr;
        int port = ntohs(sin->sin_port);
        if (port == 443 || port == 80 || port == 8080 || (port >= 10000 && port <= 20000)) {
            return true;
        }
    }
    
    return false;
}

bool FakeLag::isGameServer(const struct sockaddr* addr) {
    if (!addr) return false;
    
    char ipStr[INET6_ADDRSTRLEN];
    
    if (addr->sa_family == AF_INET) {
        const struct sockaddr_in* sin = (const struct sockaddr_in*)addr;
        inet_ntop(AF_INET, &sin->sin_addr, ipStr, sizeof(ipStr));
    } else if (addr->sa_family == AF_INET6) {
        const struct sockaddr_in6* sin6 = (const struct sockaddr_in6*)addr;
        inet_ntop(AF_INET6, &sin6->sin6_addr, ipStr, sizeof(ipStr));
    } else {
        return false;
    }
    
    std::string ip(ipStr);
    
    // Verificar se IP comeca com algum dos prefixos conhecidos
    for (const auto& prefix : knownGameServers) {
        if (ip.rfind(prefix, 0) == 0) {
            return true;
        }
    }
    
    return false;
}

size_t FakeLag::getQueuedPacketsCount() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return packetQueue.size();
}

} // namespace GhostSystems
