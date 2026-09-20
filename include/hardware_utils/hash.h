#ifndef HASH_H
#define HASH_H

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <nmmintrin.h>
#elif defined(__arm__) || defined(__aarch64__)
#include <arm_acle.h>
#endif
#include <stdint.h>

uint32_t crc32(uint32_t input)
{
    #if defined(__SSE4_2__)
        return _mm_crc32_u32(0, input);
    #elif defined(__ARM_FEATURE_CRC32)
        return __crc32w(0, input);
    #else
        uint32_t crc = 0xFFFFFFFFU;

        for (int i = 0; i < 4; i++)
        {
            crc ^= (uint8_t)input;

            for (int bit = 0; bit < 8; bit++)
            {
                if (crc & 1U)
                    crc = (crc >> 1) ^ 0xEDB88320U;
                else
                    crc >>= 1;
            }

            input >>= 8;
        }

        return crc ^ 0xFFFFFFFFU;
    #endif
}


#endif