#ifndef DAEDMATHS_H__
#define DAEDMATHS_H__

#include <math.h>

inline float Sqrt_VFPU(float x)
{
    float result;

    __asm__ volatile (
            "mtv %1, S000\n"
            "vsqrt.s S001, S000\n"
            "mfv %0, S001\n"
    : "=r"(result) : "r"(x) );

    return result;
}

inline float InvSqrt_VFPU(float x)
{
   float result;
   __asm__ volatile (
      "mtv     %1, S000\n"
      "vrsq.s S000, S000\n"
      "mfv     %0, S000\n"
   : "=r"(result) : "r"(x));

   return result;
} 

inline float Sqrt_CPU(float x)
{
	return sqrtf( x );
}

inline float InvSqrt_CPU(float x)
{
	return 1.0f / sqrtf( x );
} 

#endif // DAEDMATHS_H__
