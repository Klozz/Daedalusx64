#define TEST_DISABLE_CACHE_FUNCS DAEDALUS_PROFILE(__FUNCTION__);



u32 Patch_osInvalICache_Mario()
{
TEST_DISABLE_CACHE_FUNCS
	u32 dwP = gGPR[REG_a0]._u32_0;
	u32 dwLen = gGPR[REG_a1]._u32_0;

#ifdef DAEDALUS_ENABLE_DYNAREC
	if (dwLen < 0x4000)
		CPU_InvalidateICacheRange(dwP, dwLen);
	else
		CPU_InvalidateICache();
#endif

	return PATCH_RET_JR_RA;
}

u32 Patch_osInvalICache_Rugrats()
{
	return Patch_osInvalICache_Mario();
}	


u32 Patch_osInvalDCache_Mario()
{
TEST_DISABLE_CACHE_FUNCS
	//u32 dwP = gGPR[REG_a0]._u32_0;
	//u32 dwLen = gGPR[REG_a1]._u32_0;

	//DBGConsole_Msg(0, "osInvalDCache(0x%08x, %d)", dwP, dwLen);

	return PATCH_RET_JR_RA;
}
u32 Patch_osInvalDCache_Rugrats()
{
TEST_DISABLE_CACHE_FUNCS
	//u32 dwP = gGPR[REG_a0]._u32_0;
	//u32 dwLen = gGPR[REG_a1]._u32_0;

	//DBGConsole_Msg(0, "osInvalDCache(0x%08x, %d)", dwP, dwLen);

	return PATCH_RET_JR_RA;
}	


u32 Patch_osWritebackDCache_Mario()
{
TEST_DISABLE_CACHE_FUNCS
	//u32 dwP = gGPR[REG_a0]._u32_0;
	//u32 dwLen = gGPR[REG_a1]._u32_0;

	//DBGConsole_Msg(0, "osWritebackDCache(0x%08x, %d)", dwP, dwLen);

	return PATCH_RET_JR_RA;
}
u32 Patch_osWritebackDCache_Rugrats()
{
TEST_DISABLE_CACHE_FUNCS
	//u32 dwP = gGPR[REG_a0]._u32_0;
	//u32 dwLen = gGPR[REG_a1]._u32_0;

	//DBGConsole_Msg(0, "osWritebackDCache(0x%08x, %d)", dwP, dwLen);

	return PATCH_RET_JR_RA;
}	


u32 Patch_osWritebackDCacheAll()
{
TEST_DISABLE_CACHE_FUNCS
	//DBGConsole_Msg(0, "osWritebackDCacheAll()");

	return PATCH_RET_JR_RA;
}
