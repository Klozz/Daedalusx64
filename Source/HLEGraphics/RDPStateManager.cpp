/*
Copyright (C) 2006,2007 StrmnNrmn

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
#include "RDPStateManager.h"


#include "Core/ROM.h"
#include "DebugDisplayList.h"

#include "Math/MathUtil.h"

#include "OSHLE/ultra_gbi.h"


extern SImageDescriptor g_TI;		// XXXX

//*****************************************************************************
//
//*****************************************************************************
CRDPStateManager::CRDPStateManager()
:	mNumReused( 0 )
,	mNumLoaded( 0 )
{
	InvalidateAllTileTextureInfo();
}

//*****************************************************************************
//
//*****************************************************************************
CRDPStateManager::~CRDPStateManager()
{
}

//*****************************************************************************
//
//*****************************************************************************
void CRDPStateManager::Reset()
{
	mLoadMap.clear();
	InvalidateAllTileTextureInfo();
}

//*****************************************************************************
//
//*****************************************************************************
void	CRDPStateManager::SetTile( const RDP_Tile & tile )
{
	u32 idx( tile.tile_idx );

	if( mTiles[ idx ] != tile )
	{
		mTiles[ idx ] = tile;
		mTileTextureInfoValid[ idx ] = false;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void	CRDPStateManager::SetTileSize( const RDP_TileSize & tile_size )
{
	u32 idx( tile_size.tile_idx );

	if( mTileSizes[ idx ] != tile_size )
	{
		// XXXX might be able to remove this with recent tile loading fixes?
		// Wetrix hack
		if( (tile_size.top > tile_size.bottom) | (tile_size.left > tile_size.right) )
		{
			DAEDALUS_DL_ERROR( "Specifying negative width/height for tile descriptor" );
			return;
		}

		mTileSizes[ idx ] = tile_size;
		mTileTextureInfoValid[ idx ] = false;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void	CRDPStateManager::LoadBlock( u32 idx, u32 address, bool swapped )
{
	u32	tmem_address( mTiles[ idx ].tmem );

	SLoadDetails &	load_details( mLoadMap[ tmem_address ] );
	load_details.Address = address;
	load_details.Pitch = ~0;
	load_details.Swapped = swapped;

	InvalidateAllTileTextureInfo();		// Can potentially invalidate all texture infos
}

//*****************************************************************************
//
//*****************************************************************************
void	CRDPStateManager::LoadTile( u32 idx, u32 address )
{
	u32	tmem_address( mTiles[ idx ].tmem );
	
	SLoadDetails &	load_details( mLoadMap[ tmem_address ] );
	load_details.Address = address;
	load_details.Pitch = g_TI.GetPitch();
	load_details.Swapped = false;

	InvalidateAllTileTextureInfo();		// Can potentially invalidate all texture infos
}

//*****************************************************************************
//
//*****************************************************************************
void	CRDPStateManager::InvalidateAllTileTextureInfo()
{
	for( u32 i = 0; i < 8; ++i )
	{
		mTileTextureInfoValid[ i ] = false;
	}
}

namespace
{
	//
	//	Limit the tile's width/height to the number of bits specified by mask_s/t.
	//	See the detailed noted in PSPRenderer::EnableTexturing for issues relating to this.
	//
	u16		GetTextureDimension( u16 tile_dimension, u8 mask )
	{
		if( mask != 0 )
		{
			return Min< u16 >( 1 << mask, tile_dimension );
		}

		return tile_dimension;
	}
}

//*****************************************************************************
//
//*****************************************************************************
const TextureInfo & CRDPStateManager::GetTextureDescriptor( u32 idx ) const
{
	DAEDALUS_ASSERT( idx < ARRAYSIZE( mTileTextureInfoValid ), "Invalid index %d", idx );
	if( !mTileTextureInfoValid[ idx ] )
	{
		TextureInfo &			ti( mTileTextureInfo[ idx ] );

		const RDP_Tile &		rdp_tile( mTiles[ idx ] );
		const RDP_TileSize &	rdp_tilesize( mTileSizes[ idx ] );
		u32						tmem_address( rdp_tile.tmem );

		LoadDetailsMap::const_iterator it( mLoadMap.find( tmem_address ) );

		u32		palette( 0 );
		u32		address( 0 );
		u32		pitch( 0 );
		bool	swapped( false );

		if( it != mLoadMap.end() )
		{
			const SLoadDetails &	load_details( it->second );

			address = load_details.Address;
			pitch = load_details.Pitch;
			swapped = load_details.Swapped;
		}
		else
		{
			DAEDALUS_DL_ERROR( "No texture has been loaded to this address" );
		}

		// If it was a block load - the pitch is determined by the tile size
		// else if it was a tile - the pitch is set when the tile is loaded
		if ( pitch == u32(~0) )
		{
			if( rdp_tile.size == G_IM_SIZ_32b )	pitch = rdp_tile.line << 4;
			else pitch = rdp_tile.line << 3;
		}

		//	Limit the tile's width/height to the number of bits specified by mask_s/t.
		//	See the detailed notes in PSPRenderer::EnableTexturing for issues relating to this.
		//
		u16		tile_width( GetTextureDimension( rdp_tilesize.GetWidth(), rdp_tile.mask_s ) );
		u16		tile_height( GetTextureDimension( rdp_tilesize.GetHeight(), rdp_tile.mask_t ) );


#ifdef DAEDALUS_ENABLE_ASSERTS
		u32		num_pixels( tile_width * tile_height );
		u32		num_bytes( pixels2bytes( num_pixels, rdp_tile.size ) );
		DAEDALUS_DL_ASSERT( num_bytes <= 4096, "Suspiciously large texture load: %d bytes (%dx%d, %dbpp)", num_bytes, tile_width, tile_height, (1<<(rdp_tile.size+2)) );
#endif
		//Quick fix to correct texturing issues and missing logo in Harvest Moon 64
		//if pixel size is 8b force palette to index 0
		if(rdp_tile.size != G_IM_SIZ_8b)
		{
			palette = rdp_tile.palette;
		}
		
		ti.SetTmemAddress( rdp_tile.tmem );
		ti.SetTLutIndex( palette ); 

		ti.SetLoadAddress( address );
		ti.SetFormat( rdp_tile.format );
		ti.SetSize( rdp_tile.size );

		// May not work if Left is not even?
#ifdef DAEDALUS_ENABLE_ASSERTS
		u32	tile_left( rdp_tilesize.left >> 2 );
		DAEDALUS_DL_ASSERT( (rdp_tile.size > 0) || (tile_left&1) == 0, "Expecting an even Left for 4bpp formats" );
#endif
		ti.SetWidth( tile_width );
		ti.SetHeight( tile_height );
		ti.SetPitch( pitch );
		ti.SetTLutFormat( gRDPOtherMode.text_tlut << G_MDSFT_TEXTLUT );
		ti.SetSwapped( swapped );
		ti.SetMirrorS( rdp_tile.mirror_s );
		ti.SetMirrorT( rdp_tile.mirror_t );

		// Hack to fix the sun in Zelda
		//
		if( g_ROM.ZELDA_HACK )
		{
			if(gRDPOtherMode.L == 0x0c184241 && ti.GetFormat() == G_IM_FMT_I /*&& ti.GetWidth() == 64*/)	
			{
				//ti.SetHeight( tile_height );	// (fix me)
				ti.SetWidth( tile_width >> 1 );	
				ti.SetPitch( pitch >> 1 );	
			}
		}
		// Hack - Extreme-G specifies RGBA/8 textures, but they're really CI8
		if( ti.GetFormat() == G_IM_FMT_RGBA && ti.GetSize() <= G_IM_SIZ_8b ) ti.SetFormat( G_IM_FMT_CI );

		// Force RGBA
		if( ti.GetFormat() == G_IM_FMT_CI && ti.GetTLutFormat() == G_TT_NONE ) ti.SetTLutFormat( G_TT_RGBA16 );

		mTileTextureInfoValid[ idx ] = true;
	}

	return mTileTextureInfo[ idx ];
}

//*****************************************************************************
//
//*****************************************************************************
// Effectively a singleton...needs refactoring
CRDPStateManager		gRDPStateManager;

