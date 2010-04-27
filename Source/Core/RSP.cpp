/*
Copyright (C) 2001-2010 StrmnNrmn

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

//
// Important Note:
//	Various chunks of this code/data are derived from Zilmar's RSP plugin,
//	which has also been used as a reference for fixing bugs in original
//	code. Many thanks to Zilmar for releasing the source to this!
//

// RSP Processor stuff
#include "stdafx.h"

#include "CPU.h"
#include "Interrupt.h"
#include "Memory.h"
#include "Registers.h"
#include "R4300.h"
#include "RSP.h"

#include "Debug/DBGConsole.h"
#include "Debug/DebugLog.h"

#include "OSHLE/ultra_rcp.h"
#include "OSHLE/ultra_R4300.h"

#define RSP_ASSERT_Q( e )			DAEDALUS_ASSERT_Q( e )

volatile bool gRSPHLEActive = false;

//*****************************************************************************
//
//*****************************************************************************
// We need similar registers to the main CPU
ALIGNED_GLOBAL(SRSPState, gRSPState, CACHE_ALIGN);

#define gRegRSP		gRSPState.CPU
#define gVectRSP	gRSPState.COP2
#define gAccuRSP	gRSPState.Accumulator
#define gFlagsRSP	gRSPState.COP2Control
#define RSPPC		gRSPState.CurrentPC
#define g_nRSPDelay	gRSPState.Delay
#define g_dwNewRSPPC gRSPState.TargetPC


REG64 EleSpec[32], Indx[32];


//*****************************************************************************
//
//*****************************************************************************

typedef void (*RSPInstruction)( OpCode op );


//*****************************************************************************
//
//*****************************************************************************

// We assume COP2 isn't called or barely called by some games, hence we can get rid off it.
// Batch'ed tested over 100 roms, everything ok
// Manually tested around 40 roms everything ok.
// Only games that seemed not to like this was DK64 and Tooie, but anyways those games never really worked before.

static void Ignored_Cop2(OpCode op);

/*
    CPU: Instructions encoded by opcode field.
    31---------26---------------------------------------------------0
    |  opcode   |                                                   |
    ------6----------------------------------------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
000 | *1    | *2    |   J   |  JAL  |  BEQ  |  BNE  | BLEZ  | BGTZ  |
001 | ADDI  | ADDIU | SLTI  | SLTIU | ANDI  |  ORI  | XORI  |  LUI  |
010 | *3    |  ---  |  *4   |  ---  |  ---  |  ---  |  ---  |  ---  |
011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
100 |  LB   |  LH   |  ---  |  LW   |  LBU  |  LHU  |  ---  |  ---  |
101 |  SB   |  SH   |  ---  |  SW   |  ---  |  ---  |  ---  |  ---  |
110 |  ---  |  ---  | *LWC2 |  ---  |  ---  |  ---  |  ---  |  ---  |
111 |  ---  |  ---  | *SWC2 |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
     *1 = SPECIAL, see SPECIAL list    *2 = REGIMM, see REGIMM list
     *3 = COP0                         *4 = COP2
     *LWC2 = RSP Load instructions     *SWC2 = RSP Store instructions
*/


// Opcode Jump Table
RSPInstruction RSP_Instruction[64] = {
	/*RSP_Special, RSP_RegImm, RSP_J, RSP_JAL, RSP_BEQ, RSP_BNE, RSP_BLEZ, RSP_BGTZ,
	RSP_ADDI, RSP_ADDIU, RSP_SLTI, RSP_SLTIU, RSP_ANDI, RSP_ORI, RSP_XORI, RSP_LUI,
	RSP_CoPro0, RSP_Unk, RSP_CoPro2, RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk,
	RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk,
	RSP_LB, RSP_LH, RSP_Unk, RSP_LW, RSP_LBU, RSP_LHU, RSP_Unk, RSP_Unk,
	RSP_SB, RSP_SH, RSP_Unk, RSP_SW, RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk,
	RSP_Unk, RSP_Unk, RSP_LWC2, RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk,
	RSP_Unk, RSP_Unk, RSP_SWC2, RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk, RSP_Unk*/
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2
};


/*
    RSP SPECIAL: Instr. encoded by function field when opcode field = SPECIAL.
    31---------26-----------------------------------------5---------0
    | = SPECIAL |                                         | function|
    ------6----------------------------------------------------6-----
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
000 |  SLL  |  ---  |  SRL  |  SRA  | SLLV  |  ---  | SRLV  | SRAV  |
001 |  JR   |  JALR |  ---  |  ---  |  ---  | BREAK |  ---  |  ---  |
010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
100 |  ADD  | ADDU  |  SUB  | SUBU  |  AND  |  OR   |  XOR  |  NOR  |
101 |  ---  |  ---  |  SLT  | SLTU  |  ---  |  ---  |  ---  |  ---  |
110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
 */

// SpecialOpCode Jump Table
RSPInstruction RSPSpecialInstruction[64] = {
	/*RSP_Special_SLL, RSP_Special_Unk,  RSP_Special_SRL, RSP_Special_SRA,  RSP_Special_SLLV, RSP_Special_Unk,   RSP_Special_SRLV, RSP_Special_SRAV,
	RSP_Special_JR,  RSP_Special_JALR, RSP_Special_Unk, RSP_Special_Unk,  RSP_Special_Unk,  RSP_Special_BREAK, RSP_Special_Unk,  RSP_Special_Unk,
	RSP_Special_Unk, RSP_Special_Unk,  RSP_Special_Unk, RSP_Special_Unk,  RSP_Special_Unk,  RSP_Special_Unk,   RSP_Special_Unk,  RSP_Special_Unk,
	RSP_Special_Unk, RSP_Special_Unk,  RSP_Special_Unk, RSP_Special_Unk,  RSP_Special_Unk,  RSP_Special_Unk,   RSP_Special_Unk,  RSP_Special_Unk,
	RSP_Special_ADD, RSP_Special_ADDU, RSP_Special_SUB, RSP_Special_SUBU, RSP_Special_AND,  RSP_Special_OR,    RSP_Special_XOR,  RSP_Special_NOR,
	RSP_Special_Unk, RSP_Special_Unk,  RSP_Special_SLT, RSP_Special_SLTU, RSP_Special_Unk,  RSP_Special_Unk,   RSP_Special_Unk,  RSP_Special_Unk,
	RSP_Special_Unk, RSP_Special_Unk,  RSP_Special_Unk, RSP_Special_Unk,  RSP_Special_Unk,  RSP_Special_Unk,   RSP_Special_Unk,  RSP_Special_Unk,
	RSP_Special_Unk, RSP_Special_Unk,  RSP_Special_Unk, RSP_Special_Unk,  RSP_Special_Unk,  RSP_Special_Unk,   RSP_Special_Unk,  RSP_Special_Unk*/
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2
};


/*
    REGIMM: Instructions encoded by the rt field when opcode field = REGIMM.
    31---------26----------20-------16------------------------------0
    | = REGIMM  |          |   rt    |                              |
    ------6---------------------5------------------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 00 | BLTZ  | BGEZ  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 10 |BLTZAL |BGEZAL |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
 */

RSPInstruction RSPRegImmInstruction[32] = {

	/*RSP_RegImm_BLTZ,   RSP_RegImm_BGEZ,   RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk,
	RSP_RegImm_Unk,    RSP_RegImm_Unk,    RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk,
	RSP_RegImm_BLTZAL, RSP_RegImm_BGEZAL, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk,
	RSP_RegImm_Unk,    RSP_RegImm_Unk,    RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk, RSP_RegImm_Unk*/
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2
};



/*
    COP0: Instructions encoded by the fmt field when opcode = COP0.
    31--------26-25------21 ----------------------------------------0
    |  = COP0   |   fmt   |                                         |
    ------6----------5-----------------------------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 00 | MFC0  |  ---  |  ---  |  ---  | MTC0  |  ---  |  ---  |  ---  |
 01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/

// COP0 Jump Table
RSPInstruction RSPCop0Instruction[32] = {
	/*RSP_Cop0_MFC0, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_MTC0, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk,
	RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk,
	RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk,
	RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk, RSP_Cop0_Unk,*/
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2
};



/*
    RSP Load: Instr. encoded by rd field when opcode field = LWC2
    31---------26-------------------15-------11---------------------0
    |  110010   |                   |   rd   |                      |
    ------6-----------------------------5----------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 00 |  LBV  |  LSV  |  LLV  |  LDV  |  LQV  |  LRV  |  LPV  |  LUV  |
 01 |  LHV  |  LFV  |  LWV  |  LTV  |  ---  |  ---  |  ---  |  ---  |
 10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/

// LWC2 Jump Table
RSPInstruction RSPLWC2Instruction[32] = {
	/*RSP_LWC2_LBV, RSP_LWC2_LSV, RSP_LWC2_LLV, RSP_LWC2_LDV, RSP_LWC2_LQV, RSP_LWC2_LRV, RSP_LWC2_LPV, RSP_LWC2_LUV,
	RSP_LWC2_LHV, RSP_LWC2_LFV, RSP_LWC2_LWV, RSP_LWC2_LTV, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk,
	RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk,
	RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk, RSP_LWC2_Unk,*/
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2
};

/*
    RSP Store: Instr. encoded by rd field when opcode field = SWC2
    31---------26-------------------15-------11---------------------0
    |  111010   |                   |   rd   |                      |
    ------6-----------------------------5----------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 00 |  SBV  |  SSV  |  SLV  |  SDV  |  SQV  |  SRV  |  SPV  |  SUV  |
 01 |  SHV  |  SFV  |  SWV  |  STV  |  ---  |  ---  |  ---  |  ---  |
 10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
*/

// SWC2 Jump Table
RSPInstruction RSPSWC2Instruction[32] = {
	/*RSP_SWC2_SBV, RSP_SWC2_SSV, RSP_SWC2_SLV, RSP_SWC2_SDV, RSP_SWC2_SQV, RSP_SWC2_SRV, RSP_SWC2_SPV, RSP_SWC2_SUV,
	RSP_SWC2_SHV, RSP_SWC2_SFV, RSP_SWC2_SWV, RSP_SWC2_STV, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk,
	RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk,
	RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk, RSP_SWC2_Unk,*/
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2
};


/*
    COP2: Instructions encoded by the fmt field when opcode = COP2.
    31--------26-25------21 ----------------------------------------0
    |  = COP2   |   fmt   |                                         |
    ------6----------5-----------------------------------------------
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
 00 | MFC2  |  ---  | CFC2  |  ---  | MTC2  |  ---  | CTC2  |  ---  |
 01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 10 |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |
 11 |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |  *1   |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
     *1 = Vector opcode
*/

RSPInstruction RSPCop2Instruction[32] = {
	/*RSP_Cop2_MFC2, RSP_Cop2_Unk, RSP_Cop2_CFC2, RSP_Cop2_Unk, RSP_Cop2_MTC2, RSP_Cop2_Unk, RSP_Cop2_CTC2, RSP_Cop2_Unk,
	RSP_Cop2_Unk, RSP_Cop2_Unk, RSP_Cop2_Unk, RSP_Cop2_Unk, RSP_Cop2_Unk, RSP_Cop2_Unk, RSP_Cop2_Unk, RSP_Cop2_Unk,
	RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector,
	RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector, RSP_Cop2_Vector*/
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2
};

/*
    Vector opcodes: Instr. encoded by the function field when opcode = COP2.
    31---------26---25------------------------------------5---------0
    |  = COP2   | 1 |                                     | function|
    ------6-------1--------------------------------------------6-----
    |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--| lo
000 | VMULF | VMULU | VRNDP | VMULQ | VMUDL | VMUDM | VMUDN | VMUDH |
001 | VMACF | VMACU | VRNDN | VMACQ | VMADL | VMADM | VMADN | VMADH |
010 | VADD  | VSUB  | VSUT? | VABS  | VADDC | VSUBC | VADDB?| VSUBB?|
011 | VACCB?| VSUCB?| VSAD? | VSAC? | VSUM? | VSAW  |  ---  |  ---  |
100 |  VLT  |  VEQ  |  VNE  |  VGE  |  VCL  |  VCH  |  VCR  | VMRG  |
101 | VAND  | VNAND |  VOR  | VNOR  | VXOR  | VNXOR |  ---  |  ---  |
110 | VRCP  | VRCPL | VRCPH | VMOV  | VRSQ  | VRSQL | VRSQH |  ---  |
110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 hi |-------|-------|-------|-------|-------|-------|-------|-------|
    Comment: Those with a ? in the end of them may not exist
*/
RSPInstruction RSPCop2VInstruction[64] = {
	/*RSP_Cop2V_VMULF, RSP_Cop2V_VMULU, RSP_Cop2V_VRNDP, RSP_Cop2V_VMULQ, RSP_Cop2V_VMUDL, RSP_Cop2V_VMUDM, RSP_Cop2V_VMUDN, RSP_Cop2V_VMUDH,
	RSP_Cop2V_VMACF, RSP_Cop2V_VMACU, RSP_Cop2V_VRNDN, RSP_Cop2V_VMACQ, RSP_Cop2V_VMADL, RSP_Cop2V_VMADM, RSP_Cop2V_VMADN, RSP_Cop2V_VMADH,
	RSP_Cop2V_VADD,  RSP_Cop2V_VSUB,  RSP_Cop2V_VSUT,  RSP_Cop2V_VABS,  RSP_Cop2V_VADDC, RSP_Cop2V_VSUBC, RSP_Cop2V_VADDB, RSP_Cop2V_VSUBB,
	RSP_Cop2V_VACCB, RSP_Cop2V_VSUCB, RSP_Cop2V_VSAD,  RSP_Cop2V_VSAC,  RSP_Cop2V_VSUM,  RSP_Cop2V_VSAW,  RSP_Cop2V_Unk,   RSP_Cop2V_Unk,
	RSP_Cop2V_VLT,   RSP_Cop2V_VEQ,   RSP_Cop2V_VNE,   RSP_Cop2V_VGE,   RSP_Cop2V_VCL,   RSP_Cop2V_VCH,   RSP_Cop2V_VCR,   RSP_Cop2V_VMRG,
	RSP_Cop2V_VAND,  RSP_Cop2V_VNAND, RSP_Cop2V_VOR,   RSP_Cop2V_VNOR,  RSP_Cop2V_VXOR,  RSP_Cop2V_VNXOR, RSP_Cop2V_Unk,   RSP_Cop2V_Unk,
	RSP_Cop2V_VRCP,  RSP_Cop2V_VRCPL, RSP_Cop2V_VRCPH, RSP_Cop2V_VMOV,  RSP_Cop2V_VRSQ,  RSP_Cop2V_VRSQL, RSP_Cop2V_VRSQH, RSP_Cop2V_Unk,
	RSP_Cop2V_Unk,   RSP_Cop2V_Unk,   RSP_Cop2V_Unk,   RSP_Cop2V_Unk,   RSP_Cop2V_Unk,   RSP_Cop2V_Unk,   RSP_Cop2V_Unk,   RSP_Cop2V_Unk*/
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2,
	Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2, Ignored_Cop2
};
/*
#define LOAD_VECTORS(dwA, dwB, dwPattern)						\
	for (u32 __i = 0; __i < 8; __i++) {								\
		g_wTemp[0]._u16[__i] = gVectRSP[dwA]._u16[__i];							\
		g_wTemp[1]._u16[__i] = gVectRSP[dwB]._u16[g_Pattern[dwPattern][__i]];	\
	}


static const u32 g_Pattern[16][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },	//?
	{ 0, 0, 2, 2, 4, 4, 6, 6 },
	{ 1, 1, 3, 3, 5, 5, 7, 7 },

	{ 0, 0, 0, 0, 4, 4, 4, 4 },
	{ 1, 1, 1, 1, 5, 5, 5, 5 },
	{ 2, 2, 2, 2, 6, 6, 6, 6 },
	{ 3, 3, 3, 3, 7, 7, 7, 7 },

	{ 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 3, 3, 3, 3, 3, 3, 3, 3 },
	{ 4, 4, 4, 4, 4, 4, 4, 4 },
	{ 5, 5, 5, 5, 5, 5, 5, 5 },
	{ 6, 6, 6, 6, 6, 6, 6, 6 },
	{ 7, 7, 7, 7, 7, 7, 7, 7 }
};
*/
//*****************************************************************************
//
//*****************************************************************************
void RSP_Reset(void)
{
	//I don't think this needed. ~ Salvy
	//
	memset( &gRSPState, 0, sizeof(gRSPState));
	memset( EleSpec, 0, sizeof( EleSpec ) );
	memset( Indx, 0, sizeof( Indx ) );

	EleSpec[16]._u64 = 0x0001020304050607LL; /* None */
	EleSpec[17]._u64 = 0x0001020304050607LL; /* None */
	EleSpec[18]._u64 = 0x0000020204040606LL; /* 0q */
	EleSpec[19]._u64 = 0x0101030305050707LL; /* 1q */
	EleSpec[20]._u64 = 0x0000000004040404LL; /* 0h */
	EleSpec[21]._u64 = 0x0101010105050505LL; /* 1h */
	EleSpec[22]._u64 = 0x0202020206060606LL; /* 2h */
	EleSpec[23]._u64 = 0x0303030307070707LL; /* 3h */
	EleSpec[24]._u64 = 0x0000000000000000LL; /* 0 */
	EleSpec[25]._u64 = 0x0101010101010101LL; /* 1 */
	EleSpec[26]._u64 = 0x0202020202020202LL; /* 2 */
	EleSpec[27]._u64 = 0x0303030303030303LL; /* 3 */
	EleSpec[28]._u64 = 0x0404040404040404LL; /* 4 */
	EleSpec[29]._u64 = 0x0505050505050505LL; /* 5 */
	EleSpec[30]._u64 = 0x0606060606060606LL; /* 6 */
	EleSpec[31]._u64 = 0x0707070707070707LL; /* 7 */

	Indx[16]._u64 = 0x0001020304050607LL; // None		0x 0001020304050607
	Indx[17]._u64 = 0x0001020304050607LL; // None		0x 0001020304050607
	Indx[18]._u64 = 0x0103050700020406LL; // 0q		0x01030507 00020406
	Indx[19]._u64 = 0x0002040601030507LL; // 1q		0x00020406 01030507
	Indx[20]._u64 = 0x0102030506070004LL; // 0h		0x010203050607 0004
	Indx[21]._u64 = 0x0002030406070105LL; // 1h		0x000203040607 0105
	Indx[22]._u64 = 0x0001030405070206LL; // 2h		0x000103040507 0206
	Indx[23]._u64 = 0x0001020405060307LL; // 3h		0x000102040506 0307
	Indx[24]._u64 = 0x0102030405060700LL; // 0		0x01020304050607 00
	Indx[25]._u64 = 0x0002030405060701LL; // 1		0x00020304050607 01
	Indx[26]._u64 = 0x0001030405060702LL; // 2		0x00010304050607 02
	Indx[27]._u64 = 0x0001020405060703LL; // 3		0x00010204050607 03
	Indx[28]._u64 = 0x0001020305060704LL; // 4		0x00010203050607 04
	Indx[29]._u64 = 0x0001020304060705LL; // 5		0x00010203040607 05
	Indx[30]._u64 = 0x0001020304050706LL; // 6		0x00010203040507 06
	Indx[31]._u64 = 0x0001020304050607LL; // 7		0x00010203040506 07

		//Indx[02]    = 0x01 03 05 07 00 02 04 06		0q
		//EleSpec[02] = 0x07 07 05 05 03 03 01 01

/*06 / 07
04 / 05
02 / 03
00 / 01
07 / 07
05 / 05
03 / 03
01 / 01*/


	for ( u32 i = 16; i < 32; i++ )
	{
		u32 count;

		// Swap 0 for 7, 1 for 6 etc
		for ( count = 0; count < 8; count++ )
		{
			Indx[i]._u8[count]    = 7 - Indx[i]._u8[count];
			EleSpec[i]._u8[count] = 7 - EleSpec[i]._u8[count];
		}

		// Reverse the order
		for (count = 0; count < 4; count ++)
		{
			// Swap Indx[i][count] and Indx[i][7-count]
			u8 temp                = Indx[i]._u8[count];
			Indx[i]._u8[count]     = Indx[i]._u8[7 - count];
			Indx[i]._u8[7 - count] = temp;
		}

	}
/*
	for ( i = 16; i < 32; i++ )
	{
		//Indx[00] = 0x0001020304050607		None
		//Indx[01] = 0x0001020304050607		None
		//Indx[02] = 0x0103050700020406		0q
		//Indx[03] = 0x0002040601030507		1q
		//Indx[04] = 0x0307000102040506		0h
		//Indx[05] = 0x0206000103040507		1h
		//Indx[06] = 0x0105000203040607		2h
		//Indx[07] = 0x0004010203050607		3h
		//Indx[08] = 0x0700010203040506		0
		//Indx[09] = 0x0600010203040507		1
		//Indx[10] = 0x0500010203040607		2
		//Indx[11] = 0x0400010203050607		3
		//Indx[12] = 0x0300010204050607		4
		//Indx[13] = 0x0200010304050607		5
		//Indx[14] = 0x0100020304050607		6
		//Indx[15] = 0x0001020304050607		7
		DBGConsole_Msg( 0, "Indx[[%02d[] = 0x%08x%08x", i - 16, Indx[i]._u32[1], Indx[i]._u32_0 );
	}
	for ( i = 16; i < 32; i++ )
	{
		//EleSpec[00] = 0x0706050403020100
		//EleSpec[01] = 0x0706050403020100
		//EleSpec[02] = 0x0707050503030101
		//EleSpec[03] = 0x0606040402020000
		//EleSpec[04] = 0x0707070703030303
		//EleSpec[05] = 0x0606060602020202
		//EleSpec[06] = 0x0505050501010101
		//EleSpec[07] = 0x0404040400000000
		//EleSpec[08] = 0x0707070707070707
		//EleSpec[09] = 0x0606060606060606
		//EleSpec[10] = 0x0505050505050505
		//EleSpec[11] = 0x0404040404040404
		//EleSpec[12] = 0x0303030303030303
		//EleSpec[13] = 0x0202020202020202
		//EleSpec[14] = 0x0101010101010101
		//EleSpec[15] = 0x0000000000000000

		DBGConsole_Msg( 0, "EleSpec[[%02d[] = 0x%08x%08x", i - 16, EleSpec[i]._u32[1], EleSpec[i]._u32_0 );
	}
*/

	DAEDALUS_ASSERT( !gRSPHLEActive, "Resetting RSP with HLE active" );
	Memory_SP_SetRegister(SP_STATUS_REG, SP_STATUS_HALT);			// SP is halted
}

//*****************************************************************************
//
//*****************************************************************************
bool RSP_IsRunning()
{
	return (Memory_SP_GetRegister( SP_STATUS_REG ) & SP_STATUS_HALT) == 0;
}

//*****************************************************************************
//
//*****************************************************************************
bool RSP_IsRunningLLE()
{
	return RSP_IsRunning() && !gRSPHLEActive;
}

//*****************************************************************************
//
//*****************************************************************************
bool RSP_IsRunningHLE()
{
	return RSP_IsRunning() && gRSPHLEActive;
}


//*****************************************************************************
//
//*****************************************************************************
void RSP_Step()
{
	DAEDALUS_ASSERT( RSP_IsRunning(), "Why is the RSP stepping when it is halted?" );
	DAEDALUS_ASSERT( !gRSPHLEActive, "Why is the RSP stepping when HLE is active?" );

	// Fetch next instruction
	u32 pc = RSPPC;
	OpCode op;
	op._u32 = *(u32 *)(g_pu8SpMemBase + ((pc & 0x0FFF) | 0x1000));

	RSP_Instruction[ op.op ]( op );

	switch (g_nRSPDelay)
	{
		case DO_DELAY:
			// We've got a delayed instruction to execute. Increment
			// PC as normal, so that subsequent instruction is executed
			RSPPC = RSPPC + 4;
			g_nRSPDelay = EXEC_DELAY;
			break;
		case EXEC_DELAY:
			// We've just executed the delayed instr. Now carry out jump
			//  as stored in g_dwNewRSPPC;
			RSPPC = g_dwNewRSPPC;
			g_nRSPDelay = NO_DELAY;
			break;
		case NO_DELAY:
			// Normal operation - just increment the PC
			RSPPC = RSPPC + 4;
			break;
	}
}

#if 0
//*****************************************************************************
//
//*****************************************************************************
void RSP_DumpVector(u32 reg)
{
	reg &= 0x1f;

	DBGConsole_Msg(0, "Vector%d", reg);
	for(u32 i = 0; i < 8; i++)
	{
		DBGConsole_Msg(0, "%d: 0x%04x", i, gVectRSP[reg]._u16[i]);
	}
}

//*****************************************************************************
//
//*****************************************************************************
void RSP_DumpVectors(u32 reg1, u32 reg2, u32 reg3)
{
	reg1 &= 0x1f;
	reg2 &= 0x1f;
	reg3 &= 0x1f;

	DBGConsole_Msg(0, "    Vec%2d\tVec%2d\tVec%2d", reg1, reg2, reg3);
	for(u32 i = 0; i < 8; i++)
	{
		DBGConsole_Msg(0, "%d: 0x%04x\t0x%04x\t0x%04x",
			i, gVectRSP[reg1]._u16[i], gVectRSP[reg2]._u16[i], gVectRSP[reg3]._u16[i]);
	}
}
#endif

//*****************************************************************************
// Ignore COP2
//*****************************************************************************
void Ignored_Cop2(OpCode op)
{
	DBGConsole_Msg(0, "[CSafely Ignored COP2]");
}