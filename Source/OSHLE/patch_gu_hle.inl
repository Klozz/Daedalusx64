#define TEST_DISABLE_GU_FUNCS DAEDALUS_PROFILE(__FUNCTION__);

static const u32 s_IdentMatrixF[16] = 
{
	0x3f800000,	0x00000000, 0x00000000,	0x00000000,
	0x00000000,	0x3f800000, 0x00000000,	0x00000000,
	0x00000000, 0x00000000, 0x3f800000,	0x00000000,
	0x00000000, 0x00000000, 0x00000000,	0x3f800000
};

static const u32 s_IdentMatrixL[16] = 
{
	0x00010000,	0x00000000,
	0x00000001,	0x00000000,
	0x00000000,	0x00010000,
	0x00000000,	0x00000001,

	0x00000000, 0x00000000,
	0x00000000,	0x00000000,
	0x00000000, 0x00000000,
	0x00000000,	0x00000000
};

u32 Patch_guMtxIdentF()
{
TEST_DISABLE_GU_FUNCS
	u32 address = gGPR[REG_a0]._u32_0;

//	DBGConsole_Msg(0, "guMtxIdentF(0x%08x)", address);

	u8 * pMtxBase = (u8 *)ReadAddress(address);


	// 0x00000000 is 0.0 in IEEE fp
	// 0x3f800000 is 1.0 in IEEE fp
	memcpy(pMtxBase, s_IdentMatrixF, sizeof(s_IdentMatrixF));

	/*
	QuickWrite32Bits(pMtxBase, 0x00, 0x3f800000);
	QuickWrite32Bits(pMtxBase, 0x04, 0);
	QuickWrite32Bits(pMtxBase, 0x08, 0);
	QuickWrite32Bits(pMtxBase, 0x0c, 0);

	QuickWrite32Bits(pMtxBase, 0x10, 0);
	QuickWrite32Bits(pMtxBase, 0x14, 0x3f800000);
	QuickWrite32Bits(pMtxBase, 0x18, 0);
	QuickWrite32Bits(pMtxBase, 0x1c, 0);

	QuickWrite32Bits(pMtxBase, 0x20, 0);
	QuickWrite32Bits(pMtxBase, 0x24, 0);
	QuickWrite32Bits(pMtxBase, 0x28, 0x3f800000);
	QuickWrite32Bits(pMtxBase, 0x2c, 0);

	QuickWrite32Bits(pMtxBase, 0x30, 0);
	QuickWrite32Bits(pMtxBase, 0x34, 0);
	QuickWrite32Bits(pMtxBase, 0x38, 0);
	QuickWrite32Bits(pMtxBase, 0x3c, 0x3f800000);*/

/*
	g_dwNumMtxIdent++;
	if ((g_dwNumMtxIdent % 10000) == 0)
	{
		DBGConsole_Msg(0, "%d guMtxIdentF calls intercepted", g_dwNumMtxIdent);
	}*/


	return PATCH_RET_JR_RA;
}



u32 Patch_guMtxIdent()
{
TEST_DISABLE_GU_FUNCS
	u32 address = gGPR[REG_a0]._u32_0;

	//DBGConsole_Msg(0, "guMtxIdent(0x%08x)", address);

	u8 * pMtxBase = (u8 *)ReadAddress(address);

	// This is a lot faster than the real method, which calls
	// glMtxIdentF followed by guMtxF2L
	memcpy(pMtxBase, s_IdentMatrixL, sizeof(s_IdentMatrixL));
/*	QuickWrite32Bits(pMtxBase, 0x00, 0x00010000);
	QuickWrite32Bits(pMtxBase, 0x04, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x08, 0x00000001);
	QuickWrite32Bits(pMtxBase, 0x0c, 0x00000000);

	QuickWrite32Bits(pMtxBase, 0x10, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x14, 0x00010000);
	QuickWrite32Bits(pMtxBase, 0x18, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x1c, 0x00000001);

	QuickWrite32Bits(pMtxBase, 0x20, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x24, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x28, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x2c, 0x00000000);

	QuickWrite32Bits(pMtxBase, 0x30, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x34, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x38, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x3c, 0x00000000);
*/

/*	g_dwNumMtxIdent++;
	if ((g_dwNumMtxIdent % 10000) == 0)
	{
		DBGConsole_Msg(0, "%d guMtxIdent calls intercepted", g_dwNumMtxIdent);
	}*/

	return PATCH_RET_JR_RA;
}



u32 Patch_guTranslateF()
{
TEST_DISABLE_GU_FUNCS
	u32 address = gGPR[REG_a0]._u32_0;
	u32 sX = gGPR[REG_a1]._u32_0;
	u32 sY = gGPR[REG_a2]._u32_0;
	u32 sZ = gGPR[REG_a3]._u32_0;

	//DBGConsole_Msg(0, "guTranslateF(0x%08x, %f, %f, %f)", address, sX, sY, sZ);

	u8 * pMtxBase = (u8 *)ReadAddress(address);


	// 0x00000000 is 0.0 in IEEE fp
	// 0x3f800000 is 1.0 in IEEE fp
	QuickWrite32Bits(pMtxBase, 0x00, 0x3f800000);
	QuickWrite32Bits(pMtxBase, 0x04, 0);
	QuickWrite32Bits(pMtxBase, 0x08, 0);
	QuickWrite32Bits(pMtxBase, 0x0c, 0);

	QuickWrite32Bits(pMtxBase, 0x10, 0);
	QuickWrite32Bits(pMtxBase, 0x14, 0x3f800000);
	QuickWrite32Bits(pMtxBase, 0x18, 0);
	QuickWrite32Bits(pMtxBase, 0x1c, 0);

	QuickWrite32Bits(pMtxBase, 0x20, 0);
	QuickWrite32Bits(pMtxBase, 0x24, 0);
	QuickWrite32Bits(pMtxBase, 0x28, 0x3f800000);
	QuickWrite32Bits(pMtxBase, 0x2c, 0);

	QuickWrite32Bits(pMtxBase, 0x30, sX);
	QuickWrite32Bits(pMtxBase, 0x34, sY);
	QuickWrite32Bits(pMtxBase, 0x38, sZ);
	QuickWrite32Bits(pMtxBase, 0x3c, 0x3f800000);

/*	g_dwNumMtxTranslate++;
	if ((g_dwNumMtxTranslate % 10000) == 0)
	{
		DBGConsole_Msg(0, "%d guMtxTranslate calls intercepted", g_dwNumMtxTranslate);
	}*/
	return PATCH_RET_JR_RA;
}

u32 Patch_guTranslate()
{
TEST_DISABLE_GU_FUNCS
	u32 address = gGPR[REG_a0]._u32_0;
	/*u32 sX = gGPR[REG_a1]._u32_0;
	u32 sY = gGPR[REG_a2]._u32_0;
	u32 sZ = gGPR[REG_a3]._u32_0;*/
	f32 sX, sY, sZ;
	const f32 fScale = 65536.0f;

	memcpy(&sX, &gGPR[REG_a1]._u32_0, sizeof(f32));
	memcpy(&sY, &gGPR[REG_a2]._u32_0, sizeof(f32));
	memcpy(&sZ, &gGPR[REG_a3]._u32_0, sizeof(f32));


	//DBGConsole_Msg(0, "guTranslate(0x%08x, %f, %f, %f) ra:0x%08x",
	//	address, ToFloat(sX), ToFloat(sY), ToFloat(sZ), (u32)g_qwGPR[REG_ra]);

	u8 * pMtxBase = (u8 *)ReadAddress(address);

	u32 x = (u32)(sX * fScale);
	u32 y = (u32)(sY * fScale);
	u32 z = (u32)(sZ * fScale);

	u32 one = (u32)(1.0f * fScale);

	u32 xyhibits = (x & 0xFFFF0000) | (y >> 16);
	u32 xylobits = (x << 16) | (y & 0x0000FFFF);

	u32 z1hibits = (z & 0xFFFF0000) | (one >> 16);
	u32 z1lobits = (z << 16) | (one & 0x0000FFFF);


	// 0x00000000 is 0.0 in IEEE fp
	// 0x3f800000 is 1.0 in IEEE fp
	QuickWrite32Bits(pMtxBase, 0x00, 0x00010000);
	QuickWrite32Bits(pMtxBase, 0x04, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x08, 0x00000001);
	QuickWrite32Bits(pMtxBase, 0x0c, 0x00000000);

	QuickWrite32Bits(pMtxBase, 0x10, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x14, 0x00010000);
	QuickWrite32Bits(pMtxBase, 0x18, xyhibits);	// xy
	QuickWrite32Bits(pMtxBase, 0x1c, z1hibits);	// z1

	QuickWrite32Bits(pMtxBase, 0x20, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x24, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x28, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x2c, 0x00000000);

	QuickWrite32Bits(pMtxBase, 0x30, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x34, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x38, xylobits);	// xy
	QuickWrite32Bits(pMtxBase, 0x3c, z1lobits);	// z1

/*	g_dwNumMtxTranslate++;
	if ((g_dwNumMtxTranslate % 10000) == 0)
	{
		DBGConsole_Msg(0, "%d guMtxTranslate calls intercepted", g_dwNumMtxTranslate);
	}*/

	return PATCH_RET_JR_RA;
}

u32 Patch_guScaleF()
{
TEST_DISABLE_GU_FUNCS
	u32 address = gGPR[REG_a0]._u32_0;
	u32 sX = gGPR[REG_a1]._u32_0;
	u32 sY = gGPR[REG_a2]._u32_0;
	u32 sZ = gGPR[REG_a3]._u32_0;

	//DBGConsole_Msg(0, "guScaleF(0x%08x, %f, %f, %f)", address, sX, sY, sZ);

	u8 * pMtxBase = (u8 *)ReadAddress(address);

	// 0x00000000 is 0.0 in IEEE fp
	// 0x3f800000 is 1.0 in IEEE fp
	QuickWrite32Bits(pMtxBase, 0x00, sX);
	QuickWrite32Bits(pMtxBase, 0x04, 0);
	QuickWrite32Bits(pMtxBase, 0x08, 0);
	QuickWrite32Bits(pMtxBase, 0x0c, 0);

	QuickWrite32Bits(pMtxBase, 0x10, 0);
	QuickWrite32Bits(pMtxBase, 0x14, sY);
	QuickWrite32Bits(pMtxBase, 0x18, 0);
	QuickWrite32Bits(pMtxBase, 0x1c, 0);

	QuickWrite32Bits(pMtxBase, 0x20, 0);
	QuickWrite32Bits(pMtxBase, 0x24, 0);
	QuickWrite32Bits(pMtxBase, 0x28, sZ);
	QuickWrite32Bits(pMtxBase, 0x2c, 0);

	QuickWrite32Bits(pMtxBase, 0x30, 0);
	QuickWrite32Bits(pMtxBase, 0x34, 0);
	QuickWrite32Bits(pMtxBase, 0x38, 0);
	QuickWrite32Bits(pMtxBase, 0x3c, 0x3f800000);

/*	g_dwNumMtxScale++;
	if ((g_dwNumMtxScale % 10000) == 0)
	{
		DBGConsole_Msg(0, "%d guMtxScale calls intercepted", g_dwNumMtxScale);
	}*/

	return PATCH_RET_JR_RA;
}

u32 Patch_guScale()
{
TEST_DISABLE_GU_FUNCS
	u32 address = gGPR[REG_a0]._u32_0;
	/*u32 sX = gGPR[REG_a1]._u32_0;
	u32 sY = gGPR[REG_a2]._u32_0;
	u32 sZ = gGPR[REG_a3]._u32_0;*/
	f32 sX, sY, sZ;
	const f32 fScale = 65536.0f;

	memcpy(&sX, &gGPR[REG_a1]._u32_0, sizeof(f32));
	memcpy(&sY, &gGPR[REG_a2]._u32_0, sizeof(f32));
	memcpy(&sZ, &gGPR[REG_a3]._u32_0, sizeof(f32));

	//DBGConsole_Msg(0, "guScale(0x%08x, %f, %f, %f)", address, sX, sY, sZ);

	u8 * pMtxBase = (u8 *)ReadAddress(address);


	u32 x = (u32)(sX * fScale);
	u32 y = (u32)(sY * fScale);
	u32 z = (u32)(sZ * fScale);
	u32 zer = (u32)(0.0f);

	u32 xzhibits = (x & 0xFFFF0000) | (zer >> 16);
	u32 xzlobits = (x << 16) | (zer & 0x0000FFFF);

	u32 zyhibits = (zer & 0xFFFF0000) | (y >> 16);
	u32 zylobits = (zer << 16) | (y & 0x0000FFFF);

	u32 zzhibits = (z & 0xFFFF0000) | (zer >> 16);
	u32 zzlobits = (z << 16) | (zer & 0x0000FFFF);

	// 0x00000000 is 0.0 in IEEE fp
	// 0x3f800000 is 1.0 in IEEE fp
	QuickWrite32Bits(pMtxBase, 0x00, xzhibits);
	QuickWrite32Bits(pMtxBase, 0x04, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x08, zyhibits);
	QuickWrite32Bits(pMtxBase, 0x0c, 0x00000000);

	QuickWrite32Bits(pMtxBase, 0x10, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x14, zzhibits);
	QuickWrite32Bits(pMtxBase, 0x18, 0x00000000);	// xy
	QuickWrite32Bits(pMtxBase, 0x1c, 0x00000001);	// z1

	QuickWrite32Bits(pMtxBase, 0x20, xzlobits);
	QuickWrite32Bits(pMtxBase, 0x24, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x28, zylobits);
	QuickWrite32Bits(pMtxBase, 0x2c, 0x00000000);

	QuickWrite32Bits(pMtxBase, 0x30, 0x00000000);
	QuickWrite32Bits(pMtxBase, 0x34, zzlobits);
	QuickWrite32Bits(pMtxBase, 0x38, 0x00000000);	// xy
	QuickWrite32Bits(pMtxBase, 0x3c, 0x00000000);	// z1

/*	g_dwNumMtxScale++;
	if ((g_dwNumMtxScale % 10000) == 0)
	{
		DBGConsole_Msg(0, "%d guMtxScale calls intercepted", g_dwNumMtxScale);
	}*/



	return PATCH_RET_JR_RA;
}




u32 Patch_guMtxF2L()
{
TEST_DISABLE_GU_FUNCS
	u32 floatMtx = gGPR[REG_a0]._u32_0;
	u32 fixedMtx = gGPR[REG_a1]._u32_0;

	const f32 fScale = 65536.0f;

	u8 * pMtxFBase = (u8 *)ReadAddress(floatMtx);
	u8 * pMtxLBaseHiBits = (u8 *)ReadAddress(fixedMtx + 0x00);
	u8 * pMtxLBaseLoBits = (u8 *)ReadAddress(fixedMtx + 0x20);
	u32 FA_tmp;
	u32 FB_tmp;
	f32 FA, FB;
	u32 a, b;
	u32 hibits;
	u32 lobits;
	s32 row;

	for (row = 0; row < 4; row++)
	{
		FA_tmp = QuickRead32Bits(pMtxFBase, row*16 + 0x0);
		FB_tmp = QuickRead32Bits(pMtxFBase, row*16 + 0x4);
		
		memcpy(&FA, &FA_tmp, sizeof(f32));
		memcpy(&FB, &FB_tmp, sizeof(f32));

		// Should be TRUNC
		a = (u32)(FA * fScale);
		b = (u32)(FB * fScale);

		hibits = (a & 0xFFFF0000) | (b >> 16);
		lobits = (a << 16) | (b & 0x0000FFFF);

		QuickWrite32Bits(pMtxLBaseHiBits, row*8 + 0, hibits);
		QuickWrite32Bits(pMtxLBaseLoBits, row*8 + 0, lobits);
		
		/////
		FA_tmp = QuickRead32Bits(pMtxFBase, row*16 + 0x8);
		FB_tmp = QuickRead32Bits(pMtxFBase, row*16 + 0xc);

		memcpy(&FA, &FA_tmp, sizeof(f32));
		memcpy(&FB, &FB_tmp, sizeof(f32));

		
		// Should be TRUNC
		a = (u32)(FA * fScale);
		b = (u32)(FB * fScale);

		hibits = (a & 0xFFFF0000) | (b >> 16);
		lobits = (a << 16) | (b & 0x0000FFFF);

		QuickWrite32Bits(pMtxLBaseHiBits, row*8 + 4, hibits);
		QuickWrite32Bits(pMtxLBaseLoBits, row*8 + 4, lobits);

	}


/*	g_dwNumMtxF2L++;
	if ((g_dwNumMtxF2L % 10000) == 0)
	{
		DBGConsole_Msg(0, "%d guMtxF2L calls intercepted", g_dwNumMtxF2L);
	}*/


	return PATCH_RET_JR_RA;
}


u32 Patch_guNormalize_Mario()
{
TEST_DISABLE_GU_FUNCS
	u32 sX = gGPR[REG_a0]._u32_0;
	u32 sY = gGPR[REG_a1]._u32_0;
	u32 sZ = gGPR[REG_a2]._u32_0;

	u32 x = Read32Bits(sX);
	u32 y = Read32Bits(sY);
	u32 z = Read32Bits(sZ);

	f32 fX, fY, fZ;

/*	f32 fX = *(f32*)&x;
	f32 fY = *(f32*)&y;
	f32 fZ = *(f32*)&z;
*/
	memcpy(&fX, &x, sizeof(f32));
	memcpy(&fY, &y, sizeof(f32));
	memcpy(&fZ, &z, sizeof(f32));

	//DBGConsole_Msg(0, "guNormalize(0x%08x %f, 0x%08x %f, 0x%08x %f)",
	//	sX, fX, sY, fY, sZ, fZ);

	f32 fLenRecip = 1.0f / pspFpuSqrt((fX * fX) + (fY * fY) + (fZ * fZ));

	fX *= fLenRecip;
	fY *= fLenRecip;
	fZ *= fLenRecip;

	memcpy(&x, &fX, sizeof(u32));
	memcpy(&y, &fY, sizeof(u32));
	memcpy(&z, &fZ, sizeof(u32));


/*	g_dwNumNormalize++;
	if ((g_dwNumNormalize % 1000) == 0)
	{
		DBGConsole_Msg(0, "%d guNormalize calls intercepted", g_dwNumNormalize);
	}*/

	Write32Bits(sX, x);
	Write32Bits(sY, y);
	Write32Bits(sZ, z);



	return PATCH_RET_JR_RA;
}

// NOT the same function as guNormalise_Mario
// This take one pointer, not 3
u32 Patch_guNormalize_Rugrats()
{
TEST_DISABLE_GU_FUNCS
	u32 sX = gGPR[REG_a0]._u32_0;
	u32 sY = sX + 4;
	u32 sZ = sX + 8;

	u32 x = Read32Bits(sX);
	u32 y = Read32Bits(sY);
	u32 z = Read32Bits(sZ);

	f32 fX, fY, fZ;

	/*	f32 fX = *(f32*)&x;
		f32 fY = *(f32*)&y;
		f32 fZ = *(f32*)&z;
	*/
	memcpy(&fX, &x, sizeof(f32));
	memcpy(&fY, &y, sizeof(f32));
	memcpy(&fZ, &z, sizeof(f32));

	//DBGConsole_Msg(0, "guNormalize(0x%08x %f, 0x%08x %f, 0x%08x %f)",
	//	sX, fX, sY, fY, sZ, fZ);

	f32 fLenRecip = 1.0f / ((fX * fX) + (fY * fY) + (fZ * fZ));

	fX *= fLenRecip;
	fY *= fLenRecip;
	fZ *= fLenRecip;

	memcpy(&x, &fX, sizeof(u32));
	memcpy(&y, &fY, sizeof(u32));
	memcpy(&z, &fZ, sizeof(u32));

/*	g_dwNumNormalize++;
	if ((g_dwNumNormalize % 1000) == 0)
	{
		DBGConsole_Msg(0, "%d guNormalize calls intercepted", g_dwNumNormalize);
	}*/

	Write32Bits(sX, x);
	Write32Bits(sY, y);
	Write32Bits(sZ, z);



	return PATCH_RET_JR_RA;
}

u32 Patch_guOrthoF()
{
TEST_DISABLE_GU_FUNCS
	u32 mtx = gGPR[REG_a0]._u32_0;
	u32 L = gGPR[REG_a1]._u32_0;
	u32 R = gGPR[REG_a2]._u32_0;
	u32 B = gGPR[REG_a3]._u32_0;
	u32 T = Read32Bits(gGPR[REG_sp]._u32_0 + 0x10);
	u32 N = Read32Bits(gGPR[REG_sp]._u32_0 + 0x14);
	u32 F = Read32Bits(gGPR[REG_sp]._u32_0 + 0x18);
	u32 S = Read32Bits(gGPR[REG_sp]._u32_0 + 0x1c);


	//DBGConsole_Msg(0, "guOrthoF(0x%08x, %f, %f", mtx, ToFloat(L), ToFloat(R));
	//DBGConsole_Msg(0, "                     %f, %f", ToFloat(B), ToFloat(T));
	//DBGConsole_Msg(0, "                     %f, %f", ToFloat(N), ToFloat(F));
	//DBGConsole_Msg(0, "                     %f)", ToFloat(S));

	u8 * pMtxBase = (u8 *)ReadAddress(mtx);

/*	f32 fL = ToFloat(L);
	f32 fR = ToFloat(R);
	f32 fB = ToFloat(B);
	f32 fT = ToFloat(T);
	f32 fN = ToFloat(N);
	f32 fF = ToFloat(F);
	f32 scale = ToFloat(S);
*/

	f32 fL, fR, fB, fT, fN, fF, scale;

	memcpy(&fL, &L, sizeof(f32));
	memcpy(&fR, &R, sizeof(f32));
	memcpy(&fB, &B, sizeof(f32));
	memcpy(&fT, &T, sizeof(f32));
	memcpy(&fN, &N, sizeof(f32));
	memcpy(&fF, &F, sizeof(f32));
	memcpy(&scale, &S, sizeof(f32));


	f32 fRmL = fR - fL;
	f32 fTmB = fT - fB;
	f32 fFmN = fF - fN;
	f32 fRpL = fR + fL;
	f32 fTpB = fT + fB;
	f32 fFpN = fF + fN;

	f32 sx = (2 * scale)/fRmL;
	f32 sy = (2 * scale)/fTmB;
	f32 sz = (-2 * scale)/fFmN;

	f32 tx = -fRpL * scale / fRmL;
	f32 ty = -fTpB * scale / fTmB;
	f32 tz = -fFpN * scale / fFmN;

	// Reuse unused old variables to store int values
	memcpy(&L, &sx, sizeof(u32));
	memcpy(&R, &sy, sizeof(u32));
	memcpy(&B, &sz, sizeof(u32));
	memcpy(&T, &tx, sizeof(u32));
	memcpy(&N, &ty, sizeof(u32));
	memcpy(&F, &tz, sizeof(u32));
	memcpy(&S, &scale, sizeof(u32));


	/*
	0   2/(r-l)
	1                2/(t-b)
	2                            -2/(f-n)
	3 -(l+r)/(r-l) -(t+b)/(t-b) -(f+n)/(f-n)     1*/

	// 0x3f800000 is 1.0 in IEEE fp
	QuickWrite32Bits(pMtxBase, 0x00, L );
	QuickWrite32Bits(pMtxBase, 0x04, 0);
	QuickWrite32Bits(pMtxBase, 0x08, 0);
	QuickWrite32Bits(pMtxBase, 0x0c, 0);

	QuickWrite32Bits(pMtxBase, 0x10, 0);
	QuickWrite32Bits(pMtxBase, 0x14, R  );
	QuickWrite32Bits(pMtxBase, 0x18, 0);
	QuickWrite32Bits(pMtxBase, 0x1c, 0);

	QuickWrite32Bits(pMtxBase, 0x20, 0);
	QuickWrite32Bits(pMtxBase, 0x24, 0);
	QuickWrite32Bits(pMtxBase, 0x28, B  );
	QuickWrite32Bits(pMtxBase, 0x2c, 0);

	QuickWrite32Bits(pMtxBase, 0x30, T  );
	QuickWrite32Bits(pMtxBase, 0x34, N  );
	QuickWrite32Bits(pMtxBase, 0x38, F  );
	QuickWrite32Bits(pMtxBase, 0x3c, S  );

	return PATCH_RET_JR_RA;
}

u32 Patch_guOrtho()
{
TEST_DISABLE_GU_FUNCS
	//DBGConsole_Msg(0, "guOrtho");
	return PATCH_RET_NOT_PROCESSED;

}

u32 Patch_guRotateF()
{
TEST_DISABLE_GU_FUNCS
	/*g_dwNumMtxRotate++;
	if ((g_dwNumMtxRotate % 10000) == 0)
	{
		DBGConsole_Msg(0, "%d guRotate calls intercepted", g_dwNumMtxRotate);
	}*/

	//D3DXMatrixRotationAxis(mat, axis, angle)

	return PATCH_RET_NOT_PROCESSED;

}
