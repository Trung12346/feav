#ifndef HASH_H
#define HASH_H

#include <stdint.h>

#define FNV_OFFSET_BASIS_64 14695981039346656037ULL
#define FNV_PRIME_64        1099511628211ULL

static inline uint64_t fnv1a_64(const void *data, size_t size)
{
    const uint8_t *p = (const uint8_t *)data;
    uint64_t hash = FNV_OFFSET_BASIS_64;

    for (size_t i = 0; i < size; ++i) {
        hash ^= p[i];
        hash *= FNV_PRIME_64;
    }

    return hash;
}

#if defined(__x86_64__) || defined(_M_X64)
    #include <immintrin.h>
    #define HAVE_AESNI defined(__AES__)
#endif

#if defined(__ARM_FEATURE_CRYPTO) || defined(__ARM_FEATURE_AES)
    #include <arm_neon.h>
    #define HAVE_ARM_AES 1
#endif

#define NO_SEED 0

#if defined(__AES__)
    #define SEED_CSS_ATTRIBUTE 5368717626ULL
    #define SEED_HTML_TAG 3221225470ULL
#elif defined(HAVE_ARM_AES)
    #define SEED_CSS_ATTRIBUTE 57281ULL
    #define SEED_HTML_TAG 1ULL
#else
    #define SEED_CSS_ATTRIBUTE 3221236384ULL
    #define SEED_HTML_TAG 2147483648ULL
#endif



static inline uint64_t hash64(uint64_t input, uint64_t seed)
{
#if defined(__AES__)
    __m128i state = _mm_set_epi64x((int64_t)seed, (int64_t)input);
    __m128i key   = _mm_set_epi64x(0x9E3779B97F4A7C15ULL,
                                    0xBF58476D1CE4E5B9ULL);
    state = _mm_aesenc_si128(state, key);
    state = _mm_aesenc_si128(state, key);

    uint64_t lo = (uint64_t)_mm_cvtsi128_si64(state);
    uint64_t hi = (uint64_t)_mm_extract_epi64(state, 1);
    return lo ^ hi;

#elif defined(HAVE_ARM_AES)
    uint8x16_t state = vreinterpretq_u8_u64(
        vcombine_u64(vcreate_u64(input), vcreate_u64(seed)));
    uint8x16_t key = vreinterpretq_u8_u64(
        vcombine_u64(vcreate_u64(0x9E3779B97F4A7C15ULL),
                      vcreate_u64(0xBF58476D1CE4E5B9ULL)));
    state = vaesmcq_u8(vaeseq_u8(state, key));
    state = vaeseq_u8(state, key);

    uint64x2_t s64 = vreinterpretq_u64_u8(state);
    return vgetq_lane_u64(s64, 0) ^ vgetq_lane_u64(s64, 1);

#else
    uint64_t x = input + seed * 0x9E3779B97F4A7C15ULL;
    x ^= x >> 30;
    x *= 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 27;
    x *= 0x94D049BB133111EBULL;
    x ^= x >> 31;
    return x;
#endif
}

#endif