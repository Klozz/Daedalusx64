/*
Copyright (C) 2010 Salvy6735

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
#include "Buttons.h"

#include <pspctrl.h>
#include <pspsdk.h>
#include <pspkernel.h>

//
/****** Wrapper for Kernel Buttons functions ******/
//

KernelButtons gKernelButtons;
//*****************************************************************************
//	Init our buttons, various stats etc
//*****************************************************************************
void InitButtons()
{
	int gGetKernelButtons = pspSdkLoadStartModule("kernelbuttons.prx", PSP_MEMORY_PARTITION_KERNEL);


	// Start our stack for either kernel or usermode buttons
	//
	gKernelButtons.mode			=  ( gGetKernelButtons >= 0 )	  ? true : false;
	gKernelButtons.style		=  ( gKernelButtons.mode == true ) ? PSP_CTRL_HOME : PSP_CTRL_SELECT;

	printf( "%s to load kernelbuttons.prx: %08X\n", 
			gKernelButtons.mode ? "Successfully" : "Failed",
			gGetKernelButtons );
}

//*****************************************************************************
//
//*****************************************************************************



