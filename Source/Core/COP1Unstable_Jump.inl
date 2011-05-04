/*
Copyright (C) 2011 StrmnNrmn
Copyright (C) 1999-2004 Joel Middendorf, <schibo@emulation64.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//*****************************************************************************
//
//*****************************************************************************
void InterpreterCheckCP1Unusable(void OpcodeAddress(u32), R4300_CALL_SIGNATURE)
{
	if(gCPUState.CPUControl[C0_SR]._u64 & SR_CU1)
	{
		OpcodeAddress(R4300_CALL_ARGUMENTS);
	}
	else
	{
		DBGConsole_Msg(0, "Thread accessing Cop1, throwing COP1 unusuable exception");
		R4300_Exception_CopUnusuable();
	}
}

//*****************************************************************************
//
//*****************************************************************************
void R4300_CALL_TYPE CU1_R4300_LWC1( R4300_CALL_SIGNATURE )
{
    InterpreterCheckCP1Unusable(&R4300_LWC1, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void R4300_CALL_TYPE CU1_R4300_LDC1( R4300_CALL_SIGNATURE )
{
    InterpreterCheckCP1Unusable(&R4300_LDC1, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
//void R4300_CALL_TYPE CU1_R4300_SWC1( R4300_CALL_SIGNATURE )
//{
//    InterpreterCheckCP1Unusable(&R4300_SWC1, R4300_CALL_ARGUMENTS);
//}
//*****************************************************************************
//
//*****************************************************************************
//void R4300_CALL_TYPE CU1_R4300_SDC1( R4300_CALL_SIGNATURE )
//{
//    InterpreterCheckCP1Unusable(&R4300_SDC1, R4300_CALL_ARGUMENTS);
//}
//-------------------------------------------------------------------------------
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_ADD(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_ADD, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_SUB(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_SUB, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_MUL(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_MUL, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_DIV(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_DIV, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_ABS(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_ABS, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_SQRT(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_SQRT, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_MOV(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_MOV, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_NEG(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_NEG, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_ROUND_L(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_ROUND_L, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_TRUNC_L(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_TRUNC_L, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_CEIL_L(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_CEIL_L, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_FLOOR_L(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_FLOOR_L, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_ROUND_W(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_ROUND_W, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_TRUNC_W(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_TRUNC_W, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_CEIL_W(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_CEIL_W, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_FLOOR_W(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_FLOOR_W, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_CVT_D(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_CVT_D, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_CVT_W(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_CVT_W, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_CVT_L(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_CVT_L, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_F(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_F, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_UN(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_UN, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_EQ(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_EQ, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_UEQ(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_UEQ, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_OLT(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_OLT, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_ULT(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_ULT, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_OLE(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_OLE, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_ULE(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_ULE, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_SF(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_SF, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_NGLE(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_NGLE, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_SEQ(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_SEQ, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_NGL(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_NGL, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_LT(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_LT, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_LE(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_LE, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_NGE(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&CU1_R4300_Cop1_S_NGE, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_S_NGT(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_S_NGT, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_MFC1(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_MFC1, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_DMFC1(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_DMFC1, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_CFC1(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_CFC1, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_MTC1(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_MTC1, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_DMTC1(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_DMTC1, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_Cop1_CTC1(R4300_CALL_SIGNATURE)
{
    InterpreterCheckCP1Unusable(&R4300_Cop1_CTC1, R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void (*CU1_COP1_S_Instruction[64]) (R4300_CALL_SIGNATURE) =
{
	CU1_R4300_Cop1_S_ADD,     CU1_R4300_Cop1_S_SUB,     CU1_R4300_Cop1_S_MUL,    CU1_R4300_Cop1_S_DIV,     CU1_R4300_Cop1_S_SQRT,    CU1_R4300_Cop1_S_ABS,     CU1_R4300_Cop1_S_MOV,    CU1_R4300_Cop1_S_NEG,
	CU1_R4300_Cop1_S_ROUND_L, CU1_R4300_Cop1_S_TRUNC_L,	CU1_R4300_Cop1_S_CEIL_L, CU1_R4300_Cop1_S_FLOOR_L, CU1_R4300_Cop1_S_ROUND_W, CU1_R4300_Cop1_S_TRUNC_W, CU1_R4300_Cop1_S_CEIL_W, CU1_R4300_Cop1_S_FLOOR_W,
	R4300_Cop1_Unk,     R4300_Cop1_Unk,     R4300_Cop1_Unk,    R4300_Cop1_Unk,     R4300_Cop1_Unk,     R4300_Cop1_Unk,     R4300_Cop1_Unk,    R4300_Cop1_Unk, 
	R4300_Cop1_Unk,     R4300_Cop1_Unk,     R4300_Cop1_Unk,    R4300_Cop1_Unk,     R4300_Cop1_Unk,     R4300_Cop1_Unk,     R4300_Cop1_Unk,    R4300_Cop1_Unk, 
	R4300_Cop1_Unk,     CU1_R4300_Cop1_S_CVT_D,   R4300_Cop1_Unk,    R4300_Cop1_Unk,     CU1_R4300_Cop1_S_CVT_W,   CU1_R4300_Cop1_S_CVT_L,   R4300_Cop1_Unk,    R4300_Cop1_Unk,
	R4300_Cop1_Unk,     R4300_Cop1_Unk,     R4300_Cop1_Unk,    R4300_Cop1_Unk,     R4300_Cop1_Unk,     R4300_Cop1_Unk,     R4300_Cop1_Unk,    R4300_Cop1_Unk, 
	CU1_R4300_Cop1_S_F,       CU1_R4300_Cop1_S_UN,      CU1_R4300_Cop1_S_EQ,     CU1_R4300_Cop1_S_UEQ,     CU1_R4300_Cop1_S_OLT,     CU1_R4300_Cop1_S_ULT,     CU1_R4300_Cop1_S_OLE,    CU1_R4300_Cop1_S_ULE,
	CU1_R4300_Cop1_S_SF,      CU1_R4300_Cop1_S_NGLE,    CU1_R4300_Cop1_S_SEQ,    CU1_R4300_Cop1_S_NGL,     CU1_R4300_Cop1_S_LT,      CU1_R4300_Cop1_S_NGE,     CU1_R4300_Cop1_S_LE,     CU1_R4300_Cop1_S_NGT
};
//*****************************************************************************
//
//*****************************************************************************
void CU1_COP1_S_instr(R4300_CALL_SIGNATURE)
{
	R4300_CALL_MAKE_OP( op_code );

	CU1_COP1_S_Instruction[op_code.cop1_funct](R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************
void (*CU1_COP1_Instruction[32]) (R4300_CALL_SIGNATURE) =
{
	CU1_R4300_Cop1_MFC1,    CU1_R4300_Cop1_DMFC1,  CU1_R4300_Cop1_CFC1, R4300_Cop1_Unk, CU1_R4300_Cop1_MTC1,   CU1_R4300_Cop1_DMTC1,  CU1_R4300_Cop1_CTC1, R4300_Cop1_Unk,
	R4300_Cop1_Unk,     R4300_Cop1_Unk,    R4300_Cop1_Unk,  R4300_Cop1_Unk, R4300_Cop1_Unk,    R4300_Cop1_Unk,    R4300_Cop1_Unk,  R4300_Cop1_Unk,
	CU1_COP1_S_instr,     R4300_Cop1_Unk,    R4300_Cop1_Unk,  R4300_Cop1_Unk, R4300_Cop1_Unk,    R4300_Cop1_Unk,    R4300_Cop1_Unk,  R4300_Cop1_Unk,
	R4300_Cop1_Unk,     R4300_Cop1_Unk,    R4300_Cop1_Unk,  R4300_Cop1_Unk, R4300_Cop1_Unk,    R4300_Cop1_Unk,    R4300_Cop1_Unk,  R4300_Cop1_Unk
};
//*****************************************************************************
//
//*****************************************************************************
void CU1_R4300_CoPro1(R4300_CALL_SIGNATURE)
{
	R4300_CALL_MAKE_OP( op_code );

	CU1_COP1_Instruction[op_code.cop1_op](R4300_CALL_ARGUMENTS);
}
//*****************************************************************************
//
//*****************************************************************************