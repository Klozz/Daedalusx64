#define TEST_DISABLE_SI_FUNCS DAEDALUS_PROFILE(__FUNCTION__);


u32 Patch___osSiCreateAccessQueue()
{
TEST_DISABLE_SI_FUNCS

#ifdef DAED_OS_MESSAGE_QUEUES

	Write32Bits(VAR_ADDRESS(osSiAccessQueueCreated), 1);

	DBGConsole_Msg(0, "Creating Si Access Queue");

	OS_HLE_osCreateMesgQueue(VAR_ADDRESS(osSiAccessQueue), VAR_ADDRESS(osSiAccessQueueBuffer), 1);

	//u32 dwQueue     = (u32)gGPR[REG_a0]._u32_0;
	//u32 dwMsg       = (u32)gGPR[REG_a1]._u32_0;
	//u32 dwBlockFlag = (u32)gGPR[REG_a2]._u32_0;

	gGPR[REG_a0]._s64 = (s64)(s32)VAR_ADDRESS(osSiAccessQueue);
	gGPR[REG_a1]._s64 = 0;		// Msg value is unimportant
	gGPR[REG_a2]._s64 = (s64)(s32)OS_MESG_NOBLOCK;
	

	return Patch_osSendMesg();

	//return PATCH_RET_JR_RA;

#else

	return PATCH_RET_NOT_PROCESSED0(__osSiCreateAccessQueue);

#endif
}


u32 Patch___osSiGetAccess()
{
TEST_DISABLE_SI_FUNCS
	u32 created = Read32Bits(VAR_ADDRESS(osSiAccessQueueCreated));

	if (created == 0)
	{
		Patch___osSiCreateAccessQueue();	// Ignore return
	}
	
	gGPR[REG_a0]._s64 = (s64)(s32)VAR_ADDRESS(osSiAccessQueue);
	gGPR[REG_a1]._s64 = gGPR[REG_sp]._s64 - 4;		// Place on stack and ignore
	gGPR[REG_a2]._s64 = (s64)(s32)OS_MESG_BLOCK;
	

	return Patch_osRecvMesg();
}

u32 Patch___osSiRelAccess()
{
TEST_DISABLE_SI_FUNCS
	gGPR[REG_a0]._s64 = (s64)(s32)VAR_ADDRESS(osSiAccessQueue);
	gGPR[REG_a1]._s64 = (s64)0;		// Place on stack and ignore
	gGPR[REG_a2]._s64 = (s64)(s32)OS_MESG_NOBLOCK;
	
	return Patch_osSendMesg();
}

inline bool IsSiDeviceBusy()
{
	u32 status = Memory_SI_GetRegister( SI_STATUS_REG );

	if (status & (SI_STATUS_DMA_BUSY | SI_STATUS_RD_BUSY))
		return true;
	else
		return false;
}

u32 Patch___osSiDeviceBusy()
{
	gGPR[REG_v0]._u32_0 = IsSiDeviceBusy();
	
	return PATCH_RET_JR_RA;
}

u32 Patch___osSiRawReadIo_Mario()
{
	u32 port = gGPR[REG_a0]._u32_0;
	u32 valAddr = gGPR[REG_a1]._u32_0;

	if (IsSiDeviceBusy() != 0)
		gGPR[REG_v0]._u32_0 = -1;
	else
	{
		port |= 0xA0000000;
		Write32Bits(valAddr, Read32Bits(port));
		gGPR[REG_v0]._u32_0 = 0;
	}
	return PATCH_RET_JR_RA;
}

u32 Patch___osSiRawReadIo_Zelda()
{
	u32 port = gGPR[REG_a0]._u32_0;
	u32 valAddr = gGPR[REG_a1]._u32_0;

	if (IsSiDeviceBusy() != 0)
		gGPR[REG_v0]._u32_0 = -1;
	else
	{
		port |= 0xA0000000;
		Write32Bits(valAddr, Read32Bits(port));
		gGPR[REG_v0]._u32_0 = 0;
	}
	return PATCH_RET_JR_RA;
}


u32 Patch___osSiRawWriteIo_Mario()
{
	u32 port = gGPR[REG_a0]._u32_0;
	u32 val = gGPR[REG_a1]._u32_0;

	if (IsSiDeviceBusy() != 0)
		gGPR[REG_v0]._u32_0 = -1;
	else
	{
		port |= 0xA0000000;
		Write32Bits(port, val);
		gGPR[REG_v0]._u32_0 = 0;
	}
	return PATCH_RET_JR_RA;
}

u32 Patch___osSiRawWriteIo_Zelda()
{
	u32 port = gGPR[REG_a0]._u32_0;
	u32 val = gGPR[REG_a1]._u32_0;

	if (IsSiDeviceBusy() != 0)
		gGPR[REG_v0]._u32_0 = -1;
	else
	{
		port |= 0xA0000000;
		Write32Bits(port, val);
		gGPR[REG_v0]._u32_0 = 0;
	}
	return PATCH_RET_JR_RA;
}

u32 Patch___osSiRawStartDma_Mario()
{
	u32 RWflag = gGPR[REG_a0]._u32_0;
	u32 SIAddr = gGPR[REG_a1]._u32_0;

	u32 PAddr = ConvertToPhysics(SIAddr);
	if (IsSiDeviceBusy())
	{
		gGPR[REG_v0]._u32_0 = ~0;
	}
	else
	{
		Memory_SI_SetRegister( SI_DRAM_ADDR_REG, PAddr);
	
		u32 flag = (RWflag == OS_READ) ? SI_PIF_ADDR_RD64B_REG : SI_PIF_ADDR_WR64B_REG;

		Write32Bits(flag | 0xA0000000, 0);

		gGPR[REG_v0]._u32_0 = 0;
	}

	return PATCH_RET_JR_RA;
}

u32 Patch___osSiRawStartDma_Rugrats()
{
	return Patch___osSiRawStartDma_Mario();
}
