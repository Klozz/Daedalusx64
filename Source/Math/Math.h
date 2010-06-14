#ifndef DAEDMATHS_H__
#define DAEDMATHS_H__

#include <math.h>

// VFPU Math :D
//
// Todo : Move to SysPSP ?
//
// Note : Matrix math => check Matrix4x4.cpp
//

/* Cycles

- sinf(v) = 0.389418, cycles: 856
- vfpu_sinf(v) = 0.389418, cycles: 160
 
- cosf(v) = 0.921061, cycles: 990
- vfpu_cosf(v) = 0.921061, cycles: 154
 
- acosf(v) = 1.159279, cycles: 1433
- vfpu_acosf(v) = 1.159280, cycles: 107
 
- coshf(v) = 1.081072, cycles: 1885
- vfpu_coshf(v) = 1.081072, cycles: 246
 
- powf(v, v) = 0.693145, cycles: 3488
- vfpu_powf(v, v) = 0.693145, cycles: 412

- fabsf(v) = 0.400000, cycles: 7
- vfpu_fabsf(v) = 0.400000, cycles: 93	<== Slower on VFPU !
 
- sqrtf(v) = 0.632456, cycles: 40
- vfpu_sqrtf(v) = 0.632455, cycles: 240	<== Slower on VFPU !

*/

// Interesting note, if we don't make thse funcs inline, we'll get compiling errors.. weird

#define PI   3.141592653589793f


inline float vfpu_invSqrt(float x)
{
	float result;
	
	// return 1.0f/sqrtf(x);
	
	__asm__ volatile (
		"mtv		%0, S000\n"
		"vrsq.s		S000, S000\n"
		"mfv		%0, S000\n"
	: "=r"(result): "r"(x));
	return result;
}

inline float vfpu_cosf(float rad) {
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_2_PI\n"
        "vmul.s  S000, S000, S001\n"
        "vcos.s  S000, S000\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(rad));
    return result;
}

inline float vfpu_sinf(float rad) {
    float result;
    __asm__ volatile (
        "mtv     %1, S000\n"
        "vcst.s  S001, VFPU_2_PI\n"
        "vmul.s  S000, S000, S001\n"
        "vsin.s  S000, S000\n"
        "mfv     %0, S000\n"
        : "=r"(result) : "r"(rad));
    return result;
}

//Below function taken from PGE
inline float vfpu_round(float x)
{
	float result;

	__asm__ volatile (
		"mtv      %1, S000\n"
		"vf2in.s  S000, S000, 0\n"
		"vi2f.s	  S000, S000, 0\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));
	
	return result;
}

inline float vfpu_fmaxf(float x, float y) {
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"mtv      %2, S001\n"
		"vmax.s   S002, S000, S001\n"
		"mfv      %0, S002\n"
	: "=r"(result) : "r"(x), "r"(y));
	return result;
}

inline float vfpu_fminf(float x, float y) {
	float result;
	__asm__ volatile (
		"mtv      %1, S000\n"
		"mtv      %2, S001\n"
		"vmin.s   S002, S000, S001\n"
		"mfv      %0, S002\n"
	: "=r"(result) : "r"(x), "r"(y));
	return result;
}

inline float vfpu_powf(float x, float y) {
	float result;
	// result = exp2f(y * log2f(x));
	__asm__ volatile (
		"mtv      %1, S000\n"
		"mtv      %2, S001\n"
		"vlog2.s  S001, S001\n"
		"vmul.s   S000, S000, S001\n"
		"vexp2.s  S000, S000\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x), "r"(y));
	return result;
}
/*

//Below Function taken from PGE - Phoenix Game Engine - Greets InsertWittyName !
inline float vfpu_abs(float x) {
    float result;

	__asm__ volatile (
		"mtv      %1, S000\n"
		"vabs.s   S000, S000\n"
		"mfv      %0, S000\n"
	: "=r"(result) : "r"(x));

	return result;
}

inline float vfpu_sqrtf(float x) {
	float result;
	__asm__ volatile (
		"mtv     %1, S000\n"
		"vsqrt.s S000, S000\n"
		"mfv     %0, S000\n"
	: "=r"(result) : "r"(x));
	return result;
}
*/


//*****************************************************************************
//FPU Math :D
//*****************************************************************************
/*
sqrtf and fabsf are alot slower on the vfpuv, so let's do em on the fpu.
Check above notes for cycles/comparison
*/

inline float pspFpuSqrt(float fs)
{
	return (__builtin_allegrex_sqrt_s(fs));
}


inline float pspFpuAbs(float fs)
{
	register float fd;
	asm (
		"abs.s %0, %1\n"
		: "=f"(fd)
		: "f"(fs)
	);
	return (fd);
}

//*****************************************************************************
//
//*****************************************************************************
// Misc

inline int pspFpuFloor(float fs)
{
	return (__builtin_allegrex_floor_w_s(fs));
}

inline int pspFpuCeil(float fs)
{
	return (__builtin_allegrex_ceil_w_s(fs));
}

inline int pspFpuTrunc(float fs)
{
	return (__builtin_allegrex_trunc_w_s(fs));
}

inline int pspFpuRound(float fs)
{
	return (__builtin_allegrex_round_w_s(fs));
}

// I'm not sure if the vfpu_fmaxf and vfpu_fminf it's slwoer or faster than fpu version
// Let's do regardless to avoid overhead.

inline float pspFpuMax(float fs1, float fs2)
{
	register float fd;
	fd = (fs1 > fs2) ? fs1 : fs2;
	return (fd);
}

inline float pspFpuMin(float fs1, float fs2)
{
	register float fd;
	fd = (fs1 < fs2) ? fs1 : fs2;
	return (fd);
}





#endif // DAEDMATHS_H__
