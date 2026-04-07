#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <algorithm>

namespace obfuscator {

constexpr uint32_t hash_compile_time(const char* str) {
    uint32_t hash = 0;
    for (int i = 0; str[i]; ++i) hash = (hash * 31) + static_cast<uint32_t>(str[i]);
    return hash;
}

struct Seed {
    static constexpr uint64_t fnv1a(const char* str) {
        uint64_t hash = 0xcbf29ce484222325ULL;
        for (size_t i = 0; str[i] != '\0'; ++i) {
            hash ^= static_cast<uint64_t>(str[i]);
            hash *= 0x100000001b3ULL;
        }
        return hash;
    }

    template <size_t N>
    static constexpr uint32_t generate(uint32_t line) {
        uint64_t s = fnv1a(__TIME__) ^ (static_cast<uint64_t>(line) * 0xdeadbeefBEEFdeadULL);
        s = (s * 6364136223846793005ULL + 1442695040888963407ULL);
        return static_cast<uint32_t>(s ^ (s >> 32));
    }
};

enum class LayerType : uint8_t {
    XOR_SIMPLE = 0,
    XOR_STATEFUL = 1,
    ROL = 2,
    ADDITIVE = 3,
    SHUFFLE = 4,
    COUNT
};

template <size_t N>
struct Meta {
    std::array<LayerType, 6> pipeline;
    uint8_t layer_count;
    uint32_t key_seed;
};

namespace layers {
    template <size_t N>
    constexpr void xor_stateful_enc(std::array<char, N>& data, uint32_t key) {
        char prev = static_cast<char>(key >> 24);
        for (size_t i = 0; i < N; ++i) {
            char k = static_cast<char>((key >> ((i % 4) * 8)) & 0xFF);
            data[i] ^= (k ^ static_cast<char>(i * 17) ^ prev);
            prev = data[i];
        }
    }

    template <size_t N>
    void xor_stateful_dec(char* data, uint32_t key) {
        char prev_base = static_cast<char>(key >> 24);
        for (int i = static_cast<int>(N) - 1; i >= 0; --i) {
            char current_enc = data[i];
            char prev = (i == 0) ? prev_base : data[i - 1];
            char k = static_cast<char>((key >> ((i % 4) * 8)) & 0xFF);
            data[i] ^= (k ^ static_cast<char>(i * 17) ^ prev);
        }
    }

    template <size_t N>
    constexpr void xor_simple_enc(std::array<char, N>& data, uint32_t key) {
        for (size_t i = 0; i < N; ++i) {
            data[i] ^= static_cast<char>((key >> (i % 4)) ^ (i * 13));
        }
    }

    template <size_t N>
    void xor_simple_dec(char* data, uint32_t key) {
        for (size_t i = 0; i < N; ++i) {
            data[i] ^= static_cast<char>((key >> (i % 4)) ^ (i * 13));
        }
    }

    template <size_t N>
    constexpr void rol_enc(std::array<char, N>& data, uint8_t shift) {
        uint8_t s = (shift % 7) + 1;
        for (size_t i = 0; i < N; ++i) {
            uint8_t val = static_cast<uint8_t>(data[i]);
            data[i] = static_cast<char>((val << s) | (val >> (8 - s)));
        }
    }

    template <size_t N>
    void ror_dec(char* data, uint8_t shift) {
        uint8_t s = (shift % 7) + 1;
        for (size_t i = 0; i < N; ++i) {
            uint8_t val = static_cast<uint8_t>(data[i]);
            data[i] = static_cast<char>((val >> s) | (val << (8 - s)));
        }
    }

    template <size_t N>
    constexpr void add_enc(std::array<char, N>& data, uint32_t key) {
        for (size_t i = 0; i < N; ++i) {
            data[i] += static_cast<char>((key >> (i % 4)) & 0xFF);
        }
    }

    template <size_t N>
    void sub_dec(char* data, uint32_t key) {
        for (size_t i = 0; i < N; ++i) {
            data[i] -= static_cast<char>((key >> (i % 4)) & 0xFF);
        }
    }

    template <size_t N>
    constexpr void shuffle_enc(std::array<char, N>& data, uint32_t key) {
        if (N < 2) return;
        for (size_t i = 0; i < N; ++i) {
            size_t j = (key ^ i) % N;
            char temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
    }

    template <size_t N>
    void unshuffle_dec(char* data, uint32_t key) {
        if (N < 2) return;
        for (int i = static_cast<int>(N) - 1; i >= 0; --i) {
            size_t j = (key ^ static_cast<size_t>(i)) % N;
            char temp = data[i];
            data[i] = data[j];
            data[j] = temp;
        }
    }
}

template <size_t N>
struct ProtectedString {
    std::array<char, N> data;
    Meta<N> meta;
    uint32_t key_part_a;

    constexpr ProtectedString(const char* str, uint32_t seed, uint32_t pswd_key = 0) 
        : data{}, meta{}, key_part_a(seed ^ 0x55555555) {
        for (size_t i = 0; i < N; ++i) data[i] = str[i];
        
        meta.key_seed = seed;
        meta.layer_count = 3 + (seed % 3);
        
        for (uint8_t i = 0; i < meta.layer_count; ++i) {
            uint32_t layer_seed = (seed >> (i * 4));
            meta.pipeline[i] = static_cast<LayerType>(layer_seed % static_cast<uint8_t>(LayerType::COUNT));
        }

        uint32_t k = seed ^ pswd_key;
        for (uint8_t i = 0; i < meta.layer_count; ++i) {
            apply_enc(meta.pipeline[i], k);
        }
    }

    constexpr void apply_enc(LayerType type, uint32_t k) {
        switch(type) {
            case LayerType::XOR_SIMPLE:   layers::xor_simple_enc<N>(data, k); break;
            case LayerType::XOR_STATEFUL: layers::xor_stateful_enc<N>(data, k); break;
            case LayerType::ROL:          layers::rol_enc<N>(data, static_cast<uint8_t>(k & 0xFF)); break;
            case LayerType::ADDITIVE:     layers::add_enc<N>(data, k); break;
            case LayerType::SHUFFLE:      layers::shuffle_enc<N>(data, k); break;
            default: break;
        }
    }

    std::string decrypt(const char* password = nullptr) const {
        char buffer[N + 1];
        for (size_t i = 0; i < N; ++i) buffer[i] = data[i];
        buffer[N] = '\0';

        uint32_t seed = (key_part_a ^ 0x55555555);
        uint32_t pswd_key = 0;
        if (password) pswd_key = hash_compile_time(password);

        uint32_t k = seed ^ pswd_key;
        volatile uint32_t junk = k * 1337UL;

        for (int i = static_cast<int>(meta.layer_count) - 1; i >= 0; --i) {
            LayerType type = meta.pipeline[i];
            junk = (junk + i) ^ (junk >> 3);

            switch(type) {
                case LayerType::XOR_SIMPLE:   layers::xor_simple_dec<N>(buffer, k); break;
                case LayerType::XOR_STATEFUL: layers::xor_stateful_dec<N>(buffer, k); break;
                case LayerType::ROL:          layers::ror_dec<N>(buffer, static_cast<uint8_t>(k & 0xFF)); break;
                case LayerType::ADDITIVE:     layers::sub_dec<N>(buffer, k); break;
                case LayerType::SHUFFLE:      layers::unshuffle_dec<N>(buffer, k); break;
                default: break;
            }
        }
        return std::string(buffer, N);
    }
};

}

#define SECURE_STR(s) []() { \
    constexpr uint32_t seed = obfuscator::Seed::generate<sizeof(s)>(__LINE__); \
    static constexpr auto ps = obfuscator::ProtectedString<sizeof(s) - 1>(s, seed); \
    return ps; \
}()

#define SECURE_STR_K(s, p) []() { \
    constexpr uint32_t seed = obfuscator::Seed::generate<sizeof(s)>(__LINE__); \
    static constexpr auto ps = obfuscator::ProtectedString<sizeof(s) - 1>(s, seed, obfuscator::hash_compile_time(p)); \
    return ps; \
}()
