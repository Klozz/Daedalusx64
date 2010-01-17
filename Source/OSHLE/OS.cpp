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

#ifdef DAED_OS_MESSAGE_QUEUES
// For keeping track of message queues we've created:
QueueVector g_MessageQueues;
#endif


bool OS_Reset()
{
#ifdef DAED_OS_MESSAGE_QUEUES
	g_MessageQueues.clear();
#endif

	return true;
}

#ifdef DAED_OS_MESSAGE_QUEUES
void OS_HLE_osCreateMesgQueue(u32 dwQueue, u32 dwMsgBuffer, u32 dwMsgCount)
{
	COSMesgQueue q(dwQueue);

	q.SetEmptyQueue(VAR_ADDRESS(osNullMsgQueue));
	q.SetFullQueue(VAR_ADDRESS(osNullMsgQueue));
	q.SetValidCount(0);
	q.SetFirst(0);
	q.SetMsgCount(dwMsgCount);
	q.SetMesgArray(dwMsgBuffer);

	//DBGConsole_Msg(0, "osCreateMsgQueue(0x%08x, 0x%08x, %d)",
	//	dwQueue, dwMsgBuffer, dwMsgCount);

	for ( u32 i = 0; i < g_MessageQueues.size(); i++)
	{
		if (g_MessageQueues[i] == dwQueue)
			return;		// Already in list

	}
	g_MessageQueues.push_back(dwQueue);
}
#endif

// ENTRYHI left untouched after call
u32 OS_HLE___osProbeTLB(u32 vaddr)
{
	u32 dwPAddr = ~0;	// Return -1 on failure

	u32 dwPID = gCPUState.CPUControl[C0_ENTRYHI]._u32_0 & TLBHI_PIDMASK;
	u32 dwVPN2 = vaddr & TLBHI_VPN2MASK;
	u32 dwPageMask;
	u32 dwEntryLo;
	int i;

	// Code from TLBP and TLBR

    for(i = 0; i < 32; i++)
	{
		if( ((g_TLBs[i].hi & TLBHI_VPN2MASK) == dwVPN2) &&
			(
				(g_TLBs[i].g) ||
				((g_TLBs[i].hi & TLBHI_PIDMASK) == dwPID)
			) )
		{
			// We've found the page, do TLBR
			dwPageMask = g_TLBs[i].mask;

			dwPageMask += 0x2000;
			dwPageMask >>= 1;

			if ((vaddr & dwPageMask) == 0)
			{
				// Even Page (EntryLo0)
				dwEntryLo = g_TLBs[i].pfne | g_TLBs[i].g;
			}
			else
			{
				// Odd Page (EntryLo1)
				dwEntryLo = g_TLBs[i].pfno | g_TLBs[i].g;
			}

			dwPageMask--;

			// If valid is not set, then the page is invalid
			if ((dwEntryLo & TLBLO_V) != 0)
			{
				dwEntryLo &= TLBLO_PFNMASK;
				dwEntryLo <<= TLBLO_PFNSHIFT;

				dwPAddr = dwEntryLo + (dwPageMask & vaddr);
			}

			break;
		}
	}

	//DBGConsole_Msg(0, "Probe: 0x%08x -> 0x%08x", vaddr, dwPAddr);
	return dwPAddr;

}
#endif