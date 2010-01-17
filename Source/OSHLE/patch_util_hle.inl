#define TEST_DISABLE_UTIL_FUNCS //DAEDALUS_PROFILE(__FUNCTION__);

u32 Patch___osAtomicDec()
{
TEST_DISABLE_UTIL_FUNCS
	DBGConsole_Msg(0, "osAtomicDec");

	u32 dwP = gGPR[REG_a0]._u32_0;
	u32 dwValue = Read32Bits(dwP);
	u32 dwRetval;

	if (dwValue != 0)
	{
		Write32Bits(dwP, dwValue - 1);
		dwRetval = 1;
	}
	else
	{
		dwRetval = 0;
	}
	
	gGPR[REG_v0]._u32_0 = dwRetval;

	return PATCH_RET_JR_RA;
}



u32 Patch_memcpy()
{
TEST_DISABLE_UTIL_FUNCS
	u32 dwDst = gGPR[REG_a0]._u32_0;
	u32 dwSrc = gGPR[REG_a1]._u32_0;
	u32 dwLen = gGPR[REG_a2]._u32_0;
	u32 i;

	//DBGConsole_Msg(0, "memcpy(0x%08x, 0x%08x, %d)", dwDst, dwSrc, dwLen);

	for (i = 0; i < dwLen; i++)
	{
		Write8Bits(dwDst + i,  Read8Bits(dwSrc + i));
	}

	// return value of dest
	gGPR[REG_v0] = gGPR[REG_a0];	

	return PATCH_RET_JR_RA;
}

u32 Patch_strlen()
{
TEST_DISABLE_UTIL_FUNCS
	u32 i;
	u32 dwString = gGPR[REG_a0]._u32_0;

	
	for (i = 0; Read8Bits(dwString+i) != 0; i++);

	gGPR[REG_v0]._s64 = (s64)(s32)i;

	return PATCH_RET_JR_RA;

}


u32 Patch_strchr()
{
TEST_DISABLE_UTIL_FUNCS
	u32 dwString = gGPR[REG_a0]._u32_0;
	u8 dwMatchChar = (u8)(gGPR[REG_a1]._u32_0 & 0xFF);
	u32 dwMatchAddr = 0;
	u8 dwSrcChar;
	u32 i;

	for (i = 0; ; i++)
	{
		dwSrcChar = Read8Bits(dwString + i);

		if (dwSrcChar == dwMatchChar)
		{
			dwMatchAddr = dwString + i;
			break;
		}

		if (dwSrcChar == 0)
		{
			dwMatchAddr = 0;
			break;
		}
	}


	gGPR[REG_v0]._s64 = (s64)(s32)dwMatchAddr;

	return PATCH_RET_JR_RA;
}

u32 Patch_strcmp()
{
	u32 i;
	u32 dwA = gGPR[REG_a0]._u32_0;
	u32 dwB = gGPR[REG_a1]._u32_0;
	u32 dwLen = gGPR[REG_a2]._u32_0;
	u8 A, B;

	DBGConsole_Msg(0, "strcmp(%s,%s,%d)", dwA, dwB, dwLen);

	for (i = 0; (A = Read8Bits(dwA+i)) != 0 && i < dwLen; i++)
	{
		B = Read8Bits(dwB + i);
		if ( A != B)
			break;
	}
	
	if (i == dwLen || (A == 0 && Read8Bits(dwB + i) == 0))
		i = 0;

	gGPR[REG_v0]._s64 = (s64)(s32)i;

	return PATCH_RET_JR_RA;
}



/*void * bcopy(const void * src, void * dst, len) */
// Note different order to src/dst than memcpy!
u32 Patch_bcopy()
{
TEST_DISABLE_UTIL_FUNCS
	u32 dwSrc = gGPR[REG_a0]._u32_0;
	u32 dwDst = gGPR[REG_a1]._u32_0;
	u32 dwLen = gGPR[REG_a2]._u32_0;

	// Set return val here (return dest)
	gGPR[REG_v0] = gGPR[REG_a1];

	if (dwLen == 0)
		return PATCH_RET_JR_RA;

	if (dwSrc == dwDst)
		return PATCH_RET_JR_RA;

	//DBGConsole_Msg(0, "bcopy(0x%08x,0x%08x,%d)", dwSrc, dwDst, dwLen);

	if (dwDst > dwSrc && dwDst < dwSrc + dwLen)
	{
		for (int i = dwLen - 1; i >= 0; i--)
		{
			Write8Bits(dwDst + i,  Read8Bits(dwSrc + i));
		}
	}
	else
	{
		for (u32 i = 0; i < dwLen; i++)
		{
			Write8Bits(dwDst + i,  Read8Bits(dwSrc + i));
		}
	}

	gGPR[REG_v0]._s64 = 0;

	return PATCH_RET_JR_RA;
}


// By Jun Su
u32 Patch_bzero() 
{ 
	u32 dwDst = gGPR[REG_a0]._u32_0; 
	u32 dwLen = gGPR[REG_a1]._u32_0; 
	
	// Assume we will only access RAM range
	void *pDst = ReadAddress(dwDst);
	memset(pDst, 0, dwLen);
	
	// return value of dest 
	gGPR[REG_v0] = gGPR[REG_a0]; 
	
	return PATCH_RET_JR_RA; 
} 
