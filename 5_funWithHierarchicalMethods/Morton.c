#include "Morton.h"


#if defined __BMI2__

// requires Haswell or better, compile with -mbmi2
#include <immintrin.h>

uint64_t xy_to_morton(uint32_t x, uint32_t y)
{
  return _pdep_u32(x, 0x55555555) | _pdep_u32(y,0xaaaaaaaa);
}

#elif defined __PCLMUL__

// requires Westmere or better, compile with -mpclmul

#include <wmmintrin.h>

static uint64_t carryless_square(uint32_t x)
{
  uint64_t val[2] = {x, 0};
  __m128i *a = (__m128i * )val;
  *a = _mm_clmulepi64_si128 (*a, *a, 0);
  return val[0];
}

uint64_t xy_to_morton(uint32_t x, uint32_t y)
{
  return carryless_square(x) | (carryless_square(y) << 1);
}

#else

uint64_t xy_to_morton(uint32_t x, uint32_t y)
{
    x = (x | (x << 16)) & 0x0000FFFF0000FFFF;
    x = (x | (x << 8)) & 0x00FF00FF00FF00FF;
    x = (x | (x << 4)) & 0x0F0F0F0F0F0F0F0F;
    x = (x | (x << 2)) & 0x3333333333333333;
    x = (x | (x << 1)) & 0x5555555555555555;

    y = (y | (y << 16)) & 0x0000FFFF0000FFFF;
    y = (y | (y << 8)) & 0x00FF00FF00FF00FF;
    y = (y | (y << 4)) & 0x0F0F0F0F0F0F0F0F;
    y = (y | (y << 2)) & 0x3333333333333333;
    y = (y | (y << 1)) & 0x5555555555555555;

    return x | (y << 1);
}

#endif