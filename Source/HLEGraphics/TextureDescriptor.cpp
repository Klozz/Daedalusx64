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
	for (u32 y = 0; y < Height; y+=3)		// Do every nth line?
	{
		// Byte fiddling won't work, but this probably doesn't matter
		hash_value = murmur2_neutral_hash( p_bytes, bytes_per_line, hash_value );
		p_bytes += Pitch;
	}

	if (GetFormat() == G_IM_FMT_CI)
	{
		u32 bytes;
		if ( GetSize() == G_IM_SIZ_4b )	bytes = 16  * 4;
		else									bytes = 256 * 4;

		p_bytes = reinterpret_cast< const u8 * >( GetPalettePtr() );
		hash_value = murmur2_neutral_hash( p_bytes, bytes, hash_value );
	}

	return hash_value;
}

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
