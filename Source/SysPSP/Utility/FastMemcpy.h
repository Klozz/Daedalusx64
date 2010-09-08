/*
Copyright (C) 2009 Raphael

E-mail:   raphael@fx-world.org
homepage: http://wordpress.fx-world.org

*/


#ifndef FASTMEMCPY_H_
#define FASTMEMCPY_H_

//*****************************************************************************
//
//*****************************************************************************
//
// Little endian 
//
inline void memcpy_vfpu( void* dst, void* src, u32 size );
//
//*****************************************************************************
//
//*****************************************************************************
//
// Big endian 
//
inline void memcpy_vfpu_b( void* dst, void* src, u32 size );
//

#endif // FASTMEMCPY_H_
