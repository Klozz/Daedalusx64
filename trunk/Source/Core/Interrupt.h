/*
Copyright (C) 2001 StrmnNrmn

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

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

enum ETLBExceptionReason
{
	EXCEPTION_TLB_REFILL_LOAD,
	EXCEPTION_TLB_REFILL_STORE,
	EXCEPTION_TLB_INVALID_LOAD,
	EXCEPTION_TLB_INVALID_STORE
};

void R4300_JumpToInterruptVector(u32 exception_vector);

void R4300_Exception_Break();
void R4300_Exception_Syscall();
void R4300_Exception_FP();
void R4300_Exception_CopUnusuable();
void R4300_Exception_TLB( u32 virtual_address, ETLBExceptionReason reason );

void R4300_Interrupt_UpdateCause3();		// Update the CAUSE_IP3 value after MI_INTR_MASK_REG or MI_INTR_REG changes
void R4300_Interrupt_CheckPostponed();

void R4300_Handle_Exception();
void R4300_Handle_Interrupt();

extern u32 gNumExceptions;
extern u32 gNumInterrupts;



#endif
