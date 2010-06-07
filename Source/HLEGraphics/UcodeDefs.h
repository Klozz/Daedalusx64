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


#endif
