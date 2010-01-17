#define TEST_DISABLE_TIMER_FUNCS DAEDALUS_PROFILE(__FUNCTION__);

u32 Patch___osSetTimerIntr()
{
TEST_DISABLE_TIMER_FUNCS
	s64 qwSum;
	s64 qwCount;
	s64 qwTimeLo = (s64)gGPR[REG_a1]._s32_0;
	//s64 qwTimeHi = (s64)(s32)gGPR[REG_a0];

	qwCount = (s64)gCPUState.CPUControl[C0_COUNT]._s32_0;	

	Write32Bits(VAR_ADDRESS(osSystemLastCount), (u32)qwCount);	

	qwSum = (s64)(s32)((s32)qwCount + (s32)qwTimeLo);

	CPU_SetCompare(qwSum);

	return PATCH_RET_JR_RA;	
}


u32 Patch___osInsertTimer()
{
TEST_DISABLE_TIMER_FUNCS
	u32 dwNewTimer = gGPR[REG_a0]._u32_0;

	u64 qwNewValue = Read64Bits(dwNewTimer + 0x10);	// Check ordering is correct?!

	
	u32 dwTopTimer = Read32Bits(VAR_ADDRESS(osTopTimer));
	u32 dwInsertTimer;
	u64 qwInsertValue;

	/*
	u64 qwNewInterval = Read64Bits(dwNewTimer + 0x08);
	DBGConsole_Msg(0, "osInsertTimer(0x%08x)", dwNewTimer);
	DBGConsole_Msg(0, "  Timer->value = 0x%08x%08x", (u32)(qwNewValue>>32), (u32)qwNewValue);
	DBGConsole_Msg(0, "  Timer->interval = 0x%08x%08x", (u32)(qwNewInterval>>32), (u32)qwNewInterval);
	*/
	
	
	dwInsertTimer = Read32Bits(dwTopTimer + 0x00);	// Read next
	qwInsertValue = Read64Bits(dwInsertTimer + 0x10);
	
	if ( dwInsertTimer == 0 )
	{
		// What gives? 
		DBGConsole_Msg( 0, "[W__osInsertTimer with NULL insert timer" );

		// We can quit, because we've not written anything
		return PATCH_RET_NOT_PROCESSED0(__osInsertTimer);
	}

	while (qwInsertValue < qwNewValue)
	{
		// Decrease by the pause for this timer
		qwNewValue -= qwInsertValue;

		dwInsertTimer = Read32Bits(dwInsertTimer + 0x0);	// Read next timer
		qwInsertValue = Read64Bits(dwInsertTimer + 0x10);

		if (dwInsertTimer == dwTopTimer)	// At the end of the list?
			break;
	}

	/// Save the modified time value
	Write64Bits(dwNewTimer + 0x10, qwNewValue);

	// Inserting before dwInsertTimer

	// Modify dwInsertTimer's values if this is not the sentinel node
	if (dwInsertTimer != dwTopTimer)
	{
		qwInsertValue -= qwNewValue;

		Write64Bits(dwInsertTimer + 0x10, qwInsertValue);
	}

	// pNewTimer->next = pInsertTimer
	Write32Bits(dwNewTimer + 0x00, dwInsertTimer);

	// pNewTimer->prev = pInsertTimer->prev
	u32 dwInsertTimerPrev = Read32Bits(dwInsertTimer + 0x04);
	Write32Bits(dwNewTimer + 0x04, dwInsertTimerPrev);

	// pInsertTimer->prev->next = pNewTimer
	Write32Bits(dwInsertTimerPrev + 0x00, dwNewTimer);

	// pInsertTimer->prev = pNewTimer
	Write32Bits(dwInsertTimer + 0x04, dwNewTimer);

	gGPR[REG_v0]._s64 = (s64)(s32)(qwNewValue >> 32);
	gGPR[REG_v1]._s64 = (s64)(s32)((u32)qwNewValue);
	
	return PATCH_RET_JR_RA;
}

u32 Patch___osTimerServicesInit_Mario()
{
TEST_DISABLE_TIMER_FUNCS
	DBGConsole_Msg(0, "Initialising Timer Services");

	Write32Bits(VAR_ADDRESS(osSystemTimeLo), 0);
	Write32Bits(VAR_ADDRESS(osSystemTimeHi), 0);

	Write32Bits(VAR_ADDRESS(osSystemCount), 0);
	Write32Bits(VAR_ADDRESS(osFrameCount), 0);

	u32 dwTimer = Read32Bits(VAR_ADDRESS(osTopTimer));

	// Make list empty
	Write32Bits(dwTimer + offsetof(OSTimer, next), dwTimer);
	Write32Bits(dwTimer + offsetof(OSTimer, prev), dwTimer);

	Write64Bits(dwTimer + offsetof(OSTimer, interval), 0);
	Write64Bits(dwTimer + offsetof(OSTimer, value), 0);
	Write64Bits(dwTimer + offsetof(OSTimer, mq), 0);
	Write64Bits(dwTimer + offsetof(OSTimer, msg), 0);

	return PATCH_RET_JR_RA;
}

// Same as above, but optimised
u32 Patch___osTimerServicesInit_Rugrats()
{
TEST_DISABLE_TIMER_FUNCS
	return Patch___osTimerServicesInit_Mario();
}



u32 Patch_osSetTime()
{
TEST_DISABLE_TIMER_FUNCS
	u32 dwTimeHi = gGPR[REG_a0]._u32_0;
	u32 dwTimeLo = gGPR[REG_a1]._u32_0;

	//DBGConsole_Msg(0, "osSetTime(0x%08x%08x)", dwTimeHi, dwTimeLo);

	Write32Bits(VAR_ADDRESS(osSystemTimeLo), dwTimeLo);
	Write32Bits(VAR_ADDRESS(osSystemTimeHi), dwTimeHi);

	return PATCH_RET_JR_RA;
}

u32 Patch_osGetTime()
{
TEST_DISABLE_TIMER_FUNCS
	u32 dwCount;
	u32 dwLastCount;
	u32 dwTimeLo;
	u32 dwTimeHi;

	dwCount = gCPUState.CPUControl[C0_COUNT]._u32_0;
	dwLastCount = Read32Bits(VAR_ADDRESS(osSystemCount));
	
	dwTimeHi = Read32Bits(VAR_ADDRESS(osSystemTimeHi));
	dwTimeLo = Read32Bits(VAR_ADDRESS(osSystemTimeLo));
	
	dwTimeLo += dwCount - dwLastCount;		// Increase by elapsed time
	
	if (dwLastCount > dwCount)				// If an overflow has occurred, increase top timer
		dwTimeHi++;

	gGPR[REG_v0]._s64 = (s64)(s32)dwTimeHi;
	gGPR[REG_v1]._s64 = (s64)(s32)dwTimeLo;

	
	return PATCH_RET_JR_RA;
	
}

u32 Patch_osSetTimer()
{
	return PATCH_RET_NOT_PROCESSED;
}
