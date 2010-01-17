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

#ifndef GSP_CUSTOM_H
#define GSP_CUSTOM_H

void DLParser_GBI0_Vtx_ShadowOfEmpire( MicroCodeCommand command );
void DLParser_RSP_Last_Legion_0x80( MicroCodeCommand command );
void DLParser_RSP_Last_Legion_0x00( MicroCodeCommand command );
void DLParser_TexRect_Last_Legion( MicroCodeCommand command );
void DLParser_RDPHalf_1_0xb4_GoldenEye( MicroCodeCommand command );
void DLParser_DLInMem( MicroCodeCommand command );
void DLParser_MtxDKR( MicroCodeCommand command );
void DLParser_MoveWord_DKR( MicroCodeCommand command );
void DLParser_GBI0_Vtx_DKR( MicroCodeCommand command );
void DLParser_GBI0_Vtx_WRUS( MicroCodeCommand command );
void DLParser_DmaTri( MicroCodeCommand command );
void DLParser_GBI0_Vtx_Gemini( MicroCodeCommand command );

#ifdef DAEDALUS_DEBUG_DISPLAYLIST

void DLParser_DumpVtxInfoDKR(u32 address, u32 v0_idx, u32 num_verts);
#endif

#endif /* GSP_CUSTOM_H */
