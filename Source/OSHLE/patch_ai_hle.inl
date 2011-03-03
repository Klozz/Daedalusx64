
#define TEST_DISABLE_AI_FUNCS //return PATCH_RET_NOT_PROCESSED;

//*****************************************************************************
//
//*****************************************************************************
u32 Patch_osAiGetLength()
{
TEST_DISABLE_AI_FUNCS
	// Getting length from osAiSetNextBuffer is faster than reading it from memory
	// Also if osAiSetNextBuffer is patched, AI_LEN_REG is no longer valid
	// Aerogauge doesn't use osAiSetNextBuffer.. so fall back to reading from memory
	//
	u32 len = gCurrentLength;
	if( len == 0 )
	{
		len = Memory_AI_GetRegister(AI_LEN_REG);
	}
	//
	// Note : Aerogage length is 1408
	//
	// Aerogauge speed hack, we should be able to apply this for other roms too soon
	// overflow length this causes to kill completely sound and get 40%+ speed up
	//
#if 0
	gGPR[REG_v0]._u32_0 = 2880; 
#else
	gGPR[REG_v0]._u32_0 = len; 
#endif
	//gGPR[REG_v0]._s64 = (s64)(s32)len;

	return PATCH_RET_JR_RA;
}


//*****************************************************************************
//
//*****************************************************************************
u32 Patch_osAiSetNextBuffer()
{
TEST_DISABLE_AI_FUNCS

	if( gAudioPluginEnabled )
	{
		// The addr argument points to the buffer in DRAM
		// Must be aligned too
		//
		u32 addr = gGPR[REG_a0]._u32_0 & 0xFFFFFF;
		u32 len  = gGPR[REG_a1]._u32_0;

		DAEDALUS_ASSERT( len < 32768, "Reached max DMA length (%d)",len );
		
		//DBGConsole_Msg(0, "osAiNextBuffer() %08X len %d bytes",addr,len);

		gCurrentLength = len;
		g_pAiPlugin->AddBufferHLE( g_pu8RamBase + addr,len );

		gGPR[REG_v0]._u32_0 = 0;
	}
	else
	{
		gGPR[REG_v0]._u32_0 = -1;	// Stop DMA operation
	}
	// I think is v0..	
	//gGPR[REG_v1]._u64 = inter;

	return PATCH_RET_JR_RA;
}

//*****************************************************************************
//
//*****************************************************************************
////// FIXME: Not implemented fully, we are missing it from the symbol table :(
u32 Patch_osAiSetFrequency()
{
TEST_DISABLE_AI_FUNCS
	return PATCH_RET_NOT_PROCESSED;

	//u32 freg = gGPR[REG_a0]._u32_0;
	
	//DBGConsole_Msg(0, "osAiSetFrequency(%d)", freg);
	//gGPR[REG_v1]._u64 = freg;

	//return PATCH_RET_JR_RA;
}

//*****************************************************************************
//
//*****************************************************************************
inline bool IsAiDeviceBusy()
{
	u32 status = Memory_AI_GetRegister( AI_STATUS_REG );

	if (status & (AI_STATUS_DMA_BUSY | AI_STATUS_FIFO_FULL))
		return true;
	else
		return false;
}
//*****************************************************************************
//
//*****************************************************************************
u32 Patch___osAiDeviceBusy()
{
	gGPR[REG_v0]._u32_0 = IsAiDeviceBusy();
	
	return PATCH_RET_JR_RA;
}

