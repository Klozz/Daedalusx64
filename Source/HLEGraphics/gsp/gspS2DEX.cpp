/*
Copyright (C) 2009 Grazz
Copyright (C) 2003-2009 Rice1964

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

#include "gspCommon.h"

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_BgCopy( MicroCodeCommand command )
{
	DL_PF("DLParser_S2DEX_BgCopy");

	uObjBg *objBg = (uObjBg*)(g_pu8RamBase + RDPSegAddr(command.cmd1));

	u16 imageX = objBg->imageX >> 5;
	u16 imageY = objBg->imageY >> 5;

	u16 imageW = objBg->imageW >> 2;
	u16 imageH = objBg->imageH >> 2;

	s16 frameX = objBg->frameX >> 2;
	s16 frameY = objBg->frameY >> 2;
	u16 frameW = objBg->frameW >> 2;
	u16 frameH = objBg->frameH >> 2;

	TextureInfo ti;

	ti.SetFormat           (objBg->imageFmt);
	ti.SetSize             (objBg->imageSiz);

	ti.SetLoadAddress      (RDPSegAddr(objBg->imagePtr));
	ti.SetWidth            (imageW);
	ti.SetHeight           (imageH);
	ti.SetPitch			   (((imageW << ti.GetSize() >> 1)>>3)<<3); //force 8-bit alignment, this what sets our correct viewport.

	ti.SetSwapped          (0);

	ti.SetTLutIndex        (objBg->imagePal);
	ti.SetTLutFormat       (2 << 14);  //RGBA16 

	CRefPtr<CTexture>       texture( CTextureCache::Get()->GetTexture( &ti ) );
	texture->GetTexture()->InstallTexture();

	PSPRenderer::Get()->Draw2DTexture( (float)imageX, (float)imageY, (float)frameX ,(float)frameY, (float)imageW, (float)imageH, (float)frameW, (float)frameH);
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_SelectDl( MicroCodeCommand command )
{	
	// YoshiStory - 0x04
	static bool warned = false;

	DL_PF( "~*Not Implemented" );
	if (!warned)
	{
		RDP_NOIMPL("RDP: S2DEX_SelectDl", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_ObjSprite( MicroCodeCommand command )
{	
	// YoshiStory uses this - 0x06
	static bool warned = false;

	DL_PF( "~*Not Implemented" );
	if (!warned)
	{
		RDP_NOIMPL("RDP: S2DEX_ObjSprite", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_ObjRectangle( MicroCodeCommand command )
{	
	// YoshiStory uses this - 0x01
	static bool warned = false;

	DL_PF( "~*Not Implemented" );
	if (!warned)
	{
		RDP_NOIMPL("RDP: S2DEX_ObjRectangle", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_ObjRendermode( MicroCodeCommand command )
{	
	// Majora's Mask ,Doubutsu no Mori, and YoshiStory Menus uses this - 0x0b
	static bool warned = false;

	DL_PF( "~*Not Implemented" );
	if (!warned)
	{
		RDP_NOIMPL("RDP: S2DEX_ObjRendermode", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_ObjLoadTxtr( MicroCodeCommand command )
{	
	// Command and Conquer and YoshiStory uses this - 0x05
	static bool warned = false;

	DL_PF( "~*Not Implemented" );
	if (!warned)
	{
		RDP_NOIMPL("RDP: S2DEX_ObjLoadTxtr)", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_ObjLdtxSprite( MicroCodeCommand command )
{	
	// YoshiStory uses this - 0xc2
	static bool warned = false;

	DL_PF( "~*Not Implemented" );
	if (!warned)
	{
		RDP_NOIMPL("RDP: S2DEX_ObjLdtxSprite", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_ObjLdtxRect( MicroCodeCommand command )
{	
	// YoshiStory uses this - 0x07
	static bool warned = false;

	DL_PF( "~*Not Implemented" );
	if (!warned)
	{
		RDP_NOIMPL("RDP: S2DEX_ObjLdtxRect", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_ObjLdtxRectR( MicroCodeCommand command )
{	
	// YoshiStory uses this - 0x08
	static bool warned = false;

	DL_PF( "~*Not Implemented" );
	if (!warned)
	{
		RDP_NOIMPL("RDP: S2DEX_ObjLdtxRectR", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_RDPHalf_0( MicroCodeCommand command )
{	
	//RDP: RSP_S2DEX_RDPHALF_0 (0xe449c0a8 0x003b40a4)
	//0x001d3c88: e449c0a8 003b40a4 RDP_TEXRECT 
	//0x001d3c90: b4000000 00000000 RSP_RDPHALF_1
	//0x001d3c98: b3000000 04000400 RSP_RDPHALF_2

	u32 pc = gDisplayListStack.back().addr;             // This points to the next instruction
	u32 NextUcode = *(u32 *)(g_pu8RamBase + pc);

	if( (NextUcode>>24) != G_GBI2_SELECT_DL )
	{
		// Pokemom Puzzle League
		if( (NextUcode>>24) == 0xB4 )
		{
			DLParser_TexRect(command);
		}
		else
		{
			RDP_NOIMPL("RDP: S2DEX_RDPHALF_0 (0x%08x 0x%08x)", command.cmd0, command.cmd1);
		}
	}
	else
	{
		RDP_NOIMPL("RDP: S2DEX_RDPHALF_0 (0x%08x 0x%08x)", command.cmd0, command.cmd1);
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_ObjMoveMem( MicroCodeCommand command )
{	
	// Ogre Battle 64 and YoshiStory uses this - 0xdc
	static bool warned = false;

	DL_PF( "~*Not Implemented" );
	if (!warned)
	{
		RDP_NOIMPL("RDP: S2DEX_ObjMoveMem", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_Bg1cyc( MicroCodeCommand command )
{	
	uObjScaleBg *objBg = (uObjScaleBg *)(g_pu8RamBase + RDPSegAddr(command.cmd1));

	u16 imageX = objBg->imageX >> 5;
	u16 imageY = objBg->imageY >> 5;

	u16 imageW = objBg->imageW >> 2;
	u16 imageH = objBg->imageH >> 2;

	s16 frameX = objBg->frameX >> 2;
	s16 frameY = objBg->frameY >> 2;
	u16 frameW = objBg->frameW >> 2;
	u16 frameH = objBg->frameH >> 2;

	TextureInfo ti;

	ti.SetFormat           (objBg->imageFmt);
	ti.SetSize             (objBg->imageSiz);

	ti.SetLoadAddress      (RDPSegAddr(objBg->imagePtr));
	ti.SetWidth            (imageW);
	ti.SetHeight           (imageH);
	ti.SetPitch			   (((imageW << ti.GetSize() >> 1)>>3)<<3); //force 8-bit alignment, this what sets our correct viewport.

	ti.SetSwapped          (0);

	ti.SetTLutIndex        (objBg->imagePal);
	ti.SetTLutFormat       (2 << 14);  //RGBA16 >> (2 << G_MDSFT_TEXTLUT)

	CRefPtr<CTexture>       texture( CTextureCache::Get()->GetTexture( &ti ) );
	texture->GetTexture()->InstallTexture();

	PSPRenderer::Get()->Draw2DTexture( (float)imageX, (float)imageY, (float)frameX ,(float)frameY, (float)imageW, (float)imageH, (float)frameW, (float)frameH);
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_S2DEX_ObjRectangleR( MicroCodeCommand command )
{	
	// Ogre Battle 64 and YoshiStory uses this - 0xda
	static bool warned = false;

	DL_PF( "~*Not Implemented" );
	if (!warned)
	{
		RDP_NOIMPL("RDP: S2DEX_ObjRectangleR", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
