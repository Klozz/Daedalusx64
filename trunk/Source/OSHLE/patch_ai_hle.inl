
#define TEST_DISABLE_AI_FUNCS //return PATCH_RET_NOT_PROCESSED;



u32 Patch_osAiGetLength()
{
TEST_DISABLE_AI_FUNCS
	return PATCH_RET_NOT_PROCESSED;
	u32 dwLen = Read32Bits(PHYS_TO_K1(AI_LEN_REG));

	//DBGConsole_Msg(0, "osAiGetLength()");

	gGPR[REG_v0]._s64 = (s64)(s32)dwLen;
	return PATCH_RET_JR_RA;
}


u32 Patch_osAiSetNextBuffer()
{
TEST_DISABLE_AI_FUNCS
	return PATCH_RET_NOT_PROCESSED;
	//u32 dwBuffer = gGPR[REG_a0]._u32_0;
	//u32 dwLen = gGPR[REG_a1]._u32_0;

	//DBGConsole_Msg(0, "osAiSetNextBuffer(0x%08x, %d (0x%04x))", dwBuffer, dwLen, dwLen);

	gGPR[REG_v1]._u64 = 0;

	return PATCH_RET_JR_RA;
}

////// FIXME: Not implemented fully
u32 Patch_osAiSetFrequency()
{
TEST_DISABLE_AI_FUNCS
	return PATCH_RET_NOT_PROCESSED;

#ifndef DAEDALUS_SILENT
	u32 dwFreq = gGPR[REG_a0]._u32_0;
	DBGConsole_Msg(0, "osAiSetFrequency(%d)", dwFreq);
#endif

	// What does this need setting to?
	gGPR[REG_v1]._u64 = 0;

	return PATCH_RET_JR_RA;
}
