/*
Copyright (C) 2009 Raphael

E-mail:   raphael@fx-world.org
homepage: http://wordpress.fx-world.org

*/


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
void memcpy_vfpu( void* dst, void* src, u32 size );
//
//*****************************************************************************
//
//*****************************************************************************
//
// Big endian 
//
void memcpy_vfpu_b( void* dst, void* src, u32 size );
//
//
//*****************************************************************************
//
//*****************************************************************************
//
// CPU only 
//
void memcpy_cpu( void* dst, void* src, u32 size );
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
        for (int j=0; j<1000; ++j)                                              \
            memcpy(d, s, n);                                                    \
        gcc_elapsed = (int)(GetCurrent()-time);									\
    }                                                                           \
    int vfpu_elapsed = 0;														\
    {                                                                           \
        u64 time = GetCurrent();												\
        for (int j=0; j<1000; ++j)                                              \
            memcpy_vfpu(d, s, n);												\
        vfpu_elapsed = (int)(GetCurrent()-time);								\
    }                                                                           \
    int cpu_elapsed = 0;														\
    {                                                                           \
        u64 time = GetCurrent();												\
        for (int j=0; j<1000; ++j)                                              \
            memcpy_cpu(d, s, n);												\
        cpu_elapsed = (int)(GetCurrent()-time);									\
    }                                                                           \
    scePowerTick(0);                                                            \
	printf("%6d bytes | GCC%12d | VFPU%12d | CPU%12d\n", (int)n, gcc_elapsed, vfpu_elapsed, cpu_elapsed); \
    }

#endif // FASTMEMCPY_H_
