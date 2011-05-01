
#define TEST_DISABLE_AI_FUNCS //DAEDALUS_PROFILE(__FUNCTION__);
//*****************************************************************************
//
//*****************************************************************************
u32 Patch_osAiGetLength()
{
TEST_DISABLE_AI_FUNCS
	// Getting length from osAiSetNextBuffer is faster than reading it from memory
	// Also if osAiSetNextBuffer is patched, AI_LEN_REG is no longer valid
	//
	u32 len = gCurrentLength;
	// Aerogauge doesn't use osAiSetNextBuffer.. so fall back to reading from memory
	//
	if( len == 0)
	{
		len = Memory_AI_GetRegister(AI_LEN_REG);
	}

	// Hardcoding 2880 here causes Aerogauge to get 40%+ speed up, yammy!
	//
	gGPR[REG_v0]._u32_0 = len; 
	//gGPR[REG_v0]._s64 = (s64)(s32)len;

	return PATCH_RET_JR_RA;
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
u32 Patch_osAiSetNextBuffer()
{
TEST_DISABLE_AI_FUNCS
	// The addr argument points to the buffer in DRAM
	// Must be aligned too
	//
	u32 addr = gGPR[REG_a0]._u32_0 & 0xFFFFFF;
	u32 len  = gGPR[REG_a1]._u32_0;

	DAEDALUS_ASSERT( len < 32768, "Reached max DMA length (%d)",len );
	DAEDALUS_ASSERT( IsAiDeviceBusy()==0, "AI Interace is busy, need to handle!!" );
	DAEDALUS_ASSERT( (addr + len) != 0x00002000, "osAiSetNextBuffer bug ?" ); 

	//DBGConsole_Msg(0, "osAiNextBuffer() %08X len %d bytes",addr,len);
	gCurrentLength = len;
	
	if( gAudioPluginEnabled )
	{
		// If Ai interface is busy, stop the dma operation
		// Can this happen??
		/*
		if (IsAiDeviceBusy() != 0)
			gGPR[REG_v0]._u32_0 = ~0;
		else
		*/
		g_pAiPlugin->AddBufferHLE( g_pu8RamBase + addr, len );
		gGPR[REG_v0]._u32_0 = 0;
	}
	else
	{
		// Stops the DMA operation when audio is disabled
		//
		gGPR[REG_v0]._u32_0 = ~0;
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
u32 Patch___osAiDeviceBusy()
{
TEST_DISABLE_AI_FUNCS
	gGPR[REG_v0]._u32_0 = IsAiDeviceBusy();
	
	return PATCH_RET_JR_RA;
}

