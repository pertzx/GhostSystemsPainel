#pragma once
#include <cstdint>
#include <cstring>

namespace Obf {
    constexpr uint32_t KEY = 0x5A3C7F2E;

    constexpr uint32_t rotl32(uint32_t x, uint32_t r) {
        return (x << r) | (x >> (32 - r));
    }

    constexpr char encChar(char c, uint32_t k, size_t i) {
        return c ^ ((k >> ((i * 7) % 24)) & 0xFF);
    }

    template<size_t N>
    struct StringLiteral {
        char data[N];
        constexpr StringLiteral(const char* str) {
            for (size_t i = 0; i < N; i++) {
                data[i] = encChar(str[i], k, i);
            }
        }
        static constexpr uint32_t k = KEY;

        char* decrypt(char* out) const {
            for (size_t i = 0; i < N; i++) {
                out[i] = encChar(data[i], k, i);
            }
            out[N - 1] = '\0';
            return out;
        }
    };

    template<size_t N>
    constexpr StringLiteral<N> make_obf(const char* str) {
        return StringLiteral<N>(str);
    }
}

#define OBF(x) ([&]() { \
    static constexpr auto _s = Obf::make_obf<sizeof(x)>(x); \
    thread_local char _buf[sizeof(x)]; \
    return _s.decrypt(_buf); \
}())

#define OBF_CONST(x) Obf::make_obf<sizeof(x)>(x)