/*
Copyright (C) 2010 StrmnNrmn
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

#ifndef _UCODE_DEFS_H_
#define _UCODE_DEFS_H_


struct Instruction
{
	union 
	{
		unsigned int cmd0;
		struct 
		{
			unsigned int arg0:24;
			unsigned int cmd:8;
		};
	};
	unsigned int cmd1;
};

struct GBI1_Matrix
{
	unsigned int	len:16;
	unsigned int	projection:1;
	unsigned int	load:1;
	unsigned int	push:1;
	unsigned int	:5;
	unsigned int	cmd:8;
	unsigned int    addr;
};


struct GBI1_PopMatrix
{
	unsigned int	:24;
	unsigned int	cmd:8;
	unsigned int	projection:1;
	unsigned int	:31;
};

struct GBI2_Matrix
{
	union 
	{
		struct 
		{
			unsigned int	param:8;
			unsigned int	len:16;
			unsigned int	cmd:8;
		};
		struct 
		{
			unsigned int	nopush:1;
			unsigned int	load:1;
			unsigned int	projection:1;
			unsigned int	:5;
			unsigned int	len2:16;
			unsigned int	cmd2:8;
		};
	};
	unsigned int    addr;
};

struct GBI0_Vtx
{
	unsigned int len:16;
	unsigned int v0:4;
	unsigned int n:4;
	unsigned int cmd:8;
	unsigned int addr;
};

struct GBI1_Vtx
{
	unsigned int len:10;
	unsigned int n:6;
	unsigned int :1;
	unsigned int v0:7;
	unsigned int cmd:8;
	unsigned int addr;
};


struct GBI2_Vtx
{
	unsigned int vend:8;
	unsigned int :4;
	unsigned int n:8;
	unsigned int :4;
	unsigned int cmd:8;
	unsigned int addr;
};

struct GBI_Texture
{
	unsigned int	enable_gbi0:1;
	unsigned int	enable_gbi2:1;
	unsigned int	:6;
	unsigned int	tile:3;
	unsigned int	level:3;
	unsigned int	:10;
	unsigned int	cmd:8;
	unsigned int	scaleT:16;
	unsigned int	scaleS:16;
};

struct SetTImg
{
	unsigned int    width:12;
	unsigned int    :7;
	unsigned int    siz:2;
	unsigned int    fmt:3;
	unsigned int	cmd:8;
	unsigned int    addr;
};

struct LoadTile
{
	unsigned int	tl:12;
	unsigned int	sl:12;
	unsigned int	cmd:8;

	unsigned int	th:12;
	unsigned int	sh:12;
	unsigned int	tile:3;
	unsigned int	pad:5;
};

struct SetColor 
{
	unsigned int	prim_level:8;
	unsigned int	prim_min_level:8;
	unsigned int	pad:8;
	unsigned int	cmd:8;

	union 
	{
		unsigned int	color;
		struct 
		{
			unsigned int fillcolor:16;
			unsigned int fillcolor2:16;
		};
		struct 
		{
			unsigned int a:8;
			unsigned int b:8;
			unsigned int g:8;
			unsigned int r:8;
		};
	};
};

struct GBI1_MoveWord
{
	unsigned int	type:8;
	unsigned int	offset:16;
	unsigned int	cmd:8;
	unsigned int	value;
};

struct GBI2_MoveWord
{
	unsigned int	offset:16;
	unsigned int	type:8;
	unsigned int	cmd:8;
	unsigned int	value;
};

struct GBI2_Tri1
{
	unsigned int v0:8;
	unsigned int v1:8;
	unsigned int v2:8;
	unsigned int cmd:8;
	unsigned int pad:24;
	unsigned int flag:8;
};

struct GBI2_Tri2
{
	unsigned int :1;
	unsigned int v3:7;
	unsigned int :1;
	unsigned int v4:7;
	unsigned int :1;
	unsigned int v5:7;
	unsigned int cmd:8;
	unsigned int :1;
	unsigned int v0:7;
	unsigned int :1;
	unsigned int v1:7;
	unsigned int :1;
	unsigned int v2:7;
	unsigned int flag:8;
};


struct GBI2_Line3D
{
	unsigned int v3:8;
	unsigned int v4:8;
	unsigned int v5:8;
	unsigned int cmd:8;

	unsigned int v0:8;
	unsigned int v1:8;
	unsigned int v2:8;
	unsigned int flag:8;
};

struct GBI1_Line3D
{
	unsigned int w0;
	unsigned int v2:8;
	unsigned int v1:8;
	unsigned int v0:8;
	unsigned int v3:8;
};

struct GBI1_Tri1
{
	unsigned int w0;
	unsigned int v2:8;
	unsigned int v1:8;
	unsigned int v0:8;
	unsigned int flag:8;
};

struct GBI1_Tri2
{
	unsigned int v5:8;
	unsigned int v4:8;
	unsigned int v3:8;
	unsigned int cmd:8;

	unsigned int v2:8;
	unsigned int v1:8;
	unsigned int v0:8;
	unsigned int flag:8;
};

struct GBI1_Dlist
{
	unsigned int	:16;
	unsigned int	param:8;
	unsigned int	cmd:8;
	unsigned int    addr;
};

union MicroCodeCommand
{
	Instruction		inst;
	GBI0_Vtx		vtx0;
	GBI1_Vtx		vtx1;
	GBI2_Vtx		vtx2;
	GBI1_Matrix		mtx1;
	GBI2_Matrix		mtx2;
	GBI1_PopMatrix	popmtx;
	GBI1_Line3D		gbi1line3d;
	GBI1_Tri1		gbi1tri1;
	GBI1_Tri2		gbi1tri2;
	GBI2_Line3D		gbi2line3d;
	GBI2_Tri1		gbi2tri1;
	GBI2_Tri2		gbi2tri2;
	GBI1_MoveWord	mw1;
	GBI2_MoveWord	mw2;
	SetColor		color;
	SetTImg			img;
	GBI1_Dlist		dlist;

	u64	force_structure_alignment;
};


#endif
