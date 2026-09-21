#ifndef HASH_H
#define HASH_H

#include <stdint.h>

#if defined(__x86_64__) || defined(_M_X64)
    #include <immintrin.h>
    #define HAVE_AESNI defined(__AES__)
#endif

#if defined(__ARM_FEATURE_CRYPTO) || defined(__ARM_FEATURE_AES)
    #include <arm_neon.h>
    #define HAVE_ARM_AES 1
#endif

static inline uint64_t hash64(uint64_t input)
{
#if defined(__AES__)
    __m128i state = _mm_set_epi64x(0, (int64_t)input);
    __m128i key   = _mm_set_epi64x(0x9E3779B97F4A7C15ULL,
                                    0xBF58476D1CE4E5B9ULL);
    state = _mm_aesenc_si128(state, key);
    state = _mm_aesenc_si128(state, key);

    uint64_t lo = (uint64_t)_mm_cvtsi128_si64(state);
    uint64_t hi = (uint64_t)_mm_extract_epi64(state, 1);
    return lo ^ hi;

#elif defined(HAVE_ARM_AES)
    uint8x16_t state = vreinterpretq_u8_u64(
        vcombine_u64(vcreate_u64(input), vcreate_u64(0)));
    uint8x16_t key = vreinterpretq_u8_u64(
        vcombine_u64(vcreate_u64(0x9E3779B97F4A7C15ULL),
                      vcreate_u64(0xBF58476D1CE4E5B9ULL)));
    state = vaeseq_u8(state, key);
    state = vaeseq_u8(state, key);

    uint64x2_t s64 = vreinterpretq_u64_u8(state);
    return vgetq_lane_u64(s64, 0) ^ vgetq_lane_u64(s64, 1);

#else
    uint64_t x = input;
    x ^= x >> 30;
    x *= 0xBF58476D1CE4E5B9ULL;
    x ^= x >> 27;
    x *= 0x94D049BB133111EBULL;
    x ^= x >> 31;
    return x;
#endif
}

#endif