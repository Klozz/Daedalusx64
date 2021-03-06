/*
Copyright (C) 2009 StrmnNrmn

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

#ifndef UCODE1_H__
#define UCODE1_H__


//*****************************************************************************
// The previous way of calculating was based on the assumption that
// there was no "n" field. I didn't realise that the n/length fields shared the
// lower 16 bits (in a 6:10 split).
// u32 length    = (command.inst.cmd0)&0xFFFF;
// u32 num_verts = (length + 1) / 0x210;                        // 528
// u32 v0_idx    = ((command.inst.cmd0>>16)&0xFF)/gVertexStride;      // /5
//*****************************************************************************
void DLParser_GBI1_Vtx( MicroCodeCommand command )
{
    //u32 length    = (command.inst.cmd0)&0xFFFF;
    //u32 num_verts = (length + 1) / 0x410;
    //u32 v0_idx    = ((command.inst.cmd0>>16)&0x3f)/2;

	u32 addr = RDPSegAddr(command.vtx1.addr);
    u32 v0   = command.vtx1.v0;
    u32 n    = command.vtx1.n;

	DL_PF("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", addr, v0, n, command.vtx1.len);

	// Only games that set this are Flying Dragon and Quake II, anyways they end up crashing due overflowing vtx indices or cliping too many vertices..
	DAEDALUS_ASSERT( (v0 + n) < 64, "Warning, attempting to load into invalid vertex positions");
	DAEDALUS_ASSERT( addr < MAX_RAM_ADDRESS, "Address out of range (0x%08x)", addr );

    PSPRenderer::Get()->SetNewVertexInfo( addr, v0, n );

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
    gNumVertices += n;
    DLParser_DumpVtxInfo( addr, v0, n );
#endif
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_ModifyVtx( MicroCodeCommand command )
{
	u32 offset =  command.modifyvtx.offset;
	u32 vert   = command.modifyvtx.vtx;
	u32 value  = command.modifyvtx.value;

	// Cures crash after swinging in Mario Golf
	if( vert > 80 )
	{
		DAEDALUS_ERROR("ModifyVtx: Invalid vertex number: %d", vert);
		return;
	}

	PSPRenderer::Get()->ModifyVertexInfo( offset, vert, value );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Mtx( MicroCodeCommand command )
{
	u32 address = RDPSegAddr(command.mtx1.addr);

	DL_PF("    Command: %s %s %s Length %d Address 0x%08x",
		command.mtx1.projection == 1 ? "Projection" : "ModelView",
		command.mtx1.load == 1 ? "Load" : "Mul",
		command.mtx1.push == 1 ? "Push" : "NoPush",
		command.mtx1.len, address);

	// Load matrix from address
	if (command.mtx1.projection)
	{
		PSPRenderer::Get()->SetProjection(address, command.mtx1.push, command.mtx1.load);
	}
	else
	{
		PSPRenderer::Get()->SetWorldView(address, command.mtx1.push, command.mtx1.load);
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_PopMtx( MicroCodeCommand command )
{
	DL_PF("    Command: (%s)",	command.inst.cmd1 ? "Projection" : "ModelView");
	DAEDALUS_ASSERT( !command.inst.cmd1, "Trying to Push/Pop projection matrices");

	// Do any of the other bits do anything?
	// So far only Extreme-G seems to Push/Pop projection matrices

	PSPRenderer::Get()->PopWorldView();
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_MoveMem( MicroCodeCommand command )
{
	u32 type     = (command.inst.cmd0>>16)&0xFF;
	u32 address  = RDPSegAddr(command.inst.cmd1);

	switch( type )
	{
		case G_MV_VIEWPORT:
			{
				DL_PF("    G_MV_VIEWPORT. Address: 0x%08x", address);
				RDP_MoveMemViewport( address );
			}
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
				u32 light_idx = (type-G_MV_L0) >> 1;
				RDP_MoveMemLight(light_idx, address);
			}
			break;

		case G_MV_MATRIX_1:
			{
				DL_PF("		Force Matrix(1): addr=%08X", address);
				// Rayman 2, Donald Duck, Tarzan, all wrestling games use this
				PSPRenderer::Get()->ForceMatrix( address );
				// ForceMatrix takes four cmds
				gDlistStack.address[gDlistStackPointer] += 24;
			}
			break;

		//Next 3 MATRIX commands should not appear, since they were in the previous command.
		//case G_MV_MATRIX_2:	/*IGNORED*/	DL_PF("     G_MV_MATRIX_2");											break;
		//case G_MV_MATRIX_3:	/*IGNORED*/	DL_PF("     G_MV_MATRIX_3");											break;
		//case G_MV_MATRIX_4:	/*IGNORED*/	DL_PF("     G_MV_MATRIX_4");											break;
		/*

		// Next 3 cmds are always ignored
		case G_MV_LOOKATY:
			DL_PF("    G_MV_LOOKATY");
			break;
		case G_MV_LOOKATX:
			DL_PF("    G_MV_LOOKATX");
			break;
		case G_MV_TXTATT:
			DL_PF("    G_MV_TXTATT");
			break;
*/
		default:
			{
				DL_PF("    GBI1 MoveMem Type: Ignored!!");
			}
			break;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_MoveWord( MicroCodeCommand command )
{
	// Type of movement is in low 8bits of cmd0.
	u32 value  = command.mw1.value;
	u32 offset = command.mw1.offset;

	switch ( command.mw1.type )
	{
	case G_MW_MATRIX:
		{
			DL_PF("    G_MW_MATRIX(1)");
			PSPRenderer::Get()->InsertMatrix(command.inst.cmd0, command.inst.cmd1);
		}
		break;

	case G_MW_NUMLIGHT:
		{
			u32 num_lights = ((value - 0x80000000) >> 5) - 1;

			DL_PF("    G_MW_NUMLIGHT: Val:%d", num_lights);
			PSPRenderer::Get()->SetNumLights(num_lights);

		}
		break;
/*
	case G_MW_CLIP:	// Seems to be unused?
		{
			DL_PF("    G_MW_CLIP  ?   : 0x%08x", value);
		}
		break;
*/
	case G_MW_SEGMENT:
		{
			u32 segment = (offset >> 2) & 0xF;
			DL_PF("    G_MW_SEGMENT Seg[%d] = 0x%08x", segment, value);
			gSegments[segment] = value;
		}
		break;

	case G_MW_FOG: // WIP, only works for a few games
		{
			f32 a = value >> 16;
			f32 b = value & 0xFFFF;

			//f32 min = b - a;
			//f32 max = b + a;
			//min = min * (1.0f / 16.0f);
			//max = max * (1.0f / 4.0f);
			f32 min = a / 256.0f;
			f32 max = b / 6.0f;

			//DL_PF(" G_MW_FOG. Mult = 0x%04x (%f), Off = 0x%04x (%f)", wMult, 255.0f * fMult, wOff, 255.0f * fOff );

			PSPRenderer::Get()->SetFogMinMax(min, max);

			//printf("1Fog %.0f | %.0f || %.0f | %.0f\n", min, max, a, b);
		}
		break;

	case G_MW_LIGHTCOL:
		{
			u32 field_offset = (offset & 0x7);
			u32 light_idx = offset >> 5;

			DL_PF("    G_MW_LIGHTCOL/0x%08x: 0x%08x", offset, value);

			if (field_offset == 0)
			{
				// Light col
				PSPRenderer::Get()->SetLightCol(light_idx, ((value>>24)&0xFF), ((value>>16)&0xFF), ((value>>8)&0xFF) );
			}
		}
		break;

	case G_MW_POINTS:	// Used in FIFA 98
		{
			DL_PF("    G_MW_POINTS");
			PSPRenderer::Get()->ModifyVertexInfo( (offset / 40), (offset % 40), value);
		}
 		break;
/*
	case G_MW_PERSPNORM:
		DL_PF("    G_MW_PERSPNORM");
		break;
*/
	default:
		{
			DL_PF("    GBI1 MoveWord Type: Ignored!!");
		}
		break;
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_CullDL( MicroCodeCommand command )
{
	u32 first = command.culldl.first;
	u32 last = command.culldl.end;

	DL_PF("    Culling using verts %d to %d\n", first, last);

	if( last < first ) return;
	if( PSPRenderer::Get()->TestVerts( first, last ) )
	{
		DL_PF("    Display list is visible, returning");
		return;
	}

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	++gNumDListsCulled;
#endif

	DL_PF("    No vertices were visible, culling rest of display list");

	DLParser_PopDL();
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_DL( MicroCodeCommand command )
{
	DAEDALUS_ASSERT( RDPSegAddr(command.dlist.addr) < MAX_RAM_ADDRESS, "Dlist address out of range" );
	DAEDALUS_ASSERT( gDlistStackPointer < 30, "Dlist array is too deep"  );

    DL_PF("    Address=0x%08x %s", RDPSegAddr(command.dlist.addr), (command.dlist.param==G_DL_NOPUSH)? "Jump" : (command.dlist.param==G_DL_PUSH)? "Push" : "?");
	DL_PF("    \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/");
	DL_PF("    ############################################");

	if( command.dlist.param == G_DL_PUSH )
		gDlistStackPointer++;

	gDlistStack.address[gDlistStackPointer] = RDPSegAddr(command.dlist.addr);
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_EndDL( MicroCodeCommand command )
{
	DLParser_PopDL();
}

//*****************************************************************************
// When the depth is less than the z value provided, branch to given address
//*****************************************************************************
void DLParser_GBI1_BranchZ( MicroCodeCommand command )
{
	//Always branching will usually just waste a bit of fillrate (PSP got plenty)
	//Games seem not to bother if we branch less than Z all the time

	//Penny racers (cars)
	//Aerogauge (skips rendering ship shadows and exaust plumes from afar)
	//OOT : Death Mountain and MM : Clock Town

	//Seems to work differently for non Zelda games as if Z axis is inverted... //Corn

	//printf("VtxDepth[%d] Zval[%d] Vtx[%d]\n", PSPRenderer::Get()->GetVtxDepth(command.branchz.vtx), (s32)command.branchz.value, command.branchz.vtx);
	//DL_PF("BranchZ VtxDepth[%d] Zval[%d] Vtx[%d]", PSPRenderer::Get()->GetVtxDepth(command.branchz.vtx), (s32)command.branchz.value, command.branchz.vtx);

	if( PSPRenderer::Get()->GetVtxDepth(command.branchz.vtx) <= (s32)command.branchz.value )
	{
		u32 address = RDPSegAddr(gRDPHalf1);

		DL_PF("    Jump -> DisplayList 0x%08x", address);

		gDlistStack.address[gDlistStackPointer] = address;
	}
}

//*****************************************************************************
// AST, Yoshi's World, Scooby Doo use this
//*****************************************************************************
void DLParser_GBI1_LoadUCode( MicroCodeCommand command )
{
	u32 code_base = (command.inst.cmd1 & 0x1fffffff);
    u32 code_size = 0x1000;
    u32 data_base = gRDPHalf1 & 0x1fffffff;         // Preceeding RDP_HALF1 sets this up
    u32 data_size = (command.inst.cmd0 & 0xFFFF) + 1;

	DLParser_InitMicrocode( code_base, code_size, data_base, data_size );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_GeometryMode( MicroCodeCommand command )
{
	const u32 mask = command.inst.cmd1;

	if(command.inst.cmd & 1)
	{
		gGeometryMode._u32 |= mask;
		DL_PF("    Setting mask -> 0x%08x", mask);
	}
	else
	{
		gGeometryMode._u32 &= ~mask;
		DL_PF("    Clearing mask -> 0x%08x", mask);
	}

	TnLPSP TnLMode;

	TnLMode.Light		= gGeometryMode.GBI1_Lighting;

	//Force this to false
	TnLMode.Texture		= 0;

	TnLMode.TexGen		= gGeometryMode.GBI1_TexGen;
	TnLMode.TexGenLin   = gGeometryMode.GBI1_TexGenLin;
	TnLMode.Fog			= gGeometryMode.GBI1_Fog;
	TnLMode.Shade		= gGeometryMode.GBI1_Shade/* & gGeometryMode.GBI1_ShadingSmooth*/;
	TnLMode.Zbuffer		= gGeometryMode.GBI1_Zbuffer;

	// CULL_BACK has priority, Fixes Mortal Kombat 4
	TnLMode.TriCull		= gGeometryMode.GBI1_CullFront | gGeometryMode.GBI1_CullBack;
	TnLMode.CullBack	= gGeometryMode.GBI1_CullBack;

	PSPRenderer::Get()->SetTnLMode( TnLMode._u32 );

	DL_PF("    ZBuffer %s",			 (gGeometryMode.GBI1_Zbuffer)		? "On" : "Off");
	DL_PF("    Culling %s",			 (gGeometryMode.GBI1_CullBack)		? "Back face" : (gGeometryMode.GBI1_CullFront) ? "Front face" : "Off");
	DL_PF("    Shade %s",			 (gGeometryMode.GBI1_Shade)			? "On" : "Off");
	DL_PF("    Smooth Shading %s",	 (gGeometryMode.GBI1_ShadingSmooth) ? "On" : "Off");
	DL_PF("    Lighting %s",		 (gGeometryMode.GBI1_Lighting)		? "On" : "Off");
	DL_PF("    Texture %s",			 (gGeometryMode.GBI1_Texture)		? "On" : "Off");
	DL_PF("    Texture Gen %s",		 (gGeometryMode.GBI1_TexGen)		? "On" : "Off");
	DL_PF("    Texture Gen Linear %s", (gGeometryMode.GBI1_TexGenLin)	? "On" : "Off");
	DL_PF("    Fog %s",				 (gGeometryMode.GBI1_Fog)			? "On" : "Off");
	DL_PF("    LOD %s",				 (gGeometryMode.GBI1_Lod)			? "On" : "Off");
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_SetOtherModeL( MicroCodeCommand command )
{
    const u32 mask = ((1 << command.othermode.len) - 1) << command.othermode.sft;

    gRDPOtherMode.L = (gRDPOtherMode.L&(~mask)) | command.othermode.data;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	RDP_SetOtherMode( gRDPOtherMode.H, gRDPOtherMode.L );
#endif
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_SetOtherModeH( MicroCodeCommand command )
{
    const u32 mask = ((1 << command.othermode.len) - 1) << command.othermode.sft;

    gRDPOtherMode.H = (gRDPOtherMode.H&(~mask)) | command.othermode.data;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	RDP_SetOtherMode( gRDPOtherMode.H, gRDPOtherMode.L );
#endif
}


//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Texture( MicroCodeCommand command )
{
	u32 tile    = command.texture.tile;

	// Seems to use 0x01
    bool enable = command.texture.enable_gbi0;

	DL_PF("    Level[%d] Tile[%d] %s", command.texture.level, tile, enable? "enable":"disable");

	PSPRenderer::Get()->SetTextureTile( tile);
	PSPRenderer::Get()->SetTextureEnable( enable);

	f32 scale_s = f32(command.texture.scaleS)  / (65535.0f * 32.0f);
	f32 scale_t = f32(command.texture.scaleT)  / (65535.0f * 32.0f);

	DL_PF("    ScaleS[%0.4f] ScaleT[%0.4f]", scale_s*32.0f, scale_t*32.0f);
	PSPRenderer::Get()->SetTextureScale( scale_s, scale_t );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Reserved( MicroCodeCommand command )
{
	// Not implemented!
	DL_UNIMPLEMENTED_ERROR( "RDP: Reserved" );
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
void DLParser_GBI1_SpNoop( MicroCodeCommand command )
{
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_RDPHalf_Cont( MicroCodeCommand command )
{
	//DBGConsole_Msg( 0, "Unexpected RDPHalf_Cont: %08x %08x", command.inst.cmd0, command.inst.cmd1 );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_RDPHalf_2( MicroCodeCommand command )
{
//	DBGConsole_Msg( 0, "Unexpected RDPHalf_2: %08x %08x", command.inst.cmd0, command.inst.cmd1 );
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_RDPHalf_1( MicroCodeCommand command )
{
	gRDPHalf1 = command.inst.cmd1;
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Tri2( MicroCodeCommand command )
{
    // While the next command pair is Tri2, add vertices
	u32 pc = gDlistStack.address[gDlistStackPointer];
    u32 * pCmdBase = (u32 *)(g_pu8RamBase + pc);

    bool tris_added = false;

    do{
        //DL_PF("    0x%08x: %08x %08x %-10s", pc-8, command.inst.cmd0, command.inst.cmd1, "G_GBI1_TRI2");

		// Vertex indices are multiplied by 10 for GBI0, by 2 for GBI1
		u32 v0_idx = command.gbi1tri2.v0 >> 1;
		u32 v1_idx = command.gbi1tri2.v1 >> 1;
		u32 v2_idx = command.gbi1tri2.v2 >> 1;

		tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);

		u32 v3_idx = command.gbi1tri2.v3 >> 1;
		u32 v4_idx = command.gbi1tri2.v4 >> 1;
		u32 v5_idx = command.gbi1tri2.v5 >> 1;

		tris_added |= PSPRenderer::Get()->AddTri(v3_idx, v4_idx, v5_idx);

		command.inst.cmd0= *pCmdBase++;
		command.inst.cmd1= *pCmdBase++;
		pc += 8;
    }while( command.inst.cmd == G_GBI1_TRI2 );

	gDlistStack.address[gDlistStackPointer] = pc-8;

    if (tris_added)
    {
            PSPRenderer::Get()->FlushTris();
    }
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Line3D( MicroCodeCommand command )
{
	if( command.gbi1line3d.v3 == 0 )
	{
		// This removes the tris that cover the screen in Flying Dragon
		// Actually this wrong, we should support line3D properly here..
		DAEDALUS_ERROR("Flying Dragon Hack -- Skipping Line3D");
		return;
	}

    // While the next command pair is Tri1, add vertices
	u32 pc	= gDlistStack.address[gDlistStackPointer];
	u32 stride = gVertexStride;
    u32 * pCmdBase = (u32 *)( g_pu8RamBase + pc );

    bool tris_added = false;

	do{
        //DL_PF("    0x%08x: %08x %08x %-10s", pc-8, command.inst.cmd0, command.inst.cmd1, "G_GBI1_LINE3D");

		u32 v0_idx   = command.gbi1line3d.v0 / stride;
		u32 v1_idx   = command.gbi1line3d.v1 / stride;
		u32 v2_idx   = command.gbi1line3d.v2 / stride;
		u32 v3_idx   = command.gbi1line3d.v3 / stride;

		tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);
		tris_added |= PSPRenderer::Get()->AddTri(v2_idx, v3_idx, v0_idx);

		command.inst.cmd0 = *pCmdBase++;
		command.inst.cmd1 = *pCmdBase++;
		pc += 8;
	}while( command.inst.cmd == G_GBI1_LINE3D );

	gDlistStack.address[gDlistStackPointer] = pc-8;

	if (tris_added)
	{
			PSPRenderer::Get()->FlushTris();
	}
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI1_Tri1( MicroCodeCommand command )
{
    //DAEDALUS_PROFILE( "DLParser_GBI1_Tri1_T" );
    // While the next command pair is Tri1, add vertices
	u32 pc	= gDlistStack.address[gDlistStackPointer];
	u32 stride = gVertexStride;
    u32 * pCmdBase = (u32 *)( g_pu8RamBase + pc );

    bool tris_added = false;

    do{
        //DL_PF("    0x%08x: %08x %08x %-10s", pc-8, command.inst.cmd0, command.inst.cmd1, "G_GBI1_TRI1");

		// Vertex indices are multiplied by 10 for Mario64, by 2 for MarioKart
        u32 v0_idx = command.gbi1tri1.v0 / stride;
        u32 v1_idx = command.gbi1tri1.v1 / stride;
        u32 v2_idx = command.gbi1tri1.v2 / stride;

        tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);

        command.inst.cmd0= *pCmdBase++;
        command.inst.cmd1= *pCmdBase++;
        pc += 8;
    }while( command.inst.cmd == G_GBI1_TRI1 );

	gDlistStack.address[gDlistStackPointer] = pc-8;

    if (tris_added)
    {
		PSPRenderer::Get()->FlushTris();
	}
}

#endif // UCODE1_H__
