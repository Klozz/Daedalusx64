
#define TEST_DISABLE_AI_FUNCS //return PATCH_RET_NOT_PROCESSED;

static u32 current_length = 0;
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
	// At this point if osAiSetNextBuffer was patched, AI_LEN_REG won't work, so we'll relly for it to get length
	//
	//u32	length(Memory_AI_GetRegister(AI_LEN_REG));
	//DAEDALUS_ASSERT(current_length != 0, "osAiGetLength : Logic Error");

	gGPR[REG_v0]._s64 = current_length; 

	return PATCH_RET_JR_RA;
}

//*****************************************************************************
//
//*****************************************************************************
u32 Patch_osAiSetNextBuffer()
{
TEST_DISABLE_AI_FUNCS

	// The addr argument points to the buffer in DRAM
	//
	u32 addr = gGPR[REG_a0]._u32_0 & 0xFFFFFF;
	u32 len  = gGPR[REG_a1]._u32_0;
	u32 inter=0;

	//DBGConsole_Msg(0, "osAiNextBuffer() %08X len %d bytes",addr,len);

	if(len > 32768)
	{
		DAEDALUS_ERROR("Reached max DMA length (%d)");
		len=32768;
	}

	current_length = len;

	if( gAudioPluginEnabled > APM_DISABLED )
	{
		// g_pu8RamBase might not be required
		g_pAiPlugin->AddBufferHLE( g_pu8RamBase + addr,len );
		inter = 0;
	}
	else
	{
		// Stop DMA operation
		inter = -1;
	}

	// I think is v0..
	gGPR[REG_v1]._u64 = inter;

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
