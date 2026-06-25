#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <cstring>
#include <android/log.h>
#include <unistd.h>

#define LOG_TAG "GhostSystems"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace GhostSystems {

struct DelayedPacket {
    int sockfd;
    void* buf;
    size_t len;
    int flags;
    sockaddr_storage dest_addr;
    socklen_t addrlen;
    std::chrono::steady_clock::time_point sendTime;
    bool isTelemetry;
};

class FakeLag {
public:
    static FakeLag& getInstance() {
        static FakeLag instance;
        return instance;
    }
    
    // Inicializa o sistema de FakeLag
    void init();
    
    // Define o delay em ms
    void setDelay(int ms);
    int getDelay() const { return delayMs; }
    
    // Ativa/desativa
    void setEnabled(bool enabled) { isEnabled = enabled; }
    bool getEnabled() const { return isEnabled; }
    
    // Hook para sendto (deve ser chamado no lugar de sendto original)
    static ssize_t hook_sendto(int sockfd, const void* buf, size_t len, int flags, 
                               const struct sockaddr* dest_addr, socklen_t addrlen);
    
    // Hook para recvfrom
    static ssize_t hook_recvfrom(int sockfd, void* buf, size_t len, int flags,
                                 struct sockaddr* src_addr, socklen_t* addrlen);
    
    // Ponteiros para funcoes originais
    static ssize_t (*orig_sendto)(int, const void*, size_t, int, const struct sockaddr*, socklen_t);
    static ssize_t (*orig_recvfrom)(int, void*, size_t, int, struct sockaddr*, socklen_t*);
    
    // Estatisticas
    size_t getQueuedPacketsCount() const;
    size_t getTotalDelayedPackets() const { return totalDelayedPackets; }
    size_t getTotalDroppedPackets() const { return totalDroppedPackets; }
    
private:
    FakeLag() : isEnabled(false), delayMs(100), stopThread(false), 
                totalDelayedPackets(0), totalDroppedPackets(0) {}
    ~FakeLag();
    
    // Thread que processa pacotes atrasados
    void processLoop();
    
    // Verifica se pacote eh de telemetria/anti-cheat
    bool isTelemetryPacket(const void* buf, size_t len, const struct sockaddr* addr);
    
    // Verifica se endereco pertence ao servidor do jogo
    bool isGameServer(const struct sockaddr* addr);
    
    // Fila de pacotes atrasados
    std::queue<DelayedPacket> packetQueue;
    std::mutex queueMutex;
    
    // Thread de processamento
    std::thread workerThread;
    
    // Configuracoes
    bool isEnabled;
    int delayMs;
    bool stopThread;
    
    // Estatisticas
    size_t totalDelayedPackets;
    size_t totalDroppedPackets;
    
    // IPs conhecidos de servidores Free Fire (para detectar telemetria)
    std::vector<std::string> knownGameServers;
};

} // namespace GhostSystems
