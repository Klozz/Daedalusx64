/*
Copyright (C) 2001,2007 StrmnNrmn

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

#include "TextureDescriptor.h"
#include "OSHLE/ultra_gbi.h"

#include "RDP.h"
#include "ConfigOptions.h"

#include "Core/Memory.h"

#include "Utility/Profiler.h"
#include "Utility/Hash.h"

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
//*************************************************************************************
//
//*************************************************************************************
namespace
{
const char * const	pszImgFormat[8] = {"RGBA", "YUV", "CI", "IA", "I", "?1", "?2", "?3"};
const u32			pnImgSize[4]   = {4, 8, 16, 32};
}

//*************************************************************************************
//
//*************************************************************************************
const char * TextureInfo::GetFormatName() const
{
	return pszImgFormat[ Format ];
}

//*************************************************************************************
//
//*************************************************************************************
u32 TextureInfo::GetSizeInBits() const	
{
	return pnImgSize[ Size ];
}
#endif
//*************************************************************************************
//
//*************************************************************************************
void TextureInfo::SetTLutFormat( u32 format )
{
	TLutFmt = format >> G_MDSFT_TEXTLUT;
}

//*************************************************************************************
//
//*************************************************************************************
u32	TextureInfo::GetTLutFormat() const
{
	return TLutFmt << G_MDSFT_TEXTLUT;
}

//*************************************************************************************
//
//*************************************************************************************
const void *	TextureInfo::GetPalettePtr() const
{
	// Want to advance 16x16bpp palette entries (i.e. 32 bytes into tmem for each palette), i.e. <<5.
	u32 address = ( 0x100 + ( u32( TLutIndex ) << 2 ) ) << 3;
	return &gTextureMemory[ address ];
}

//*************************************************************************************
//
//*************************************************************************************
u32	TextureInfo::GetWidthInBytes() const
{
	return pixels2bytes( Width, Size );
}

//*************************************************************************************
//
//*************************************************************************************
#if 1 //1->new hash(fast), 0-> old hash(expensive)
u32 TextureInfo::GenerateHashValue() const
{
	DAEDALUS_PROFILE( "TextureInfo::GenerateHashValue" );
	
	// If CRC checking is disabled, always return 0
	if ( gCheckTextureHashFrequency == 0 ) return 0;

	//DAEDALUS_ASSERT( (GetLoadAddress() + Height * Pitch) < 4*1024*1024, "Address of texture is out of bounds" );

	const u8 *p_bytes = g_pu8RamBase + GetLoadAddress();
	u32 bytes_per_line = GetWidthInBytes();	//Get number of bytes per line
	u32 hash_value = 0;
	
	u32 step = bytes_per_line + (bytes_per_line >> 2);
	if (Height > 15)	step = (Height >> 3) * bytes_per_line + (bytes_per_line >> 2);
	if (bytes_per_line > 16) bytes_per_line = 16;	//Limit to 16 bytes
	
	//We want to sample the data  as far apart as possible
	//u32 step = ((Pitch-16) / 6) & 0xFFFC;
	//if (bytes_per_line > 16) bytes_per_line = 16;	//Limit to 16 bytes

	u32 shift=0;
	for (u32 y = 0; y < 6; y++)		// Hash X Lines per texture
	{
		//hash_value = murmur2_neutral_hash( p_bytes, bytes_per_line, hash_value);
		//for (u32 z = 0; z < bytes_per_line; z++) hash_value ^= p_bytes[z] << ((z & 3) << 3);	//Do check sum for each line
		for (u32 z = 0; z < bytes_per_line; z++)
		{
			hash_value ^= p_bytes[z] << shift++;	//Do check sum for each line
			if (shift > 24) shift=0;
		}
		p_bytes += step;	//Advance pointer to next line
	}

//	printf("%08X %d %d %d %d %d\n",hash_value,step, Size, Pitch, Height, Width);
	return hash_value;
}

#else
u32 TextureInfo::GenerateHashValue() const
{
	DAEDALUS_PROFILE( "TextureInfo::GenerateHashValue" );

	// If CRC checking is disabled, always return 0
	if ( gCheckTextureHashFrequency == 0 )
		return 0;
	
	u32 bytes_per_line( GetWidthInBytes() );

	//DBGConsole_Msg(0, "BytesPerLine: %d", bytes_per_line);
	
	// A very simple crc - just summation
	u32 hash_value( 0 );

	//DAEDALUS_ASSERT( (GetLoadAddress() + Height * Pitch) < 4*1024*1024, "Address of texture is out of bounds" );

	const u8 * p_bytes( g_pu8RamBase + GetLoadAddress() );
	u32 step;
	if (Height > 4)
	{
		step = (Height/2)-1;
	}else{
		step = 1;
	}

	for (u32 y = 0; y < Height; y+=step)		// Hash 3 Lines per texture
	{
		// Byte fiddling won't work, but this probably doesn't matter
		hash_value = murmur2_neutral_hash( p_bytes, bytes_per_line, hash_value );
		p_bytes += (Pitch * step);
	}

	if (GetFormat() == G_IM_FMT_CI)
	{
		u32 bytes;
		if ( GetSize() == G_IM_SIZ_4b )	bytes = 16  * 4;
		else							bytes = 256 * 4;

		p_bytes = reinterpret_cast< const u8 * >( GetPalettePtr() );
		hash_value = murmur2_neutral_hash( p_bytes, bytes, hash_value );
	}

	return hash_value;
}
#endif
//*************************************************************************************
//
//*************************************************************************************
ETextureFormat	TextureInfo::SelectNativeFormat() const
{
	switch (Format)
	{
	case G_IM_FMT_RGBA:
		switch (Size)
		{
		case G_IM_SIZ_16b:
			return TexFmt_5551;
		case G_IM_SIZ_32b:
			return TexFmt_8888;
		}
		break;
		
	case G_IM_FMT_YUV:
		break;

	case G_IM_FMT_CI:
		switch (Size)
		{
		case G_IM_SIZ_4b: // 4bpp
			switch (GetTLutFormat())
			{
			case G_TT_RGBA16:
				return TexFmt_CI4_8888;
			case G_TT_IA16:
				return TexFmt_CI4_8888;
			}
			break;
			
		case G_IM_SIZ_8b: // 8bpp
			switch(GetTLutFormat())
			{
			case G_TT_RGBA16:
				return TexFmt_CI8_8888;
			case G_TT_IA16:
				return TexFmt_CI8_8888;
			}
			break;
		}
		
		break;

	case G_IM_FMT_IA:
		switch (Size)
		{
		case G_IM_SIZ_4b:
			return TexFmt_4444;
		case G_IM_SIZ_8b:
			return TexFmt_4444;
		case G_IM_SIZ_16b:
			return TexFmt_8888;
		}
		break;

	case G_IM_FMT_I:
		switch (Size)
		{
		case G_IM_SIZ_4b:
			return TexFmt_4444;
		case G_IM_SIZ_8b:
			return TexFmt_8888;
		}
		break;
	}

	// Unhandled!
	DAEDALUS_ERROR( "Unhandled texture format" );
	return TexFmt_8888;

}
