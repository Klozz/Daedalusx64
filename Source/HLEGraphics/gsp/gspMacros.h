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

void DLParser_GBI1_Tri1( MicroCodeCommand command );
void DLParser_GBI1_Tri2( MicroCodeCommand command );
void DLParser_GBI1_Line3D( MicroCodeCommand command );


#endif /* GSP_MACROS_H */
