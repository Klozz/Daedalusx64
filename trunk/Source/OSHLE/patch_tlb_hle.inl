#define TEST_DISABLE_TLB_FUNCS DAEDALUS_PROFILE(__FUNCTION__);

u32 Patch_osMapTLB()
{
TEST_DISABLE_TLB_FUNCS
	//osMapTLB(s32, OSPageMask, void *, u32, u32, s32)
#ifndef DAEDALUS_SILENT
	u32 dwW = gGPR[REG_a0]._u32_0;
	u32 dwX = gGPR[REG_a1]._u32_0;
	u32 dwY = gGPR[REG_a2]._u32_0;
	u32 dwZ = gGPR[REG_a3]._u32_0;
	u32 dwA = Read32Bits(gGPR[REG_sp]._u32_0 + 0x10);
	u32 dwB = Read32Bits(gGPR[REG_sp]._u32_0 + 0x14);

	DBGConsole_Msg(0, "[WosMapTLB(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x)]",
		dwW,dwX,dwY,dwZ,dwA,dwB);
#endif
	return PATCH_RET_NOT_PROCESSED;
}




// ENTRYHI left untouched after call
u32 Patch___osProbeTLB()
{   
TEST_DISABLE_TLB_FUNCS	
	u32 dwVAddr = gGPR[REG_a0]._u32_0;

	gGPR[REG_v0]._s64 = OS_HLE___osProbeTLB(dwVAddr);

	//DBGConsole_Msg(0, "Probe: 0x%08x -> 0x%08x", dwVAddr, dwPAddr);

	return PATCH_RET_JR_RA;
}

u32 Patch_osVirtualToPhysical_Mario()
{
TEST_DISABLE_TLB_FUNCS
	u32 dwVAddr = gGPR[REG_a0]._u32_0;

	//DBGConsole_Msg(0, "osVirtualToPhysical(0x%08x)", (u32)gGPR[REG_a0]);

	if (IS_KSEG0(dwVAddr))
	{
		gGPR[REG_v0]._s64 = (s64)(s32)K0_TO_PHYS(dwVAddr);
	}
	else if (IS_KSEG1(dwVAddr))
	{
		gGPR[REG_v0]._s64 = (s64)(s32)K1_TO_PHYS(dwVAddr);
	}
	else
	{
		gGPR[REG_v0]._s64 = OS_HLE___osProbeTLB(dwVAddr);
	}

	return PATCH_RET_JR_RA;

}

// IDentical - just optimised
u32 Patch_osVirtualToPhysical_Rugrats()
{
TEST_DISABLE_TLB_FUNCS
	//DBGConsole_Msg(0, "osVirtualToPhysical(0x%08x)", (u32)gGPR[REG_a0]);
	return Patch_osVirtualToPhysical_Mario();
}
