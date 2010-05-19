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
        u32 address = RDPSegAddr(command.cmd1);

        address &= (MAX_RAM_ADDRESS-1);

        g_Sprite2DInfo.spritePtr = (SpriteStruct *)(g_ps8RamBase+address);

		// ToDo : Fix me

        // Update the ucode table if we call Sprite2DBase, because we will need them
        /*if(gInstructionLookup[G_GBI1_SPRITE2D_DRAW] != DLParser_GBI1_Sprite2DDraw){
                SetCommand(G_GBI1_SPRITE2D_SCALEFLIP, DLParser_GBI1_Sprite2DScaleFlip);
                SetCommand(G_GBI1_SPRITE2D_DRAW, DLParser_GBI1_Sprite2DDraw);
        }*/
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Sprite2DScaleFlip( MicroCodeCommand command )
{
        g_Sprite2DInfo.scaleX = (((command.cmd1)>>16)   &0xFFFF)/1024.0f;
        g_Sprite2DInfo.scaleY = ( (command.cmd1)        &0xFFFF)/1024.0f;

        if( ((command.cmd1)&0xFFFF) < 0x100 )
        {
                g_Sprite2DInfo.scaleY = g_Sprite2DInfo.scaleX;
        }

        g_Sprite2DInfo.flipX = (unsigned short)(((command.cmd0)>>8)     &0xFF);
        g_Sprite2DInfo.flipY = (unsigned short)( (command.cmd0)         &0xFF);
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Sprite2DDraw( MicroCodeCommand command )
{
        //DL_PF("Not fully implemented");
        g_Sprite2DInfo.px = (short)(((command.cmd1)>>16)&0xFFFF)>>2;
        g_Sprite2DInfo.py = (short)( (command.cmd1)     &0xFFFF)>>2;

		// This a hack for Wipeout.
		// TODO : Find a workaround and remove this hack..
        if(g_Sprite2DInfo.spritePtr){
                if(g_Sprite2DInfo.spritePtr->SubImageWidth == 0){
                      DAEDALUS_DL_ERROR("Hack: Width or Height are 0. Skipping Sprite2DDraw");
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

                u32 imageX              = g_Sprite2DInfo.spritePtr->SourceImageOffsetS;
                u32 imageY              = g_Sprite2DInfo.spritePtr->SourceImageOffsetT;
                u32 imageW              = ti.GetWidth();
                u32 imageH              = ti.GetHeight();

                u32 frameX              = g_Sprite2DInfo.px;
                u32 frameY              = g_Sprite2DInfo.py;
                u32 frameW              = g_Sprite2DInfo.spritePtr->SubImageWidth / g_Sprite2DInfo.scaleX;
                u32 frameH              = g_Sprite2DInfo.spritePtr->SubImageHeight / g_Sprite2DInfo.scaleY;

                PSPRenderer::Get()->Draw2DTexture( (float)imageX, (float)imageY, (float)frameX ,(float)frameY, (float)imageW, (float)imageH, (float)frameW, (float)frameH);
                g_Sprite2DInfo.spritePtr = 0;
        }

		//Fix me

        // Restore ucode table
		/*
        SetCommand(G_GBI1_SPRITE2D_SCALEFLIP, DLParser_GBI1_CullDL);
        SetCommand(G_GBI1_SPRITE2D_DRAW, DLParser_GBI1_PopMtx);
		*/
}
