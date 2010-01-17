#define TEST_DISABLE_MESG_FUNCS //return PATCH_RET_NOT_PROCESSED;




u32 Patch_osSetEventMesg_Mario()
{
TEST_DISABLE_MESG_FUNCS
	u32 dwEvent = gGPR[REG_a0]._u32_0;
	u32 dwQueue = gGPR[REG_a1]._u32_0;
	u32 dwMsg   = gGPR[REG_a2]._u32_0;

	/*if (dwEvent < 23)
	{
		DBGConsole_Msg(0, "osSetEventMesg(%s, 0x%08x, 0x%08x)", 
			g_szEventStrings[dwEvent], dwQueue, dwMsg);
	}
	else
	{
		DBGConsole_Msg(0, "osSetEventMesg(%d, 0x%08x, 0x%08x)", 
			dwEvent, dwQueue, dwMsg);
	}*/
		

	u32 dwP = VAR_ADDRESS(osEventMesgArray) + (dwEvent * 8);

	Write32Bits(dwP + 0x0, dwQueue);
	Write32Bits(dwP + 0x4, dwMsg);

	return PATCH_RET_JR_RA;
}

u32 Patch_osSetEventMesg_Zelda()
{
TEST_DISABLE_MESG_FUNCS

	u32 dwEvent = gGPR[REG_a0]._u32_0;
	u32 dwQueue = gGPR[REG_a1]._u32_0;
	u32 dwMsg   = gGPR[REG_a2]._u32_0;

	/*if (dwEvent < 23)
	{
		DBGConsole_Msg(0, "osSetEventMesg(%s, 0x%08x, 0x%08x)", 
			g_szEventStrings[dwEvent], dwQueue, dwMsg);
	}
	else
	{
		DBGConsole_Msg(0, "osSetEventMesg(%d, 0x%08x, 0x%08x)", 
			dwEvent, dwQueue, dwMsg);
	}*/
		

	u32 dwP = VAR_ADDRESS(osEventMesgArray) + (dwEvent * 8);

	Write32Bits(dwP + 0x0, dwQueue);
	Write32Bits(dwP + 0x4, dwMsg);

	return PATCH_RET_JR_RA;
}


u32 Patch_osCreateMesgQueue_Mario()
{
TEST_DISABLE_MESG_FUNCS

#ifdef DAED_OS_MESSAGE_QUEUES
	u32 dwQueue    = gGPR[REG_a0]._u32_0;
	u32 dwMsgBuf   = gGPR[REG_a1]._u32_0;
	u32 dwMsgCount = gGPR[REG_a2]._u32_0;

	OS_HLE_osCreateMesgQueue(dwQueue, dwMsgBuf, dwMsgCount);

	return PATCH_RET_JR_RA;
#else
	return PATCH_RET_NOT_PROCESSED;
#endif
}
// Exactly the same - just optimised slightly
u32 Patch_osCreateMesgQueue_Rugrats()
{
TEST_DISABLE_MESG_FUNCS

#ifdef DAED_OS_MESSAGE_QUEUES
	u32 dwQueue    = gGPR[REG_a0]._u32_0;
	u32 dwMsgBuf   = gGPR[REG_a1]._u32_0;
	u32 dwMsgCount = gGPR[REG_a2]._u32_0;

	OS_HLE_osCreateMesgQueue(dwQueue, dwMsgBuf, dwMsgCount);

	return PATCH_RET_JR_RA;
#else
	return PATCH_RET_NOT_PROCESSED;
#endif
}


u32 Patch_osRecvMesg()
{
TEST_DISABLE_MESG_FUNCS
	u32 dwQueue     = gGPR[REG_a0]._u32_0;
	u32 dwMsg       = gGPR[REG_a1]._u32_0;
	u32 dwBlockFlag = gGPR[REG_a2]._u32_0;

	u32 dwValidCount = Read32Bits(dwQueue + 0x8);
	u32 dwMsgCount = Read32Bits(dwQueue + 0x10);

	/*if (dwQueue == 0x80007d40)
	{
	DBGConsole_Msg(0, "Thread: 0x%08x", Read32Bits(VAR_ADDRESS(osActiveThread)));
	DBGConsole_Msg(0, "osRecvMsg(0x%08x, 0x%08x, %s) (%d/%d pending)", 
		dwQueue, dwMsg, dwBlockFlag == OS_MESG_BLOCK ? "Block" : "Don't Block",
		dwValidCount, dwMsgCount);
	}*/

	// If there are no valid messages, then we either block until 
	// one becomes available, or return immediately
	if (dwValidCount == 0)
	{
		if (dwBlockFlag == OS_MESG_NOBLOCK)
		{
			// Don't block
			gGPR[REG_v0]._s64 = (s64)(s32)~0;
			return PATCH_RET_JR_RA;
		}
		else
		{
			// We can't handle, as this would result in a yield (tricky)
			return PATCH_RET_NOT_PROCESSED0(osRecvMesg);
		}
	}

	//DBGConsole_Msg(0, "  Processing Pending");

	u32 dwFirst = Read32Bits(dwQueue + 0x0c);
	
	//Store message in pointer
	if (dwMsg != 0)
	{
		//DBGConsole_Msg(0, "  Retrieving message");
		
		u32 dwMsgBase = Read32Bits(dwQueue + 0x14);

		// Offset to first valid message
		dwMsgBase += dwFirst * 4;

		Write32Bits(dwMsg, Read32Bits(dwMsgBase));

	}


	// Point first to the next valid message
	if (dwMsgCount == 0)
	{
		DBGConsole_Msg(0, "Invalid message count");
		// We would break here!
	}
	else if (dwMsgCount == u32(~0) && dwFirst+1 == 0x80000000)
	{
		DBGConsole_Msg(0, "Invalid message count/first");
		// We would break here!
	}
	else
	{
		//DBGConsole_Msg(0, "  Generating next valid message number");
		dwFirst = (dwFirst + 1) % dwMsgCount;

		Write32Bits(dwQueue + 0x0c, dwFirst);
	}

	// Decrease the number of valid messages
	dwValidCount--;

	Write32Bits(dwQueue + 0x8, dwValidCount);

	// Start thread pending on the fullqueue
	u32 dwFullQueueThread = Read32Bits(dwQueue + 0x04);
	u32 dwNextThread = Read32Bits(dwFullQueueThread + 0x00);



	// If the first thread is not the idle thread, start it
	if (dwNextThread != 0)
	{
		//DBGConsole_Msg(0, "  Activating sleeping thread");

		// From Patch___osPopThread():
		Write32Bits(dwQueue + 0x04, dwNextThread);

		gGPR[REG_a0]._u32_0 = dwFullQueueThread;

		// FIXME - How to we set the status flag here?
		return Patch_osStartThread();
	}

	// Set success status
	gGPR[REG_v0]._u64 = 0;

	return PATCH_RET_JR_RA;
}



u32 Patch_osSendMesg()
{
TEST_DISABLE_MESG_FUNCS
	u32 dwQueue     = gGPR[REG_a0]._u32_0;
	u32 dwMsg       = gGPR[REG_a1]._u32_0;
	u32 dwBlockFlag = gGPR[REG_a2]._u32_0;

	u32 dwValidCount = Read32Bits(dwQueue + 0x8);
	u32 dwMsgCount = Read32Bits(dwQueue + 0x10);
	
	/*if (dwQueue == 0x80007d40)
	{
		DBGConsole_Msg(0, "Thread: 0x%08x", Read32Bits(VAR_ADDRESS(osActiveThread)));
	DBGConsole_Msg(0, "osSendMsg(0x%08x, 0x%08x, %s) (%d/%d pending)", 
		dwQueue, dwMsg, dwBlockFlag == OS_MESG_BLOCK ? "Block" : "Don't Block",
		dwValidCount, dwMsgCount);
	}*/

	// If the message queue is full, then we either block until 
	// space becomes available, or return immediately
	if (dwValidCount >= dwMsgCount)
	{
		if (dwBlockFlag == OS_MESG_NOBLOCK)
		{
			// Don't block
			gGPR[REG_v0]._s64 = (s64)(s32)~0;
			return PATCH_RET_JR_RA;
		}
		else
		{
			// We can't handle, as this would result in a yield (tricky)
			return PATCH_RET_NOT_PROCESSED0(osSendMesg);
		}
	}

	//DBGConsole_Msg(0, "  Processing Pending");

	u32 dwFirst = Read32Bits(dwQueue + 0x0c);
	
	// Point first to the next valid message
	if (dwMsgCount == 0)
	{
		DBGConsole_Msg(0, "Invalid message count");
		// We would break here!
	}
	else if (dwMsgCount == u32(~0) && dwFirst+dwValidCount == 0x80000000)
	{
		DBGConsole_Msg(0, "Invalid message count/first");
		// We would break here!
	}
	else
	{
		u32 dwSlot = (dwFirst + dwValidCount) % dwMsgCount;
		
		u32 dwMsgBase = Read32Bits(dwQueue + 0x14);

		// Offset to first valid message
		dwMsgBase += dwSlot * 4;

		Write32Bits(dwMsgBase, dwMsg);

	}
	
	// Increase the number of valid messages
	dwValidCount++;

	Write32Bits(dwQueue + 0x8, dwValidCount);

	// Start thread pending on the fullqueue
	u32 dwEmptyQueueThread = Read32Bits(dwQueue + 0x00);
	u32 dwNextThread = Read32Bits(dwEmptyQueueThread + 0x00);


	// If the first thread is not the idle thread, start it
	if (dwNextThread != 0)
	{
		//DBGConsole_Msg(0, "  Activating sleeping thread");

		// From Patch___osPopThread():
		Write32Bits(dwQueue + 0x00, dwNextThread);

		gGPR[REG_a0]._s64 = (s64)(s32)dwEmptyQueueThread;

		// FIXME - How to we set the status flag here?
		return Patch_osStartThread();
	}

	// Set success status
	gGPR[REG_v0]._u64 = 0;

	return PATCH_RET_JR_RA;
}
