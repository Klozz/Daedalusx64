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

#include "DLParser.h"
#include "PSPRenderer.h"
#include "PixelFormatN64.h"
#include "SysPSP/Graphics/PixelFormatPSP.h"
#include "RDP.h"
#include "RDPStateManager.h"
#include "DebugDisplayList.h"
#include "TextureCache.h"
#include "ConvertImage.h"			// Convert555ToRGBA
#include "Microcode.h"

#include "Utility/Profiler.h"
#include "Utility/IO.h"

#include "Graphics/GraphicsContext.h"

#include "Debug/Dump.h"
#include "Debug/DBGConsole.h"

#include "Core/Memory.h"
#include "Core/ROM.h"
#include "Core/CPU.h"

#include "OSHLE/ultra_sptask.h"
#include "OSHLE/ultra_gbi.h"
#include "OSHLE/ultra_rcp.h"

#include "Test/BatchTest.h"

#include "ConfigOptions.h"

#include <vector>


const u32	MAX_RAM_ADDRESS = (8*1024*1024);

const char *	gDisplayListRootPath = "DisplayLists";
const char *	gDisplayListDumpPathFormat = "dl%04d.txt";

#define N64COL_GETR( col )		(u8((col) >> 24))
#define N64COL_GETG( col )		(u8((col) >> 16))
#define N64COL_GETB( col )		(u8((col) >>  8))
#define N64COL_GETA( col )		(u8((col)      ))

#define N64COL_GETR_F( col )	(N64COL_GETR(col) / 255.0f)
#define N64COL_GETG_F( col )	(N64COL_GETG(col) / 255.0f)
#define N64COL_GETB_F( col )	(N64COL_GETB(col) / 255.0f)
#define N64COL_GETA_F( col )	(N64COL_GETA(col) / 255.0f)



void	RDP_GFX_Force_Matrix(u32 address);

void	MatrixFromN64FixedPoint( Matrix4x4 & mat, u32 address )
{
	const f32 fRecip = 1.0f / 65536.0f;

	const u8 *	base( g_pu8RamBase );

	if (address + 64 <= MAX_RAM_ADDRESS)
	{
		for (u32 i = 0; i < 4; i++)
		{
			for (u32 j = 0; j < 4; j++)
			{
				s16 hi = *(s16 *)(base + ((address+(i<<3)+(j<<1)     )^0x2));
				u16 lo = *(u16 *)(base + ((address+(i<<3)+(j<<1) + 32)^0x2));

				mat.m[i][j] = ((hi<<16) | lo ) * fRecip;
			}
		}
	}
	//else
	//{
	//	DBGConsole_Msg(0, "Mtx: Address invalid (0x%08x)", address);
	//}
}

//*************************************************************************************
// Macros
//*************************************************************************************
#ifdef DAEDALUS_GFX_PLUGIN

inline void FinishRDPJob()
{
	*g_GraphicsInfo.xMI_INTR_REG |= MI_INTR_DP;
	g_GraphicsInfo.CheckInterrupts();
}

#else

inline void 	FinishRDPJob()
{
	Memory_MI_SetRegisterBits(MI_INTR_REG, MI_INTR_DP);
	gCPUState.AddJob(CPU_CHECK_INTERRUPTS);
}

#endif


#ifdef DAEDALUS_DEBUG_DISPLAYLIST
void DLParser_DumpVtxInfo(u32 address, u32 v0_idx, u32 num_verts);
//void DLParser_DumpVtxInfoDKR(u32 address, u32 v0_idx, u32 num_verts);

u32 gNumDListsCulled;
u32 gNumVertices;
#endif

u32 gRDPFrame = 0;

//*****************************************************************************
//
//*****************************************************************************
u32 gOtherModeL   = 0;
u32 gOtherModeH   = 0;
u32 gRDPHalf1 = 0;

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                    uCode Config                      //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

// This is the multiplier applied to vertex indices. 
// For GBI0, it is 10.
// For GBI1/2 it is 2.
u32 gVertexStride = 10;

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                      Dumping                         //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
static bool gDumpNextDisplayList = false;

FILE * gDisplayListFile = NULL;

#endif

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                     GFX State                        //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

struct N64Light
{
	u32 Colour;
	u32	ColourCopy;
	f32 x,y,z;			// Direction
	u32 pad;
};


u32	gSegments[16];
static N64Light  g_N64Lights[8];
SImageDescriptor g_TI = { G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, 0 };
SImageDescriptor g_CI = { G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, 0 };
SImageDescriptor g_DI = { G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, 0 };








//u32		gPalAddresses[ 4096 ];


// The display list PC stack. Before this was an array of 10
// items, but this way we can nest as deeply as necessary. 

struct DList
{
	u32 addr;
	u32 limit;
	// Push/pop?
};

std::vector< DList > gDisplayListStack;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
static u32					gCurrentInstructionCount = 0;			// Used for debugging display lists
static u32					gTotalInstructionCount = 0;
static u32					gInstructionCountLimit = UNLIMITED_INSTRUCTION_COUNT;
#endif

u32 gAmbientLightIdx = 0;

u32 gTextureTile = 0;
u32 gTextureLevel = 0;

static u32 gFillColor		= 0xFFFFFFFF;

static bool gFirstCall = true;				// Used to keep track of when we're processing the first display list


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                      Strings                         //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
namespace
{
const char *gFormatNames[8] = {"RGBA", "YUV", "CI", "IA", "I", "?1", "?2", "?3"};
const char *gSizeNames[4]   = {"4bpp", "8bpp", "16bpp", "32bpp"};
const char *gOnOffNames[2]  = {"Off", "On"};

const char *gTLUTTypeName[4] = {"None", "?", "RGBA16", "IA16"};

const char * sc_colcombtypes32[32] =
{
	"Combined    ", "Texel0      ",
	"Texel1      ", "Primitive   ", 
	"Shade       ", "Env         ",
	"1           ", "CombAlp     ",
	"Texel0_Alp  ", "Texel1_Alp  ",
	"Prim_Alpha  ", "Shade_Alpha ",
	"Env_Alpha   ", "LOD_Frac    ",
	"PrimLODFrac ", "K5          ",
	"?           ", "?           ",
	"?           ", "?           ",
	"?           ", "?           ",
	"?           ", "?           ",
	"?           ", "?           ",
	"?           ", "?           ",
	"?           ", "?           ",
	"?           ",	"0           "
};
const char *sc_colcombtypes16[16] =
{
	"Combined    ", "Texel0      ",
	"Texel1      ", "Primitive   ", 
	"Shade       ", "Env         ",
	"1           ", "CombAlp     ",
	"Texel0_Alp  ", "Texel1_Alp  ",
	"Prim_Alp    ", "Shade_Alpha ",
	"Env_Alpha   ", "LOD_Frac    ",
	"PrimLOD_Frac", "0           "
};
const char *sc_colcombtypes8[8] =
{
	"Combined    ", "Texel0      ",
	"Texel1      ", "Primitive   ", 
	"Shade       ", "Env         ",
	"1           ", "0           ",
};
}
#endif

// Mask down to 0x003FFFFF?
#define RDPSegAddr(seg) ( (gSegments[((seg)>>24)&0x0F]&0x00ffffff) + ((seg)&0x00FFFFFF) )

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

// static void DLParser_GBI2_RDPHalf_1( MicroCodeCommand command );
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


//*****************************************************************************
//
//*****************************************************************************
const char * gInstructionName[256];
MicroCodeInstruction gInstructionLookup[256];

//DKR: 00229BA8: 05710080 001E4AF0 CMD G_DMATRI  Triangles 9 at 801E4AF0

//*****************************************************************************
//
//*****************************************************************************
bool DLParser_Initialise()
{
	gFirstCall = true;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	gDumpNextDisplayList = false;
	gDisplayListFile = NULL;
#endif

	//
	// Reset all the RDP registers
	//
	gRDPOtherMode._u64 = 0;
	gRDPOtherMode.pad = G_RDP_RDPSETOTHERMODE;
	gRDPOtherMode.blender = 0x0050;

	return true;
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_Finalise()
{
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                      Logging                         //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

//*****************************************************************************
// Identical to the above, but uses DL_PF
//*****************************************************************************
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
static void DLParser_DumpMux( u64 mux )
{
	u32 mux0 = (u32)(mux>>32);
	u32 mux1 = (u32)(mux);
	
	u32 aRGB0  = (mux0>>20)&0x0F;	// c1 c1		// a0
	u32 bRGB0  = (mux1>>28)&0x0F;	// c1 c2		// b0
	u32 cRGB0  = (mux0>>15)&0x1F;	// c1 c3		// c0
	u32 dRGB0  = (mux1>>15)&0x07;	// c1 c4		// d0

	u32 aA0    = (mux0>>12)&0x07;	// c1 a1		// Aa0
	u32 bA0    = (mux1>>12)&0x07;	// c1 a2		// Ab0
	u32 cA0    = (mux0>>9 )&0x07;	// c1 a3		// Ac0
	u32 dA0    = (mux1>>9 )&0x07;	// c1 a4		// Ad0

	u32 aRGB1  = (mux0>>5 )&0x0F;	// c2 c1		// a1
	u32 bRGB1  = (mux1>>24)&0x0F;	// c2 c2		// b1
	u32 cRGB1  = (mux0    )&0x1F;	// c2 c3		// c1
	u32 dRGB1  = (mux1>>6 )&0x07;	// c2 c4		// d1
	
	u32 aA1    = (mux1>>21)&0x07;	// c2 a1		// Aa1
	u32 bA1    = (mux1>>3 )&0x07;	// c2 a2		// Ab1
	u32 cA1    = (mux1>>18)&0x07;	// c2 a3		// Ac1
	u32 dA1    = (mux1    )&0x07;	// c2 a4		// Ad1

	DL_PF("    Mux: 0x%08x%08x", mux0, mux1);

	DL_PF("    RGB0: (%s - %s) * %s + %s", sc_colcombtypes16[aRGB0], sc_colcombtypes16[bRGB0], sc_colcombtypes32[cRGB0], sc_colcombtypes8[dRGB0]);		
	DL_PF("    A0  : (%s - %s) * %s + %s", sc_colcombtypes8[aA0], sc_colcombtypes8[bA0], sc_colcombtypes8[cA0], sc_colcombtypes8[dA0]);
	DL_PF("    RGB1: (%s - %s) * %s + %s", sc_colcombtypes16[aRGB1], sc_colcombtypes16[bRGB1], sc_colcombtypes32[cRGB1], sc_colcombtypes8[dRGB1]);		
	DL_PF("    A1  : (%s - %s) * %s + %s", sc_colcombtypes8[aA1],  sc_colcombtypes8[bA1], sc_colcombtypes8[cA1],  sc_colcombtypes8[dA1]);
}
#endif


//*****************************************************************************
//
//*****************************************************************************
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
static void	DLParser_DumpTaskInfo( const OSTask * pTask )
{
	DL_PF( "Task:         %08x",      pTask->t.type  );
	DL_PF( "Flags:        %08x",      pTask->t.flags  );
	DL_PF( "BootCode:     %08x", (u32)pTask->t.ucode_boot  );
	DL_PF( "BootCodeSize: %08x",      pTask->t.ucode_boot_size  );

	DL_PF( "uCode:        %08x", (u32)pTask->t.ucode );
	DL_PF( "uCodeSize:    %08x",      pTask->t.ucode_size );
	DL_PF( "uCodeData:    %08x", (u32)pTask->t.ucode_data );
	DL_PF( "uCodeDataSize:%08x",      pTask->t.ucode_data_size );

	DL_PF( "Stack:        %08x", (u32)pTask->t.dram_stack );
	DL_PF( "StackS:       %08x",      pTask->t.dram_stack_size );
	DL_PF( "Output:       %08x", (u32)pTask->t.output_buff );
	DL_PF( "OutputS:      %08x", (u32)pTask->t.output_buff_size );

	DL_PF( "Data( PC ):   %08x", (u32)pTask->t.data_ptr );
	DL_PF( "DataSize:     %08x",      pTask->t.data_size );
	DL_PF( "YieldData:    %08x", (u32)pTask->t.yield_data_ptr );
	DL_PF( "YieldDataSize:%08x",      pTask->t.yield_data_size );
}
#endif


static GBIVersion gGBIVersion = GBI_2;

//*****************************************************************************
//
//*****************************************************************************
static void DLParser_SetuCode( GBIVersion gbi_version, UCodeVersion ucode_version )
{
	//DBGConsole_Msg(0, "[GReconfiguring RDP to process ucode %d]", ucode);

	for ( u32 i = 0; i < 256; i++ )
	{
		gInstructionLookup[ i ] = DLParser_Nothing;
		gInstructionName[ i ] = "???";
	}
	
#define SetCommand( cmd, func )					\
		gInstructionLookup[ cmd ] = func;		\
		gInstructionName[ cmd ] = #cmd;
		
	//SetCommand( 0x08			, DLParser_GBI1_SpNoop );
	//SetCommand( 0x09			, DLParser_GBI1_SpNoop );
	//SetCommand( 0x0b			, DLParser_GBI1_SpNoop );
	//SetCommand( 0x0a			, DLParser_GBI1_SpNoop );

	SetCommand( G_RDP_SETCIMG			, DLParser_SetCImg );
	SetCommand( G_RDP_SETZIMG			, DLParser_SetZImg );
	SetCommand( G_RDP_SETTIMG			, DLParser_SetTImg );
	SetCommand( G_RDP_SETCOMBINE		, DLParser_SetCombine );
	SetCommand( G_RDP_SETENVCOLOR		, DLParser_SetEnvColor );
	SetCommand( G_RDP_SETPRIMCOLOR		, DLParser_SetPrimColor );
	SetCommand( G_RDP_SETBLENDCOLOR		, DLParser_SetBlendColor );
	SetCommand( G_RDP_SETFOGCOLOR		, DLParser_SetFogColor );
	SetCommand( G_RDP_SETFILLCOLOR		, DLParser_SetFillColor );
	SetCommand( G_RDP_FILLRECT			, DLParser_FillRect );
	SetCommand( G_RDP_SETTILE			, DLParser_SetTile );
	SetCommand( G_RDP_LOADTILE			, DLParser_LoadTile );
	SetCommand( G_RDP_LOADBLOCK			, DLParser_LoadBlock );
	SetCommand( G_RDP_SETTILESIZE		, DLParser_SetTileSize );
	SetCommand( G_RDP_LOADTLUT			, DLParser_LoadTLut );
	SetCommand( G_RDP_RDPSETOTHERMODE	, DLParser_RDPSetOtherMode );
	SetCommand( G_RDP_SETPRIMDEPTH		, DLParser_SetPrimDepth );
	SetCommand( G_RDP_SETSCISSOR		, DLParser_SetScissor );
	SetCommand( G_RDP_SETCONVERT		, DLParser_SetConvert );
	SetCommand( G_RDP_SETKEYR			, DLParser_SetKeyR );
	SetCommand( G_RDP_SETKEYGB			, DLParser_SetKeyGB );
	SetCommand( G_RDP_RDPFULLSYNC		, DLParser_RDPFullSync );
	SetCommand( G_RDP_RDPTILESYNC		, DLParser_RDPTileSync );
	SetCommand( G_RDP_RDPPIPESYNC		, DLParser_RDPPipeSync );
	SetCommand( G_RDP_RDPLOADSYNC		, DLParser_RDPLoadSync );
	SetCommand( G_RDP_TEXRECTFLIP		, DLParser_TexRectFlip );
	SetCommand( G_RDP_TEXRECT			, DLParser_TexRect );

	if ( gbi_version == GBI_0 ||
		 gbi_version == GBI_0_WR ||
		 gbi_version == GBI_0_GE ||
		 gbi_version == GBI_0_DKR ||
		 gbi_version == GBI_0_JFG ||
		 gbi_version == GBI_0_LL ||
		 gbi_version == GBI_0_SE ||
		 gbi_version == GBI_1 )
	{

		MicroCodeInstruction	DLParser_GBI1_Tri1_Instruction;
		MicroCodeInstruction	DLParser_GBI1_Tri2_Instruction;
		MicroCodeInstruction	DLParser_GBI1_Line3D_Instruction;
		switch( gbi_version )
		{
		case GBI_0:
		case GBI_0_GE:
			gVertexStride = 10;
			DLParser_GBI1_Tri1_Instruction = DLParser_GBI1_Tri1_T< 10 >;
			DLParser_GBI1_Tri2_Instruction = DLParser_GBI1_Tri2_T< 10 >;
			DLParser_GBI1_Line3D_Instruction = DLParser_GBI1_Line3D_T< 10 >;
			break;
		case GBI_0_DKR:	
			gVertexStride = 10;
			DLParser_GBI1_Tri1_Instruction = DLParser_GBI1_Tri1_T< 10 >;
			DLParser_GBI1_Tri2_Instruction = DLParser_GBI1_Tri2_T< 10 >;
			DLParser_GBI1_Line3D_Instruction = DLParser_GBI1_Line3D_T< 10 >;
			break;
		case GBI_0_WR:
			gVertexStride = 5;	
			DLParser_GBI1_Tri1_Instruction = DLParser_GBI1_Tri1_T< 5 >;
			DLParser_GBI1_Tri2_Instruction = DLParser_GBI1_Tri2_T< 5 >;
			DLParser_GBI1_Line3D_Instruction = DLParser_GBI1_Line3D_T< 5 >;
		case GBI_0_SE:
			gVertexStride = 5; // Don't set higher than this
			DLParser_GBI1_Tri1_Instruction = DLParser_GBI1_Tri1_T< 5 >;
			DLParser_GBI1_Tri2_Instruction = DLParser_GBI1_Tri2_T< 5 >;
			DLParser_GBI1_Line3D_Instruction = DLParser_GBI1_Line3D_T< 5 >;
			break;

			// Default case is GBI_1
		case GBI_1:
		default:
			gVertexStride = 2;	
			DLParser_GBI1_Tri1_Instruction = DLParser_GBI1_Tri1_T< 2 >;
			DLParser_GBI1_Tri2_Instruction = DLParser_GBI1_Tri2_T< 2 >;
			DLParser_GBI1_Line3D_Instruction = DLParser_GBI1_Line3D_T< 2 >;
			break;
		}

		// DMA commands
		SetCommand( G_GBI1_SPNOOP        , DLParser_GBI1_SpNoop );
		SetCommand( G_GBI1_MTX	         , DLParser_GBI1_Mtx );
		SetCommand( G_GBI1_RESERVED0     , DLParser_GBI1_Reserved0 );
		SetCommand( G_GBI1_MOVEMEM	     , DLParser_GBI1_MoveMem );
		SetCommand( G_GBI1_VTX	         , DLParser_GBI1_Vtx );
		SetCommand( G_GBI1_RESERVED1	 , DLParser_GBI1_Reserved1 );
		SetCommand( G_GBI1_DL			 , DLParser_GBI1_DL );
		SetCommand( G_GBI1_RESERVED2	 , DLParser_GBI1_Reserved2 );
		SetCommand( G_GBI1_RESERVED3	 , DLParser_GBI1_Reserved3 );
		SetCommand( G_GBI1_SPRITE2D_BASE , DLParser_GBI1_Sprite2DBase );

		// IMMEDIATE commands
		SetCommand( G_GBI1_NOOP          , DLParser_GBI1_Noop );
		
		SetCommand( G_GBI1_TRI1					, DLParser_GBI1_Tri1_Instruction );
		SetCommand( G_GBI1_CULLDL				, DLParser_GBI1_CullDL );
		SetCommand( G_GBI1_POPMTX				, DLParser_GBI1_PopMtx );
		SetCommand( G_GBI1_MOVEWORD				, DLParser_GBI1_MoveWord );
		SetCommand( G_GBI1_TEXTURE				, DLParser_GBI1_Texture );
		SetCommand( G_GBI1_SETOTHERMODE_H		, DLParser_GBI1_SetOtherModeH );
		SetCommand( G_GBI1_SETOTHERMODE_L		, DLParser_GBI1_SetOtherModeL );
		SetCommand( G_GBI1_ENDDL				, DLParser_GBI1_EndDL );
		SetCommand( G_GBI1_SETGEOMETRYMODE		, DLParser_GBI1_SetGeometryMode );
		SetCommand( G_GBI1_CLEARGEOMETRYMODE	, DLParser_GBI1_ClearGeometryMode );
		SetCommand( G_GBI1_LINE3D				, DLParser_GBI1_Line3D_Instruction );
		SetCommand( G_GBI1_RDPHALF_1			, DLParser_GBI1_RDPHalf_1 );
		SetCommand( G_GBI1_RDPHALF_2			, DLParser_GBI1_RDPHalf_2 );

		if ( ucode_version == F3DEX ||
			 ucode_version == F3DLP ||
			 ucode_version == F3DLX )
		{
			SetCommand( G_GBI1_MODIFYVTX			, DLParser_GBI1_ModifyVtx );
			SetCommand( G_GBI1_TRI2					, DLParser_GBI1_Tri2_Instruction );
			SetCommand( G_GBI1_BRANCH_Z				, DLParser_GBI1_BranchZ );
		}
		else
		{
			SetCommand( G_GBI1_RDPHALF_CONT			, DLParser_GBI1_RDPHalf_Cont );
		}

		if ( ucode_version == F3DEX ||
			 ucode_version == F3DLP ||
			 ucode_version == F3DLX || // Airboarder 64 needs Load_Ucode.
			 ucode_version == S2DEX )
		{
			SetCommand( G_GBI1_LOAD_UCODE			, DLParser_GBI1_LoadUCode );
		}
		if ( ucode_version == S2DEX )
		{
			SetCommand( G_GBI1_OBJ_RECTANGLE_R		, DLParser_S2DEX_ObjRectangleR );
			SetCommand( G_GBI1_OBJ_MOVEMEM			, DLParser_S2DEX_ObjMoveMem );
			SetCommand( G_GBI1_RDPHALF_0			, DLParser_S2DEX_RDPHalf_0 );
			SetCommand( G_GBI1_OBJ_RECTANGLE	 	, DLParser_S2DEX_ObjRectangle );
			SetCommand( G_GBI1_OBJ_SPRITE			, DLParser_S2DEX_ObjSprite );
			SetCommand( G_GBI1_SELECT_DL			, DLParser_S2DEX_SelectDl );
			SetCommand( G_GBI1_OBJ_LOADTXTR			, DLParser_S2DEX_ObjLoadTxtr );
			SetCommand( G_GBI1_OBJ_LDTX_SPRITE		, DLParser_S2DEX_ObjLdtxSprite );
			SetCommand( G_GBI1_OBJ_LDTX_RECT		, DLParser_S2DEX_ObjLdtxRect );
			SetCommand( G_GBI1_OBJ_LDTX_RECT_R		, DLParser_S2DEX_ObjLdtxRectR );
			SetCommand( G_GBI1_BG_1CYC				, DLParser_S2DEX_Bg1cyc );
			SetCommand( G_GBI1_BG_COPY				, DLParser_S2DEX_BgCopy ); 
			SetCommand( G_GBI1_OBJ_RENDERMODE		, DLParser_S2DEX_ObjRendermode ); 
		}
		
		if ( gbi_version == GBI_0 )
		{
			gInstructionLookup[G_GBI1_VTX] = DLParser_GBI0_Vtx;			gInstructionName[G_GBI1_VTX]		= "G_GBI0_VTX";
			gInstructionLookup[G_GBI1_TRI2] = DLParser_GBI0_Tri2;		gInstructionName[G_GBI1_TRI2]		= "G_GBI0_TRI2";
		}
		else if ( gbi_version == GBI_0_WR )
		{
			gInstructionLookup[G_GBI1_VTX] = DLParser_GBI0_Vtx_WRUS;	gInstructionName[G_GBI1_VTX]		= "G_GBI0_VTX_WRUS";
		} 
		else if (gbi_version == GBI_0_DKR || gbi_version == GBI_0_JFG)
		//if (gbi_version equals a certain number, OR gbi_version is non-zero) 
		//{....we need if (gbi_version equalt this number OR gbi_version eqauls that number)
		//therefor, we need to repeat the gbi_verion ==
		{
			// DKR
			SetCommand( G_DMATRI					, DLParser_DmaTri );
			SetCommand( G_DLINMEM					, DLParser_DLInMem );
			//LoadedUcodeMap[0xbf]=DLParser_Set_Addr_Ucode6;
			gInstructionLookup[G_GBI1_MOVEWORD] = DLParser_MoveWord_DKR;		gInstructionName[G_GBI1_MOVEWORD]		= "G_MoveWord_DKR";

			gInstructionLookup[G_GBI1_TRI2] = DLParser_GBI0_Tri2;		gInstructionName[G_GBI1_TRI2]		= "G_GBI0_TRI2";

			if( gbi_version == GBI_0_JFG )	
			{
				gInstructionLookup[G_GBI1_VTX] = DLParser_GBI0_Vtx_Gemini;		gInstructionName[G_GBI1_VTX]		= "G_GBI0_Vtx_Gemini";
			}
			else
			{
				gInstructionLookup[G_GBI1_MTX] = DLParser_MtxDKR;		gInstructionName[G_GBI1_MTX]		= "G_MtxDKR";
				gInstructionLookup[G_GBI1_VTX] = DLParser_GBI0_Vtx_DKR;		gInstructionName[G_GBI1_VTX]		= "G_GBI0_VTX_DKR";

			}

		}
		else if ( gbi_version == GBI_0_GE )
		{
			gInstructionLookup[G_GBI1_VTX] = DLParser_GBI0_Vtx;			gInstructionName[G_GBI1_VTX]		= "G_GBI0_VTX";
			gInstructionLookup[G_GBI1_TRI2] = DLParser_GBI0_Tri2;		gInstructionName[G_GBI1_TRI2]		= "G_GBI0_TRI2";
//			gInstructionLookup[G_GBI1_RDPHALF_1] = DLParser_RDPHalf_1_0xb4_GoldenEye;	gInstructionName[G_GBI1_RDPHALF_1]		= "G_RDPHalf_1_0xb4_GoldenEye";
		}
		else if ( gbi_version == GBI_0_SE )
		{
                       gInstructionLookup[G_GBI1_VTX] = DLParser_GBI0_Vtx_ShadowOfEmpire;                      gInstructionName[G_GBI1_VTX]            = "G_GBI0_Vtx_ShadowOfEmpire";
					   //gInstructionLookup[G_GBI1_TRI1] = DLParser_GBI0_Tri1_ShadowOfEmpire;                      gInstructionName[G_GBI1_TRI1]            = "G_GBI0_Tri1_ShadowOfEmpire";
					   //gInstructionLookup[G_GBI1_LINE3D] = DLParser_GBI0_Quad_ShadowOfEmpire;                      gInstructionName[G_GBI1_LINE3D]            = "G_GBI0_Quad_ShadowOfEmpire";
                }
                else if ( gbi_version == GBI_0_LL )
                {
                        SetCommand(0x80							, DLParser_RSP_Last_Legion_0x80);
                        SetCommand(0x00							, DLParser_RSP_Last_Legion_0x00);

                        gInstructionLookup[G_RDP_TEXRECT] =  DLParser_TexRect_Last_Legion;                            gInstructionName[G_RDP_TEXRECT]         = "G_TexRect_Last_Legion";
                }

	}
	else
	{
		SetCommand( G_GBI2_RDPHALF_2		, DLParser_GBI2_RDPHalf_2 );
		SetCommand( G_GBI2_SETOTHERMODE_H	, DLParser_GBI2_SetOtherModeH );
		SetCommand( G_GBI2_SETOTHERMODE_L	, DLParser_GBI2_SetOtherModeL );
		SetCommand( G_GBI2_RDPHALF_1		, DLParser_GBI1_RDPHalf_1 );
		SetCommand( G_GBI2_SPNOOP			, DLParser_GBI2_SpNoop );
		SetCommand( G_GBI2_ENDDL			, DLParser_GBI2_EndDL );
		SetCommand( G_GBI2_DL				, DLParser_GBI2_DL );
		SetCommand( G_GBI2_LOAD_UCODE		, DLParser_GBI1_LoadUCode );
		SetCommand( G_GBI2_MOVEMEM			, DLParser_GBI2_MoveMem );
		SetCommand( G_GBI2_MOVEWORD			, DLParser_GBI2_MoveWord );
		SetCommand( G_GBI2_MTX				, DLParser_GBI2_Mtx );
		SetCommand( G_GBI2_GEOMETRYMODE		, DLParser_GBI2_GeometryMode );
		SetCommand( G_GBI2_POPMTX			, DLParser_GBI2_PopMtx );
		SetCommand( G_GBI2_TEXTURE			, DLParser_GBI2_Texture );
		SetCommand( G_GBI2_DMA_IO			, DLParser_GBI2_DMA_IO );
		SetCommand( G_GBI2_SPECIAL_1		, DLParser_GBI2_Special1 );
		SetCommand( G_GBI2_SPECIAL_2		, DLParser_GBI2_Special2 );
		SetCommand( G_GBI2_SPECIAL_3		, DLParser_GBI2_Special3 );

		SetCommand( G_GBI2_NOOP				, DLParser_GBI2_Noop );
		SetCommand( G_GBI2_VTX				, DLParser_GBI2_Vtx );
		SetCommand( G_GBI2_MODIFYVTX		, DLParser_GBI1_ModifyVtx ); // Share the same function.
		SetCommand( G_GBI2_CULLDL			, DLParser_GBI2_CullDL );
		SetCommand( G_GBI2_BRANCH_Z			, DLParser_GBI1_BranchZ );
		SetCommand( G_GBI2_TRI1				, DLParser_GBI2_Tri1 );
		SetCommand( G_GBI2_TRI2				, DLParser_GBI2_Tri2 );
		SetCommand( G_GBI2_LINE3D			, DLParser_GBI2_Line3D );

		if( ucode_version == S2DEX )
		{
			SetCommand( G_GBI2_OBJ_RECTANGLE_R		, DLParser_S2DEX_ObjRectangleR );
			SetCommand( G_GBI2_OBJ_MOVEMEM			, DLParser_S2DEX_ObjMoveMem );
			SetCommand( G_GBI2_RDPHALF_0			, DLParser_S2DEX_RDPHalf_0 );
			SetCommand( G_GBI2_OBJ_RECTANGLE	 	, DLParser_S2DEX_ObjRectangle );
			SetCommand( G_GBI2_OBJ_SPRITE			, DLParser_S2DEX_ObjSprite );
			SetCommand( G_GBI2_SELECT_DL			, DLParser_S2DEX_SelectDl );
			SetCommand( G_GBI2_OBJ_LOADTXTR			, DLParser_S2DEX_ObjLoadTxtr );
			SetCommand( G_GBI2_OBJ_LDTX_SPRITE		, DLParser_S2DEX_ObjLdtxSprite );
			SetCommand( G_GBI2_OBJ_LDTX_RECT		, DLParser_S2DEX_ObjLdtxRect );
			SetCommand( G_GBI2_OBJ_LDTX_RECT_R		, DLParser_S2DEX_ObjLdtxRectR );
			SetCommand( G_GBI2_BG_1CYC				, DLParser_S2DEX_Bg1cyc );
			SetCommand( G_GBI2_BG_COPY				, DLParser_S2DEX_BgCopy ); 
			SetCommand( G_GBI2_OBJ_RENDERMODE		, DLParser_S2DEX_ObjRendermode ); 
		}

	}

	gGBIVersion = gbi_version;
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_PushDisplayList( const DList & dl )
{
	gDisplayListStack.push_back( dl );

	if ( gDisplayListStack.size() > 30 )
	{
		// Stack is too deep - probably an error
		DAEDALUS_DL_ERROR( "Stack is over 30 - something is probably wrong\n" );
	}

	DL_PF("    Pushing DisplayList 0x%08x", dl.addr);
	DL_PF(" ");
	DL_PF("\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/");
	DL_PF("############################################");
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_CallDisplayList( const DList & dl )
{
	gDisplayListStack.back() = dl;

	DL_PF("    Jumping to DisplayList 0x%08x", dl.addr);
	DL_PF(" ");
	DL_PF("\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/");
	DL_PF("############################################");
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_PopDL()
{
	DL_PF("      Returning from DisplayList");
	DL_PF("############################################");
	DL_PF("/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\");
	DL_PF(" ");

	if( !gDisplayListStack.empty() )
	{
		gDisplayListStack.pop_back();
	}
}

//*****************************************************************************
// Reads the next command from the display list, updates the PC.
//*****************************************************************************
bool	DLParser_FetchNextCommand( MicroCodeCommand * p_command )
{
	if( gDisplayListStack.empty() )
		return false;

	// Current PC is the last value on the stack
	DList &		entry( gDisplayListStack.back() );
	u32			pc( entry.addr );

	if ( pc > MAX_RAM_ADDRESS )
	{
		DBGConsole_Msg(0, "Display list PC is out of range: 0x%08x", pc );
		return false;
	}

	p_command->cmd0 = g_pu32RamBase[(pc>>2)+0];
	p_command->cmd1 = g_pu32RamBase[(pc>>2)+1];

	entry.addr = pc + 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	DL_PF("[%05d] 0x%08x: %08x %08x %-10s", gCurrentInstructionCount, pc, p_command->cmd0, p_command->cmd1, gInstructionName[ p_command->cmd ]);
	gCurrentInstructionCount++;

	if( gInstructionCountLimit != UNLIMITED_INSTRUCTION_COUNT )
	{
		if( gCurrentInstructionCount >= gInstructionCountLimit )
		{
			return false;
		}
	}
#endif

	return true;
}

//*************************************************************************************
// 
//*************************************************************************************
static void HandleDumpDisplayList( OSTask * pTask )
{
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	if (gDumpNextDisplayList)
	{
		DBGConsole_Msg( 0, "Dumping display list" );
		static u32 count = 0;

		char szFilePath[MAX_PATH+1];
		char szFileName[MAX_PATH+1];
		char szDumpDir[MAX_PATH+1];

		IO::Path::Combine(szDumpDir, g_ROM.settings.GameName.c_str(), gDisplayListRootPath);
	
		Dump_GetDumpDirectory(szFilePath, szDumpDir);

		sprintf(szFileName, "dl%04d.txt", count++);

		IO::Path::Append(szFilePath, szFileName);

		gDisplayListFile = fopen( szFilePath, "w" );
		if (gDisplayListFile != NULL)
			DBGConsole_Msg(0, "RDP: Dumping Display List as %s", szFilePath);
		else
			DBGConsole_Msg(0, "RDP: Couldn't create dumpfile %s", szFilePath);

		DLParser_DumpTaskInfo( pTask );

		// Clear flag as we're done
		gDumpNextDisplayList = false;
	}
#endif
}

//*****************************************************************************
// 
//*****************************************************************************
void	DLParser_InitMicrocode( u32 code_base, u32 code_size, u32 data_base, u32 data_size )
{
	GBIVersion gbi_version;
	UCodeVersion ucode_version;

	GBIMicrocode_DetectVersion( code_base, code_size, data_base, data_size, &gbi_version, &ucode_version );
	DLParser_SetuCode( gbi_version, ucode_version );
}


//*****************************************************************************
//
//*****************************************************************************
#ifdef DAEDALUS_ENABLE_PROFILING
SProfileItemHandle * gpProfileItemHandles[ 256 ];

#define PROFILE_DL_CMD( cmd )								\
	if(gpProfileItemHandles[ (cmd) ] == NULL)				\
	{														\
		gpProfileItemHandles[ (cmd) ] = new SProfileItemHandle( CProfiler::Get()->AddItem( gInstructionName[ (cmd) ] ) );		\
	}														\
	CAutoProfile		_auto_profile( *gpProfileItemHandles[ (cmd) ] )

#else 

#define PROFILE_DL_CMD( cmd )		do { } while(0)

#endif

//*****************************************************************************
//	Process the entire display list in one go
//*****************************************************************************
static void	DLParser_ProcessDList()
{
	MicroCodeCommand command;

	if( gCleanSceneEnabled && CGraphicsContext::CleanScene )
	{
		CGraphicsContext::Get()->Clear(true, false);
		CGraphicsContext::CleanScene = false;
	}

	while (DLParser_FetchNextCommand( &command ))
	{
		PROFILE_DL_CMD( command.cmd );

		gInstructionLookup[ command.cmd ]( command );

		// Check limit
		if (!gDisplayListStack.empty())
		{
			gDisplayListStack.back().limit--;
			if (gDisplayListStack.back().limit == u32(~0))
			{
				DL_PF("**EndDLInMem");
				gDisplayListStack.pop_back();
			}	
		}
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_Process()
{
	DAEDALUS_PROFILE( "DLParser_Process" );

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	gTotalInstructionCount = 0;
#endif

	if ( !CGraphicsContext::Get()->IsInitialised() || !PSPRenderer::IsAvailable() )
	{
		return;
	}

	// Shut down the debug console when we start rendering
	// TODO: Clear the front/backbuffer the first time this function is called
	// to remove any stuff lingering on the screen.
	if(gFirstCall)
	{
		CDebugConsole::Get()->EnableConsole( false );

		CGraphicsContext::Get()->ClearAllSurfaces();

		gFirstCall = false;
	}

	OSTask * pTask = (OSTask *)(g_pu8SpMemBase + 0x0FC0);
	u32 code_base = (u32)pTask->t.ucode & 0x1fffffff;
	u32 code_size = pTask->t.ucode_size;
	u32 data_base = (u32)pTask->t.ucode_data & 0x1fffffff;
	u32 data_size = pTask->t.ucode_data_size;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	GBIMicrocode_ResetMicrocodeHistory();
#endif
	DLParser_InitMicrocode( code_base, code_size, data_base, data_size );

	//
	// Not sure what to init this with. We should probably read it from the dmem
	//
	gRDPOtherMode._u64 = 0;
	gRDPOtherMode.pad = G_RDP_RDPSETOTHERMODE;
	gRDPOtherMode.blender = 0x0050;
	gRDPOtherMode.alpha_compare = 1;

	gOtherModeL = u32( gRDPOtherMode._u64 );
	gOtherModeH = u32( gRDPOtherMode._u64 >> 32 );

	gRDPFrame++;

	CTextureCache::Get()->PurgeOldTextures();

	// Initialise stack
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	gCurrentInstructionCount = 0;
	gNumDListsCulled = 0;
	gNumVertices = 0;
#endif
	gDisplayListStack.clear();
	DList dl;
	dl.addr = (u32)pTask->t.data_ptr;
	dl.limit = ~0;
	gDisplayListStack.push_back(dl);

	gRDPStateManager.Reset();

	//
	// Prepare to dump this displaylist, if necessary
	//
	HandleDumpDisplayList( pTask );

	DL_PF("DP: Firing up RDP!");

	extern bool gFrameskipActive;
	if(!gFrameskipActive)
	{
	// ZBuffer/BackBuffer clearing is caught elsewhere, but we could force it here
		PSPRenderer::Get()->Reset();
		PSPRenderer::Get()->BeginScene();
		DLParser_ProcessDList();
		PSPRenderer::Get()->EndScene();
	}

	// Do this regardless!
	FinishRDPJob();

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	if (gDisplayListFile != NULL)
	{
		fclose( gDisplayListFile );
		gDisplayListFile = NULL;
	}

	gTotalInstructionCount = gCurrentInstructionCount;
#endif

#ifdef DAEDALUS_BATCH_TEST_ENABLED
	CBatchTestEventHandler * handler( BatchTest_GetHandler() );
	if( handler )
		handler->OnDisplayListComplete();
#endif
}

//*****************************************************************************
//
//*****************************************************************************
void RDP_MoveMemLight(u32 light_idx, u32 address)
{
	if( light_idx >= 16 )
	{
		DBGConsole_Msg(0, "Warning: invalid light # = %d", light_idx);
		return;
	}

	s8 * pcBase = g_ps8RamBase + address;
	u32 * pdwBase = (u32 *)pcBase;

	g_N64Lights[light_idx].Colour     = pdwBase[0];
	g_N64Lights[light_idx].ColourCopy = pdwBase[1];
	g_N64Lights[light_idx].x			= f32(pcBase[8 ^ 0x3]);
	g_N64Lights[light_idx].y			= f32(pcBase[9 ^ 0x3]);
	g_N64Lights[light_idx].z			= f32(pcBase[10 ^ 0x3]);
					
	DL_PF("       RGBA: 0x%08x, RGBACopy: 0x%08x, x: %f, y: %f, z: %f", 
		g_N64Lights[light_idx].Colour,
		g_N64Lights[light_idx].ColourCopy,
		g_N64Lights[light_idx].x,
		g_N64Lights[light_idx].y,
		g_N64Lights[light_idx].z);

	if (light_idx == gAmbientLightIdx)
	{
		DL_PF("      (Ambient Light)");

		u32 n64col( g_N64Lights[light_idx].Colour );

		PSPRenderer::Get()->SetAmbientLight( v4( N64COL_GETR_F(n64col), N64COL_GETG_F(n64col), N64COL_GETB_F(n64col), 1.0f ) );
	}
	else
	{
		
		DL_PF("      (Normal Light)");

		PSPRenderer::Get()->SetLightCol(light_idx, g_N64Lights[light_idx].Colour);
		if (pdwBase[2] == 0)	// Direction is 0!
		{
			DL_PF("      Light is invalid");
		}
		else
		{
			PSPRenderer::Get()->SetLightDirection(light_idx, 
										  g_N64Lights[light_idx].x,
										  g_N64Lights[light_idx].y,
										  g_N64Lights[light_idx].z);
		}
	}
}

//*****************************************************************************
//
//*****************************************************************************
//0x000b46b0: dc080008 800b46a0 G_GBI2_MOVEMEM
//    Type: 08 Len: 08 Off: 0000
//        Scale: 640 480 511 0 = 160,120
//        Trans: 640 480 511 0 = 160,120
//vscale is the scale applied to the normalized homogeneous coordinates after 4x4 projection transformation 
//vtrans is the offset added to the scaled number

void RDP_MoveMemViewport(u32 address)
{
	if( address+16 >= MAX_RAM_ADDRESS )
	{
		DBGConsole_Msg(0, "MoveMem Viewport, invalid memory");
		return;
	}

	s16 scale[4];
	s16 trans[4];

	// address is offset into RD_RAM of 8 x 16bits of data...
	scale[0] = *(s16 *)(g_pu8RamBase + ((address+(0*2))^0x2));
	scale[1] = *(s16 *)(g_pu8RamBase + ((address+(1*2))^0x2));
	scale[2] = *(s16 *)(g_pu8RamBase + ((address+(2*2))^0x2));
	scale[3] = *(s16 *)(g_pu8RamBase + ((address+(3*2))^0x2));

	trans[0] = *(s16 *)(g_pu8RamBase + ((address+(4*2))^0x2));
	trans[1] = *(s16 *)(g_pu8RamBase + ((address+(5*2))^0x2));
	trans[2] = *(s16 *)(g_pu8RamBase + ((address+(6*2))^0x2));
	trans[3] = *(s16 *)(g_pu8RamBase + ((address+(7*2))^0x2));

	// With D3D we had to ensure that the vp coords are positive, so
	// we truncated them to 0. This happens a lot, as things
	// seem to specify the scale as the screen w/2 h/2

	v3 vec_scale( scale[0] / 4.0f, scale[1] / 4.0f, scale[2] / 4.0f );
	v3 vec_trans( trans[0] / 4.0f, trans[1] / 4.0f, trans[2] / 4.0f );

	PSPRenderer::Get()->SetN64Viewport( vec_scale, vec_trans );

	DL_PF("        Scale: %d %d %d %d", scale[0], scale[1], scale[2], scale[3]);
	DL_PF("        Trans: %d %d %d %d", trans[0], trans[1], trans[2], trans[3]);
}

//*****************************************************************************
//
//*****************************************************************************
#define RDP_NOIMPL_WARN(op)				DAEDALUS_DL_ERROR( op )
#define RDP_NOIMPL( op, cmd0, cmd1 )	DAEDALUS_DL_ERROR( "Not Implemented: %s 0x%08x 0x%08x", op, cmd0, cmd1 )

//*****************************************************************************
//
//*****************************************************************************


//Nintro64 uses Sprite2d 
void DLParser_Nothing( MicroCodeCommand command )
{
	DAEDALUS_DL_ERROR( "RDP Command %08x Does not exist...", command.cmd0 );

	// Terminate!
	{
	//	DBGConsole_Msg(0, "Warning, DL cut short with unknown command: 0x%08x 0x%08x", command.cmd0, command.cmd1);

		DLParser_PopDL();
	}

}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_SpNoop( MicroCodeCommand command )
{
}

//*****************************************************************************
//
//*****************************************************************************
#define DL_UNIMPLEMENTED_ERROR( msg )			\
{												\
	static bool shown = false;					\
	if (!shown )								\
	{											\
		DAEDALUS_DL_ERROR( "%s: %08x %08x", (msg), command.cmd0, command.cmd1 );				\
		shown = true;							\
	}											\
}

//*****************************************************************************
//
//*****************************************************************************

void DLParser_GBI2_SpNoop( MicroCodeCommand command )
{
	// Noop
}
/* Duplicated Command <== Remove me
void DLParser_GBI2_RDPHalf_1( MicroCodeCommand command )
{
	gRDPHalf1 = u32(command._u64 & 0xffffffff);
}
*/
void DLParser_GBI2_RDPHalf_2( MicroCodeCommand command )
{

}

void DLParser_GBI2_DMA_IO( MicroCodeCommand command )
{
	DL_PF( "~*Not Implemented (G_DMA_IO in GBI 2)" );
	DL_UNIMPLEMENTED_ERROR( "G_DMA_IO" );
}

void DLParser_GBI2_Special1( MicroCodeCommand command )
{
	DL_PF( "~*Not Implemented (G_SPECIAL_1 in GBI 2)" );
	DL_UNIMPLEMENTED_ERROR( "G_DMA_SPECIAL_1" );
}

void DLParser_GBI2_Special2( MicroCodeCommand command )
{
	DL_PF( "~*Not Implemented (G_SPECIAL_2 in GBI 2)" );
	DL_UNIMPLEMENTED_ERROR( "G_DMA_SPECIAL_2" );
}

void DLParser_GBI2_Special3( MicroCodeCommand command )
{
	DL_PF( "~*Not Implemented (G_SPECIAL_3 in GBI 2)" );
	DL_UNIMPLEMENTED_ERROR( "G_DMA_SPECIAL_3" );
}


void DLParser_GBI2_Noop( MicroCodeCommand command )
{
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Reserved0( MicroCodeCommand command )
{	
	DL_PF( "~*Not Implemented" );

	// Spiderman
	static bool warned = false;

	if (!warned)
	{
		RDP_NOIMPL("RDP: Reserved1 (0x%08x 0x%08x)", command.cmd0, command.cmd1);
		warned = true;
	}
}
//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Reserved1( MicroCodeCommand command )
{	
	DL_PF( "~*Not Implemented" );

	// Spiderman
	static bool warned = false;

	if (!warned)
	{
		RDP_NOIMPL("RDP: Reserved1 (0x%08x 0x%08x)", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Reserved2( MicroCodeCommand command )
{	
	DL_PF( "~*Not Implemented" );

	// Spiderman
	static bool warned = false;

	if (!warned)
	{
		RDP_NOIMPL("RDP: Reserved2 (0x%08x 0x%08x)", command.cmd0, command.cmd1);
		warned = true;
	}
}


//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Reserved3( MicroCodeCommand command )
{	
	DL_PF( "~*Not Implemented" );

	// Spiderman
	static bool warned = false;

	if (!warned)
	{
		RDP_NOIMPL("RDP: Reserved3 (0x%08x 0x%08x)", command.cmd0, command.cmd1);
		warned = true;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Noop( MicroCodeCommand command )
{
}


//*****************************************************************************
//
//*****************************************************************************
//u32 keyB,keyG,keyA,keyR;
//float fKeyA;
void DLParser_SetKeyGB( MicroCodeCommand command )
{

//keyB = ((command.cmd1)>>8)&0xFF;
//keyG = ((command.cmd1)>>24)&0xFF;
//keyA = (keyR + keyG + keyB)/3;
//fKeyA = keyA/255.0f;

}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetKeyR( MicroCodeCommand command )
{
//	keyR = ((command.cmd1)>>8)&0xFF;
//	keyA = (keyR+ keyG+keyB)/3;
//	fKeyA = keyA/255.0f;
}

//*****************************************************************************
//
//*****************************************************************************
//int g_convk0,g_convk1,g_convk2,g_convk3,g_convk4,g_convk5;
//float g_convc0,g_convc1,g_convc2,g_convc3,g_convc4,g_convc5;
void DLParser_SetConvert( MicroCodeCommand command )
{
//	int temp;

//	temp = ((command.cmd0)>>13)&0x1FF;
//	g_convk0 = temp>0xFF ? -(temp-0x100) : temp;

//	temp = ((command.cmd0)>>4)&0x1FF;
//	g_convk1 = temp>0xFF ? -(temp-0x100) : temp;

//	temp = (command.cmd0)&0xF;
//	temp = (temp<<5)|(((command.cmd0)>>27)&0x1F);
//	g_convk2 = temp>0xFF ? -(temp-0x100) : temp;

//	temp = ((command.cmd1)>>18)&0x1FF;
//	g_convk3 = temp>0xFF ? -(temp-0x100) : temp;

//	temp = ((command.cmd1)>>9)&0x1FF;
//	g_convk4 = temp>0xFF ? -(temp-0x100) : temp;

//	temp = (command.cmd1)&0x1FF;
//	g_convk5 = temp>0xFF ? -(temp-0x100) : temp;

//	g_convc0 = g_convk5/255.0f+1.0f;
//	g_convc1 = g_convk0/255.0f*g_convc0;
//	g_convc2 = g_convk1/255.0f*g_convc0;
//	g_convc3 = g_convk2/255.0f*g_convc0;
//	g_convc4 = g_convk3/255.0f*g_convc0;
}


//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetPrimDepth( MicroCodeCommand command )
{
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	u32 z  = (command.cmd1 >> 16) & 0xFFFF;
	u32 dz = (command.cmd1      ) & 0xFFFF;

	DL_PF("SetPrimDepth: 0x%08x 0x%08x - z: 0x%04x dz: 0x%04x", command.cmd0, command.cmd1, z, dz);
#endif	
	// Not implemented!
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_RDPSetOtherMode( MicroCodeCommand command )
{
	DL_PF( "      RDPSetOtherMode: 0x%08x 0x%08x", command.cmd0, command.cmd1 );

	gOtherModeH = command.cmd0;
	gOtherModeL = command.cmd1;

	RDP_SetOtherMode( command.cmd0, command.cmd1 );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_RDPHalf_Cont( MicroCodeCommand command )
{
	//DBGConsole_Msg( 0, "Unexpected RDPHalf_Cont: %08x %08x", command.cmd0, command.cmd1 );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_RDPHalf_2( MicroCodeCommand command )
{
//	DBGConsole_Msg( 0, "Unexpected RDPHalf_2: %08x %08x", command.cmd0, command.cmd1 );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_RDPHalf_1( MicroCodeCommand command )
{
	gRDPHalf1 = u32(command._u64 & 0xffffffff);
}


//*****************************************************************************
//
//*****************************************************************************
void DLParser_RDPLoadSync( MicroCodeCommand command )	{ /*DL_PF("LoadSync: (Ignored)");*/ }
void DLParser_RDPPipeSync( MicroCodeCommand command )	{ /*DL_PF("PipeSync: (Ignored)");*/ }
void DLParser_RDPTileSync( MicroCodeCommand command )	{ /*DL_PF("TileSync: (Ignored)");*/ }

//*****************************************************************************
//
//*****************************************************************************
void DLParser_RDPFullSync( MicroCodeCommand command )
{ 
	// We now do this regardless
	FinishRDPJob();

	/*DL_PF("FullSync: (Generating Interrupt)");*/
}




//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_MoveWord( MicroCodeCommand command )
{
	// Type of movement is in low 8bits of cmd0.

	u32 index = command.cmd0 & 0xFF;
	u32 offset = (command.cmd0 >> 8) & 0xFFFF;

	switch (index)
	{
	case G_MW_MATRIX:
		DL_PF("    G_MW_MATRIX");
		RDP_NOIMPL_WARN("GBI1: G_MW_MATRIX Not Implemented");

		//printf( "GBI1: MW_MATRIX: %08x %08x\n", command.cmd0, command.cmd1 );
		//switch( offset )
		//{
		//case G_MWO_MATRIX_XX_XY_I:
		//case G_MWO_MATRIX_XZ_XW_I:
		//case G_MWO_MATRIX_YX_YY_I:
		//case G_MWO_MATRIX_YZ_YW_I:
		//case G_MWO_MATRIX_ZX_ZY_I:
		//case G_MWO_MATRIX_ZZ_ZW_I:
		//case G_MWO_MATRIX_WX_WY_I:
		//case G_MWO_MATRIX_WZ_WW_I:
		//case G_MWO_MATRIX_XX_XY_F:
		//case G_MWO_MATRIX_XZ_XW_F:
		//case G_MWO_MATRIX_YX_YY_F:
		//case G_MWO_MATRIX_YZ_YW_F:
		//case G_MWO_MATRIX_ZX_ZY_F:
		//case G_MWO_MATRIX_ZZ_ZW_F:
		//case G_MWO_MATRIX_WX_WY_F:
		//case G_MWO_MATRIX_WZ_WW_F:
		//}

		break;
	case G_MW_NUMLIGHT:
		//#define NUML(n)		(((n)+1)*32 + 0x80000000)
		{
			u32 num_lights = ((command.cmd1-0x80000000)/32) - 1;
			DL_PF("    G_MW_NUMLIGHT: Val:%d", num_lights);

			gAmbientLightIdx = num_lights;
			PSPRenderer::Get()->SetNumLights(num_lights);

		}
		break;
	case G_MW_CLIP:
		{
			switch (offset)
			{
			case G_MWO_CLIP_RNX:		DL_PF("    G_MW_CLIP  NegX: %d", (s32)(s16)command.cmd1);			break;
			case G_MWO_CLIP_RNY:		DL_PF("    G_MW_CLIP  NegY: %d", (s32)(s16)command.cmd1);			break;
			case G_MWO_CLIP_RPX:		DL_PF("    G_MW_CLIP  PosX: %d", (s32)(s16)command.cmd1);			break;
			case G_MWO_CLIP_RPY:		DL_PF("    G_MW_CLIP  PosY: %d", (s32)(s16)command.cmd1);			break;
			default:					DL_PF("    G_MW_CLIP  ?   : 0x%08x", command.cmd1);					break;
			}

			//RDP_NOIMPL_WARN("G_MW_CLIP Not Implemented");
		}
		break;
	case G_MW_SEGMENT:
		{
			u32 segment = (offset >> 2) & 0xF;
			u32 base = command.cmd1;
			DL_PF("    G_MW_SEGMENT Seg[%d] = 0x%08x", segment, base);
			gSegments[segment] = base;
		}
		break;
	case G_MW_FOG:
		{
			u16 wMult = u16(command.cmd1 >> 16);
			u16 wOff  = u16(command.cmd1      );

			f32 fMult = ((f32)(s16)wMult) / 65536.0f;
			f32 fOff  = ((f32)(s16)wOff)  / 65536.0f;

			DL_PF("     G_MW_FOG. Mult = 0x%04x (%f), Off = 0x%04x (%f)", wMult, 255.0f * fMult, wOff, 255.0f * fOff );

			PSPRenderer::Get()->SetFogMult( 255.0f * fMult );
			PSPRenderer::Get()->SetFogOffset( 255.0f * fOff );
		}
		//RDP_NOIMPL_WARN("G_MW_FOG Not Implemented");
		break;
	case G_MW_LIGHTCOL:
		{
			u32 light_idx = offset / 0x20;
			u32 field_offset = (offset & 0x7);

			DL_PF("    G_MW_LIGHTCOL/0x%08x: 0x%08x", offset, command.cmd1);

			switch (field_offset)
			{
			case 0:
				//g_N64Lights[light_idx].Colour = command.cmd1;
				// Light col, not the copy
				if (light_idx == gAmbientLightIdx)
				{
					u32 n64col( command.cmd1 );

					PSPRenderer::Get()->SetAmbientLight( v4( N64COL_GETR_F(n64col), N64COL_GETG_F(n64col), N64COL_GETB_F(n64col), 1.0f ) );
				}
				else
				{
					PSPRenderer::Get()->SetLightCol(light_idx, command.cmd1);
				}
				break;

			case 4:
				break;

			default:
				//DBGConsole_Msg(0, "G_MW_LIGHTCOL with unknown offset 0x%08x", field_offset);
				break;
			}
		}

		break;
	case G_MW_POINTS:
		{
			u32 vtx = offset/40;
			u32 where = offset - vtx*40;
			u32 val = command.cmd1;	// Odd value given

			DL_PF("    G_MW_POINTS"); // Used in FIFA 98

			PSPRenderer::Get()->ModifyVertexInfo(where, vtx, val);
		}
 		break;
	case G_MW_PERSPNORM:
		DL_PF("    G_MW_PERSPNORM");
		//RDP_NOIMPL_WARN("G_MW_PESPNORM Not Implemented");		// Used in Starfox - sets to 0xa
	//	if ((short)command.cmd1 != 10)
	//		DBGConsole_Msg(0, "PerspNorm: 0x%04x", (short)command.cmd1);	
		break;
	default:
		DL_PF("    Type: Unknown");
		RDP_NOIMPL_WARN("Unknown MoveWord");
		break;
	}

}


//*****************************************************************************
//
//*****************************************************************************
//0016A710: DB020000 00000018 CMD Zelda_MOVEWORD  Mem[2][00]=00000018 Lightnum=0
//001889F0: DB020000 00000030 CMD Zelda_MOVEWORD  Mem[2][00]=00000030 Lightnum=2
void DLParser_GBI2_MoveWord( MicroCodeCommand command )
{
	u32 type   = (command.cmd0 >> 16) & 0xFF;
	u32 offset = (command.cmd0      ) & 0xFFFF;

# define G_MW_FORCEMTX		0x0c
/*
#define G_MW_MATRIX			0x00
#define G_MW_NUMLIGHT		0x02
#define G_MW_CLIP			0x04
#define G_MW_SEGMENT		0x06
#define G_MW_FOG			0x08
#define G_MW_LIGHTCOL		0x0a
#ifdef	F3DEX_GBI_2x
# define G_MW_FORCEMTX		0x0c
#else	// F3DEX_GBI_2
# define G_MW_POINTS		0x0c
#endif	// F3DEX_GBI_2
#define	G_MW_PERSPNORM		0x0e
*/

	switch (type)
	{
	case G_MW_MATRIX:
		{
			RDP_NOIMPL_WARN( "GBI2: G_MW_MATRIX not implemented" );

			//printf( "GBI2: MW_MATRIX: %08x %08x\n", command.cmd0, command.cmd1 );

		//switch( offset )
		//{
		//case G_MWO_MATRIX_XX_XY_I:
		//case G_MWO_MATRIX_XZ_XW_I:
		//case G_MWO_MATRIX_YX_YY_I:
		//case G_MWO_MATRIX_YZ_YW_I:
		//case G_MWO_MATRIX_ZX_ZY_I:
		//case G_MWO_MATRIX_ZZ_ZW_I:
		//case G_MWO_MATRIX_WX_WY_I:
		//case G_MWO_MATRIX_WZ_WW_I:
		//case G_MWO_MATRIX_XX_XY_F:
		//case G_MWO_MATRIX_XZ_XW_F:
		//case G_MWO_MATRIX_YX_YY_F:
		//case G_MWO_MATRIX_YZ_YW_F:
		//case G_MWO_MATRIX_ZX_ZY_F:
		//case G_MWO_MATRIX_ZZ_ZW_F:
		//case G_MWO_MATRIX_WX_WY_F:
		//case G_MWO_MATRIX_WZ_WW_F:
		//}

		}
		break;
	case G_MW_NUMLIGHT:
		{
			// Lightnum
			// command.cmd1:
			// 0x18 = 24 = 0 lights
			// 0x30 = 48 = 2 lights

			u32 num_lights = command.cmd1/24;
			DL_PF("     G_MW_NUMLIGHT: %d", num_lights);

			gAmbientLightIdx = num_lights;
			PSPRenderer::Get()->SetNumLights(num_lights);
		}
		break;

	case G_MW_CLIP:
		{
			switch (offset)
			{
			case G_MWO_CLIP_RNX:		DL_PF("     G_MW_CLIP  NegX: %d", (s32)(s16)command.cmd1);			break;
			case G_MWO_CLIP_RNY:		DL_PF("     G_MW_CLIP  NegY: %d", (s32)(s16)command.cmd1);			break;
			case G_MWO_CLIP_RPX:		DL_PF("     G_MW_CLIP  PosX: %d", (s32)(s16)command.cmd1);			break;
			case G_MWO_CLIP_RPY:		DL_PF("     G_MW_CLIP  PosY: %d", (s32)(s16)command.cmd1);			break;
			default:					DL_PF("     G_MW_CLIP");											break;
			}
			//RDP_NOIMPL_WARN("G_MW_CLIP Not Implemented");
		}
		break;

	case G_MW_SEGMENT:
		{
			u32 segment = offset / 4;
			u32 address	= command.cmd1;

			DL_PF( "      G_MW_SEGMENT Segment[%d] = 0x%08x", segment, address );

			gSegments[segment] = address;
		}
		break;
	case G_MW_FOG:
		{
			u16 wMult = u16(command.cmd1 >> 16);
			u16 wOff  = u16(command.cmd1      );

			f32 fMult = ((f32)(s16)wMult) / 65536.0f;
			f32 fOff  = ((f32)(s16)wOff)  / 65536.0f;

			DL_PF("     G_MW_FOG. Mult = 0x%04x (%f), Off = 0x%04x (%f)", wMult, 255.0f * fMult, wOff, 255.0f * fOff );

			PSPRenderer::Get()->SetFogMult( 255.0f * fMult );
			PSPRenderer::Get()->SetFogOffset( 255.0f * fOff );
		}
		break;
	case G_MW_LIGHTCOL:
		{
			u32 light_idx = offset / 0x18;
			u32 field_offset = (offset & 0x7);

			DL_PF("     G_MW_LIGHTCOL/0x%08x: 0x%08x", offset, command.cmd1);

			switch (field_offset)
			{
			case 0:
				//g_N64Lights[light_idx].Colour = command.cmd1;
				// Light col, not the copy
				if (light_idx == gAmbientLightIdx)
				{
					u32 n64col( command.cmd1 );

					PSPRenderer::Get()->SetAmbientLight( v4( N64COL_GETR_F(n64col), N64COL_GETG_F(n64col), N64COL_GETB_F(n64col), 1.0f ) );
				}
				else
				{
					PSPRenderer::Get()->SetLightCol(light_idx, command.cmd1);
				}
				break;

			case 4:
				break;

			default:
				//DBGConsole_Msg(0, "G_MW_LIGHTCOL with unknown offset 0x%08x", field_offset);
				break;
			}
		}
		break;

	case G_MW_FORCEMTX:
		RDP_NOIMPL_WARN( "G_MW_FORCEMTX not implemented" );
		break;

	case G_MW_PERSPNORM:
		DL_PF("     G_MW_PERSPNORM 0x%04x", (s16)command.cmd1);
		break;

	default:
		{
			DL_PF("      Ignored!!");

		}
		break;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_MoveMem( MicroCodeCommand command )
{
	u32 type     = (command.cmd0>>16)&0xFF;
	u32 length   = (command.cmd0)&0xFFFF;
	u32 address  = RDPSegAddr(command.cmd1);

	use(length);

	switch (type)
	{
		case G_MV_VIEWPORT:
			{
				DL_PF("    G_MV_VIEWPORT. Address: 0x%08x, Length: 0x%04x", address, length);
				RDP_MoveMemViewport( address );
			}
			break;
		case G_MV_LOOKATY:
			DL_PF("    G_MV_LOOKATY");
			break;
		case G_MV_LOOKATX:
			DL_PF("    G_MV_LOOKATX");
			break;
		case G_MV_L0:
		case G_MV_L1:
		case G_MV_L2:
		case G_MV_L3:
		case G_MV_L4:
		case G_MV_L5:
		case G_MV_L6:
		case G_MV_L7:
			{
				u32 light_idx = (type-G_MV_L0)/2;
				DL_PF("    G_MV_L%d", light_idx);

				// Ensure type == G_MV_L0 -- G_MV_L7
				//	if (type < G_MV_L0 || type > G_MV_L7 || ((type & 0x1) != 0))
				//		break;

				DL_PF("    Light%d: Length:0x%04x, Address: 0x%08x", light_idx, length, address);

				RDP_MoveMemLight(light_idx, address);
			}
			break;
		case G_MV_TXTATT:
			DL_PF("    G_MV_TXTATT");
			RDP_NOIMPL_WARN("G_MV_TXTATT Not Implemented");
			break;
			//Next 3 MATRIX commands should be ignored since they were in the previous command.
		case G_MV_MATRIX_1:
			RDP_GFX_Force_Matrix(address);
			break;
		case G_MV_MATRIX_2:	/*IGNORED*/
			break;
		case G_MV_MATRIX_3:	/*IGNORED*/
			break;
		case G_MV_MATRIX_4:	/*IGNORED*/
			break;
		default:
			DL_PF("    Type: Unknown");
			{
				static bool warned = false;
				if (!warned)
				{
					DBGConsole_Msg(0, "Unknown Move Type: %d", type);
					warned = true;
				}
			}
			break;
	}
}


//*****************************************************************************
//
//*****************************************************************************
/*

001889F8: DC08060A 80188708 CMD Zelda_MOVEMEM  Movemem[0806] <- 80188708
!light 0 color 0.12 0.16 0.35 dir 0.01 0.00 0.00 0.00 (2 lights) [ 1E285A00 1E285A00 01000000 00000000 ]
data(00188708): 1E285A00 1E285A00 01000000 00000000 
00188A00: DC08090A 80188718 CMD Zelda_MOVEMEM  Movemem[0809] <- 80188718
!light 1 color 0.23 0.25 0.30 dir 0.01 0.00 0.00 0.00 (2 lights) [ 3C404E00 3C404E00 01000000 00000000 ]
data(00188718): 3C404E00 3C404E00 01000000 00000000 
00188A08: DC080C0A 80188700 CMD Zelda_MOVEMEM  Movemem[080C] <- 80188700
!light 2 color 0.17 0.16 0.26 dir 0.23 0.31 0.70 0.00 (2 lights) [ 2C294300 2C294300 1E285A00 1E285A00 ]
*/
/*
ZeldaMoveMem: 0xdc080008 0x801984d8
SetScissor: x0=416 y0=72 x1=563 y1=312 mode=0
// Mtx
ZeldaMoveWord:0xdb0e0000 0x00000041 Ignored
ZeldaMoveMem: 0xdc08000a 0x80198538
ZeldaMoveMem: 0xdc08030a 0x80198548

ZeldeMoveMem: Unknown Type. 0xdc08000a 0x80198518
ZeldeMoveMem: Unknown Type. 0xdc08030a 0x80198528
ZeldeMoveMem: Unknown Type. 0xdc08000a 0x80198538
ZeldeMoveMem: Unknown Type. 0xdc08030a 0x80198548
ZeldeMoveMem: Unknown Type. 0xdc08000a 0x80198518
ZeldeMoveMem: Unknown Type. 0xdc08030a 0x80198528
ZeldeMoveMem: Unknown Type. 0xdc08000a 0x80198538
ZeldeMoveMem: Unknown Type. 0xdc08030a 0x80198548


0xa4001120: <0x0c000487> JAL       0x121c        Seg2Addr(t8)				dram
0xa4001124: <0x332100fe> ANDI      at = t9 & 0x00fe
0xa4001128: <0x937309c1> LBU       s3 <- 0x09c1(k1)							len
0xa400112c: <0x943402f0> LHU       s4 <- 0x02f0(at)							dmem
0xa4001130: <0x00191142> SRL       v0 = t9 >> 0x0005
0xa4001134: <0x959f0336> LHU       ra <- 0x0336(t4)
0xa4001138: <0x080007f6> J         0x1fd8        SpMemXfer
0xa400113c: <0x0282a020> ADD       s4 = s4 + v0								dmem

ZeldaMoveMem: 0xdc08000a 0x8010e830 Type: 0a Len: 08 Off: 4000
ZeldaMoveMem: 0xdc08030a 0x8010e840 Type: 0a Len: 08 Off: 4018
// Light
ZeldaMoveMem: 0xdc08060a 0x800ff368 Type: 0a Len: 08 Off: 4030
ZeldaMoveMem: 0xdc08090a 0x800ff360 Type: 0a Len: 08 Off: 4048
//VP
ZeldaMoveMem: 0xdc080008 0x8010e3c0 Type: 08 Len: 08 Off: 4000

*/

# define G_GBI1_MV_VIEWPORT	0x80
# define G_GBI1_MV_LOOKATY	0x82
# define G_GBI1_MV_LOOKATX	0x84
# define G_GBI1_MV_L0	0x86
# define G_GBI1_MV_L1	0x88
# define G_GBI1_MV_L2	0x8a
# define G_GBI1_MV_L3	0x8c
# define G_GBI1_MV_L4	0x8e
# define G_GBI1_MV_L5	0x90
# define G_GBI1_MV_L6	0x92
# define G_GBI1_MV_L7	0x94
# define G_GBI1_MV_TXTATT	0x96
# define G_GBI1_MV_MATRIX_1	0x9e	// NOTE: this is in moveword table
# define G_GBI1_MV_MATRIX_2	0x98
# define G_GBI1_MV_MATRIX_3	0x9a
# define G_GBI1_MV_MATRIX_4	0x9c

// 0,2,4,6 are reserved by G_MTX
# define G_GBI2_MV_VIEWPORT	8
# define G_GBI2_MV_LIGHT	10
# define G_GBI2_MV_POINT	12
# define G_GBI2_MV_MATRIX	14		// NOTE: this is in moveword table
# define G_GBI2_MVO_LOOKATX	(0*24)
# define G_GBI2_MVO_LOOKATY	(1*24)
# define G_GBI2_MVO_L0	(2*24)
# define G_GBI2_MVO_L1	(3*24)
# define G_GBI2_MVO_L2	(4*24)
# define G_GBI2_MVO_L3	(5*24)
# define G_GBI2_MVO_L4	(6*24)
# define G_GBI2_MVO_L5	(7*24)
# define G_GBI2_MVO_L6	(8*24)
# define G_GBI2_MVO_L7	(9*24)

void RDP_GFX_Force_Matrix(u32 address)
{
	if (address + 64 > MAX_RAM_ADDRESS)
	{
		DBGConsole_Msg(0, "ForceMtx: Address invalid (0x%08x)", address);
		return;
	}

    Matrix4x4 mat;
	MatrixFromN64FixedPoint(mat, address);
	PSPRenderer::Get()->SetProjection(mat, true, PSPRenderer::MATRIX_LOAD);
}


void DLParser_GBI2_MoveMem( MicroCodeCommand command )
{


	u32 address	 = RDPSegAddr(command.cmd1);
	//u32 offset = (command.cmd0 >> 8) & 0xFFFF;
	u32 type	 = (command.cmd0     ) & 0xFE;

	u32 length  = (command.cmd0 >> 16) & 0xFF;
	u32 offset2 = (command.cmd0 >> 5) & 0x3FFF;

	use(length);

	DL_PF("    Type: %02x Len: %02x Off: %04x", type, length, offset2);

	//ZeldaMoveMem: Unknown Type. 0xdc08030a 0x80063160
	// offset = 0803
	// type = 0a

	s8 * pcBase = g_ps8RamBase + address;

	use(pcBase);

	switch (type)
	{
	case G_GBI2_MV_VIEWPORT:
		{
			RDP_MoveMemViewport( address );
		}
		break;
	case G_GBI2_MV_LIGHT:
		{

		switch (offset2)
		{
		case 0x00:
			{
				DL_PF("    G_MV_LOOKATX %f %f %f", f32(pcBase[8 ^ 0x3]), f32(pcBase[9 ^ 0x3]), f32(pcBase[10 ^ 0x3]));
			}
			break;
		case 0x18:
			{
				DL_PF("    G_MV_LOOKATY %f %f %f", f32(pcBase[8 ^ 0x3]), f32(pcBase[9 ^ 0x3]), f32(pcBase[10 ^ 0x3]));
			}
			break;
		default:		//0x30/48/60
			{
				u32 light_idx = (offset2 - 0x30)/0x18;
				DL_PF("    Light %d:", light_idx);
				RDP_MoveMemLight(light_idx, address);
			}
			break;
		}
		break;

		}
	 case G_GBI2_MV_MATRIX:
		DL_PF("		Force Matrix: addr=%08X", address);
		RDP_GFX_Force_Matrix(address);
		break;
	case G_GBI2_MVO_L0:
	case G_GBI2_MVO_L1:
	case G_GBI2_MVO_L2:
	case G_GBI2_MVO_L3:
	case G_GBI2_MVO_L4:
	case G_GBI2_MVO_L5:
	case G_GBI2_MVO_L6:
	case G_GBI2_MVO_L7:
		DL_PF("Zelda Move Light");
		RDP_NOIMPL_WARN("Zelda Move Light Not Implemented");
		break;
	case G_GBI2_MV_POINT:
		DL_PF("Zelda Move Point");
		RDP_NOIMPL_WARN("Zelda Move Point Not Implemented");
		break;
	case G_GBI2_MVO_LOOKATX:
		if( (command.cmd0) == 0xDC170000 && ((command.cmd1)&0xFF000000) == 0x80000000 )
		{
			// Ucode for Evangelion.v64, the ObjMatrix cmd
			// DLParser_S2DEX_ObjMoveMem(command);
			// XXX DLParser_S2DEX_ObjMoveMem not implemented yet anyways..
			RDP_NOIMPL_WARN("G_GBI2_MVO_LOOKATX Not Implemented");
		}
		break;
	case G_GBI2_MVO_LOOKATY:
		RDP_NOIMPL_WARN("Not implemented ZeldaMoveMem LOOKATY");
		break;
	case 0x02:
		if( (command.cmd0) == 0xDC070002 && ((command.cmd1)&0xFF000000) == 0x80000000 )
		{
			// DLParser_S2DEX_ObjMoveMem(command);
			// XXX DLParser_S2DEX_ObjMoveMem not implemented yet anyways..
			RDP_NOIMPL_WARN("G_GBI2_MV_0x02 Not Implemented");
			break;
		}
	default: //Unhandled
		DBGConsole_Msg(0, "GBI2 MoveMem: Unknown Type. 0x%08x 0x%08x", command.cmd0, command.cmd1);
		u32 * base = (u32 *)pcBase;
		
		for (u32 i = 0; i < 4; i++)
		{
			DL_PF("    %08x %08x %08x %08x", base[0], base[1], base[2], base[3]);
			base+=4;
		}
		break;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_DumpVtxInfo(u32 address, u32 v0_idx, u32 num_verts)
{
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	if (gDisplayListFile != NULL)
	{
		s8 *pcSrc = (s8 *)(g_pu8RamBase + address);
		s16 *psSrc = (s16 *)(g_pu8RamBase + address);

		for ( u32 idx = v0_idx; idx < v0_idx + num_verts; idx++ )
		{
			f32 x = f32(psSrc[0^0x1]);
			f32 y = f32(psSrc[1^0x1]);
			f32 z = f32(psSrc[2^0x1]);

			u16 wFlags = u16(PSPRenderer::Get()->GetVtxFlags( idx )); //(u16)psSrc[3^0x1];

			u8 a = pcSrc[12^0x3];
			u8 b = pcSrc[13^0x3];
			u8 c = pcSrc[14^0x3];
			u8 d = pcSrc[15^0x3];
			
			s16 nTU = psSrc[4^0x1];
			s16 nTV = psSrc[5^0x1];

			f32 tu = f32(nTU) / 32.0f;
			f32 tv = f32(nTV) / 32.0f;

			const v4 & t = PSPRenderer::Get()->GetTransformedVtxPos( idx );

			psSrc += 8;			// Increase by 16 bytes
			pcSrc += 16;

			DL_PF(" #%02d Flags: 0x%04x Pos: {% 6f,% 6f,% 6f} Tex: {%+7.2f,%+7.2f}, Extra: %02x %02x %02x %02x (transf: {% 6f,% 6f,% 6f})",
				idx, wFlags, x, y, z, tu, tv, a, b, c, d, t.x, t.y, t.z );
		}
	}
#endif
}

//*****************************************************************************
//
//*****************************************************************************

void DLParser_SetScissor( MicroCodeCommand command )
{
	// The coords are all in 8:2 fixed point
	u32 x0   = (command.cmd0>>12)&0xFFF;
	u32 y0   = (command.cmd0>>0 )&0xFFF;
	u32 mode = (command.cmd1>>24)&0x03;
	u32 x1   = (command.cmd1>>12)&0xFFF;
	u32 y1   = (command.cmd1>>0 )&0xFFF;
	u32 address  = RDPSegAddr(command.cmd1);

	use(mode);

	if (gViewPortHackEnabled && g_CI.Address%0x100 != 0 )
	{
		// right half screen
		RDP_MoveMemViewport( address );
	}

	DL_PF("    x0=%d y0=%d x1=%d y1=%d mode=%d", x0/4, y0/4, x1/4, y1/4, mode);

	// Set the cliprect now...
	if ( x0 < x1 && y0 < y1 )
	{
		PSPRenderer::Get()->SetScissor( x0/4, y0/4, x1/4, y1/4 );
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetTile( MicroCodeCommand command )
{
	RDP_Tile tile;
	tile.cmd0 = command.cmd0;
	tile.cmd1 = command.cmd1;
	
	RDP_SetTile( tile );
	gRDPStateManager.SetTile( tile.tile_idx, tile );

	DL_PF("    Tile:%d  Fmt: %s/%s Line:%d TMem:0x%04x Palette:%d", tile.tile_idx, gFormatNames[tile.format], gSizeNames[tile.size], tile.line, tile.tmem, tile.palette);
	DL_PF( "         S: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x", gOnOffNames[tile.clamp_s],gOnOffNames[tile.mirror_s], tile.mask_s, tile.shift_s );
	DL_PF( "         T: Clamp: %s Mirror:%s Mask:0x%x Shift:0x%x", gOnOffNames[tile.clamp_t],gOnOffNames[tile.mirror_t], tile.mask_t, tile.shift_t );

}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetTileSize( MicroCodeCommand command )
{
	RDP_TileSize tile;
	tile.cmd0 = command.cmd0;
	tile.cmd1 = command.cmd1;

	RDP_SetTileSize( tile );

	DL_PF("    Tile:%d (%d,%d) -> (%d,%d) [%d x %d]",
		tile.tile_idx, tile.left/4, tile.top/4,
		        tile.right/4, tile.bottom/4,
				((tile.right/4) - (tile.left/4)) + 1,
				((tile.bottom/4) - (tile.top/4)) + 1);

	gRDPStateManager.SetTileSize( tile.tile_idx, tile );
}


//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetTImg( MicroCodeCommand command )
{
	g_TI.Format		= (command.cmd0>>21)&0x7;
	g_TI.Size		= (command.cmd0>>19)&0x3;
	g_TI.Width		= (command.cmd0&0x0FFF) + 1;
	g_TI.Address	= RDPSegAddr(command.cmd1);

	DL_PF("    Image: 0x%08x Fmt: %s/%s Width: %d (Pitch: %d)", g_TI.Address, gFormatNames[g_TI.Format], gSizeNames[g_TI.Size], g_TI.Width, g_TI.GetPitch());
}


//*****************************************************************************
//
//*****************************************************************************
void DLParser_LoadBlock( MicroCodeCommand command )
{
	u32 uls			= ((command.cmd0>>12)&0x0FFF)/4;
	u32 ult			= ((command.cmd0    )&0x0FFF)/4;
	//u32 pad		=  (command.cmd1>>27)&0x1f;
	u32 tile_idx	=  (command.cmd1>>24)&0x07;
	u32 pixels		= ((command.cmd1>>12)&0x0FFF) + 1;		// Number of bytes-1?
	u32 dxt			=  (command.cmd1    )&0x0FFF;		// 1.11 fixed point

	use(pixels);

	u32		quadwords;
	bool	swapped;

	if (dxt == 0)
	{
		quadwords = 1;
		swapped = true;
	}
	else
	{
		quadwords = 2048 / dxt;						// #Quad Words
		swapped = false;
	}

	u32		bytes( quadwords * 8 );
	u32		width( bytes2pixels( bytes, g_TI.Size ) );
	u32		pixel_offset( (width * ult) + uls );
	u32		offset( pixels2bytes( pixel_offset, g_TI.Size ) );

	u32		src_offset(g_TI.Address + offset);

	DL_PF("    Tile:%d (%d,%d) %d pixels DXT:0x%04x = %d QWs => %d pixels/line", tile_idx, uls, ult, pixels, dxt, quadwords, width);
	DL_PF("    Offset: 0x%08x", src_offset);

	gRDPStateManager.LoadBlock( tile_idx, src_offset, swapped );

	RDP_TileSize tile;
	tile.cmd0 = command.cmd0;
	tile.cmd1 = command.cmd1;
	RDP_LoadBlock( tile );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_LoadTile( MicroCodeCommand command )
{
	RDP_TileSize tile;
	tile.cmd0 = command.cmd0;
	tile.cmd1 = command.cmd1;
	
	DL_PF("    Tile:%d (%d,%d) -> (%d,%d) [%d x %d]",	tile.tile_idx, tile.left/4, tile.top/4, tile.right/4 + 1, tile.bottom / 4 + 1, (tile.right - tile.left)/4+1, (tile.bottom - tile.top)/4+1);
	DL_PF("    Offset: 0x%08x",							g_TI.GetOffset( tile.left, tile.top ) );

	gRDPStateManager.LoadTile( tile );

	RDP_LoadTile( tile );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_LoadTLut( MicroCodeCommand command )
{
	u32 uls     = ((command.cmd0 >> 12) & 0xfff)/4;
	u32 tile_idx= ((command.cmd1 >> 24) & 0x07);
	u32 lrs     = ((command.cmd1 >> 12) & 0xfff)/4;
	u32 lrt     = ((command.cmd1      ) & 0xfff)/4;
	u32 count   = (lrs - uls);

	use(tile_idx);
	use(lrs);
	use(lrt);
	use(count);

	// Format is always 16bpp - RGBA16 or IA16:
	// I've no idea why these two are added - seems to work for 007!

	//const RDP_Tile &	rdp_tile( gRDPStateManager.GetTile( tile_idx ) );
	//gPalAddresses[ rdp_tile.tmem ] = g_TI.Address + offset;

	RDP_TileSize tile;
	tile.cmd0 = command.cmd0;
	tile.cmd1 = command.cmd1;

	RDP_LoadTLut( tile );

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	u32 ult     = ((command.cmd0      ) & 0xfff)/4;
	u32 offset = (uls + ult)*2;

	if (gDisplayListFile != NULL)
	{
		char str[300] = "";
		char item[300];
		u16 * pBase = (u16 *)(g_pu8RamBase + (g_TI.Address + offset));
		u32 i;


		DL_PF("    LoadTLut Tile:%d, (%d,%d) -> (%d,%d), Count %d",
			tile_idx, uls, ult, lrs, lrt, count);
		// This is sometimes wrong (in 007) tlut fmt is set after 
		// tlut load, but before tile load
		DL_PF("    Fmt is %s", gTLUTTypeName[gRDPOtherMode.text_tlut]);

		for (i = 0; i < count; i++)
		{
			u16 wEntry = pBase[i ^ 0x1];

			if (i % 8 == 0)
			{
				DL_PF(str);

				// Clear
				sprintf(str, "%03d: ", i);
			}

			PixelFormats::N64::Pf5551	n64col( wEntry );
			PixelFormats::Psp::Pf8888	pspcol( PixelFormats::Psp::Pf8888::Make( n64col ) );

			sprintf(item, "0x%04x (0x%08x) ", n64col.Bits, pspcol.Bits );
			strcat(str, item);
		}
		DL_PF(str);
	}
#endif
}



//*****************************************************************************
//
//*****************************************************************************
void DLParser_TexRect( MicroCodeCommand command )
{
	MicroCodeCommand command2;
	MicroCodeCommand command3;

	//
	// Fetch the next two instructions
	//
	if( !DLParser_FetchNextCommand( &command2 ) ||
		!DLParser_FetchNextCommand( &command3 ) )
		return;

	RDP_TexRect tex_rect;
	tex_rect.cmd0 = command.cmd0;
	tex_rect.cmd1 = command.cmd1;
	tex_rect.cmd2 = command2.cmd1;
	tex_rect.cmd3 = command3.cmd1;

	v2 d( tex_rect.dsdx / 1024.0f, tex_rect.dtdy / 1024.0f );
	v2 xy0( tex_rect.x0 / 4.0f, tex_rect.y0 / 4.0f );
	v2 xy1( tex_rect.x1 / 4.0f, tex_rect.y1 / 4.0f );
	v2 uv0( tex_rect.s / 32.0f, tex_rect.t / 32.0f );
	v2 uv1;

	if ((gOtherModeH & G_CYC_COPY) == G_CYC_COPY)
	{
		d.x /= 4.0f;	// In copy mode 4 pixels are copied at once.
	}

	uv1.x = uv0.x + d.x * ( xy1.x - xy0.x );
	uv1.y = uv0.y + d.y * ( xy1.y - xy0.y );

	//DL_PF("    Tile:%d Screen(%f,%f) -> (%f,%f)",				   tex_rect.tile_idx, xy0.x, xy0.y, xy1.x, xy1.y);
	//DL_PF("           Tex:(%#5f,%#5f) -> (%#5f,%#5f) (DSDX:%#5f DTDY:%#5f)",          uv0.x, uv0.y, uv1.x, uv1.y, d.x, d.y);
	//DL_PF(" ");

	PSPRenderer::Get()->TexRect( tex_rect.tile_idx, xy0, xy1, uv0, uv1 );
}

//*****************************************************************************
//
//*****************************************************************************

void DLParser_TexRectFlip( MicroCodeCommand command )
{ 
	MicroCodeCommand command2;
	MicroCodeCommand command3;

	//
	// Fetch the next two instructions
	//
	if( !DLParser_FetchNextCommand( &command2 ) ||
		!DLParser_FetchNextCommand( &command3 ) )
		return;

	RDP_TexRect tex_rect;
	tex_rect.cmd0 = command.cmd0;
	tex_rect.cmd1 = command.cmd1;
	tex_rect.cmd2 = command2.cmd1;
	tex_rect.cmd3 = command3.cmd1;

	v2 d( tex_rect.dsdx / 1024.0f, tex_rect.dtdy / 1024.0f );
	v2 xy0( tex_rect.x0 / 4.0f, tex_rect.y0 / 4.0f );
	v2 xy1( tex_rect.x1 / 4.0f, tex_rect.y1 / 4.0f );
	v2 uv0( tex_rect.s / 32.0f, tex_rect.t / 32.0f );
	v2 uv1;

	if ((gOtherModeH & G_CYC_COPY) == G_CYC_COPY)
	{
		d.x /= 4.0f;	// In copy mode 4 pixels are copied at once.
	}

	uv1.x = uv0.x + d.x * ( xy1.y - xy0.y );		// Flip - use y
	uv1.y = uv0.y + d.y * ( xy1.x - xy0.x );		// Flip - use x

	//DL_PF("    Tile:%d Screen(%f,%f) -> (%f,%f)",				   tex_rect.tile_idx, xy0.x, xy0.y, xy1.x, xy1.y);
	//DL_PF("           Tex:(%#5f,%#5f) -> (%#5f,%#5f) (DSDX:%#5f DTDY:%#5f)",          uv0.x, uv0.y, uv1.x, uv1.y, d.x, d.y);
	//DL_PF(" ");
	
	PSPRenderer::Get()->TexRectFlip( tex_rect.tile_idx, xy0, xy1, uv0, uv1 );
}


//*****************************************************************************
//
//*****************************************************************************
void DLParser_FillRect( MicroCodeCommand command )
{ 
	u32 x0   = (command.cmd1>>12)&0xFFF;
	u32 y0   = (command.cmd1>>0 )&0xFFF;
	u32 x1   = (command.cmd0>>12)&0xFFF;
	u32 y1   = (command.cmd0>>0 )&0xFFF;

	v2 xy0( x0 / 4.0f, y0 / 4.0f );
	v2 xy1( x1 / 4.0f, y1 / 4.0f );

	// Note, in some modes, the right/bottom lines aren't drawn

	DL_PF("    (%d,%d) (%d,%d)", x0, y0, x1, y1);

	// TODO - In 1/2cycle mode, skip bottom/right edges!?

/*	// Doesn't work properly anyways...
	if ((strncmp((char*)g_ROM.rh.Name, "BANJO TOOIE", 11) == 0))
	{
		// Skip this
		return;
	}
*/
	if (g_DI.Address == g_CI.Address)
	{
		// Clear the Z Buffer
		//s_pD3DX->SetClearDepth(depth from set fill color);
		CGraphicsContext::Get()->Clear( false, true );

		DL_PF("    Clearing ZBuffer");
	}
	else
	{
		// Clear the screen if large rectangle?
		// This seems to mess up with the Zelda game select screen
		// For some reason it draws a large rect over the entire
		// display, right at the end of the dlist. It sets the primitive
		// colour just before, so maybe I'm missing something??
		{

			/*if (((x1+1)-x0) == gViWidth &&
				((y1+1)-y0) == gViHeight &&
				gFillColor == 0xFF000000)
			{
				DL_PF("  Filling Rectangle (screen clear)");
				CGraphicsContext::Get()->Clear(true, false);

				//DBGConsole_Msg(0, "DL ColorBuffer = 0x%08x", g_CI.Address);
			}
			else*/
			{
				// TODO - Check colour image format to work out how this should be decoded!

				c32		colour;
				
				if ( g_CI.Size == G_IM_SIZ_16b )
				{
					PixelFormats::N64::Pf5551	c( (u16)gFillColor );

					colour = PixelFormats::convertPixelFormat< c32, PixelFormats::N64::Pf5551 >( c );

					//printf( "FillRect: %08x, %04x\n", colour.GetColour(), c.Bits );
				}
				else
				{
					colour = c32( gFillColor );
				}
				DL_PF("    Filling Rectangle");
				PSPRenderer::Get()->FillRect( xy0, xy1, colour.GetColour() );
			}
		}

	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetZImg( MicroCodeCommand command )
{
	DL_PF("    Image: 0x%08x", RDPSegAddr(command.cmd1));

	g_DI.Address = RDPSegAddr(command.cmd1);
}

//*****************************************************************************
//
//*****************************************************************************
//#define STORE_CI	{g_CI.Address = newaddr;g_CI.Format = format;g_CI.Size = size;g_CI.Width = width;g_CI.Bpl=Bpl;}

void DLParser_SetCImg( MicroCodeCommand command )
{
	u32 format = (command.cmd0>>21)&0x7;
	u32 size   = (command.cmd0>>19)&0x3;
	u32 width  = (command.cmd0&0x0FFF) + 1;
	u32 newaddr	= RDPSegAddr(command.cmd1) & 0x00FFFFFF;
	//u32 bpl		= width << size >> 1;	// Bpl doesn't seem to exist on real N64, but it seems to help.. Unused for now..

	DL_PF("    Image: 0x%08x", RDPSegAddr(command.cmd1));
	DL_PF("    Fmt: %s Size: %s Width: %d", gFormatNames[ format ], gSizeNames[ size ], width);

	if( g_CI.Address == newaddr && g_CI.Format == format && g_CI.Size == size && g_CI.Width == width )
	{
		DL_PF("    Set CIMG to the same address, no change, skipped");
		//DBGConsole_Msg(0, "SetCImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", g_CI.Address, gFormatNames[ format ], gSizeNames[ size ], width);
		return;
	}

	//g_CI.Bpl = bpl;
	g_CI.Address = newaddr;
	g_CI.Format = format;
	g_CI.Size = size;
	g_CI.Width = width;
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetCombine( MicroCodeCommand command )
{
	u32 mux0 = command.cmd0&0x00FFFFFF;
	u32 mux1 = command.cmd1;

	u64 mux = (((u64)mux0) << 32) | (u64)mux1;

	RDP_SetMux( mux );
	
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	if (gDisplayListFile != NULL)
	{
		DLParser_DumpMux( (((u64)mux0) << 32) | (u64)mux1 );
	}
#endif
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetFillColor( MicroCodeCommand command )
{
	gFillColor = command.cmd1;

	PixelFormats::N64::Pf5551	n64col( (u16)gFillColor );

	DL_PF( "    Color5551=0x%04x", n64col.Bits );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetFogColor( MicroCodeCommand command )
{
	u8 r	= N64COL_GETR(command.cmd1);
	u8 g	= N64COL_GETG(command.cmd1);
	u8 b	= N64COL_GETB(command.cmd1);
	u8 a	= N64COL_GETA(command.cmd1);

	DL_PF("    RGBA: %d %d %d %d", r, g, b, a);

	c32	fog_colour( r, g, b, a );

	PSPRenderer::Get()->SetFogColour( fog_colour );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetBlendColor( MicroCodeCommand command )
{
	u8 r	= N64COL_GETR(command.cmd1);
	u8 g	= N64COL_GETG(command.cmd1);
	u8 b	= N64COL_GETB(command.cmd1);
	u8 a	= N64COL_GETA(command.cmd1);

	use(r);
	use(g);
	use(b);

	DL_PF("    RGBA: %d %d %d %d", r, g, b, a);

	PSPRenderer::Get()->SetAlphaRef(a);
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetPrimColor( MicroCodeCommand command )
{
	u32 m	= (command.cmd0>>8)&0xFF;
	u32 l	= (command.cmd0)&0xFF;
	u8 r	= N64COL_GETR(command.cmd1);
	u8 g	= N64COL_GETG(command.cmd1);
	u8 b	= N64COL_GETB(command.cmd1);
	u8 a	= N64COL_GETA(command.cmd1);

	use(m);
	use(l);

	DL_PF("    M:%d L:%d RGBA: %d %d %d %d", m, l, r, g, b, a);

	c32	prim_colour( r, g, b, a );

	PSPRenderer::Get()->SetPrimitiveColour( prim_colour );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetEnvColor( MicroCodeCommand command )
{
	u8 r	= N64COL_GETR(command.cmd1);
	u8 g	= N64COL_GETG(command.cmd1);
	u8 b	= N64COL_GETB(command.cmd1);
	u8 a	= N64COL_GETA(command.cmd1);

	DL_PF("    RGBA: %d %d %d %d", r, g, b, a);

	c32	env_colour( r, g, b, a );

	PSPRenderer::Get()->SetEnvColour( env_colour );
}



#ifdef DAEDALUS_DEBUG_DISPLAYLIST
//*****************************************************************************
//
//*****************************************************************************
void DLParser_DumpNextDisplayList()
{
	gDumpNextDisplayList = true;
}
#endif

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
//*****************************************************************************
//
//*****************************************************************************
u32 DLParser_GetTotalInstructionCount()
{
	return gTotalInstructionCount;
}
#endif

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
//*****************************************************************************
//
//*****************************************************************************
u32 DLParser_GetInstructionCountLimit()
{
	return gInstructionCountLimit;
}
#endif

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
//*****************************************************************************
//
//*****************************************************************************
void DLParser_SetInstructionCountLimit( u32 limit )
{
	gInstructionCountLimit = limit;
}
#endif
