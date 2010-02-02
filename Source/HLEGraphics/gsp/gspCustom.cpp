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

#include "gspCommon.h"


// DKR verts are extra 4 bytes
//*****************************************************************************
//
//*****************************************************************************
void DLParser_DumpVtxInfoDKR(u32 address, u32 v0_idx, u32 num_verts)
{
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	if (gDisplayListFile != NULL)
	{
		s32 i;

		s16 * psSrc = (s16 *)(g_pu8RamBase + address);

		i = 0;
		for ( u32 idx = v0_idx; idx < v0_idx + num_verts; idx++ )
		{
			f32 x = f32(psSrc[(i + 0) ^ 1]);
			f32 y = f32(psSrc[(i + 1) ^ 1]);
			f32 z = f32(psSrc[(i + 2) ^ 1]);

			//u16 wFlags = PSPRenderer::Get()->GetVtxFlags( idx ); //(u16)psSrc[3^0x1];

			u16 wA = psSrc[(i + 3) ^ 1];
			u16 wB = psSrc[(i + 4) ^ 1];

			u8 a = u8(wA>>8);
			u8 b = u8(wA);
			u8 c = u8(wB>>8);
			u8 d = u8(wB);

			const v4 & t = PSPRenderer::Get()->GetTransformedVtxPos( idx );


			DL_PF(" #%02d Pos: {% 6f,% 6f,% 6f} Extra: %02x %02x %02x %02x (transf: {% 6f,% 6f,% 6f})",
				idx, x, y, z, a, b, c, d, t.x, t.y, t.z );

			i+=5;
		}


		u16 * pwSrc = (u16 *)(g_pu8RamBase + address);
		i = 0;
		for( u32 idx = v0_idx; idx < v0_idx + num_verts; idx++ )
		{
			DL_PF(" #%02d %04x %04x %04x %04x %04x",
				idx, pwSrc[(i + 0) ^ 1],
				pwSrc[(i + 1) ^ 1],
				pwSrc[(i + 2) ^ 1],
				pwSrc[(i + 3) ^ 1],
				pwSrc[(i + 4) ^ 1]);
			
			i += 5;
		}

	}
#endif
}



//*****************************************************************************
//
//*****************************************************************************

void DLParser_GBI0_Vtx_Gemini( MicroCodeCommand command )
{
	u32 address = RDPSegAddr((command.cmd1));
	u32 v0_idx =  (((command.cmd0)>>9)&0x1F);
	u32 num_verts  = (((command.cmd0) >>19 )&0x1F);


	DL_PF("    Address 0x%08x, v0: %d, Num: %d", address, v0_idx, num_verts);

	if (v0_idx >= 32)
		v0_idx = 31;

	if ((v0_idx + num_verts) > 32)
	{
		DBGConsole_Msg(0, "Warning, attempting to load into invalid vertex positions");
		num_verts = 32 - v0_idx;
	}


	//if( dwAddr == 0 || dwAddr < 0x2000)
	//{
	//	address = (command.cmd1)+RDPSegAddr(dwDKRVtxAddr);
	//}

	// Check that address is valid...
	if ((address + (num_verts*16)) > MAX_RAM_ADDRESS)
	{
		DBGConsole_Msg(0, "SetNewVertexInfoDKR: Address out of range (0x%08x)", address);
	}
	else
	{
		PSPRenderer::Get()->SetNewVertexInfoDKR(address, v0_idx, num_verts);

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
		gNumVertices += num_verts;
		DLParser_DumpVtxInfoDKR(address, v0_idx, num_verts);
#endif
	}
}

//*****************************************************************************
//
//*****************************************************************************
// MOVE!
void DLParser_GBI0_Vtx_ShadowOfEmpire( MicroCodeCommand command )
{
      u32 address = RDPSegAddr((command.cmd1));
      u32 len = ((command.cmd0))&0xffff;

      u32 n= (((command.cmd0) >> 4) & 0xfff) / 33 + 1;
      u32 v0 = 0;

      use(len);

      DL_PF("    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", address, v0, n, len);

      if (v0 >= 32)
              v0 = 31;
      
      if ((v0 + n) > 32)
      {
              DBGConsole_Msg(0, "Warning, attempting to load into invalid vertex positions");
              n = 32 - v0;
      }

      PSPRenderer::Get()->SetNewVertexInfoVFPU( address, v0, n );

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
      gNumVertices += n;
      DLParser_DumpVtxInfo( address, v0, n );
#endif
}

//*****************************************************************************
// MOVE
//*****************************************************************************
// BB2k
// DKR
//00229B70: 07020010 000DEFC8 CMD G_DLINMEM  Displaylist at 800DEFC8 (stackp 1, limit 2)
//00229A58: 06000000 800DE520 CMD G_GBI1_DL  Displaylist at 800DE520 (stackp 1, limit 0)
//00229B90: 07070038 00225850 CMD G_DLINMEM  Displaylist at 80225850 (stackp 1, limit 7)

void DLParser_DLInMem( MicroCodeCommand command )
{
	u32		length( (command.cmd0 >> 16) & 0xFF );
	u32		push( G_DL_PUSH ); //(command.cmd0 >> 16) & 0xFF;
	u32		address( 0x00000000 | command.cmd1 ); //RDPSegAddr(command.cmd1);

	DL_PF("    Address=0x%08x Push: 0x%02x", address, push);

	DList dl;
	dl.addr = address;
	dl.limit = length;

	switch (push)
	{
	case G_DL_PUSH:			DLParser_PushDisplayList( dl );		break;
	case G_DL_NOPUSH:		DLParser_CallDisplayList( dl );		break;
	}
}

//*****************************************************************************
//
//*****************************************************************************
/*
00229C28: 01400040 002327C0 CMD G_MTX  {Matrix} at 802327C0 ind 1  Load:Mod 
00229BB8: 01400040 00232740 CMD G_MTX  {Matrix} at 80232740 ind 1  Load:Mod 
00229BF0: 01400040 00232780 CMD G_MTX  {Matrix} at 80232780 ind 1  Load:Mod 
00229B28: 01000040 002326C0 CMD G_MTX  {Matrix} at 802326C0  Mul:Mod 
00229B78: 01400040 00232700 CMD G_MTX  {Matrix} at 80232700  Mul:Mod 
*/

// 0x80 seems to be mul
// 0x40 load


void DLParser_MtxDKR( MicroCodeCommand command )
{	
	u32 address     = RDPSegAddr(command.cmd1);
	u32 mtx_command = (command.cmd0>>16)&0xFF;
	u32 length      = (command.cmd0)    &0xFFFF;

	use(length);

	PSPRenderer::EMatrixLoadStyle load_command = mtx_command & G_GBI1_MTX_LOAD ? PSPRenderer::MATRIX_LOAD : PSPRenderer::MATRIX_MUL;
	bool push( ( mtx_command & G_GBI1_MTX_PUSH ) != 0 );
	
	if (mtx_command == 0)
	{
	//	PSPRenderer::Get()->ResetMatrices();
		load_command = PSPRenderer::MATRIX_LOAD;
	}

	if (mtx_command & 0x80)
	{
		load_command = PSPRenderer::MATRIX_MUL;
	}
	else
	{	
		load_command = PSPRenderer::MATRIX_LOAD;
	}
		load_command = PSPRenderer::MATRIX_LOAD;
//00229B00: BC000008 64009867 CMD G_MOVEWORD  Mem[8][00]=64009867 Fogrange 0.203..0.984 zmax=0.000000
//0x0021eef0: bc000008 64009867 G_MOVEWORD
	
/*
	if (mtx_command & 0x40)
	{*/
		push = false;
/*	}
	else
	{
		push = true;
	}*/
	DL_PF("    Command: %s %s %s Length %d Address 0x%08x",
		(mtx_command & G_GBI1_MTX_PROJECTION) ? "Projection" : "ModelView",
		(mtx_command & G_GBI1_MTX_LOAD) ? "Load" : "Mul",	
		(mtx_command & G_GBI1_MTX_PUSH) ? "Push" : "NoPush",
		length, address);

	if (address + 64 > MAX_RAM_ADDRESS)
	{
		DBGConsole_Msg(0, "Mtx: Address invalid (0x%08x)", address);
		return;
	}

	// Load matrix from address
	Matrix4x4 mat;
	MatrixFromN64FixedPoint( mat, address );

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	if (gDisplayListFile != NULL)
	{
		DL_PF(
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n"
			" %#+12.5f %#+12.5f %#+12.5f %#+12.5f\n",
			mat.m[0][0], mat.m[0][1], mat.m[0][2], mat.m[0][3],
			mat.m[1][0], mat.m[1][1], mat.m[1][2], mat.m[1][3],
			mat.m[2][0], mat.m[2][1], mat.m[2][2], mat.m[2][3],
			mat.m[3][0], mat.m[3][1], mat.m[3][2], mat.m[3][3]);
	}
#endif

	//mat.m[3][0] = mat.m[3][1] = mat.m[3][2] = 0;
	//mat.m[3][3] = 1;

	/*if (mtx_command & G_GBI1_MTX_PROJECTION)
	{
		// So far only Extreme-G seems to Push/Pop projection matrices	
		PSPRenderer::Get()->SetProjection(mat, push, load_command);
	}
	else*/
	//if (push)
	{
		PSPRenderer::Get()->SetWorldView(mat, push, load_command);

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
		PSPRenderer::Get()->PrintActive();
#endif
	}
	/*else
	{

	}*/
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_MoveWord_DKR( MicroCodeCommand command )
{
	u32 num_lights;

	switch ((command.cmd0) & 0xFF)
	{
	case G_MW_NUMLIGHT:
		{
			num_lights = (command.cmd1)&0x7;
			DL_PF("    G_MW_NUMLIGHT: Val:%d", num_lights);

			gAmbientLightIdx = num_lights;
			PSPRenderer::Get()->SetNumLights(num_lights);
		}
	case G_MW_LIGHTCOL:
		{
			//DKR
			PSPRenderer::Get()->ResetMatrices();
		}
		break;
	default:
		DLParser_GBI1_MoveWord( command );
		break;
	}
}
//*****************************************************************************
//
//*****************************************************************************
void DLParser_GBI0_Vtx_DKR( MicroCodeCommand command )
{
	u32 address = RDPSegAddr(command.cmd1);
	u32 v0_idx =  0;
	u32 num_verts  = ((command.cmd0 & 0xFFF) - 0x08) / 0x12;

	DL_PF("    Address 0x%08x, v0: %d, Num: %d", address, v0_idx, num_verts);

	if (v0_idx >= 32)
		v0_idx = 31;
	
	if ((v0_idx + num_verts) > 32)
	{
		DL_PF("        Warning, attempting to load into invalid vertex positions");
		DBGConsole_Msg(0, "DLParser_GBI0_Vtx_DKR: Warning, attempting to load into invalid vertex positions");
		num_verts = 32 - v0_idx;
	}

	// Check that address is valid...
	if ((address + (num_verts*16)) > MAX_RAM_ADDRESS)
	{
		DBGConsole_Msg(0, "SetNewVertexInfoDKR: Address out of range (0x%08x)", address);
	}
	else
	{
		PSPRenderer::Get()->SetNewVertexInfoDKR(address, v0_idx, num_verts);

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
		gNumVertices += num_verts;
		DLParser_DumpVtxInfoDKR(address, v0_idx, num_verts);
#endif

	}
}

//*****************************************************************************
// The previous way of calculating was based on the assumption that
// there was no "n" field. I didn't realise that the n/length fields shared the
// lower 16 bits (in a 7:9 split).
// u32 length    = (command.cmd0)&0xFFFF;
// u32 num_verts = (length + 1) / 0x210;					// 528
// u32 v0_idx    = ((command.cmd0>>16)&0xFF)/VertexStride;	// /5
//*****************************************************************************
void DLParser_GBI0_Vtx_WRUS( MicroCodeCommand command )
{
	u32 address = RDPSegAddr(command.cmd1);
	
	u32 v0  = ((command.cmd0 >>16 ) & 0xff) / 5;
	u32 n   =  (command.cmd0 >>9  ) & 0x7f;
	u32 len =  (command.cmd0      ) & 0x1ff;

	use(len);

	DL_PF( "    Address 0x%08x, v0: %d, Num: %d, Length: 0x%04x", address, v0, n, len );

	if ( (v0 + n) > 32 )
	{
		DL_PF("    *Warning, attempting to load into invalid vertex positions");
		DBGConsole_Msg(0, "DLParser_GBI0_Vtx_WRUS: Warning, attempting to load into invalid vertex positions");
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
void DLParser_DmaTri( MicroCodeCommand command )
{
	bool tris_added = false;
	u32 address = RDPSegAddr(command.cmd1);


	u32 flag = (command.cmd0 & 0x00FF0000) >> 16;
	if (flag&1) 
		PSPRenderer::Get()->SetCullMode(false,true);
	else
		PSPRenderer::Get()->SetCullMode(false,false);


	u32 count = ((command.cmd0 & 0xFFF0) >> 4);
	u32 i;
	u32 * pData = &g_pu32RamBase[address/4];

	for (i = 0; i < count; i++)
	{
		DL_PF("    0x%08x: %08x %08x %08x %08x", address + i*16, pData[0], pData[1], pData[2], pData[3]);

		u32 info = pData[ 0 ];

		u32 v0_idx = (info >> 16) & 0x1F;
		u32 v1_idx = (info >>  8) & 0x1F;
		u32 v2_idx = (info      ) & 0x1F;

		//// Generate texture coordinates
		s16 s0( s16(pData[1]>>16) );
		s16 t0( s16(pData[1]&0xFFFF) );
		s16 s1( s16(pData[2]>>16) );
		s16 t1( s16(pData[2]&0xFFFF) );
		s16 s2( s16(pData[3]>>16) );
		s16 t2( s16(pData[3]&0xFFFF) );

		tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);

		PSPRenderer::Get()->SetVtxTextureCoord( v0_idx, s0, t0 );
		PSPRenderer::Get()->SetVtxTextureCoord( v1_idx, s1, t1 );
		PSPRenderer::Get()->SetVtxTextureCoord( v2_idx, s2, t2 );

		pData += 4;
	}

	if (tris_added)	
	{
		PSPRenderer::Get()->FlushTris();
	}
}
//*****************************************************************************

//IS called Last Legion, but is used for several other games like: Dark Rift, Toukon Road, Toukon Road 2.

//*****************************************************************************
// Only thing I can't figure out why are the characters on those games invisble?
// Dark Rift runs properly without custom microcodes, and has the same symptoms...
// We need Turbo3D ucode support, actually a modified version of it, thanks Gonetz for the info :D

void DLParser_RSP_Last_Legion_0x80( MicroCodeCommand command )
{     
      gDisplayListStack.back().addr += 16;
	  DL_PF("DLParser_RSP_Last_Legion_0x80");
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_RSP_Last_Legion_0x00( MicroCodeCommand command )
{

      gDisplayListStack.back().addr += 16;
	  DL_PF("DLParser_RSP_Last_Legion_0x00");

      if( (command.cmd0) == 0 && (command.cmd1) )
      {
              u32 newaddr = RDPSegAddr((command.cmd1));
              if( newaddr >= MAX_RAM_ADDRESS )
              {
                      DLParser_PopDL();
                      return;
              }

              u32 pc1 = *(u32 *)(g_pu8RamBase + newaddr+8*1+4);
              u32 pc2 = *(u32 *)(g_pu8RamBase + newaddr+8*4+4);
              pc1 = RDPSegAddr(pc1);
              pc2 = RDPSegAddr(pc2);

              if( pc1 && pc1 != 0xffffff && pc1 < MAX_RAM_ADDRESS)
              {
                      // Need to call both DL
                      DList dl;
                      dl.addr = pc1;
                      dl.limit = ~0;
                      gDisplayListStack.push_back(dl);
              }

              if( pc2 && pc2 != 0xffffff && pc2 < MAX_RAM_ADDRESS )
              {
                      DList dl;
                      dl.addr = pc2;
                      dl.limit = ~0;
                      gDisplayListStack.push_back(dl);
              }
      }
      else if( (command.cmd1) == 0 )
      {
              DLParser_PopDL();
      }
      else
      {
              DLParser_Nothing( command );
              DLParser_PopDL();
      }
}

//*****************************************************************************
//
//*****************************************************************************
void DLParser_TexRect_Last_Legion( MicroCodeCommand command )
{
	u32 pc = gDisplayListStack.back().addr;		// This points to the next instruction
	u32 command2 = *(u32 *)(g_ps8RamBase + pc);
	u32 command3 = *(u32 *)(g_ps8RamBase + pc+4);

	gDisplayListStack.back().addr += 8;

	DL_PF("0x%08x: %08x %08x", pc, *(u32 *)(g_ps8RamBase + pc+0), *(u32 *)(g_ps8RamBase + pc+4));


	RDP_TexRect tex_rect;
	tex_rect.cmd0 = command.cmd0;
	tex_rect.cmd1 = command.cmd1;
	
	//Fisnihing up instructions ! 
	tex_rect.cmd2 = command2;
	tex_rect.cmd3 = command3;

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

	PSPRenderer::Get()->TexRect( tex_rect.tile_idx, xy0, xy1, uv0, uv1 );
}

//*****************************************************************************
//*GoldenEye 007 - Sky Fix
//*****************************************************************************
void DLParser_RDPHalf_1_0xb4_GoldenEye( MicroCodeCommand command )
{
	if( (command.cmd1>>24) == 0xce )
		{
		//
		// Fetch the next two instructions
		//
		MicroCodeCommand command2;
		MicroCodeCommand command3;


	//for debugging
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
		u32 address = gDisplayListStack.back().addr;		// This points to the next instruction
		u32 dw1 = *(u32 *)(g_ps8RamBase + address+8*0+4);
		u32 dw2 = *(u32 *)(g_ps8RamBase + address+8*1+4);
		u32 dw3 = *(u32 *)(g_ps8RamBase + address+8*2+4);
		u32 dw4 = *(u32 *)(g_ps8RamBase + address+8*3+4);
		u32 dw5 = *(u32 *)(g_ps8RamBase + address+8*4+4);
		u32 dw6 = *(u32 *)(g_ps8RamBase + address+8*5+4);
		u32 dw7 = *(u32 *)(g_ps8RamBase + address+8*6+4);
		u32 dw8 = *(u32 *)(g_ps8RamBase + address+8*7+4);
		u32 dw9 = *(u32 *)(g_ps8RamBase + address+8*8+4);
#endif


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

		DL_PF("Golden Eye Sky Debug ");
		DL_PF("    Tile:%d Screen(%f,%f) -> (%f,%f)",				   tex_rect.tile_idx, xy0.x, xy0.y, xy1.x, xy1.y);
		DL_PF("           Tex:(%#5f,%#5f) -> (%#5f,%#5f) (DSDX:%#5f DTDY:%#5f)",          uv0.x, uv0.y, uv1.x, uv1.y, d.x, d.y);
		DL_PF(" Word 1: %u, Word 2: %u, Word 3: %u, Word 4: %u, Word 5: %u, Word 6: %u, Word 7: %u, Word 8: %u, Word 9: %u", dw1, dw2, dw3, dw4, dw5, dw6, dw7, dw8, dw9);
		DL_PF(" ");

		PSPRenderer::Get()->TexRect( tex_rect.tile_idx, xy0, xy1, uv0, uv1 );
	}

	 //Skips the next few uneeded RDP_Half commands since we use them here or they are unneeded
	gDisplayListStack.back().addr += 312;

	gRDPHalf1 = u32(command._u64 & 0xffffffff);

}