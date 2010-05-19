/*
Copyright (C) 2010 StrmnNrmn

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

//
// This mostly based from ucode.h from Rice Video and Glide Napalm :)
//

//*****************************************************************************
// GBI1
//*****************************************************************************

void DLParser_Nothing( MicroCodeCommand command );
static void DLParser_GBI1_SpNoop( MicroCodeCommand command );
static void DLParser_GBI1_MoveMem( MicroCodeCommand command );

static void DLParser_GBI1_Reserved0( MicroCodeCommand command );
static void DLParser_GBI1_Reserved1( MicroCodeCommand command );
static void DLParser_GBI1_Reserved2( MicroCodeCommand command );
static void DLParser_GBI1_Reserved3( MicroCodeCommand command );

static void DLParser_GBI1_RDPHalf_Cont( MicroCodeCommand command );
static void DLParser_GBI1_RDPHalf_2( MicroCodeCommand command );
static void DLParser_GBI1_RDPHalf_1( MicroCodeCommand command );
void DLParser_GBI1_MoveWord( MicroCodeCommand command );
static void DLParser_GBI1_Noop( MicroCodeCommand command );

//*****************************************************************************
// GBI2
//*****************************************************************************
static void DLParser_GBI2_Noop( MicroCodeCommand command );
static void DLParser_GBI2_DMA_IO( MicroCodeCommand command );
static void DLParser_GBI2_Special1( MicroCodeCommand command );
static void DLParser_GBI2_Special2( MicroCodeCommand command );
static void DLParser_GBI2_Special3( MicroCodeCommand command );
static void DLParser_GBI2_MoveWord( MicroCodeCommand command );
static void DLParser_GBI2_MoveMem( MicroCodeCommand command );

static void DLParser_GBI2_RDPHalf_2( MicroCodeCommand command );
static void DLParser_GBI2_SpNoop( MicroCodeCommand command );



// Include ucode header files
#include "gsp/gspMacros.h"
#include "gsp/gspSprite2D.h"
#include "gsp/gspS2DEX.h"
#include "gsp/gspCustom.h"


//*****************************************************************************
// RDP Commands
//*****************************************************************************
void DLParser_TexRect( MicroCodeCommand command );
static void DLParser_TexRectFlip( MicroCodeCommand command );
static void DLParser_RDPLoadSync( MicroCodeCommand command );
static void DLParser_RDPPipeSync( MicroCodeCommand command );
static void DLParser_RDPTileSync( MicroCodeCommand command );
static void DLParser_RDPFullSync( MicroCodeCommand command );
static void DLParser_SetKeyGB( MicroCodeCommand command );
static void DLParser_SetKeyR( MicroCodeCommand command );
static void DLParser_SetConvert( MicroCodeCommand command );
static void DLParser_SetScissor( MicroCodeCommand command );
static void DLParser_SetPrimDepth( MicroCodeCommand command );
static void DLParser_RDPSetOtherMode( MicroCodeCommand command );
static void DLParser_LoadTLut( MicroCodeCommand command );
static void DLParser_SetTileSize( MicroCodeCommand command );
static void DLParser_LoadBlock( MicroCodeCommand command );
static void DLParser_LoadTile( MicroCodeCommand command );
static void DLParser_SetTile( MicroCodeCommand command );
static void DLParser_FillRect( MicroCodeCommand command );
static void DLParser_SetFillColor( MicroCodeCommand command );
static void DLParser_SetFogColor( MicroCodeCommand command );
static void DLParser_SetBlendColor( MicroCodeCommand command );
static void DLParser_SetPrimColor( MicroCodeCommand command );
static void DLParser_SetEnvColor( MicroCodeCommand command );
static void DLParser_SetCombine( MicroCodeCommand command );
static void DLParser_SetTImg( MicroCodeCommand command );
static void DLParser_SetZImg( MicroCodeCommand command );
static void DLParser_SetCImg( MicroCodeCommand command );


static void DLParser_TriFill( MicroCodeCommand command );
static void DLParser_TriFillZ( MicroCodeCommand command );
static void DLParser_TriTxtr( MicroCodeCommand command );
static void DLParser_TriTxtrZ( MicroCodeCommand command );
static void DLParser_TriShade( MicroCodeCommand command );
static void DLParser_TriShadeZ( MicroCodeCommand command );
static void DLParser_TriShadeTxtr( MicroCodeCommand command );
static void DLParser_TriShadeTxtrZ( MicroCodeCommand command );

void DLParser_GBI2_DL_Count( MicroCodeCommand command );
void DLParser_GBI2_0x8( MicroCodeCommand command );

static MicroCodeInstruction gInstructionLookup[11][256] =
{
	// uCode 0 - RSP SW 2.0X
	// Games: Super Mario 64, Tetrisphere, Demos
	{
		DLParser_GBI1_SpNoop,   DLParser_GBI1_Mtx,		  DLParser_GBI1_Reserved0, DLParser_GBI1_MoveMem,
		DLParser_GBI0_Vtx,	   DLParser_GBI1_Reserved1,	  DLParser_GBI1_DL,		 DLParser_GBI1_Reserved2,
		DLParser_GBI1_Reserved3, DLParser_GBI1_Sprite2DBase, DLParser_Nothing,	 DLParser_Nothing,
		DLParser_Nothing,   DLParser_Nothing,		  DLParser_Nothing,	 DLParser_Nothing,
	//10
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//20
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//30
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//40
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//50
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//60
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//70
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

	//80
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//90
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//a0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//b0
		DLParser_Nothing,	DLParser_GBI0_Tri2,			DLParser_GBI1_RDPHalf_Cont,		DLParser_GBI1_RDPHalf_2,
		DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D,		DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
		DLParser_GBI1_EndDL,		DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH,		DLParser_GBI1_Texture,
		DLParser_GBI1_MoveWord,  DLParser_GBI1_PopMtx,		DLParser_GBI1_CullDL,			DLParser_GBI1_Tri1,

	//c0
		DLParser_GBI1_Noop,   DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
	//d0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//e0
		DLParser_Nothing,	  DLParser_Nothing,		DLParser_Nothing,	   DLParser_Nothing,
		DLParser_TexRect,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
	//f0
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
	},
	    // uCode 1 - F3DEX 1.XX
        // 00-3f
        // games: Mario Kart, Star Fox
	{
		DLParser_GBI1_SpNoop,				   DLParser_GBI1_Mtx,		  DLParser_GBI1_Reserved0, DLParser_GBI1_MoveMem,
		DLParser_GBI1_Vtx,					   DLParser_GBI1_Reserved1,	  DLParser_GBI1_DL,		 DLParser_GBI1_Reserved2,
        DLParser_GBI1_Reserved3,			   DLParser_GBI1_Sprite2DBase,           DLParser_Nothing,                      DLParser_Nothing,
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
      // 40-7f: unused
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
      // 80-bf: Immediate commands
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_GBI1_LoadUCode,   
		DLParser_GBI1_BranchZ,	DLParser_GBI1_Tri2,			DLParser_GBI1_ModifyVtx,		DLParser_GBI1_RDPHalf_2,
		DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D,		DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
		DLParser_GBI1_EndDL,		DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH,		DLParser_GBI1_Texture,
		DLParser_GBI1_MoveWord,  DLParser_GBI1_PopMtx,		DLParser_GBI1_CullDL,			DLParser_GBI1_Tri1,
      // c0-ff: RDP commands
        DLParser_GBI1_Noop,               DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
		DLParser_TexRect,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
	},

		// Ucode:F3DEX_GBI_2
		// Zelda and new games
	{
		DLParser_GBI2_Noop,	  DLParser_GBI2_Vtx,		 DLParser_GBI1_ModifyVtx, DLParser_GBI2_CullDL,
		DLParser_GBI1_BranchZ,	DLParser_GBI2_Tri1,	 DLParser_GBI2_Tri2,		 DLParser_GBI2_Quad,
		DLParser_GBI2_0x8,	  DLParser_S2DEX_Bg1cyc, DLParser_S2DEX_BgCopy,  DLParser_S2DEX_ObjRendermode,
		DLParser_Nothing,  DLParser_Nothing,	 DLParser_Nothing,	 DLParser_Nothing,
	//10
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//20
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//30
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//40
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//50
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//60
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//70
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

	//80
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//90
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//a0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_GBI1_LoadUCode,
	//b0
		DLParser_GBI1_BranchZ,	DLParser_GBI0_Tri2,			DLParser_GBI1_ModifyVtx,		DLParser_GBI1_RDPHalf_2,
		DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D,		DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
		DLParser_GBI1_EndDL,		DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH,		DLParser_GBI1_Texture,
		DLParser_GBI1_MoveWord,  DLParser_GBI1_PopMtx,		DLParser_GBI1_CullDL,			DLParser_GBI1_Tri1,

	//c0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
	//d0
		DLParser_Nothing,  DLParser_Nothing,		 DLParser_Nothing,	 DLParser_Nothing,
		DLParser_Nothing,  DLParser_GBI2_DL_Count,	 DLParser_GBI2_DMA_IO, DLParser_GBI2_Texture,
		DLParser_GBI2_PopMtx,  DLParser_GBI2_GeometryMode,          DLParser_GBI2_Mtx,		 DLParser_GBI2_MoveWord,
		DLParser_GBI2_MoveMem, DLParser_GBI1_LoadUCode,	 DLParser_GBI2_DL,        DLParser_GBI2_EndDL,
	//e0
		DLParser_GBI2_SpNoop, DLParser_GBI1_RDPHalf_1,   DLParser_GBI2_SetOtherModeL, DLParser_GBI2_SetOtherModeH,
		DLParser_TexRect,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
	//f0
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
	},

		// uCode 6 - RSP SW 2.0 Diddy
		// Games: Diddy Kong Racing
	{
		DLParser_GBI1_SpNoop,   DLParser_MtxDKR, DLParser_GBI1_Reserved0, DLParser_GBI1_MoveMem,
		DLParser_GBI0_Vtx_DKR,	   DLParser_DmaTri,		  DLParser_GBI1_DL,		 DLParser_DLInMem,
		DLParser_GBI1_Reserved3, DLParser_GBI1_Sprite2DBase, DLParser_Nothing,	 DLParser_Nothing,
		DLParser_Nothing,   DLParser_Nothing,		  DLParser_Nothing,	 DLParser_Nothing, 
		//10
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//20
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//30
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//40
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//50
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//60
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//70
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//80
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//90
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//a0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//b0
		DLParser_Nothing,	DLParser_GBI0_Tri2,			DLParser_GBI1_RDPHalf_Cont,		DLParser_GBI1_RDPHalf_2,
		DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D,		DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
		DLParser_GBI1_EndDL,		DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH,		DLParser_GBI1_Texture,
		DLParser_MoveWord_DKR,  DLParser_GBI1_PopMtx,		DLParser_GBI1_CullDL,			DLParser_GBI1_Tri1,/*addr6*/
		//c0
		DLParser_GBI1_Noop,   DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
		//d0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//e0
		DLParser_Nothing,	  DLParser_Nothing,		DLParser_Nothing,	   DLParser_Nothing,
		DLParser_TexRect,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
		//f0
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
		//g0
	},
			
		// Ucode: S2DEX 1.--
		// Games: Yoshi's Story
	
	{
		DLParser_GBI1_SpNoop,	  DLParser_S2DEX_Bg1cyc/*2*/,	 DLParser_S2DEX_BgCopy, DLParser_S2DEX_ObjRectangle,
		DLParser_S2DEX_ObjSprite, DLParser_S2DEX_ObjMoveMem, DLParser_GBI1_DL,		DLParser_GBI1_Reserved0,
		DLParser_GBI1_Reserved0,	  DLParser_Nothing,		 DLParser_Nothing,	DLParser_Nothing,
		DLParser_Nothing,	  DLParser_Nothing,		 DLParser_Nothing,	DLParser_Nothing,

	//10
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//20
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//30
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//40
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//50
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//60
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//70
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

	//80
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//90
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//a0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_GBI1_LoadUCode,
	//b0
		DLParser_S2DEX_SelectDl, DLParser_S2DEX_ObjRendermode/*2*/, DLParser_S2DEX_ObjRectangleR,  DLParser_GBI1_RDPHalf_2,
		DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D,		DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
		DLParser_GBI1_EndDL,		DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH,		DLParser_GBI1_Texture,
		DLParser_GBI1_MoveWord,  DLParser_GBI1_PopMtx,		DLParser_GBI1_CullDL,			DLParser_GBI1_Tri1,

	//c0
		DLParser_GBI1_Noop,				DLParser_S2DEX_ObjLoadTxtr, DLParser_S2DEX_ObjLdtxSprite, DLParser_S2DEX_ObjLdtxRect,
		DLParser_S2DEX_ObjLdtxRectR, DLParser_Nothing,		 DLParser_Nothing,			  DLParser_Nothing,
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
	//d0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//e0
		DLParser_Nothing,	  DLParser_Nothing,		DLParser_Nothing,	   DLParser_Nothing,
		DLParser_S2DEX_RDPHalf_0,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
	//f0
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
	},

	// Ucode 3 - S2DEX GBI2
	{

		DLParser_GBI1_Noop,	  DLParser_S2DEX_ObjRectangle,	 DLParser_S2DEX_ObjSprite, DLParser_GBI2_CullDL,
		DLParser_S2DEX_SelectDl, DLParser_S2DEX_ObjLoadTxtr, DLParser_S2DEX_ObjLdtxSprite,		DLParser_S2DEX_ObjLdtxRect,
		DLParser_S2DEX_ObjLdtxRectR,	  DLParser_S2DEX_Bg1cyc,		 DLParser_S2DEX_BgCopy,	DLParser_S2DEX_ObjRendermode,
		DLParser_Nothing,	  DLParser_Nothing,		 DLParser_Nothing,	DLParser_Nothing,
		//10
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//20
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//30
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//40
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//50
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//60
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//70
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//80
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//90
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//a0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_GBI1_LoadUCode,
		//b0
		DLParser_GBI1_BranchZ,	DLParser_GBI0_Tri2,			DLParser_GBI1_ModifyVtx,		DLParser_GBI1_RDPHalf_2,
		DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D,		DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
		DLParser_GBI1_EndDL,		DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH,		DLParser_GBI1_Texture,
		DLParser_GBI1_MoveWord,  DLParser_GBI1_PopMtx,		DLParser_GBI1_CullDL,			DLParser_GBI1_Tri1,
		//c0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
		//d0
		DLParser_Nothing,  DLParser_Nothing,		 DLParser_Nothing,	 DLParser_Nothing,
		DLParser_Nothing,  DLParser_GBI2_DL_Count,	 DLParser_GBI2_DMA_IO, DLParser_GBI2_Texture,
		DLParser_GBI2_PopMtx,  DLParser_GBI2_GeometryMode,          DLParser_GBI2_Mtx,		 DLParser_GBI2_MoveWord,
		DLParser_GBI2_MoveMem, DLParser_GBI1_LoadUCode,	 DLParser_GBI2_DL,        DLParser_GBI2_EndDL,
		//e0
		DLParser_GBI2_SpNoop, DLParser_GBI1_RDPHalf_1,   DLParser_GBI2_SetOtherModeL, DLParser_GBI2_SetOtherModeH,
		DLParser_TexRect,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
		//f0
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
	},

		// uCode 0 - FAST3D.XX
        // 00-3f
        // games: Wave Racer USA
	{
		DLParser_GBI1_SpNoop,   DLParser_GBI1_Mtx,		  DLParser_GBI1_Reserved0, DLParser_GBI1_MoveMem,
		DLParser_GBI0_Vtx_WRUS,	   DLParser_GBI1_Reserved1,	  DLParser_GBI1_DL,		 DLParser_GBI1_Reserved2,
		DLParser_GBI1_Reserved3, DLParser_GBI1_Sprite2DBase, DLParser_Nothing,	 DLParser_Nothing,
		DLParser_Nothing,   DLParser_Nothing,		  DLParser_Nothing,	 DLParser_Nothing,
	//10
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//20
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//30
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//40
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//50
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//60
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//70
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

	//80
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//90
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//a0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//b0
		DLParser_Nothing,	DLParser_GBI0_Tri2,			DLParser_GBI1_RDPHalf_Cont,		DLParser_GBI1_RDPHalf_2,
		DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D,		DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
		DLParser_GBI1_EndDL,		DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH,		DLParser_GBI1_Texture,
		DLParser_GBI1_MoveWord,  DLParser_GBI1_PopMtx,		DLParser_GBI1_CullDL,			DLParser_GBI1_Tri1,

	//c0
		DLParser_GBI1_Noop,   DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
	//d0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//e0
		DLParser_Nothing,	  DLParser_Nothing,		DLParser_Nothing,	   DLParser_Nothing,
		DLParser_TexRect,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
	//f0
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
	},

	// uCode 11 - RSP SW 2.0 Gemini
	// Games: Gemini and Mickey
	{
		DLParser_GBI1_SpNoop,   DLParser_MtxDKR, DLParser_GBI1_Reserved0, DLParser_GBI1_MoveMem,
		DLParser_GBI0_Vtx_Gemini,	   DLParser_DmaTri,		  DLParser_GBI1_DL,		 DLParser_DLInMem,
		DLParser_GBI1_Reserved3, DLParser_GBI1_Sprite2DBase, DLParser_Nothing,	 DLParser_Nothing,
		DLParser_Nothing,   DLParser_Nothing,		  DLParser_Nothing,	 DLParser_Nothing, 
		//10
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//20
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//30
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//40
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//50
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//60
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//70
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//80
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//90
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//a0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		//b0
		DLParser_Nothing,	DLParser_GBI0_Tri2,			DLParser_GBI1_RDPHalf_Cont,		DLParser_GBI1_RDPHalf_2,
		DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D,		DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
		DLParser_GBI1_EndDL,		DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH,		DLParser_GBI1_Texture,
		DLParser_MoveWord_DKR,  DLParser_GBI1_PopMtx,		DLParser_GBI1_CullDL,			DLParser_GBI1_Tri1,/*addr6*/
		//c0
		DLParser_GBI1_Noop,   DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
		//d0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, 
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		//e0
		DLParser_Nothing,	  DLParser_Nothing,		DLParser_Nothing,	   DLParser_Nothing,
		DLParser_TexRect,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
		//f0
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
		//g0
	},

	//uCode 16 - RSP SW 2.0D EXT
	//Games: Star Wars: Shadows of the Empire
	{
		DLParser_GBI1_SpNoop,   DLParser_GBI1_Mtx,		  DLParser_GBI1_Reserved0, DLParser_GBI1_MoveMem,
		DLParser_GBI0_Vtx_ShadowOfEmpire,	   DLParser_GBI1_Reserved1,	  DLParser_GBI1_DL,		 DLParser_GBI1_Reserved2,
		DLParser_GBI1_Reserved3, DLParser_GBI1_Sprite2DBase, DLParser_Nothing,	 DLParser_Nothing,
		DLParser_Nothing,   DLParser_Nothing,		  DLParser_Nothing,	 DLParser_Nothing,
	//10
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//20
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//30
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//40
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//50
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//60
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//70
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

	//80
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//90
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//a0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//b0
		DLParser_Nothing,	DLParser_GBI0_Tri2,			DLParser_GBI1_RDPHalf_Cont,		DLParser_GBI1_RDPHalf_2,
		DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D,		DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
		DLParser_GBI1_EndDL,		DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH,		DLParser_GBI1_Texture,
		DLParser_GBI1_MoveWord,  DLParser_GBI1_PopMtx,		DLParser_GBI1_CullDL,			DLParser_GBI1_Tri1,

	//c0
		DLParser_GBI1_Noop,   DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
	//d0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//e0
		DLParser_Nothing,	  DLParser_Nothing,		DLParser_Nothing,	   DLParser_Nothing,
		DLParser_TexRect,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
	//f0
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
	},

	    // uCode 1 - F3DEX Last Legion
        // 00-3f
        // games: Last Legion, Toukon, Toukon 2 
	{
		DLParser_GBI1_SpNoop,				   DLParser_GBI1_Mtx,		  DLParser_GBI1_Reserved0, DLParser_GBI1_MoveMem,
		DLParser_GBI1_Vtx,					   DLParser_GBI1_Reserved1,	  DLParser_GBI1_DL,		 DLParser_GBI1_Reserved2,
        DLParser_GBI1_Reserved3,			   DLParser_GBI1_Sprite2DBase,           DLParser_Nothing,                      DLParser_Nothing,
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
      // 40-7f: unused
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
      // 80-bf: Immediate commands
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,        
        DLParser_Nothing,                      DLParser_Nothing,                      DLParser_Nothing,                      DLParser_GBI1_LoadUCode,   
		DLParser_GBI1_BranchZ,	DLParser_GBI1_Tri2,			DLParser_GBI1_ModifyVtx,		DLParser_GBI1_RDPHalf_2,
		DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D,		DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
		DLParser_GBI1_EndDL,		DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH,		DLParser_GBI1_Texture,
		DLParser_GBI1_MoveWord,  DLParser_GBI1_PopMtx,		DLParser_GBI1_CullDL,			DLParser_GBI1_Tri1,
      // c0-ff: RDP commands
        DLParser_GBI1_Noop,               DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
        DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
		DLParser_TexRect_Last_Legion,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
	},

	// uCode 0 - RSP SW 2.0X
	// Games: Golden Eye
	{
		DLParser_GBI1_SpNoop,   DLParser_GBI1_Mtx,		  DLParser_GBI1_Reserved0, DLParser_GBI1_MoveMem,
		DLParser_GBI0_Vtx,	   DLParser_GBI1_Reserved1,	  DLParser_GBI1_DL,		 DLParser_GBI1_Reserved2,
		DLParser_GBI1_Reserved3, DLParser_GBI1_Sprite2DBase, DLParser_Nothing,	 DLParser_Nothing,
		DLParser_Nothing,   DLParser_Nothing,		  DLParser_Nothing,	 DLParser_Nothing,
	//10
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//20
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//30
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//40
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//50
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//60
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//70
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

	//80
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//90
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//a0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//b0
		DLParser_Nothing,	DLParser_GBI0_Tri2,			DLParser_GBI1_RDPHalf_Cont,		DLParser_GBI1_RDPHalf_2,
		DLParser_RDPHalf1_GoldenEye, DLParser_GBI1_Line3D,		DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
		DLParser_GBI1_EndDL,		DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH,		DLParser_GBI1_Texture,
		DLParser_GBI1_MoveWord,  DLParser_GBI1_PopMtx,		DLParser_GBI1_CullDL,			DLParser_GBI1_Tri1,

	//c0
		DLParser_GBI1_Noop,   DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,  DLParser_Nothing,
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
	//d0
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
		DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//e0
		DLParser_Nothing,	  DLParser_Nothing,		DLParser_Nothing,	   DLParser_Nothing,
		DLParser_TexRect,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
	//f0
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
	},
			
};


	        // uCode 2 - F3DEX 2.XX
        // games: Zelda 64
       /* {
		// 00-3f
		DLParser_GBI2_SpNoop,                                 DLParser_GBI2_Vtx,                             DLParser_GBI1_ModifyVtx,                  DLParser_GBI2_CullDL,
		DLParser_GBI1_BranchZ,                   DLParser_GBI2_Tri1,                               DLParser_GBI2_Quad,                           DLParser_GBI2_Quad,
		DLParser_GBI2_Line3D,                             DLParser_S2DEX_Bg1cyc,                    DLParser_S2DEX_BgCopy,                    DLParser_S2DEX_ObjRendermode,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,
		DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,
		DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,
		DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,                               DLParser_GBI2_Tri2,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,

		// 40-7f: unused
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,

		// 80-bf: unused
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,
		DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,                                  DLParser_Nothing,

		// c0-ff: RDP commands mixed with uc2 commands
		DLParser_GBI2_Noop,               DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
		DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,    
		DLParser_TriFill,	 DLParser_TriFillZ,	  DLParser_TriTxtr,	    DLParser_TriTxtrZ,
		DLParser_TriShade,	 DLParser_TriShadeZ,	  DLParser_TriShadeTxtr, DLParser_TriShadeTxtrZ,
		DLParser_Nothing,                  DLParser_Nothing,                  DLParser_Nothing,                  DLParser_GBI2_Special3,
		DLParser_GBI2_Special2,           uc2_dlist_cnt,          DLParser_GBI2_DMA_IO,             DLParser_GBI2_Texture,
		DLParser_GBI2_PopMtx,         DLParser_GBI2_GeometryMode,          DLParser_GBI2_Mtx,             DLParser_GBI2_MoveWord,
		DLParser_GBI2_MoveMem,            DLParser_GBI1_LoadUCode,         DLParser_GBI2_DL,        DLParser_GBI2_EndDL,
		DLParser_GBI2_SpNoop,                 DLParser_GBI1_RDPHalf_1,          DLParser_GBI2_SetOtherModeL, DLParser_GBI2_SetOtherModeH,
		DLParser_TexRect,	  DLParser_TexRectFlip, DLParser_RDPLoadSync,  DLParser_RDPPipeSync,
		DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB,	   DLParser_SetKeyR,
		DLParser_SetConvert,  DLParser_SetScissor,  DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
		DLParser_LoadTLut,	  DLParser_Nothing,		  DLParser_SetTileSize,	 DLParser_LoadBlock, 
		DLParser_LoadTile,	  DLParser_SetTile,		  DLParser_FillRect,	 DLParser_SetFillColor,
		DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
		DLParser_SetCombine,  DLParser_SetTImg,		  DLParser_SetZImg,		 DLParser_SetCImg
        },*/


