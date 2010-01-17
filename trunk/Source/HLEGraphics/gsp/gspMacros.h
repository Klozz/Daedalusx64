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

// gSP1Quadrangle
// gSP1Triangle			- DLParser_GBI1_Tri1_T/DLParser_GBI2_Tri1
// gSP2Triangles		- DLParser_GBI0_Tri2/DLParser_GBI2_Tri2/DLParser_GBI1_Tri2_T
// gSPBgRect1Cyc
// gSPBgRectCopy
// gSPBranchLessZ		- DLParser_GBI1_BranchZ
// gSPBranchLessZrg
// gSPBranchList
// gSPClearGeometryMode		- DLParser_GBI1_ClearGeometryMode
// gSPClipRatio
// gSPCullDisplayList		- DLParser_GBI1_CullDL/DLParser_GBI2_CullDL
// gSPDIsplayList		- DLParser_GBI1_DL/DLParser_GBI2_DL
// gSPEndDIsplayList		- DLParser_GBI1_EndDL/DLParser_GBI2_EndDL
// gSPFogPosition
// gSPForceMatrix
// gSPGeometryMode		- DLParser_GBI2_GeometryMode
// gSPInsertMatrix
// gSPLight
// gSPLightColor
// gSPLine3D			- DLParser_GBI2_Line3D/DLParser_GBI1_Line3D_T
// gSPLineW3D
// gSPLoadUcode			- DLParser_GBI1_LoadUCode
// gSPLoadUcodeL
// gSPLookAt
// gSPLookAtX
// gSPLookAtY
// gSPMatrix			- DLParser_GBI1_Mtx/DLParser_GBI2_Mtx
// gSPModifyVertex		- DLParser_GBI1_ModifyVtx
// gSPNearClip
// gSPNumLights
// gSPObjLoadTxRect
// gSPObjLoadTxRectR
// gSPObjLoadTxSprite
// gSPObjLoadTxtr
// gSPObjMatrix
// gSPObjRectangle
// gSPObjRectangleR
// gSPObjRenderMode
// gSPObjSprite
// gSPObjSubMatrix
// gSPPerspNormalize
// gSPPopMatrix			- DLParser_GBI1_PopMtx/DLParser_GBI2_PopMtx
// gSPScisTextureRectangle
// gSPSegment
// gSPSelectBranchDL
// gSPSelectDL
// gSPSetGeometryMode		- DLParser_GBI1_SetGeometryMode
// gSPSetLights
// gSPSetOtherMode		- DLParser_GBI1_SetOtherModeL/DLParser_GBI1_SetOtherModeH/DLParser_GBI2_SetOtherModeL/DLParser_GBI2_SetOtherModeH
// gSPSetStatus
// gSPSprite2DBase		- At gspSprite2D.h
// gSPSprite2DDraw		- At gspSprite2D.h
// gSPSprite2DScaleFlip		- At gspSprite2D.h
// gSPTexture			- DLParser_GBI1_Texture/DLParser_GBI2_Texture
// gSPTextureRectangle
// gSPTextureRectangleFlip
// gSPVertex			- DLParser_GBI0_Vtx/DLParser_GBI1_Vtx/DLParser_GBI2_Vtx
// gSPViewport
// gSPZGetMtx
// gSPZGetUMem
// gSPZLight
// gSPZLightMaterial
// gSPZLinkSubDL
// gSPZMixS16
// gSPZMixS8
// gSPZMixU8
// gSPZMtxCat
// gSPZMtxTrnsp3x3
// gSPZMultMPMtx
// gSPZPerspNormalize
// gSPZRdpCmd
// gSPZSegment
// gSPZSendMessage
// gSPZSetAmbient
// gSPZSetDefuse
// gSPZSetLookAt
// gSPZSetMtx
// gSPZSetSubDL
// gSPZSetUMem
// gSPZViewPort
// gSPZWaitSignal
// gSPZXfmLights

//***************************************************************************** 
// Used by
//***************************************************************************** 
// NOT CHECKED 
/* 
#define G_ZBUFFER               0x00000001 
#define G_TEXTURE_ENABLE        0x00000002      // Microcode use only  
#define G_SHADE                 0x00000004      // enable Gouraud interp 
#define G_SHADING_SMOOTH        0x00000200      // flat or smooth shaded 
#define G_CULL_FRONT            0x00001000 
#define G_CULL_BACK             0x00002000 
#define G_CULL_BOTH             0x00003000      // To make code cleaner 
#define G_FOG                   0x00010000 
#define G_LIGHTING              0x00020000 
#define G_TEXTURE_GEN           0x00040000 
#define G_TEXTURE_GEN_LINEAR    0x00080000*/ 
 
#define G_ZELDA_ZBUFFER         G_ZBUFFER               // Guess 
#define G_ZELDA_CULL_BACK       /*G_CULL_FRONT */ 0x00000200 
#define G_ZELDA_CULL_FRONT      /*G_CULL_BACK  */ 0x00000400 
#define G_ZELDA_FOG             G_FOG 
#define G_ZELDA_LIGHTING        G_LIGHTING 
#define G_ZELDA_TEXTURE_GEN     G_TEXTURE_GEN 
#define G_ZELDA_SHADING_FLAT    G_TEXTURE_GEN_LINEAR 
//#define G_ZELDA_SHADE         0x00080000 
#define G_CLIPPING              0x00800000


void DLParser_GBI1_CullDL				( MicroCodeCommand command );
void DLParser_GBI2_CullDL				( MicroCodeCommand command );
void DLParser_GBI1_DL					( MicroCodeCommand command );
void DLParser_GBI2_DL					( MicroCodeCommand command );
void DLParser_GBI1_EndDL				( MicroCodeCommand command );
void DLParser_GBI2_EndDL				( MicroCodeCommand command );

void DLParser_GBI1_BranchZ				( MicroCodeCommand command );

void DLParser_GBI1_LoadUCode				( MicroCodeCommand command );

void DLParser_GBI1_SetGeometryMode			( MicroCodeCommand command );
void DLParser_GBI1_ClearGeometryMode			( MicroCodeCommand command );
void DLParser_GBI2_GeometryMode				( MicroCodeCommand command );

void DLParser_GBI1_SetOtherModeL			( MicroCodeCommand command );
void DLParser_GBI1_SetOtherModeH			( MicroCodeCommand command );
void DLParser_GBI2_SetOtherModeL			( MicroCodeCommand command );
void DLParser_GBI2_SetOtherModeH			( MicroCodeCommand command );

void DLParser_GBI1_Texture				( MicroCodeCommand command );
void DLParser_GBI2_Texture				( MicroCodeCommand command );

void DLParser_GBI0_Vtx					( MicroCodeCommand command );
void DLParser_GBI1_Vtx					( MicroCodeCommand command );
void DLParser_GBI2_Vtx					( MicroCodeCommand command );
void DLParser_GBI1_ModifyVtx				( MicroCodeCommand command );
//void DLParser_GBI2_ModifyVtx				( MicroCodeCommand command );

void DLParser_GBI1_Mtx					( MicroCodeCommand command );
void DLParser_GBI2_Mtx					( MicroCodeCommand command );

void DLParser_GBI1_PopMtx				( MicroCodeCommand command );
void DLParser_GBI2_PopMtx				( MicroCodeCommand command );

void DLParser_GBI2_Line3D				( MicroCodeCommand command );

void DLParser_GBI2_Tri1					( MicroCodeCommand command );
void DLParser_GBI0_Tri2					( MicroCodeCommand command );
void DLParser_GBI2_Tri2					( MicroCodeCommand command );

// For some reason these have to be declared here.
// Local Functions
//static void DLParser_InitGeometryMode();

//*****************************************************************************
/* Mariokart etc*/
//*****************************************************************************
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

                if ( command.cmd == G_GBI1_TRI2 )
                {
//                        DL_PF("0x%08x: %08x %08x %-10s", pc-8, command.cmd0, command.cmd1, gInstructionName[ command.cmd ]);
                }
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

                if ( command.cmd == G_GBI1_LINE3D )
                {
//                        DL_PF("0x%08x: %08x %08x %-10s", pc-8, command.cmd0, command.cmd1, gInstructionName[ command.cmd ]);
                }
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

                if ( command.cmd == G_GBI1_TRI1 )
                {
  //                      DL_PF("0x%08x: %08x %08x %-10s", pc-8, command.cmd0, command.cmd1, gInstructionName[ command.cmd ]);
                }
        }

        gDisplayListStack.back().addr = pc-8;

        if (tris_added)
        {
                PSPRenderer::Get()->FlushTris();
        }
}

#endif /* GSP_MACROS_H */
