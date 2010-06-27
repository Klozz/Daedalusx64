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

Sprite2DInfo g_Sprite2DInfo;
//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Sprite2DBase( MicroCodeCommand command )
{
	u32 pc = gDisplayListStack.back().addr;		// This points to the next instruction
    u32 address = RDPSegAddr(command.inst.cmd1);

    address &= (MAX_RAM_ADDRESS-1);

    g_Sprite2DInfo.spritePtr = (SpriteStruct *)(g_ps8RamBase+address);

	// Jump to the next command
	command.inst.cmd0 = g_pu32RamBase[(pc>>2)+0];

	if ( (command.inst.cmd0>>24) == G_GBI1_SPRITE2D_SCALEFLIP )
	{
		// Really important! see below why
		command.inst.cmd1 = g_pu32RamBase[(pc>>2)+1];

		// Increment instruction, otherwise we won't get to the next command !!
		pc += 8;

		DLParser_GBI1_Sprite2DScaleFlip( command );

		// Jump to the next command
		command.inst.cmd0 = g_pu32RamBase[(pc>>2)+0];
	}

	if ( (command.inst.cmd0>>24) == G_GBI1_SPRITE2D_DRAW )
	{
		// Really important, otherwise we'll loose the next command
		command.inst.cmd1 = g_pu32RamBase[(pc>>2)+1];

		DLParser_GBI1_Sprite2DDraw( command );
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Sprite2DScaleFlip( MicroCodeCommand command )
{
	g_Sprite2DInfo.scaleX = (((command.inst.cmd1)>>16)   &0xFFFF)/1024.0f;
	g_Sprite2DInfo.scaleY = ( (command.inst.cmd1)        &0xFFFF)/1024.0f;

	if( ((command.inst.cmd1)&0xFFFF) < 0x100 )
	{
		g_Sprite2DInfo.scaleY = g_Sprite2DInfo.scaleX;
	}

	g_Sprite2DInfo.flipX = (u8)(((command.inst.cmd0)>>8)     &0xFF);
	g_Sprite2DInfo.flipY = (u8)( (command.inst.cmd0)         &0xFF);
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Sprite2DDraw( MicroCodeCommand command )
{
    //DL_PF("Not fully implemented");
    g_Sprite2DInfo.px = (s32)(((command.inst.cmd1)>>16)&0xFFFF)/4;
    g_Sprite2DInfo.py = (s32)( (command.inst.cmd1)     &0xFFFF)/4;


    if(g_Sprite2DInfo.spritePtr)
	{
		// This a hack for Wipeout.
		// TODO : Find a workaround to remove this hack..
		if(g_Sprite2DInfo.spritePtr->SubImageWidth == 0)
		{
			DAEDALUS_ERROR("Hack: Width is 0. Skipping Sprite2DDraw");
			g_Sprite2DInfo.spritePtr = 0;
			return;
		}

		TextureInfo ti;

		ti.SetFormat            (g_Sprite2DInfo.spritePtr->SourceImageType);
		ti.SetSize              (g_Sprite2DInfo.spritePtr->SourceImageBitSize);

		ti.SetLoadAddress       (RDPSegAddr(g_Sprite2DInfo.spritePtr->SourceImagePointer));

		ti.SetWidth             (g_Sprite2DInfo.spritePtr->SubImageWidth);
		ti.SetHeight            (g_Sprite2DInfo.spritePtr->SubImageHeight);
		ti.SetPitch             (g_Sprite2DInfo.spritePtr->Stride << ti.GetSize() >> 1);

		ti.SetSwapped           (0);

		ti.SetTLutIndex        ((u32)(g_pu8RamBase+RDPSegAddr(g_Sprite2DInfo.spritePtr->TlutPointer)));
		ti.SetTLutFormat       (2 << 14);  //RGBA16 

		CRefPtr<CTexture>       texture( CTextureCache::Get()->GetTexture( &ti ) );
		texture->GetTexture()->InstallTexture();

		f32 imageX              = g_Sprite2DInfo.spritePtr->SourceImageOffsetS;
		f32 imageY              = g_Sprite2DInfo.spritePtr->SourceImageOffsetT;
		f32 imageW              = ti.GetWidth();
		f32 imageH              = ti.GetHeight();

		f32 frameX              = g_Sprite2DInfo.px;
		f32 frameY              = g_Sprite2DInfo.py;
		f32 frameW              = g_Sprite2DInfo.spritePtr->SubImageWidth / g_Sprite2DInfo.scaleX;
		f32 frameH              = g_Sprite2DInfo.spritePtr->SubImageHeight / g_Sprite2DInfo.scaleY;

		PSPRenderer::Get()->Draw2DTexture( imageX, imageY, frameX ,frameY, imageW, imageH, frameW, frameH);
		//g_Sprite2DInfo.spritePtr = 0; // Why ?
    }

}