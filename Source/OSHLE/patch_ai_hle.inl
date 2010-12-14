
#define TEST_DISABLE_AI_FUNCS //return PATCH_RET_NOT_PROCESSED;

//*****************************************************************************
//
//*****************************************************************************
u32 Patch_osAiGetLength()
{
TEST_DISABLE_AI_FUNCS
	//return PATCH_RET_NOT_PROCESSED;
	//u32 len = Read32Bits(PHYS_TO_K1(AI_LEN_REG));

	//DBGConsole_Msg(0, "osAiGetLength()");

	//gGPR[REG_v0]._s64 = (s64)(s32)len;

	// This is faster than reading the length from memory
	// Todo get length from osAiSetNextBuffer, should be the fastest way

	u32	length(Memory_AI_GetRegister(AI_LEN_REG));

	gGPR[REG_v0]._s64 = length; 

	return PATCH_RET_JR_RA;
}

//*****************************************************************************
//
//*****************************************************************************
//ToDo : Implement correctly
//
u32 Patch_osAiSetNextBuffer()
{
TEST_DISABLE_AI_FUNCS
	return PATCH_RET_NOT_PROCESSED;
	//u32 buffer = gGPR[REG_a0]._u32_0;
	//u32 len = gGPR[REG_a1]._u32_0;

	//DBGConsole_Msg(0, "osAiSetNextBuffer(0x%08x, %d (0x%04x))", buffer, len, len);

	//gGPR[REG_v1]._u64 =  0;

	return PATCH_RET_JR_RA;
}
//*****************************************************************************
//
//*****************************************************************************
////// FIXME: Not implemented fully
u32 Patch_osAiSetFrequency()
{
TEST_DISABLE_AI_FUNCS
	return PATCH_RET_NOT_PROCESSED;

#ifndef DAEDALUS_SILENT
	u32 freg = gGPR[REG_a0]._u32_0;
	DBGConsole_Msg(0, "osAiSetFrequency(%d)", freg);
#endif

	// What does this need setting to?
	gGPR[REG_v1]._u64 = 0;

	return PATCH_RET_JR_RA;
}
