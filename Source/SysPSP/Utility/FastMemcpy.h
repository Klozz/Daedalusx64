/*
Copyright (C) 2009 Raphael

E-mail:   raphael@fx-world.org
homepage: http://wordpress.fx-world.org

*/
#include <psppower.h>

#ifndef FASTMEMCPY_H_
#define FASTMEMCPY_H_


//
u64	GetCurrent();	// For profilling
//

//*****************************************************************************
//
//*****************************************************************************
//
// Little endian 
//
void memcpy_vfpu_LE( void* dst, void* src, u32 size );
//
//*****************************************************************************
//
//*****************************************************************************
//
// Big endian 
//
void memcpy_vfpu_BE( void* dst, void* src, u32 size );
//
//
//*****************************************************************************
//
//*****************************************************************************
//
// CPU only 
//
void memcpy_cpu_LE( void* dst, void* src, u32 size );
//
//
//*****************************************************************************
//
//*****************************************************************************
//
void memcpy_vfpu_o( void* dst, void* src, u32 size );
// For testing memcpy speed
// Lowest the fastest :)
//
#define MEMCPY_TEST(d, s, n) {                                               \
    int gcc_elapsed = 0;                                                        \
    {                                                                           \
        u64 time = GetCurrent();												\
        for (int j=0; j<100; ++j)                                              \
            memcpy(d, s, n);                                                    \
        gcc_elapsed = (int)(GetCurrent()-time);									\
    }                                                                           \
    int vfpu_elapsed = 0;														\
    {                                                                           \
        u64 time = GetCurrent();												\
        for (int j=0; j<100; ++j)                                              \
            memcpy_vfpu_LE(d, s, n);												\
        vfpu_elapsed = (int)(GetCurrent()-time);								\
    }                                                                           \
    int cpu_elapsed = 0;														\
    {                                                                           \
        u64 time = GetCurrent();												\
        for (int j=0; j<100; ++j)                                              \
            memcpy_cpu_LE(d, s, n);												\
        cpu_elapsed = (int)(GetCurrent()-time);									\
    }                                                                           \
    scePowerTick(0);                                                            \
	printf("%6d bytes | GCC%5d | VFPU%5d | CPU%5d\n", (int)n, gcc_elapsed, vfpu_elapsed, cpu_elapsed); \
    }

#endif // FASTMEMCPY_H_
