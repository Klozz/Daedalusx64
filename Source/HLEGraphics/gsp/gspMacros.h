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

//
//	gSP Macros
//

#ifndef GSP_MACROS_H
#define GSP_MACROS_H


//***************************************************************************** 
//
//***************************************************************************** 

#define G_ZELDA_ZBUFFER				G_ZBUFFER	// Guess 
#define G_ZELDA_CULL_BACK			0x00000200  /*G_CULL_FRONT */ 
#define G_ZELDA_CULL_FRONT			0x00000400  /*G_CULL_BACK  */
#define G_ZELDA_FOG					G_FOG 
#define G_ZELDA_LIGHTING			G_LIGHTING 
#define G_ZELDA_TEXTURE_GEN			G_TEXTURE_GEN 
#define G_ZELDA_TEXTURE_GEN_LINEAR	G_TEXTURE_GEN_LINEAR 
#define G_ZELDA_SHADING_SMOOTH		0x00200000

//***************************************************************************** 
//
//***************************************************************************** 

void DLParser_GBI1_CullDL	( MicroCodeCommand command );
void DLParser_GBI2_CullDL	( MicroCodeCommand command );
void DLParser_GBI1_DL		( MicroCodeCommand command );
void DLParser_GBI2_DL		( MicroCodeCommand command );
void DLParser_GBI1_EndDL	( MicroCodeCommand command );
void DLParser_GBI2_EndDL	( MicroCodeCommand command );
void DLParser_GBI1_BranchZ	( MicroCodeCommand command );
void DLParser_GBI1_LoadUCode( MicroCodeCommand command );

void DLParser_GBI1_SetGeometryMode	( MicroCodeCommand command );
void DLParser_GBI1_ClearGeometryMode( MicroCodeCommand command );
void DLParser_GBI2_GeometryMode		( MicroCodeCommand command );
void DLParser_GBI1_SetOtherModeL	( MicroCodeCommand command );
void DLParser_GBI1_SetOtherModeH	( MicroCodeCommand command );
void DLParser_GBI2_SetOtherModeL	( MicroCodeCommand command );
void DLParser_GBI2_SetOtherModeH	( MicroCodeCommand command );

void DLParser_GBI1_Texture	( MicroCodeCommand command );
void DLParser_GBI2_Texture	( MicroCodeCommand command );

void DLParser_GBI0_Vtx		( MicroCodeCommand command );
void DLParser_GBI1_Vtx		( MicroCodeCommand command );
void DLParser_GBI2_Vtx		( MicroCodeCommand command );
void DLParser_GBI1_ModifyVtx( MicroCodeCommand command );

void DLParser_GBI1_Mtx		( MicroCodeCommand command );
void DLParser_GBI2_Mtx		( MicroCodeCommand command );
void DLParser_GBI1_PopMtx	( MicroCodeCommand command );
void DLParser_GBI2_PopMtx	( MicroCodeCommand command );

void DLParser_GBI2_Quad		( MicroCodeCommand command );
void DLParser_GBI2_Line3D	( MicroCodeCommand command );
void DLParser_GBI2_Tri1		( MicroCodeCommand command );
void DLParser_GBI0_Tri2		( MicroCodeCommand command );
void DLParser_GBI2_Tri2		( MicroCodeCommand command );


//*****************************************************************************
/* Mariokart etc*/
//*****************************************************************************

//
// For some reason these have to be declared here.
// Local Functions
//static void DLParser_InitGeometryMode();
//

template < int VertexStride >
void DLParser_GBI1_Tri2_T( MicroCodeCommand command )
{
        // While the next command pair is Tri2, add vertices
        u32 pc = gDisplayListStack.back().addr;
        u32 * pCmdBase = (u32 *)(g_pu8RamBase + pc);

        bool tris_added = false;

        while (command.cmd == G_GBI1_TRI2)
        {
                // Vertex indices are multiplied by 10 for GBI0, by 2 for GBI1
                u32 v0_idx = ((command.cmd1>>16)&0xFF) / VertexStride;
                u32 v1_idx = ((command.cmd1>>8 )&0xFF) / VertexStride;
                u32 v2_idx = ((command.cmd1    )&0xFF) / VertexStride;

                u32 v3_idx = ((command.cmd0>>16)&0xFF) / VertexStride;
                u32 v4_idx = ((command.cmd0>>8 )&0xFF) / VertexStride;
                u32 v5_idx = ((command.cmd0    )&0xFF) / VertexStride;

                tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);
                tris_added |= PSPRenderer::Get()->AddTri(v3_idx, v4_idx, v5_idx);

                command.cmd0= *pCmdBase++;
                command.cmd1= *pCmdBase++;
                pc += 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
                if ( command.cmd == G_GBI1_TRI2 )
                {
//                        DL_PF("0x%08x: %08x %08x %-10s", pc-8, command.cmd0, command.cmd1, gInstructionName[ command.cmd ]);
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
template< int VertexStride >
void DLParser_GBI1_Line3D_T( MicroCodeCommand command )
{
        // While the next command pair is Tri1, add vertices
        u32 pc = gDisplayListStack.back().addr;
        u32 * pCmdBase = (u32 *)( g_pu8RamBase + pc );

        bool tris_added = false;

        while ( command.cmd == G_GBI1_LINE3D )
        {
                u32 v3_idx   = ((command.cmd1>>24)&0xFF) / VertexStride;
                u32 v0_idx   = ((command.cmd1>>16)&0xFF) / VertexStride;
                u32 v1_idx   = ((command.cmd1>>8 )&0xFF) / VertexStride;
                u32 v2_idx   = ((command.cmd1    )&0xFF) / VertexStride;

                tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);
                tris_added |= PSPRenderer::Get()->AddTri(v2_idx, v3_idx, v0_idx);

                command.cmd0 = *pCmdBase++;
                command.cmd1 = *pCmdBase++;
                pc += 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
                if ( command.cmd == G_GBI1_LINE3D )
                {
//                        DL_PF("0x%08x: %08x %08x %-10s", pc-8, command.cmd0, command.cmd1, gInstructionName[ command.cmd ]);
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
template< int VertexStride >
void DLParser_GBI1_Tri1_T( MicroCodeCommand command )
{
        DAEDALUS_PROFILE( "DLParser_GBI1_Tri1_T" );

        // While the next command pair is Tri1, add vertices
        u32 pc = gDisplayListStack.back().addr;
        u32 * pCmdBase = (u32 *)( g_pu8RamBase + pc );

        bool tris_added = false;

        while (command.cmd == G_GBI1_TRI1)
        {
                //u32 flags = (command.cmd1>>24)&0xFF;
                // Vertex indices are multiplied by 10 for Mario64, by 2 for MarioKart
                u32 v0_idx = ((command.cmd1>>16)&0xFF) / VertexStride;
                u32 v1_idx = ((command.cmd1>>8 )&0xFF) / VertexStride;
                u32 v2_idx = ((command.cmd1    )&0xFF) / VertexStride;

                tris_added |= PSPRenderer::Get()->AddTri(v0_idx, v1_idx, v2_idx);

                command.cmd0= *pCmdBase++;
                command.cmd1= *pCmdBase++;
                pc += 8;

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
                if ( command.cmd == G_GBI1_TRI1 )
                {
  //                      DL_PF("0x%08x: %08x %08x %-10s", pc-8, command.cmd0, command.cmd1, gInstructionName[ command.cmd ]);
                }
#endif
        }

        gDisplayListStack.back().addr = pc-8;

        if (tris_added)
        {
                PSPRenderer::Get()->FlushTris();
        }
}

#endif /* GSP_MACROS_H */
