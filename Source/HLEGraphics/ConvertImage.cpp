/*
Copyright (C) 2001 StrmnNrmn

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
	  
*/

#include "stdafx.h"
#include "ConvertImage.h"
#include "TextureDescriptor.h"

#include "DebugDisplayList.h"
#include "Core/Memory.h"

#include "PixelFormatN64.h"
#include "SysPSP/Graphics/PixelFormatPSP.h"

#include "Math/MathUtil.h"

#include "RDP.h"

#include "OSHLE/ultra_gbi.h"

using namespace PixelFormats;

namespace
{

const u8 OneToEight[2] =
{
	0x00,		// 0 -> 00 00 00 00
	0xff		// 1 -> 11 11 11 11
};

const u8 OneToFour[2] =
{
	0x00,		// 0 -> 00 00 
	0x0f		// 1 -> 11 11 
};

const u8 TwoToEight[4] =
{
	0x00,		// 00 -> 00 00 00 00
	0x55,		// 01 -> 01 01 01 01
	0xaa,		// 10 -> 10 10 10 10
	0xff		// 11 -> 11 11 11 11
};

const u8 TwoToFour[4] =
{
	0x0,		// 00 -> 00 00 
	0x5,		// 01 -> 01 01 
	0xa,		// 10 -> 10 10 
	0xf			// 11 -> 11 11
};

const u8 ThreeToEight[8] =
{
	0x00,		// 000 -> 00 00 00 00
	0x24,		// 001 -> 00 10 01 00
	0x49,		// 010 -> 01 00 10 01
	0x6d,       // 011 -> 01 10 11 01
	0x92,       // 100 -> 10 01 00 10
	0xb6,		// 101 -> 10 11 01 10
	0xdb,		// 110 -> 11 01 10 11
	0xff		// 111 -> 11 11 11 11
};

const u8 ThreeToFour[8] =
{
	0x0,		// 000 -> 00 00 00 00
	0x2,		// 001 -> 00 10 01 00
	0x4,		// 010 -> 01 00 10 01
	0x6,       // 011 -> 01 10 11 01
	0x9,       // 100 -> 10 01 00 10
	0xb,		// 101 -> 10 11 01 10
	0xd,		// 110 -> 11 01 10 11
	0xf		// 111 -> 11 11 11 11
};

const u8 FourToEight[16] = 
{
	0x00, 0x11, 0x22, 0x33,
	0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb,
	0xcc, 0xdd, 0xee, 0xff
};

const u16 FourToSixteen[16] = 
{
	0x0000, 0x1111, 0x2222, 0x3333,
	0x4444, 0x5555, 0x6666, 0x7777,
	0x8888, 0x9999, 0xaaaa, 0xbbbb,
	0xcccc, 0xdddd, 0xeeee, 0xffff
};

const u8 FiveToEight[32] =
{
	0x00, // 00000 -> 00000000
	0x08, // 00001 -> 00001000
	0x10, // 00010 -> 00010000
	0x18, // 00011 -> 00011000
	0x21, // 00100 -> 00100001
	0x29, // 00101 -> 00101001
	0x31, // 00110 -> 00110001
	0x39, // 00111 -> 00111001
	0x42, // 01000 -> 01000010
	0x4a, // 01001 -> 01001010
	0x52, // 01010 -> 01010010
	0x5a, // 01011 -> 01011010
	0x63, // 01100 -> 01100011
	0x6b, // 01101 -> 01101011
	0x73, // 01110 -> 01110011
	0x7b, // 01111 -> 01111011
	
	0x84, // 10000 -> 10000100
	0x8c, // 10001 -> 10001100
	0x94, // 10010 -> 10010100
	0x9c, // 10011 -> 10011100
	0xa5, // 10100 -> 10100101
	0xad, // 10101 -> 10101101
	0xb5, // 10110 -> 10110101
	0xbd, // 10111 -> 10111101
	0xc6, // 11000 -> 11000110
	0xce, // 11001 -> 11001110
	0xd6, // 11010 -> 11010110
	0xde, // 11011 -> 11011110
	0xe7, // 11100 -> 11100111
	0xef, // 11101 -> 11101111
	0xf7, // 11110 -> 11110111
	0xff  // 11111 -> 11111111
};


template< u32 Size >
struct SByteswapInfo;

template<> struct SByteswapInfo< 1 >
{
	enum { Fiddle = 3 };
};
template<> struct SByteswapInfo< 2 >
{
	enum { Fiddle = 2 };
};
template<> struct SByteswapInfo< 4 >
{
	enum { Fiddle = 0 };
};

template< u32 Size >
struct SSwizzleInfo;

template<> struct SSwizzleInfo< 1 >
{
	enum { Swizzle = 4 };
};
template<> struct SSwizzleInfo< 2 >
{
	enum { Swizzle = 2 };
};
template<> struct SSwizzleInfo< 4 >
{
	enum { Swizzle = 2 };
};


//*****************************************************************************
//
//*****************************************************************************
template < typename OutT >
struct SConvertGeneric
{
typedef void (*ConvertRowFunction)( OutT * p_dst, const u8 * p_src_base, u32 offset, u32 width );


static void ConvertGeneric( const TextureDestInfo & dst,
							const TextureInfo & ti,
							ConvertRowFunction swapped_fn,
							ConvertRowFunction unswapped_fn )
{
	OutT *				p_dst( reinterpret_cast< OutT * >( dst.pSurface ) );
	
	const u8 *			p_src_base( g_pu8RamBase );
	u32					base_offset( ti.GetLoadAddress() );
	u32					src_pitch( ti.GetPitch() );

	if ( ti.IsSwapped())
	{
		for (u32 y = 0; y < ti.GetHeight(); y++)
		{
			if ((y&1) == 0)
			{
				unswapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth() );
			}
			else
			{
				swapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth() );
			}

			base_offset += src_pitch;
			p_dst = reinterpret_cast< OutT * >( (u8*)p_dst + dst.Pitch );
		}
	}
	else
	{
		for (u32 y = 0; y < ti.GetHeight(); y++)
		{
			unswapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth() );

			base_offset += src_pitch;
			p_dst = reinterpret_cast< OutT * >( (u8*)p_dst + dst.Pitch );
		}
	}
}

};

typedef void (*ConvertPalettisedRowFunction)( Psp::Pf8888 * p_dst, const u8 * p_src_base, u32 offset, u32 width, const u16 * p_palette );

void ConvertGenericPalettised( const TextureDestInfo & dst, const TextureInfo & ti, ConvertPalettisedRowFunction swapped_fn, ConvertPalettisedRowFunction unswapped_fn )
{
	Psp::Pf8888 *		p_dst( reinterpret_cast< Psp::Pf8888 * >( dst.pSurface ) );

	const u8 *			p_src_base( g_pu8RamBase );
	u32					base_offset( ti.GetLoadAddress() );
	u32					src_pitch( ti.GetPitch() );

	const u16 *			p_palette( reinterpret_cast< const u16 * >( ti.GetPalettePtr() ) );

	if (ti.IsSwapped())
	{
		for (u32 y = 0; y < ti.GetHeight(); y++)
		{
			if ((y&1) == 0)
			{
				unswapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth(), p_palette );
			}
			else
			{
				swapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth(), p_palette );
			}

			base_offset += src_pitch;
			p_dst = reinterpret_cast< Psp::Pf8888 * >( (u8*)p_dst + dst.Pitch );
		}
	}
	else
	{
		for (u32 y = 0; y < ti.GetHeight(); y++)
		{
			unswapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth(), p_palette );

			base_offset += src_pitch;
			p_dst = reinterpret_cast< Psp::Pf8888 * >( (u8*)p_dst + dst.Pitch );
		}
	}
}


typedef void (*ConvertPaletteFn)( Psp::Pf8888 * p_dst, const u8 * p_palette, u32 entries );
typedef void (*ConvertPalettisedCI4RowFunction)( Psp::PfCI44 * p_dst, const u8 * p_src_base, u32 offset, u32 width );

void ConvertGenericPalettisedCI4( const TextureDestInfo & dst, const TextureInfo & ti, ConvertPalettisedCI4RowFunction swapped_fn, ConvertPalettisedCI4RowFunction unswapped_fn, ConvertPaletteFn palette_fn )
{
	Psp::PfCI44 *		p_dst( reinterpret_cast< Psp::PfCI44 * >( dst.pSurface ) );

	const u8 *			p_src_base( g_pu8RamBase );
	u32					base_offset( ti.GetLoadAddress() );
	u32					src_pitch( ti.GetPitch() );

	if (ti.IsSwapped())
	{
		for (u32 y = 0; y < ti.GetHeight(); y++)
		{
			if ((y&1) == 0)
			{
				unswapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth() );
			}
			else
			{
				swapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth() );
			}

			base_offset += src_pitch;
			p_dst = reinterpret_cast< Psp::PfCI44 * >( (u8*)p_dst + dst.Pitch );
		}
	}
	else
	{
		for (u32 y = 0; y < ti.GetHeight(); y++)
		{
			unswapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth() );

			base_offset += src_pitch;
			p_dst = reinterpret_cast< Psp::PfCI44 * >( (u8*)p_dst + dst.Pitch );
		}
	}


	Psp::Pf8888 *		p_dst_palette( reinterpret_cast< Psp::Pf8888 * >( dst.Palette ) );
	const u8 *			p_palette( reinterpret_cast< const u8 * >( ti.GetPalettePtr() ) );

	palette_fn( p_dst_palette, p_palette, 16 );
}

typedef void (*ConvertPalettisedCI8RowFunction)( Psp::PfCI8 * p_dst, const u8 * p_src_base, u32 offset, u32 width );

void ConvertGenericPalettisedCI8( const TextureDestInfo & dst, const TextureInfo & ti, ConvertPalettisedCI8RowFunction swapped_fn, ConvertPalettisedCI8RowFunction unswapped_fn, ConvertPaletteFn palette_fn )
{
	Psp::PfCI8 *		p_dst( reinterpret_cast< Psp::PfCI8 * >( dst.pSurface ) );

	const u8 *			p_src_base( g_pu8RamBase );
	u32					base_offset( ti.GetLoadAddress() );
	u32					src_pitch( ti.GetPitch() );

	if (ti.IsSwapped())
	{
		for (u32 y = 0; y < ti.GetHeight(); y++)
		{
			if ((y&1) == 0)
			{
				unswapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth() );
			}
			else
			{
				swapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth() );
			}

			base_offset += src_pitch;
			p_dst = reinterpret_cast< Psp::PfCI8 * >( (u8*)p_dst + dst.Pitch );
		}
	}
	else
	{
		for (u32 y = 0; y < ti.GetHeight(); y++)
		{
			unswapped_fn( p_dst, p_src_base, base_offset, ti.GetWidth() );

			base_offset += src_pitch;
			p_dst = reinterpret_cast< Psp::PfCI8 * >( (u8*)p_dst + dst.Pitch );
		}
	}


	Psp::Pf8888 *		p_dst_palette( reinterpret_cast< Psp::Pf8888 * >( dst.Palette ) );
	const u8 *			p_palette( reinterpret_cast< const u8 * >( ti.GetPalettePtr() ) );

	palette_fn( p_dst_palette, p_palette, 256 );
}

//*****************************************************************************
//
//*****************************************************************************
template < typename InT >
struct SConvert
{
	enum { Fiddle = SByteswapInfo< sizeof( InT ) >::Fiddle };
	enum { Swizzle = SSwizzleInfo< sizeof( InT ) >::Swizzle };

	//
	//	This routine converts from any format which is > 1 byte to any Psp format.
	//
	template < typename OutT, u32 InFiddle, u32 OutFiddle > 
	static inline void ConvertRow( OutT * p_dst, const u8 * p_src_base, u32 offset, u32 width )
	{
		DAEDALUS_DL_ASSERT( IsAligned( offset, sizeof( InT ) ), "Offset should be correctly aligned" );

		//
		//	Need to be careful of this - ensure that it's doing the right thing in all cases and not overflowing rows.
		//	This is to ensure that we correctly convert all the texels in a row, even when we're fiddling.
		//	If we have a fiddle of 2 for instance, and the row is not a multiple of the fiddle amount
		//	then we don't convert enough pixels (we actually poke some values in past the end of the row)
		//	and get some random noise at the end instead.
		//
		//	There may well be an easier (and less gross)  way of doing this if we move the OutFiddle calculation
		//	into the source pixel lookup, and just have p_dst[x] = ...
		//
		width = AlignPow2( width, 1<<OutFiddle );

		for (u32 x = 0; x < width; x++)
		{
			InT	colour( *reinterpret_cast< const InT * >( &p_src_base[offset ^ InFiddle] ) );

			p_dst[x ^ OutFiddle] = convertPixelFormat< OutT, InT >( colour );

			offset += sizeof( InT );
		}
	}

	template < typename OutT >
	static inline void ConvertTextureT( const TextureDestInfo & dst, const TextureInfo & ti )
	{
		SConvertGeneric< OutT >::ConvertGeneric( dst, ti, ConvertRow< OutT, Fiddle, Swizzle >, ConvertRow< OutT, Fiddle, 0 > );
	}

	static void ConvertTexture( const TextureDestInfo & dst, const TextureInfo & ti )
	{
		switch( dst.Format )
		{
		case TexFmt_5650:	ConvertTextureT< Psp::Pf5650 >( dst, ti ); return;
		case TexFmt_5551:	ConvertTextureT< Psp::Pf5551 >( dst, ti ); return;
		case TexFmt_4444:	ConvertTextureT< Psp::Pf4444 >( dst, ti ); return;
		case TexFmt_8888:	ConvertTextureT< Psp::Pf8888 >( dst, ti ); return;

		case TexFmt_CI4_8888: break;
		case TexFmt_CI8_8888: break;

		}

		DAEDALUS_DL_ERROR( "Unhandled format" );
	}
};

//*****************************************************************************
//
//*****************************************************************************
struct SConvertIA4
{
	enum { Fiddle = 0x3 };

	template < typename OutT, u32 F >
	static inline void ConvertRow( OutT * p_dst, const u8 * p_src_base, u32 offset, u32 width ) 
	{
		// Do two pixels at a time
		for (u32 x = 0; x < width; x+=2)
		{
			u8 b = p_src_base[offset ^ F];

			// Even
			p_dst[x + 0] = OutT( ThreeToEight[(b & 0xE0) >> 5],
								 ThreeToEight[(b & 0xE0) >> 5],
								 ThreeToEight[(b & 0xE0) >> 5],
								 OneToEight[(b & 0x10)   >> 4]);	
			// Odd
			p_dst[x + 1] = OutT( ThreeToEight[(b & 0x0E) >> 1],
								 ThreeToEight[(b & 0x0E) >> 1],
								 ThreeToEight[(b & 0x0E) >> 1],
								 OneToEight[(b & 0x01)     ] );
			offset++;
		}

		if(width & 1)
		{
			u8 b = p_src_base[offset ^ F];

			// Even
			p_dst[width-1] = OutT( ThreeToEight[(b & 0xE0) >> 5],
								 ThreeToEight[(b & 0xE0) >> 5],
								 ThreeToEight[(b & 0xE0) >> 5],
								 OneToEight[(b & 0x10)   >> 4]);	
		}
	}

	template < typename OutT >
	static inline void ConvertTextureT( const TextureDestInfo & dst, const TextureInfo & ti )
	{
		SConvertGeneric< OutT >::ConvertGeneric( dst, ti, ConvertRow< OutT, 0x4 | Fiddle >, ConvertRow< OutT, Fiddle > );
	}

	static void ConvertTexture( const TextureDestInfo & dst, const TextureInfo & ti )
	{
		switch( dst.Format )
		{
		case TexFmt_5650:	ConvertTextureT< Psp::Pf5650 >( dst, ti ); return;
		case TexFmt_5551:	ConvertTextureT< Psp::Pf5551 >( dst, ti ); return;
		case TexFmt_4444:	ConvertTextureT< Psp::Pf4444 >( dst, ti ); return;
		case TexFmt_8888:	ConvertTextureT< Psp::Pf8888 >( dst, ti ); return;

		case TexFmt_CI4_8888: break;
		case TexFmt_CI8_8888: break;

		}

		DAEDALUS_DL_ERROR( "Unhandled format" );
	}
};

//*****************************************************************************
//
//*****************************************************************************
struct SConvertI4
{
	enum { Fiddle = 0x3 };

	template< typename OutT, u32 F >
	static inline void ConvertRow( OutT * p_dst, const u8 * p_src_base, u32 offset, u32 width )
	{
		// Do two pixels at a time
		for ( u32 x = 0; x+1 < width; x+=2 )
		{
			u8 b = p_src_base[offset ^ F];

			// Even
			p_dst[x + 0] = OutT( FourToEight[(b & 0xF0)>>4],
								 FourToEight[(b & 0xF0)>>4],
								 FourToEight[(b & 0xF0)>>4],
								 FourToEight[(b & 0xF0)>>4] );	
			// Odd
			p_dst[x + 1] = OutT( FourToEight[(b & 0x0F)],
								 FourToEight[(b & 0x0F)],
								 FourToEight[(b & 0x0F)],
								 FourToEight[(b & 0x0F)] );

			offset++;
		}

		if(width & 1)
		{
			u8 b = p_src_base[offset ^ F];

			// Even
			p_dst[width-1] = OutT( FourToEight[(b & 0xF0)>>4],
								   FourToEight[(b & 0xF0)>>4],
								   FourToEight[(b & 0xF0)>>4],
								   FourToEight[(b & 0xF0)>>4] );	

		}
	}

	template < typename OutT >
	static inline void ConvertTextureT( const TextureDestInfo & dst, const TextureInfo & ti )
	{
		SConvertGeneric< OutT >::ConvertGeneric( dst, ti, ConvertRow< OutT, 0x4 | Fiddle >, ConvertRow< OutT, Fiddle > );
	}

	static void ConvertTexture( const TextureDestInfo & dst, const TextureInfo & ti )
	{
		switch( dst.Format )
		{
		case TexFmt_5650:	ConvertTextureT< Psp::Pf5650 >( dst, ti ); return;
		case TexFmt_5551:	ConvertTextureT< Psp::Pf5551 >( dst, ti ); return;
		case TexFmt_4444:	ConvertTextureT< Psp::Pf4444 >( dst, ti ); return;
		case TexFmt_8888:	ConvertTextureT< Psp::Pf8888 >( dst, ti ); return;

		case TexFmt_CI4_8888: break;
		case TexFmt_CI8_8888: break;

		}

		DAEDALUS_DL_ERROR( "Unhandled format" );
	}
};

//*****************************************************************************
//
//*****************************************************************************
template< typename PalT, u32 F > void ConvertPalette( Psp::Pf8888 * p_dst, const u8 * p_palette, u32 entries )
{
	const PalT *	p_n64pal( reinterpret_cast< const PalT * >( p_palette ) );

	for( u32 i = 0; i < entries; ++i )
	{
		p_dst[ i ] = Psp::Pf8888::Make( p_n64pal[ i ^ F ] );
	}
}

//*****************************************************************************
//
//*****************************************************************************
template< u32 F > void ConvertCI4_Row( Psp::PfCI44 * p_dst, const u8 * p_src_base, u32 offset, u32 width )
{
	for (u32 x = 0; x+1 < width; x+=2)
	{
		u8 b = p_src_base[offset ^ F];

		p_dst[ x/2 ].Bits = (b >> 4) | (b << 4);

		offset++;
	}

	// Handle any remaining odd pixels
	if( width & 1 )
	{
		u8 b = p_src_base[offset ^ F];

		p_dst[ width/2 ].Bits = (b >> 4) | 0;
	}
}

//*****************************************************************************
//
//*****************************************************************************
template< typename PalT, u32 F > void ConvertCI4_Row_To_8888( Psp::Pf8888 * p_dst, const u8 * p_src_base, u32 offset, u32 width, const u16 * p_palette )
{
	const PalT *	p_n64pal( reinterpret_cast< const PalT * >( p_palette ) );

	for (u32 x = 0; x < width; x+=2)
	{
		u8 b = p_src_base[offset ^ F];

		u8 bhi = (b&0xf0)>>4;
		u8 blo = (b&0x0f);

		p_dst[ x + 0 ] = Psp::Pf8888::Make( p_n64pal[ bhi ^ 0x1 ] );	// Remember palette is in different endian order!
		p_dst[ x + 1 ] = Psp::Pf8888::Make( p_n64pal[ blo ^ 0x1 ] );

		offset++;
	}

	// Handle any remaining odd pixels
	if(width & 1)
	{
		u8 b = p_src_base[offset ^ F];

		u8 bhi = (b&0xf0)>>4;

		p_dst[width-1] = Psp::Pf8888::Make( p_n64pal[ bhi ^ 0x1 ] );	// Remember palette is in different endian order!
	}
}

//*****************************************************************************
//
//*****************************************************************************
template< u32 F > void ConvertCI8_Row( Psp::PfCI8 * p_dst, const u8 * p_src_base, u32 offset, u32 width )
{
	for (u32 x = 0; x < width; x++)
	{
		u8 b = p_src_base[offset ^ F];

		p_dst[ x ].Bits = b;

		offset++;
	}
}

//*****************************************************************************
//
//*****************************************************************************
template< typename PalT, u32 F > void ConvertCI8_Row_To_8888( Psp::Pf8888 * p_dst, const u8 * p_src_base, u32 offset, u32 width, const u16 * p_palette )
{
	const PalT *	p_n64pal( reinterpret_cast< const PalT * >( p_palette ) );

	for (u32 x = 0; x < width; x++)
	{
		u8 b = p_src_base[offset ^ F];

		p_dst[ x ] = Psp::Pf8888::Make( p_n64pal[ b ^ 0x1 ] );	// Remember palette is in different endian order!

		offset++;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void ConvertRGBA16(const TextureDestInfo & dst, const TextureInfo & ti)
{
	SConvert< N64::Pf5551 >::ConvertTexture( dst, ti );
}

//*****************************************************************************
//
//*****************************************************************************
void ConvertRGBA32(const TextureDestInfo & dst, const TextureInfo & ti)
{
	// Did have Fiddle of 8 here, pretty sure this was wrong (should have been 4)
	SConvert< N64::Pf8888 >::ConvertTexture( dst, ti );
}

//*****************************************************************************
// E.g. Dear Mario text
// Copy, Score etc
//*****************************************************************************
void ConvertIA4(const TextureDestInfo & dst, const TextureInfo & ti)
{
	SConvertIA4::ConvertTexture( dst, ti );
}

//*****************************************************************************
// E.g Mario's head textures
//*****************************************************************************
void ConvertIA8(const TextureDestInfo & dst, const TextureInfo & ti)
{
	SConvert< N64::PfIA8 >::ConvertTexture( dst, ti );
}

//*****************************************************************************
// E.g. camera's clouds, shadows
//*****************************************************************************
void ConvertIA16(const TextureDestInfo & dst, const TextureInfo & ti)
{
	SConvert< N64::PfIA16 >::ConvertTexture( dst, ti );
}

//*****************************************************************************
// Used by MarioKart
//*****************************************************************************
void ConvertI4(const TextureDestInfo & dst, const TextureInfo & ti)
{
	SConvertI4::ConvertTexture( dst, ti );
}

//*****************************************************************************
// Used by MarioKart
//*****************************************************************************
void ConvertI8(const TextureDestInfo & dst, const TextureInfo & ti)
{
	SConvert< N64::PfI8 >::ConvertTexture( dst, ti );
}

//*****************************************************************************
// Used by Starfox intro
//*****************************************************************************
void ConvertCI4_RGBA16(const TextureDestInfo & dst, const TextureInfo & ti)
{
	switch( dst.Format )
	{
	case TexFmt_8888:
		ConvertGenericPalettised( dst, ti, ConvertCI4_Row_To_8888< N64::Pf5551, 0x4 | 0x3 >, ConvertCI4_Row_To_8888< N64::Pf5551, 0x3 > );
		break;

	case TexFmt_CI4_8888:
		ConvertGenericPalettisedCI4( dst, ti, ConvertCI4_Row< 0x4 | 0x3 >, ConvertCI4_Row< 0x3 >, ConvertPalette< N64::Pf5551, 0x1 > );
		break;

	default:
		DAEDALUS_ERROR( "Unhandled format for CI4RGB16 textures" );
		break;
	}
}

//*****************************************************************************
// Used by Starfox intro
//*****************************************************************************
void ConvertCI4_IA16(const TextureDestInfo & dst, const TextureInfo & ti)
{
	switch( dst.Format )
	{
	case TexFmt_8888:
		ConvertGenericPalettised( dst, ti, ConvertCI4_Row_To_8888< N64::PfIA16, 0x4 | 0x3 >, ConvertCI4_Row_To_8888< N64::PfIA16, 0x3 > );
		break;

	case TexFmt_CI4_8888:
		ConvertGenericPalettisedCI4( dst, ti, ConvertCI4_Row< 0x4 | 0x3 >, ConvertCI4_Row< 0x3 >, ConvertPalette< N64::PfIA16, 0x1 > );
		break;

	default:
		DAEDALUS_ERROR( "Unhandled format for CI4IA16 textures" );
		break;
	}
}

//*****************************************************************************
// Used by MarioKart for Cars etc
//*****************************************************************************
void ConvertCI8_RGBA16(const TextureDestInfo & dst, const TextureInfo & ti)
{
	switch( dst.Format )
	{
	case TexFmt_8888:
		ConvertGenericPalettised( dst, ti, ConvertCI8_Row_To_8888< N64::Pf5551, 0x4 | 0x3 >, ConvertCI8_Row_To_8888< N64::Pf5551, 0x3 > );
		break;

	case TexFmt_CI8_8888:
		ConvertGenericPalettisedCI8( dst, ti, ConvertCI8_Row< 0x4 | 0x3 >, ConvertCI8_Row< 0x3 >, ConvertPalette< N64::Pf5551, 0x1 > );
		break;

	default:
		DAEDALUS_ERROR( "Unhandled format for CI8RGB16 textures" );
		break;
	}

}

//*****************************************************************************
// Used by MarioKart for Cars etc
//*****************************************************************************
void ConvertCI8_IA16(const TextureDestInfo & dst, const TextureInfo & ti)
{
	switch( dst.Format )
	{
	case TexFmt_8888:
		ConvertGenericPalettised( dst, ti, ConvertCI8_Row_To_8888< N64::PfIA16, 0x4 | 0x3 >, ConvertCI8_Row_To_8888< N64::PfIA16, 0x3 > );
		break;

	case TexFmt_CI8_8888:
		ConvertGenericPalettisedCI8( dst, ti, ConvertCI8_Row< 0x4 | 0x3 >, ConvertCI8_Row< 0x3 >, ConvertPalette< N64::PfIA16, 0x1 > );
		break;

	default:
		DAEDALUS_ERROR( "Unhandled format for CI8IA16 textures" );
		break;
	}
}

} // anonymous namespace

//*****************************************************************************
//
//*****************************************************************************
bool	ConvertTexture( const TextureDestInfo & dst, const TextureInfo & ti )
{
	bool	handled( false );

	switch (ti.GetFormat())
	{
	case G_IM_FMT_RGBA:
		switch (ti.GetSize())
		{
		case G_IM_SIZ_16b:
			ConvertRGBA16( dst, ti );
			handled = true;
			break;
		case G_IM_SIZ_32b:
			ConvertRGBA32( dst, ti );
			handled = true;
			break;
		}
		break;

	case G_IM_FMT_YUV:
		break;

	case G_IM_FMT_CI:
		switch (ti.GetSize())
		{
		case G_IM_SIZ_4b: // 4bpp
			switch (ti.GetTLutFormat())
			{
				//case G_TT_NONE:
			case G_TT_RGBA16:
				ConvertCI4_RGBA16( dst, ti );
				handled = true;
				break;
			case G_TT_IA16:
				ConvertCI4_IA16( dst, ti );
				handled = true;
				break;
			}
			break;

		case G_IM_SIZ_8b: // 8bpp
			switch(ti.GetTLutFormat())
			{
			case G_TT_RGBA16:
				ConvertCI8_RGBA16( dst, ti );
				handled = true;
				break;
			case G_TT_IA16:
				ConvertCI8_IA16( dst, ti );
				handled = true;
				break;
			}
			break;
		}

		break;

	case G_IM_FMT_IA:
		switch (ti.GetSize())
		{
		case G_IM_SIZ_4b:
			ConvertIA4( dst, ti );
			handled = true;
			break;
		case G_IM_SIZ_8b:
			ConvertIA8( dst, ti );
			handled = true;
			break;
		case G_IM_SIZ_16b:
			ConvertIA16( dst, ti );
			handled = true;
			break;
		case G_IM_SIZ_32b:
			break;
		}
		break;

	case G_IM_FMT_I:
		switch (ti.GetSize())
		{
		case G_IM_SIZ_4b:
			ConvertI4( dst, ti );
			handled = true;
			break;
		case G_IM_SIZ_8b:
			ConvertI8( dst, ti );
			handled = true;
			break;
		}
		break;
	default:
		break;
	}

	DAEDALUS_ASSERT( handled, "Unhandled texture format" );
	return handled;
}
