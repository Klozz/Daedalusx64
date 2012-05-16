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

#include "stdafx.h"

#ifdef DAEDALUS_ENABLE_OS_HOOKS
// This file contains high level os emulation routines
#include "OS.h"
#include "OSMesgQueue.h"
#include "ultra_R4300.h"

#ifdef DUMPOSFUNCTIONS
#ifdef DAED_OS_MESSAGE_QUEUES
// For keeping track of message queues we've created:
QueueVector g_MessageQueues;
#endif
#endif

//*****************************************************************************
//
//*****************************************************************************
void OS_Reset()
{
#ifdef DUMPOSFUNCTIONS
#ifdef DAED_OS_MESSAGE_QUEUES
	g_MessageQueues.clear();
#endif
#endif
}

#ifdef DAED_OS_MESSAGE_QUEUES
//*****************************************************************************
//
//*****************************************************************************
void OS_HLE_osCreateMesgQueue(u32 queue, u32 msgBuffer, u32 msgCount)
{
	COSMesgQueue q(queue);

	const u32 addr = VAR_ADDRESS(osNullMsgQueue);

	q.SetEmptyQueue(addr);
	q.SetFullQueue(addr);
	q.SetValidCount(0);
	q.SetFirst(0);
	q.SetMsgCount(msgCount);
	q.SetMesgArray(msgBuffer);

	//DBGConsole_Msg(0, "osCreateMsgQueue(0x%08x, 0x%08x, %d)",
	//	queue, msgBuffer, msgCount);
#ifdef DUMPOSFUNCTIONS
	for ( u32 i = 0; i < g_MessageQueues.size(); i++)
	{
		if (g_MessageQueues[i] == queue)
			return;		// Already in list

	}
	g_MessageQueues.push_back(queue);
#endif
}
#endif

//*****************************************************************************
//
//*****************************************************************************
// ENTRYHI left untouched after call
u32 OS_HLE___osProbeTLB(u32 vaddr)
{
	u32 PAddr = ~0;	// Return -1 on failure

	u32 pid = gCPUState.CPUControl[C0_ENTRYHI]._u32_0 & TLBHI_PIDMASK;
	u32 vpn2 = vaddr & TLBHI_VPN2MASK;
	u32 pageMask;
	u32 entryLo;

	// Code from TLBP and TLBR

    for(u32 i = 0; i < 32; i++)
	{
		const TLBEntry & tlb = g_TLBs[i];

		if( ((tlb.hi & TLBHI_VPN2MASK) == vpn2) && ( (tlb.g) || ((tlb.hi & TLBHI_PIDMASK) == pid) ) )
		{

			// We've found the page, do TLBR
			pageMask = tlb.mask;

			pageMask += 0x2000;
			pageMask >>= 1;

			if ((vaddr & pageMask) == 0)
			{
				// Even Page (EntryLo0)
				entryLo = tlb.pfne | tlb.g;
			}
			else
			{
				// Odd Page (EntryLo1)
				entryLo = tlb.pfno | tlb.g;
			}

			pageMask--;

			// If valid is not set, then the page is invalid
			if ((entryLo & TLBLO_V) != 0)
			{
				entryLo &= TLBLO_PFNMASK;
				entryLo <<= TLBLO_PFNSHIFT;

				PAddr = entryLo + (pageMask & vaddr);
			}

			break;
		}
	}

	//DBGConsole_Msg(0, "Probe: 0x%08x -> 0x%08x", vaddr, PAddr);
	return PAddr;

}
#endif