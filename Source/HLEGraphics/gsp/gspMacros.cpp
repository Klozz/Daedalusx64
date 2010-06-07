/*
Copyright (C) 2009 StrmnNrmn
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

//
//	gSP Macros
//

#include "gspCommon.h"


u32 gGeometryMode = 0;

//*****************************************************************************
// gSPVertex( Vtx *v, u32 n, u32 v0 )
//*****************************************************************************
void DLParser_GBI0_Vtx( MicroCodeCommand *command )
{
        u32 address = RDPSegAddr(command->cmd1);

        u32 len = (command->cmd0)&0xFFFF;
        u32 v0 =  (command->cmd0>>16)&0x0F;
        u32 n  = ((command->cmd0>>20)&0x0F)+1;

        use(len);

        DL_PF("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", address, v0, n, len);

        if ( (v0 + n) > 32 )
        {
                DL_PF("        Warning, attempting to load into invalid vertex positions");
                DBGConsole_Msg(0, "DLParser_GBI0_Vtx: Warning, attempting to load into invalid vertex positions");
                return;
        }

        // Check that address is valid...
        if ( (address + (n*16)) > MAX_RAM_ADDRESS )
        {
                DBGConsole_Msg( 0, "SetNewVertexInfoVFPU: Address out of range (0x%08x)", address );
        }
        else
        {
                PSPRenderer::Get()->SetNewVertexInfoVFPU( address, v0, n );

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
                gNumVertices += n;
                DLParser_DumpVtxInfo( address, v0, n );
#endif
        }
}

//*****************************************************************************
// The previous way of calculating was based on the assumption that
// there was no "n" field. I didn't realise that the n/length fields shared the
// lower 16 bits (in a 6:10 split).
// u32 length    = (command->cmd0)&0xFFFF;
// u32 num_verts = (length + 1) / 0x210;                        // 528
// u32 v0_idx    = ((command->cmd0>>16)&0xFF)/VertexStride;      // /5
//*****************************************************************************
void DLParser_GBI1_Vtx( MicroCodeCommand *command )
{
        u32 address = RDPSegAddr(command->cmd1);

        //u32 length    = (command->cmd0)&0xFFFF;
        //u32 num_verts = (length + 1) / 0x410;
        //u32 v0_idx    = ((command->cmd0>>16)&0x3f)/2;

        u32 v0  = (command->cmd0 >>17 ) & 0x7f;          // ==((x>>16)&0xff)/2
        u32 n   = (command->cmd0 >>10 ) & 0x3f;
        u32 len = (command->cmd0      ) & 0x3ff;

        use(len);

        DL_PF("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", address, v0, n, len);

        if ( address > MAX_RAM_ADDRESS )
        {
                DL_PF("     Address out of range - ignoring load");
                return;
        }

        if ( (v0 + n) > 64 )
        {
                DL_PF("        Warning, attempting to load into invalid vertex positions");
                DBGConsole_Msg( 0, "        DLParser_GBI1_Vtx: Warning, attempting to load into invalid vertex positions" );
                return;
        }

        PSPRenderer::Get()->SetNewVertexInfoVFPU( address, v0, n );

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
        gNumVertices += n;
        DLParser_DumpVtxInfo( address, v0, n );
#endif
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI2_Vtx( MicroCodeCommand *command )
{
        u32 address = RDPSegAddr(command->cmd1);
        u32 vend   = ((command->cmd0   )&0xFFF)/2;
        u32 n      = ((command->cmd0>>12)&0xFFF);

        u32 v0          = vend - n;

        DL_PF( "    Address 0x%08x, vEnd: %d, v0: %d, Num: %d", address, vend, v0, n );

        if ( vend > 64 )
        {
                DL_PF( "    *Warning, attempting to load into invalid vertex positions" );
                DBGConsole_Msg( 0, "DLParser_GBI2_Vtx: Warning, attempting to load into invalid vertex positions: %d -> %d", v0, v0+n );
                return;
        }

        // Check that address is valid...
        if ( (address + (n*16) ) > MAX_RAM_ADDRESS )
        {
                DBGConsole_Msg( 0, "SetNewVertexInfoVFPU: Address out of range (0x%08x)", address );
        }
        else
        {
                PSPRenderer::Get()->SetNewVertexInfoVFPU( address, v0, n );

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
                gNumVertices += n;
                DLParser_DumpVtxInfo( address, v0, n );
#endif
        }
}

//*****************************************************************************
// gspModifyVertex( s32 vtx, u32 where, u32 val )
//*****************************************************************************
void DLParser_GBI1_ModifyVtx( MicroCodeCommand *command )
{
	const u32 FACTOR = 2; // This might need a bit of changing for other microcodes

	u32 w =  (command->cmd0 >> 16) & 0xFF;
	u32 vert   = ((command->cmd0      ) & 0xFFFF) / FACTOR;
	u32 value  = command->cmd1;

	if( vert > 80 )
	{
		DAEDALUS_DL_ERROR("ModifyVtx: Invalid vertex number: %d", vert);
		return;
	}

	switch ( w )
	{
	case G_MWO_POINT_RGBA:
	case G_MWO_POINT_ST:
	case G_MWO_POINT_XYSCREEN:
	case G_MWO_POINT_ZSCREEN:
		PSPRenderer::Get()->ModifyVertexInfo(w, vert, value);
		break;
	default:
		DBGConsole_Msg( 0, "ModifyVtx - Setting vert data 0x%02x, 0x%08x", w, value );
		DL_PF( "      Setting unknown value: 0x%02x, 0x%08x", w, value );
		break;
	}
}

//*****************************************************************************
//
//*****************************************************************************
//
// gSPMatrix
//

void DLParser_GBI1_Mtx( MicroCodeCommand *command )
{
	GBI1_Matrix* temp = (GBI1_Matrix*)command;

	u32 address = RDPSegAddr(temp->addr);

	DL_PF("    Command: %s %s %s Length %d Address 0x%08x",
		temp->projection == 1 ? "Projection" : "ModelView",
		temp->load == 1 ? "Load" : "Mul",
		temp->push == 1 ? "Push" : "NoPush",
		temp->len, address);

	// Load matrix from address
	MatrixFromN64FixedPoint( address );

	if (temp->projection)
	{
		PSPRenderer::Get()->SetProjection(mat, temp->push, temp->load);
	}
	else
	{
		PSPRenderer::Get()->SetWorldView(mat, temp->push, temp->load);
	}
}

extern u32 ConkerVtxZAddr;
//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI2_Mtx( MicroCodeCommand *command )
{
	ConkerVtxZAddr = 0;	// For Conker BFD
	GBI2_Matrix* temp = (GBI2_Matrix*)command;

	u32 address = RDPSegAddr(temp->addr);

	DL_PF("    Command: %s %s %s Length %d Address 0x%08x",
		temp->projection ? "Projection" : "ModelView",
		temp->load ? "Load" : "Mul",
		temp->nopush == 0 ? "Push" : "No Push",
		temp->len, address);

	if (address + 64 > MAX_RAM_ADDRESS)
	{
		DBGConsole_Msg(0, "ZeldaMtx: Address invalid (0x%08x)", address);
		return;
	}

	// Load matrix from address
	MatrixFromN64FixedPoint( address );

	if (temp->projection)
	{
		PSPRenderer::Get()->SetProjection(mat, temp->nopush==0, temp->load);
	}
	else
	{
		PSPRenderer::Get()->SetWorldView(mat, temp->nopush==0, temp->load);
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_PopMtx( MicroCodeCommand *command )
{
	GBI1_PopMatrix* temp = (GBI1_PopMatrix*)command;

	DL_PF("    Command: (%s)",	temp->projection ? "Projection" : "ModelView");

	// Do any of the other bits do anything?
	// So far only Extreme-G seems to Push/Pop projection matrices

	if (temp->projection)
	{
		PSPRenderer::Get()->PopProjection();
	}
	else
	{
		PSPRenderer::Get()->PopWorldView();
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI2_PopMtx( MicroCodeCommand *command )
{
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	u8 mtx_command = (u8)(command->cmd0 & 0xFF);

	use(mtx_command);

	DL_PF("        Command: 0x%02x (%s)", mtx_command, (mtx_command & G_GBI2_MTX_PROJECTION) ? "Projection" : "ModelView");
#endif

	PSPRenderer::Get()->PopWorldView();
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_CullDL( MicroCodeCommand *command )
{
		return;
		// Broken we need to fix it !
		// Enabling it brakes several Fast3D games, see : AeroGauge and Space Station SV
		// Need to find out why, most likely we'll have to separate this from GBI1/GBI0?

        u32 first = ((command->cmd0) & 0xFFF) / VertexStride;
        u32 last  = ((command->cmd1) & 0xFFF) / VertexStride;

        DL_PF("    Culling using verts %d to %d", first, last);

        // Mask into range
        first &= 0x1f;
        last &= 0x1f;

		if( last < first )	return;
		for (u32 i=first; i<=last; i++)
        {
                if (PSPRenderer::Get()->GetVtxFlags( i ) == 0)
                {
                        DL_PF("    Vertex %d is visible, continuing with display list processing", i);
                        return;
                }
        }

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
        gNumDListsCulled++;
#endif

        DL_PF("    No vertices were visible, culling rest of display list");

        DLParser_PopDL();

}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI2_CullDL( MicroCodeCommand *command )
{

        u32 first = ((command->cmd0) & 0xfff) / 2;
        u32 last  = ((command->cmd1) & 0xfff) / 2;

		if( last < first )	return;		// Fixes Aidyn Chronicles

        DL_PF("    Culling using verts %d to %d", first, last);

        if ( PSPRenderer::Get()->TestVerts( first, last ) )
        {
                DL_PF( "    Display list is visible, returning" );
        }
        else
        {
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
                gNumDListsCulled++;
#endif
                DL_PF("    Display list is invisible, culling");

                DLParser_PopDL();
        }
}

//*****************************************************************************
// gSPDisplayList
//*****************************************************************************
void DLParser_GBI1_DL( MicroCodeCommand *command )
{
        u32             push( (command->cmd0 >> 16) & 0xFF );
        u32             address( RDPSegAddr(command->cmd1) );

		if( address > MAX_RAM_ADDRESS ) // Fixes Shadow of Empire
		{
			DAEDALUS_DL_ERROR("Error: DL addr out of range (0x%08x)", address);
			address &= (MAX_RAM_ADDRESS-1);
		}

        DL_PF("    Address=0x%08x Push: 0x%02x", address, push);

        DList dl;
        dl.addr = address;
        dl.limit = ~0;

        switch (push)
        {
	        case G_DL_PUSH:                 DLParser_PushDisplayList( dl );         break;
	        case G_DL_NOPUSH:               DLParser_CallDisplayList( dl );         break;
        }
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI2_DL(  MicroCodeCommand *command  )
{
        u32             push( (command->cmd0 >> 16) & 0x01 );
        u32             address( RDPSegAddr(command->cmd1) );

		if( address > MAX_RAM_ADDRESS )
		{
			DAEDALUS_DL_ERROR("Error: DL addr out of range (0x%08x)", address);
			address &= (MAX_RAM_ADDRESS-1);
		}

        DL_PF("    Push:0x%02x Addr: 0x%08x", push, address);

        DList dl;
        dl.addr = address;
        dl.limit = ~0;

        switch (push)
        {
	        case G_DL_PUSH:                 DLParser_PushDisplayList( dl );         break;
       		case G_DL_NOPUSH:               DLParser_CallDisplayList( dl );         break;
        }
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_EndDL( MicroCodeCommand *command )
{
        DLParser_PopDL();
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI2_EndDL( MicroCodeCommand *command )
{
        DLParser_PopDL();
}

//*****************************************************************************
// When the depth is less than the z value provided, branch to given address
//*****************************************************************************
void DLParser_GBI1_BranchZ( MicroCodeCommand *command )
{
		u32 vtx = (command->cmd0 & 0xFFF) >> 1;

		f32 vtxdepth = PSPRenderer::Get()->GetTransformedVtxPos(vtx).z/PSPRenderer::Get()->GetTransformedVtxPos(vtx).w;

		//if( vtxdepth <= (command->cmd1) )
		if( vtxdepth <= (s32)(command->cmd1) || gNeedHackforZelda )		// For some reasons we sometimes fail the branch depth on OOT and MM....so we force the gNeedHackforZelda true.
        {																// See OOT : Death Mountain and MM : Outside of Clock Town.
                u32 pc = gDisplayListStack.back().addr;					// This points to the next instruction
                u32 dl = *(u32 *)(g_pu8RamBase + pc-12);
                u32 address = RDPSegAddr(dl);

				//address = RDPSegAddr(dl);; // Is this necessary?

                DL_PF("BranchZ to DisplayList 0x%08x", address);

                DList Dl;
                Dl.addr = address;
                Dl.limit = ~0;
                gDisplayListStack.push_back(Dl);
        }
}

//***************************************************************************** 
// 
//***************************************************************************** 
// AST, Yoshi's World, Scooby Doo, and Mario Golf use this 
//
// Broken : Fix me !!
void DLParser_GBI1_LoadUCode( MicroCodeCommand *command ) 
{ 
	/*u32 code_base = (u32)(command->_u64 & 0x1fffffff);
    u32 code_size = 0x1000; 
    u32 data_base = gRDPHalf1 & 0x1fffffff;         // Preceeding RDP_HALF1 sets this up
    u32 data_size = u32((command->_u64>>32) & 0xffff) + 1;

	DLParser_InitMicrocode( code_base, code_size, data_base, data_size ); */
}

//*****************************************************************************
//
//*****************************************************************************
static void DLParser_InitGeometryMode()
{
        bool bCullFront         = (gGeometryMode & G_CULL_FRONT)		? true : false;
        bool bCullBack          = (gGeometryMode & G_CULL_BACK)			? true : false;
		if( bCullFront && bCullBack )
		{
			DAEDALUS_ERROR(" Warning : Both front and back are culled ");
			bCullFront = false; // should never cull front
		}
		PSPRenderer::Get()->SetCullMode(bCullFront, bCullBack);

        bool bShade				= (gGeometryMode & G_SHADE)				? true : false;
        bool bShadeSmooth       = (gGeometryMode & G_SHADING_SMOOTH)	? true : false;

        bool bFog				= (gGeometryMode & G_FOG)				? true : false;
        bool bTextureGen        = (gGeometryMode & G_TEXTURE_GEN)		? true : false;

        bool bLighting			= (gGeometryMode & G_LIGHTING)			? true : false;
        bool bZBuffer           = (gGeometryMode & G_ZBUFFER)			? true : false;

        PSPRenderer::Get()->SetSmooth( bShade );
        PSPRenderer::Get()->SetSmoothShade( bShadeSmooth );

        PSPRenderer::Get()->SetFogEnable( bFog );
        PSPRenderer::Get()->SetTextureGen(bTextureGen);

        PSPRenderer::Get()->SetLighting( bLighting );
        PSPRenderer::Get()->ZBufferEnable( bZBuffer );
}


//***************************************************************************** 
// 
//*****************************************************************************
void DLParser_GBI1_ClearGeometryMode( MicroCodeCommand *command )
{
        u32 mask = (command->cmd1);
        
        gGeometryMode &= ~mask;

        DLParser_InitGeometryMode();

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
        if (gDisplayListFile != NULL)
        {
                DL_PF("    Mask=0x%08x", mask);
                if (mask & G_ZBUFFER)                           DL_PF("  Disabling ZBuffer");
                if (mask & G_TEXTURE_ENABLE)                    DL_PF("  Disabling Texture");
                if (mask & G_SHADE)                             DL_PF("  Disabling Shade");
                if (mask & G_SHADING_SMOOTH)                    DL_PF("  Disabling Smooth Shading");
                if (mask & G_CULL_FRONT)                        DL_PF("  Disabling Front Culling");
                if (mask & G_CULL_BACK)                         DL_PF("  Disabling Back Culling");
                if (mask & G_FOG)                               DL_PF("  Disabling Fog");
                if (mask & G_LIGHTING)                          DL_PF("  Disabling Lighting");
                if (mask & G_TEXTURE_GEN)                       DL_PF("  Disabling Texture Gen");
                if (mask & G_TEXTURE_GEN_LINEAR)                DL_PF("  Disabling Texture Gen Linear");
                if (mask & G_LOD)                               DL_PF("  Disabling LOD (no impl)");
        }
#endif
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_SetGeometryMode(  MicroCodeCommand *command  )
{
    u32 mask = command->cmd1;

    gGeometryMode |= mask;

    DLParser_InitGeometryMode();

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
    if (gDisplayListFile != NULL)
    {
            DL_PF("    Mask=0x%08x", mask);
            if (mask & G_ZBUFFER)                           DL_PF("  Enabling ZBuffer");
            if (mask & G_TEXTURE_ENABLE)                    DL_PF("  Enabling Texture");
            if (mask & G_SHADE)                             DL_PF("  Enabling Shade");
            if (mask & G_SHADING_SMOOTH)                    DL_PF("  Enabling Smooth Shading");
            if (mask & G_CULL_FRONT)                        DL_PF("  Enabling Front Culling");
            if (mask & G_CULL_BACK)                         DL_PF("  Enabling Back Culling");
            if (mask & G_FOG)                               DL_PF("  Enabling Fog");
            if (mask & G_LIGHTING)                          DL_PF("  Enabling Lighting");
            if (mask & G_TEXTURE_GEN)                       DL_PF("  Enabling Texture Gen");
            if (mask & G_TEXTURE_GEN_LINEAR)                DL_PF("  Enabling Texture Gen Linear");
            if (mask & G_LOD)                               DL_PF("  Enabling LOD (no impl)");
    }
#endif
}

//*****************************************************************************
//
//*****************************************************************************
//
// Seems to be AND (command->cmd0&0xFFFFFF) OR (command->cmd1&0xFFFFFF)
//
void DLParser_GBI2_GeometryMode( MicroCodeCommand *command )
{
    u32 and_bits = (command->cmd0) & 0x00FFFFFF;
    u32 or_bits  = (command->cmd1) & 0x00FFFFFF;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
    if (gDisplayListFile != NULL)
    {
            DL_PF("    0x%08x 0x%08x =(x & 0x%08x) | 0x%08x", command->cmd0, command->cmd1, and_bits, or_bits);

            if ((~and_bits) & G_ZELDA_ZBUFFER)						DL_PF("  Disabling ZBuffer");
            if ((~and_bits) & G_ZELDA_SHADING_SMOOTH)				DL_PF("  Disabling Flat Shading");
            if ((~and_bits) & G_ZELDA_CULL_FRONT)                   DL_PF("  Disabling Front Culling");
            if ((~and_bits) & G_ZELDA_CULL_BACK)                    DL_PF("  Disabling Back Culling");
            if ((~and_bits) & G_ZELDA_FOG)							DL_PF("  Disabling Fog");
            if ((~and_bits) & G_ZELDA_LIGHTING)						DL_PF("  Disabling Lighting");
            if ((~and_bits) & G_ZELDA_TEXTURE_GEN)                  DL_PF("  Disabling Texture Gen");
			if ((~and_bits) & G_ZELDA_TEXTURE_GEN_LINEAR)			DL_PF("  Enabling Texture Gen Linear");

            if (or_bits & G_ZELDA_ZBUFFER)							DL_PF("  Enabling ZBuffer");
            if (or_bits & G_ZELDA_SHADING_SMOOTH)					DL_PF("  Enabling Flat Shading");
            if (or_bits & G_ZELDA_CULL_FRONT)						DL_PF("  Enabling Front Culling");
            if (or_bits & G_ZELDA_CULL_BACK)						DL_PF("  Enabling Back Culling");
            if (or_bits & G_ZELDA_FOG)								DL_PF("  Enabling Fog");
            if (or_bits & G_ZELDA_LIGHTING)							DL_PF("  Enabling Lighting");
            if (or_bits & G_ZELDA_TEXTURE_GEN)						DL_PF("  Enabling Texture Gen");
			if (or_bits & G_ZELDA_TEXTURE_GEN_LINEAR)               DL_PF("  Enabling Texture Gen Linear");
    }
#endif

    gGeometryMode &= and_bits;
    gGeometryMode |= or_bits;


    bool bCullFront         = (gGeometryMode & G_ZELDA_CULL_FRONT)			? true : false;
    bool bCullBack          = (gGeometryMode & G_ZELDA_CULL_BACK)			? true : false;

//  bool bShade				= (gGeometryMode & G_SHADE)						? true : false;
//  bool bFlatShade         = (gGeometryMode & G_ZELDA_SHADING_SMOOTH)		? true : false;
	bool bFlatShade         = (gGeometryMode & G_ZELDA_TEXTURE_GEN_LINEAR)	? true : false;
    if (gFlatShadeDisabled)
		bFlatShade			= false;	// Hack for Tiger Honey Hunt

    bool bFog				= (gGeometryMode & G_ZELDA_FOG)					? true : false;
    bool bTextureGen        = (gGeometryMode & G_ZELDA_TEXTURE_GEN)			? true : false;

    bool bLighting			= (gGeometryMode & G_ZELDA_LIGHTING)			? true : false;
    bool bZBuffer           = (gGeometryMode & G_ZELDA_ZBUFFER)				? true : false;

    PSPRenderer::Get()->SetCullMode(bCullFront, bCullBack);

    PSPRenderer::Get()->SetSmooth( !bFlatShade );
    PSPRenderer::Get()->SetSmoothShade( true );             // Always do this - not sure which bit to use

    PSPRenderer::Get()->SetFogEnable( bFog );
    PSPRenderer::Get()->SetTextureGen(bTextureGen);

    PSPRenderer::Get()->SetLighting( bLighting );
    PSPRenderer::Get()->ZBufferEnable( bZBuffer );

    //DLParser_InitGeometryMode();
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_SetOtherModeL( MicroCodeCommand *command )
{
        u32 shift  = (command->cmd0>>8)&0xFF;
        u32 length = (command->cmd0   )&0xFF;
        u32 data   =  command->cmd1;

        u32 mask = ((1<<length)-1)<<shift;

        gOtherModeL = (gOtherModeL&(~mask)) | data;

        RDP_SetOtherMode( gOtherModeH, gOtherModeL );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_SetOtherModeH( MicroCodeCommand *command )
{
        u32 shift  = (command->cmd0>>8)&0xFF;
        u32 length = (command->cmd0   )&0xFF;
        u32 data   =  command->cmd1;

        u32 mask = ((1<<length)-1)<<shift;

        gOtherModeH = (gOtherModeH&(~mask)) | data;

        RDP_SetOtherMode( gOtherModeH, gOtherModeL );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI2_SetOtherModeL( MicroCodeCommand *command )
{
        u32 shift  = (command->cmd0>>8)&0xFF;
        u32 length = (command->cmd0   )&0xFF;
        u32 data   =  command->cmd1;

        // Mask is constructed slightly differently
        u32 mask = (u32)((s32)(0x80000000)>>length)>>shift;

        gOtherModeL = (gOtherModeL&(~mask)) | data;

        RDP_SetOtherMode( gOtherModeH, gOtherModeL );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI2_SetOtherModeH( MicroCodeCommand *command )
{
        u32 shift  = (command->cmd0>>8)&0xFF;
        u32 length = (command->cmd0   )&0xFF;
        u32 data   =  command->cmd1;

        // Mask is constructed slightly differently
        u32 mask = (u32)((s32)(0x80000000)>>length)>>shift;

        gOtherModeH = (gOtherModeH&(~mask)) | data;

        RDP_SetOtherMode( gOtherModeH, gOtherModeL );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Texture( MicroCodeCommand *command )
{
    gTextureLevel = (command->cmd0>>11)&0x07;
    gTextureTile  = (command->cmd0>>8 )&0x07;

    bool enable =    ((command->cmd0    )&0xFF) != 0;                        // Seems to use 0x01
    f32 scale_s = f32((command->cmd1>>16)&0xFFFF) / (65536.0f * 32.0f);
    f32 scale_t = f32((command->cmd1    )&0xFFFF) / (65536.0f * 32.0f);

    DL_PF("    Level: %d Tile: %d %s", gTextureLevel, gTextureTile, enable ? "enabled":"disabled");
    DL_PF("    ScaleS: %f, ScaleT: %f", scale_s*32.0f, scale_t*32.0f);

    PSPRenderer::Get()->SetTextureEnable( enable );
    PSPRenderer::Get()->SetTextureScale( scale_s, scale_t );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI2_Texture( MicroCodeCommand *command )
{
    gTextureLevel = (command->cmd0>>11)&0x07;
    gTextureTile  = (command->cmd0>>8 )&0x07;

    bool enable =    ((command->cmd0    )&0xFF) != 0;                        // Seems to use 0x02
    f32 scale_s = f32((command->cmd1>>16)&0xFFFF) / (65536.0f * 32.0f);
    f32 scale_t = f32((command->cmd1    )&0xFFFF) / (65536.0f * 32.0f);

    DL_PF("    Level: %d Tile: %d %s", gTextureLevel, gTextureTile, enable ? "enabled":"disabled");
    DL_PF("    ScaleS: %f, ScaleT: %f", scale_s*32.0f, scale_t*32.0f);

    PSPRenderer::Get()->SetTextureEnable( enable );
    PSPRenderer::Get()->SetTextureScale( scale_s, scale_t );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI2_Quad( MicroCodeCommand *command )
{
	// Ok this using the same fucntion as Line3D, which is wrong
	// This command is supposed to draw the missing heart on Zelda: OOT and MM.
	// Because of that we are marking Quad cmd as unimplemented, since we are still mising the heart.


    // While the next command pair is Tri2, add vertices
    u32 pc = gDisplayListStack.back().addr;
    u32 * pCmdBase = (u32 *)(g_pu8RamBase + pc);

    bool tris_added = false;

    while ( command->cmd == G_GBI2_QUAD )
    {
            // Vertex indices are multiplied by 10 for Mario64, by 2 for MarioKart
            u32 v2_idx = ((command->cmd1>>16)&0xFF)/2;
            u32 v1_idx = ((command->cmd1>>8 )&0xFF)/2;
            u32 v0_idx = ((command->cmd1    )&0xFF)/2;

            u32 v5_idx = ((command->cmd0>>16)&0xFF)/2;
            u32 v4_idx = ((command->cmd0>>8 )&0xFF)/2;
            u32 v3_idx = ((command->cmd0    )&0xFF)/2;

            tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);
            tris_added |= PSPRenderer::Get()->AddTri(v3_idx, v4_idx, v5_idx);

            command->cmd0 = *pCmdBase++;
            command->cmd1 = *pCmdBase++;
            pc += 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
            if ( command->cmd == G_GBI2_QUAD )
            {
                    DL_PF("0x%08x: %08x %08x %-10s", pc-8, command->cmd0, command->cmd1, gInstructionName[ command->cmd ]);
            }
#endif
    }
    gDisplayListStack.back().addr = pc-8;

    if (tris_added)
    {
            PSPRenderer::Get()->FlushTris();
    }
}

//*****************************************************************************
//
//*****************************************************************************
// XXX SpiderMan uses this command->
void DLParser_GBI2_Line3D( MicroCodeCommand *command )
{	
    // While the next command pair is Tri2, add vertices
    u32 pc = gDisplayListStack.back().addr;
    u32 * pCmdBase = (u32 *)(g_pu8RamBase + pc);

    bool tris_added = false;

    while ( command->cmd == G_GBI2_LINE3D )
    {
            // Vertex indices are multiplied by 10 for Mario64, by 2 for MarioKart
            u32 v2_idx = ((command->cmd1>>16)&0xFF)/2;
            u32 v1_idx = ((command->cmd1>>8 )&0xFF)/2;
            u32 v0_idx = ((command->cmd1    )&0xFF)/2;

            u32 v5_idx = ((command->cmd0>>16)&0xFF)/2;
            u32 v4_idx = ((command->cmd0>>8 )&0xFF)/2;
            u32 v3_idx = ((command->cmd0    )&0xFF)/2;

            tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);
            tris_added |= PSPRenderer::Get()->AddTri(v3_idx, v4_idx, v5_idx);

            command->cmd0 = *pCmdBase++;
            command->cmd1 = *pCmdBase++;
            pc += 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
            if ( command->cmd == G_GBI2_LINE3D )
            {
                    DL_PF("0x%08x: %08x %08x %-10s", pc-8, command->cmd0, command->cmd1, gInstructionName[ command->cmd ]);
            }
#endif
    }
    gDisplayListStack.back().addr = pc-8;

    if (tris_added)
    {
            PSPRenderer::Get()->FlushTris();
    }
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI2_Tri1( MicroCodeCommand *command )
{
    // While the next command pair is Tri1, add vertices
    u32 pc = gDisplayListStack.back().addr;
    u32 * pCmdBase = (u32 *)(g_pu8RamBase + pc);

    bool tris_added = false;

    while ( command->cmd == G_GBI2_TRI1 )
    {
            //u32 flags = (command->cmd1>>24)&0xFF;
            u32 v0_idx = ((command->cmd0    )&0xFF)/2;
            u32 v1_idx = ((command->cmd0>>8 )&0xFF)/2;
            u32 v2_idx = ((command->cmd0>>16)&0xFF)/2;

            tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);

            command->cmd0 = *pCmdBase++;
            command->cmd1 = *pCmdBase++;
            pc += 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
            if ( command->cmd == G_GBI2_TRI1 )
            {
                    DL_PF("0x%08x: %08x %08x %-10s", pc-8, command->cmd0, command->cmd1, gInstructionName[ command->cmd ]);
            }
#endif			
    }
    gDisplayListStack.back().addr = pc-8;

    if (tris_added)
    {
            PSPRenderer::Get()->FlushTris();
    }
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI0_Tri2( MicroCodeCommand *command )
{
    // While the next command pair is Tri2, add vertices
    u32 pc = gDisplayListStack.back().addr;
    u32 * pCmdBase = (u32 *)(g_pu8RamBase + pc);

    bool tris_added = false;

    while (command->cmd == G_GBI1_TRI2)
    {
            // Vertex indices are exact
            u32 idxB = (command->cmd1    ) & 0xF;
            u32 idxA = (command->cmd1>> 4) & 0xF;
            u32 idxE = (command->cmd1>> 8) & 0xF;
            u32 idxD = (command->cmd1>>12) & 0xF;
            u32 idxH = (command->cmd1>>16) & 0xF;
            u32 idxG = (command->cmd1>>20) & 0xF;
            u32 idxK = (command->cmd1>>24) & 0xF;
            u32 idxJ = (command->cmd1>>28) & 0xF;

            u32 idxC = (command->cmd0    ) & 0xF;
            u32 idxF = (command->cmd0>> 4) & 0xF;
            u32 idxI = (command->cmd0>> 8) & 0xF;
            u32 idxL = (command->cmd0>>12) & 0xF;

            //u32 flags = (command->cmd0>>16)&0xFF;

            // Don't check the first two tris for degenerates
            tris_added |= PSPRenderer::Get()->AddTri(idxA, idxC, idxB);
            tris_added |= PSPRenderer::Get()->AddTri(idxD, idxF, idxE);
            if (idxG != idxI && idxI != idxH && idxH != idxG)
            {
                    tris_added |= PSPRenderer::Get()->AddTri(idxG, idxI, idxH);
            }

            if (idxJ != idxL && idxL != idxK && idxK != idxJ)
            {
                    tris_added |= PSPRenderer::Get()->AddTri(idxJ, idxL, idxK);
            }

            command->cmd0= *pCmdBase++;
            command->cmd1= *pCmdBase++;
            pc += 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
            if ( command->cmd == G_GBI1_TRI2 )
            {
                    DL_PF("0x%08x: %08x %08x %-10s", pc-8, command->cmd0, command->cmd1, gInstructionName[ command->cmd ]);
            }
#endif
    }
    gDisplayListStack.back().addr = pc-8;


    if (tris_added)
    {
            PSPRenderer::Get()->FlushTris();
    }
}

//*****************************************************************************
// While the next command pair is Tri2, add vertices
//*****************************************************************************
void DLParser_GBI2_Tri2( MicroCodeCommand *command )
{
    u32 pc = gDisplayListStack.back().addr;
    u32 * pCmdBase = (u32 *)(g_pu8RamBase + pc);

    bool tris_added = false;

    while ( command->cmd == G_GBI2_TRI2 )
    {
            u32 v2_idx = ((command->cmd1>>16)&0xFF)/2;
            u32 v1_idx = ((command->cmd1>>8 )&0xFF)/2;
            u32 v0_idx = ((command->cmd1    )&0xFF)/2;

            u32 v5_idx = ((command->cmd0>>16)&0xFF)/2;
            u32 v4_idx = ((command->cmd0>>8 )&0xFF)/2;
            u32 v3_idx = ((command->cmd0    )&0xFF)/2;

            tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);
            tris_added |= PSPRenderer::Get()->AddTri(v3_idx, v4_idx, v5_idx);

            command->cmd0 = *pCmdBase++;
            command->cmd1 = *pCmdBase++;
            pc += 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
            if ( command->cmd == G_GBI2_TRI2 )
            {
                    DL_PF("0x%08x: %08x %08x %-10s", pc-8, command->cmd0, command->cmd1, gInstructionName[ command->cmd ]);
            }
#endif
    }
    gDisplayListStack.back().addr = pc-8;

    if (tris_added)
    {
            PSPRenderer::Get()->FlushTris();
    }
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Tri2( MicroCodeCommand *command )
{
        // While the next command pair is Tri2, add vertices
        u32 pc = gDisplayListStack.back().addr;
        u32 * pCmdBase = (u32 *)(g_pu8RamBase + pc);

        bool tris_added = false;

        while (command->cmd == G_GBI1_TRI2)
        {
                // Vertex indices are multiplied by 10 for GBI0, by 2 for GBI1
                u32 v0_idx = ((command->cmd1>>16)&0xFF) / VertexStride;
                u32 v1_idx = ((command->cmd1>>8 )&0xFF) / VertexStride;
                u32 v2_idx = ((command->cmd1    )&0xFF) / VertexStride;

                u32 v3_idx = ((command->cmd0>>16)&0xFF) / VertexStride;
                u32 v4_idx = ((command->cmd0>>8 )&0xFF) / VertexStride;
                u32 v5_idx = ((command->cmd0    )&0xFF) / VertexStride;

                tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);
                tris_added |= PSPRenderer::Get()->AddTri(v3_idx, v4_idx, v5_idx);

                command->cmd0= *pCmdBase++;
                command->cmd1= *pCmdBase++;
                pc += 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
                if ( command->cmd == G_GBI1_TRI2 )
                {
//                        DL_PF("0x%08x: %08x %08x %-10s", pc-8, command->cmd0, command->cmd1, gInstructionName[ command->cmd ]);
                }
#endif
        }
        gDisplayListStack.back().addr = pc-8;

        if (tris_added)
        {
                PSPRenderer::Get()->FlushTris();
        }
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Line3D( MicroCodeCommand *command )
{
        // While the next command pair is Tri1, add vertices
        u32 pc = gDisplayListStack.back().addr;
        u32 * pCmdBase = (u32 *)( g_pu8RamBase + pc );

        bool tris_added = false;

        while ( command->cmd == G_GBI1_LINE3D )
        {
                u32 v3_idx   = ((command->cmd1>>24)&0xFF) / VertexStride;
                u32 v0_idx   = ((command->cmd1>>16)&0xFF) / VertexStride;
                u32 v1_idx   = ((command->cmd1>>8 )&0xFF) / VertexStride;
                u32 v2_idx   = ((command->cmd1    )&0xFF) / VertexStride;

                tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);
                tris_added |= PSPRenderer::Get()->AddTri(v2_idx, v3_idx, v0_idx);

                command->cmd0 = *pCmdBase++;
                command->cmd1 = *pCmdBase++;
                pc += 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
                if ( command->cmd == G_GBI1_LINE3D )
                {
//                        DL_PF("0x%08x: %08x %08x %-10s", pc-8, command->cmd0, command->cmd1, gInstructionName[ command->cmd ]);
                }
#endif
        }

        gDisplayListStack.back().addr = pc-8;

        if (tris_added)
        {
                PSPRenderer::Get()->FlushTris();
        }
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Tri1( MicroCodeCommand *command )
{
        DAEDALUS_PROFILE( "DLParser_GBI1_Tri1_T" );

		DAEDALUS_ERROR("vertex : %d",VertexStride);


        // While the next command pair is Tri1, add vertices
        u32 pc = gDisplayListStack.back().addr;
        u32 * pCmdBase = (u32 *)( g_pu8RamBase + pc );

        bool tris_added = false;

        while (command->cmd == G_GBI1_TRI1)
        {
                //u32 flags = (command->cmd1>>24)&0xFF;
                // Vertex indices are multiplied by 10 for Mario64, by 2 for MarioKart
                u32 v0_idx = ((command->cmd1>>16)&0xFF) / VertexStride;
                u32 v1_idx = ((command->cmd1>>8 )&0xFF) / VertexStride;
                u32 v2_idx = ((command->cmd1    )&0xFF) / VertexStride;

                tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);

                command->cmd0= *pCmdBase++;
                command->cmd1= *pCmdBase++;
                pc += 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
                if ( command->cmd == G_GBI1_TRI1 )
                {
  //                      DL_PF("0x%08x: %08x %08x %-10s", pc-8, command->cmd0, command->cmd1, gInstructionName[ command->cmd ]);
                }
#endif
        }

        gDisplayListStack.back().addr = pc-8;

        if (tris_added)
        {
                PSPRenderer::Get()->FlushTris();
        }
}