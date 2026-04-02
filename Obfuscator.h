#pragma once

namespace GhostSystems {

    // Simples ofuscação de strings XOR em tempo de compilação
    // para evitar detecção básica por análise estática
    template <unsigned int N, char K>
    struct ObfuscatedString {
        char data[N];

        constexpr ObfuscatedString(const char(&str)[N]) : data{0} {
            for (unsigned int i = 0; i < N; ++i) {
                data[i] = str[i] ^ K;
            }
        }

        const char* decrypt() {
            static char decrypted[N];
            for (unsigned int i = 0; i < N; ++i) {
                decrypted[i] = data[i] ^ K;
            }
            return decrypted;
        }
    };

} // namespace GhostSystems

#define OBFUSCATE(str) (GhostSystems::ObfuscatedString<sizeof(str), 0x5A>(str).decrypt())
