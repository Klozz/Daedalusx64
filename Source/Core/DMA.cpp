/*
Copyright (C) 2001 StrmnNrmn

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

// Various stuff to map an address onto the correct memory region

#include "stdafx.h"

#include "DMA.h"
#include "Memory.h"
#include "RSP.h"
#include "RSP_HLE.h"
#include "CPU.h"
#include "ROM.h"
#include "ROMBuffer.h"
#include "PIF.h"
#include "Interrupt.h"
#include "Save.h"

#include "Debug/DebugLog.h"
#include "Debug/DBGConsole.h"

#include "OSHLE/OSTask.h"
#include "OSHLE/patch.h"

u32 s_nNumDmaTransfers = 0;		// Incremented on every Cart->RDRAM Xfer
u32 s_nTotalDmaTransferSize = 0;	// Total size of every Cart->RDRAM Xfer
u32 s_nNumSPTransfers = 0;			// Incremented on every RDRAM->SPMem Xfer
u32 s_nTotalSPTransferSize = 0;	// Total size of every RDRAM->SPMem Xfer

bool gDMAUsed = false;

#ifndef DAEDALUS_PUBLIC_RELEASE
bool gLogSpDMA = false;
#endif

//Taken from psp-programming forum (Raphael) 
//little endian tweaked by Corn *might still miss some endian case in CPU part(size<64)  
void memcpy_vfpu( void* dst, void* src, u32 size )
{
	u8* src8 = (u8*)src;
	u8* dst8 = (u8*)dst;
	u8* udst8;
	u8* dst64a;

	// < 8 isn't worth trying any optimisations...
	if (size<8) goto bytecopy;

	// < 64 means we don't gain anything from using vfpu...
	if (size<64)
	{
		// Align dst on 4 bytes or just resume if already done
		while (((((u32)dst8) & 0x3)!=0) && size) {
			//*dst8++ = *src8++;
			*(u8*)((u32)dst8++ ^ U8_TWIDDLE) = *(u8*)((u32)src8++ ^ U8_TWIDDLE);
			size--;
		}
		if (size<4) goto bytecopy;

		// We are dst aligned now and >= 4 bytes to copy
		u32* src32 = (u32*)src8;
		u32* dst32 = (u32*)dst8;
		switch(((u32)src8)&0x3)
		{
			case 0:
				while (size&0xC)
				{
					*dst32++ = *src32++;
					size -= 4;
				}
				if (size==0) return;		// fast out
				while (size>=16)
				{
					*dst32++ = *src32++;
					*dst32++ = *src32++;
					*dst32++ = *src32++;
					*dst32++ = *src32++;
					size -= 16;
				}
				if (size==0) return;		// fast out
				src8 = (u8*)src32;
				dst8 = (u8*)dst32;
				break;
			default:
				{
					register u32 a, b, c, d;
					while (size>=4)
					{
						a = *src8++;
						b = *src8++;
						c = *src8++;
						d = *src8++;
//						*dst32++ = (d << 24) | (c << 16) | (b << 8) | a;
						*dst32++ = (a << 24) | (b << 16) | (c << 8) | d;
						size -= 4;
					}
					if (size==0) return;		// fast out
					dst8 = (u8*)dst32;
				}
				break;
		}
		goto bytecopy;
	}

	// Align dst on 16 bytes to gain from vfpu aligned stores
	while ((((u32)dst8) & 0xF)!=0 && size) {
		//*dst8++ = *src8++;
		*(u8*)((u32)dst8++ ^ U8_TWIDDLE) = *(u8*)((u32)src8++ ^ U8_TWIDDLE);
		size--;
	}

	// We use uncached dst to use VFPU writeback and free cpu cache for src only
	udst8 = (u8*)((u32)dst8 | 0x40000000);
	// We need the 64 byte aligned address to make sure the dcache is invalidated correctly
	dst64a = (u8*)((u32)dst8&~0x3F);
	// Invalidate the first line that matches up to the dst start
	if (size>=64)
	asm(".set	push\n"					// save assembler option
		".set	noreorder\n"			// suppress reordering
		"cache 0x1B, 0(%0)\n"
		"addiu	%0, %0, 64\n"
		"sync\n"
		".set	pop\n"
		:"+r"(dst64a));
	switch(((u32)src8&0xF))
	{
		// src aligned on 16 bytes too? nice!
		case 0:
			while (size>=64)
			{
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"cache	0x1B,  0(%2)\n"			// Dcache writeback invalidate
					"lv.q	c000,  0(%1)\n"
					"lv.q	c010, 16(%1)\n"
					"lv.q	c020, 32(%1)\n"
					"lv.q	c030, 48(%1)\n"
					"sync\n"						// Wait for allegrex writeback
					"sv.q	c000,  0(%0), wb\n"
					"sv.q	c010, 16(%0), wb\n"
					"sv.q	c020, 32(%0), wb\n"
					"sv.q	c030, 48(%0), wb\n"
					// Lots of variable updates... but get hidden in sv.q latency anyway
					"addiu  %3, %3, -64\n"
					"addiu	%2, %2, 64\n"
					"addiu	%1, %1, 64\n"
					"addiu	%0, %0, 64\n"
					".set	pop\n"					// restore assembler option
					:"+r"(udst8),"+r"(src8),"+r"(dst64a),"+r"(size)
					:
					:"memory"
					);
			}
			if (size>16)
			{
				// Invalidate the last cache line where the max remaining 63 bytes are
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"cache	0x1B, 0(%0)\n"
					"sync\n"
					".set	pop\n"					// restore assembler option
					::"r"(dst64a));
				while (size>=16)
				{
					asm(".set	push\n"					// save assembler option
						".set	noreorder\n"			// suppress reordering
						"lv.q	c000, 0(%1)\n"
						"sv.q	c000, 0(%0), wb\n"
						// Lots of variable updates... but get hidden in sv.q latency anyway
						"addiu	%2, %2, -16\n"
						"addiu	%1, %1, 16\n"
						"addiu	%0, %0, 16\n"
						".set	pop\n"					// restore assembler option
						:"+r"(udst8),"+r"(src8),"+r"(size)
						:
						:"memory"
						);
				}
			}
			asm(".set	push\n"					// save assembler option
				".set	noreorder\n"			// suppress reordering
				"vflush\n"						// Flush VFPU writeback cache
				".set	pop\n"					// restore assembler option
				);
			dst8 = (u8*)((u32)udst8 & ~0x40000000);
			break;
		// src is only qword unaligned but word aligned? We can at least use ulv.q
		case 4:
		case 8:
		case 12:
			while (size>=64)
			{
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"cache	0x1B,  0(%2)\n"			// Dcache writeback invalidate
					"ulv.q	c000,  0(%1)\n"
					"ulv.q	c010, 16(%1)\n"
					"ulv.q	c020, 32(%1)\n"
					"ulv.q	c030, 48(%1)\n"
					"sync\n"						// Wait for allegrex writeback
					"sv.q	c000,  0(%0), wb\n"
					"sv.q	c010, 16(%0), wb\n"
					"sv.q	c020, 32(%0), wb\n"
					"sv.q	c030, 48(%0), wb\n"
					// Lots of variable updates... but get hidden in sv.q latency anyway
					"addiu  %3, %3, -64\n"
					"addiu	%2, %2, 64\n"
					"addiu	%1, %1, 64\n"
					"addiu	%0, %0, 64\n"
					".set	pop\n"					// restore assembler option
					:"+r"(udst8),"+r"(src8),"+r"(dst64a),"+r"(size)
					:
					:"memory"
					);
			}
			if (size>16)
			// Invalidate the last cache line where the max remaining 63 bytes are
			asm(".set	push\n"					// save assembler option
				".set	noreorder\n"			// suppress reordering
				"cache	0x1B, 0(%0)\n"
				"sync\n"
				".set	pop\n"					// restore assembler option
				::"r"(dst64a));
			while (size>=16)
			{
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"ulv.q	c000, 0(%1)\n"
					"sv.q	c000, 0(%0), wb\n"
					// Lots of variable updates... but get hidden in sv.q latency anyway
					"addiu	%2, %2, -16\n"
					"addiu	%1, %1, 16\n"
					"addiu	%0, %0, 16\n"
					".set	pop\n"					// restore assembler option
					:"+r"(udst8),"+r"(src8),"+r"(size)
					:
					:"memory"
					);
			}
			asm(".set	push\n"					// save assembler option
				".set	noreorder\n"			// suppress reordering
				"vflush\n"						// Flush VFPU writeback cache
				".set	pop\n"					// restore assembler option
				);
			dst8 = (u8*)((u32)udst8 & ~0x40000000);
			break;
		// src not aligned? too bad... have to use unaligned reads
		default:
			while (size>=64)
			{
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"cache 0x1B,  0(%2)\n"

					"lwr	 $8,  0(%1)\n"			//
					"lwl	 $8,  3(%1)\n"			// $8  = *(s + 0)
					"lwr	 $9,  4(%1)\n"			//
					"lwl	 $9,  7(%1)\n"			// $9  = *(s + 4)
					"lwr	$10,  8(%1)\n"			//
					"lwl	$10, 11(%1)\n"			// $10 = *(s + 8)
					"lwr	$11, 12(%1)\n"			//
					"lwl	$11, 15(%1)\n"			// $11 = *(s + 12)
					"mtv	 $8, s000\n"
					"mtv	 $9, s001\n"
					"mtv	$10, s002\n"
					"mtv	$11, s003\n"

					"lwr	 $8, 16(%1)\n"
					"lwl	 $8, 19(%1)\n"
					"lwr	 $9, 20(%1)\n"
					"lwl	 $9, 23(%1)\n"
					"lwr	$10, 24(%1)\n"
					"lwl	$10, 27(%1)\n"
					"lwr	$11, 28(%1)\n"
					"lwl	$11, 31(%1)\n"
					"mtv	 $8, s010\n"
					"mtv	 $9, s011\n"
					"mtv	$10, s012\n"
					"mtv	$11, s013\n"
					
					"lwr	 $8, 32(%1)\n"
					"lwl	 $8, 35(%1)\n"
					"lwr	 $9, 36(%1)\n"
					"lwl	 $9, 39(%1)\n"
					"lwr	$10, 40(%1)\n"
					"lwl	$10, 43(%1)\n"
					"lwr	$11, 44(%1)\n"
					"lwl	$11, 47(%1)\n"
					"mtv	 $8, s020\n"	
					"mtv	 $9, s021\n"
					"mtv	$10, s022\n"
					"mtv	$11, s023\n"

					"lwr	 $8, 48(%1)\n"
					"lwl	 $8, 51(%1)\n"
					"lwr	 $9, 52(%1)\n"
					"lwl	 $9, 55(%1)\n"
					"lwr	$10, 56(%1)\n"
					"lwl	$10, 59(%1)\n"
					"lwr	$11, 60(%1)\n"
					"lwl	$11, 63(%1)\n"
					"mtv	 $8, s030\n"
					"mtv	 $9, s031\n"
					"mtv	$10, s032\n"
					"mtv	$11, s033\n"
					
					"sync\n"
					"sv.q 	c000,  0(%0), wb\n"
					"sv.q 	c010, 16(%0), wb\n"
					"sv.q 	c020, 32(%0), wb\n"
					"sv.q 	c030, 48(%0), wb\n"
					// Lots of variable updates... but get hidden in sv.q latency anyway
					"addiu	%3, %3, -64\n"
					"addiu	%2, %2, 64\n"
					"addiu	%1, %1, 64\n"
					"addiu	%0, %0, 64\n"
					".set	pop\n"					// restore assembler option
					:"+r"(udst8),"+r"(src8),"+r"(dst64a),"+r"(size)
					:
					:"$8","$9","$10","$11","memory"
					);
			}
			if (size>16)
			// Invalidate the last cache line where the max remaining 63 bytes are
			asm(".set	push\n"					// save assembler option
				".set	noreorder\n"			// suppress reordering
				"cache	0x1B, 0(%0)\n"
				"sync\n"
				".set	pop\n"					// restore assembler option
				::"r"(dst64a));
			while (size>=16)
			{
				asm(".set	push\n"					// save assembler option
					".set	noreorder\n"			// suppress reordering
					"lwr	 $8,  0(%1)\n"			//
					"lwl	 $8,  3(%1)\n"			// $8  = *(s + 0)
					"lwr	 $9,  4(%1)\n"			//
					"lwl	 $9,  7(%1)\n"			// $9  = *(s + 4)
					"lwr	$10,  8(%1)\n"			//
					"lwl	$10, 11(%1)\n"			// $10 = *(s + 8)
					"lwr	$11, 12(%1)\n"			//
					"lwl	$11, 15(%1)\n"			// $11 = *(s + 12)
					"mtv	 $8, s000\n"
					"mtv	 $9, s001\n"
					"mtv	$10, s002\n"
					"mtv	$11, s003\n"

					"sv.q	c000, 0(%0), wb\n"
					// Lots of variable updates... but get hidden in sv.q latency anyway
					"addiu	%2, %2, -16\n"
					"addiu	%1, %1, 16\n"
					"addiu	%0, %0, 16\n"
					".set	pop\n"					// restore assembler option
					:"+r"(udst8),"+r"(src8),"+r"(size)
					:
					:"$8","$9","$10","$11","memory"
					);
			}
			asm(".set	push\n"					// save assembler option
				".set	noreorder\n"			// suppress reordering
				"vflush\n"						// Flush VFPU writeback cache
				".set	pop\n"					// restore assembler option
				);
			dst8 = (u8*)((u32)udst8 & ~0x40000000);
			break;
	}
	
bytecopy:
	// Copy the remains byte per byte...
	while (size--)
	{
		//*dst8++ = *src8++;
		*(u8*)((u32)dst8++ ^ U8_TWIDDLE) = *(u8*)((u32)src8++ ^ U8_TWIDDLE);
	}
}

//*****************************************************************************
// Heavily based on Mupen
//*****************************************************************************
//123 to 57
void DMA_SP_CopyFromRDRAM()
{
	u32 spmem_address_reg = Memory_SP_GetRegister(SP_MEM_ADDR_REG);
	u32 rdram_address_reg = Memory_SP_GetRegister(SP_DRAM_ADDR_REG);
	u32 rdlen_reg         = Memory_SP_GetRegister(SP_RD_LEN_REG);
//	u32 i;

	if ((spmem_address_reg & 0x1000) > 0)
	{
		memcpy_vfpu(&g_pu8SpImemBase[(spmem_address_reg & 0xFFF)],
					&g_pu8RamBase[(rdram_address_reg & 0xFFFFFF)],
					(rdlen_reg & 0xFFF)+1 );

		//for (i=0; i<((rdlen_reg & 0xFFF)+1); i++)
		//{
		//	g_pu8SpImemBase[((spmem_address_reg & 0xFFF)+i)^U8_TWIDDLE]=
		//	g_pu8RamBase[((rdram_address_reg & 0xFFFFFF)+i)^U8_TWIDDLE];
		//}
	}
	else
	{
		memcpy_vfpu(&g_pu8SpDmemBase[(spmem_address_reg & 0xFFF)],
					&g_pu8RamBase[(rdram_address_reg & 0xFFFFFF)],
					(rdlen_reg & 0xFFF)+1 ); 
		
		//for (i=0; i<((rdlen_reg & 0xFFF)+1); i++)
		//{
		//	g_pu8SpDmemBase[((spmem_address_reg & 0xFFF)+i)^U8_TWIDDLE]=
		//	g_pu8RamBase[((rdram_address_reg & 0xFFFFFF)+i)^U8_TWIDDLE];
		//}
	}
}

//*****************************************************************************
//Heavily based on Mupen
//*****************************************************************************
void DMA_SP_CopyToRDRAM()
{
	u32 spmem_address_reg = Memory_SP_GetRegister(SP_MEM_ADDR_REG);
	u32 rdram_address_reg = Memory_SP_GetRegister(SP_DRAM_ADDR_REG);
	u32 wrlen_reg         = Memory_SP_GetRegister(SP_WR_LEN_REG);
//	u32 i;
    
	if ((spmem_address_reg & 0x1000) > 0)
	{
		memcpy_vfpu(&g_pu8RamBase[(rdram_address_reg & 0xFFFFFF)],
					&g_pu8SpImemBase[(spmem_address_reg & 0xFFF)],
					(wrlen_reg & 0xFFF)+1 ); 
		
		//for (i=0; i<((wrlen_reg & 0xFFF)+1); i++)
		//{
		//	g_pu8RamBase[((rdram_address_reg & 0xFFFFFF)+i)^U8_TWIDDLE]=
		//	g_pu8SpImemBase[((spmem_address_reg & 0xFFF)+i)^U8_TWIDDLE];
		//}
	}
	else
	{
		memcpy_vfpu(&g_pu8RamBase[(rdram_address_reg & 0xFFFFFF)],
					&g_pu8SpDmemBase[(spmem_address_reg & 0xFFF)],
					(wrlen_reg & 0xFFF)+1 ); 

		//for (i=0; i<((wrlen_reg & 0xFFF)+1); i++)
		//{
		//	g_pu8RamBase[((rdram_address_reg & 0xFFFFFF)+i)^U8_TWIDDLE]=
		//	g_pu8SpDmemBase[((spmem_address_reg & 0xFFF)+i)^U8_TWIDDLE];
		//}
	}
}
//*****************************************************************************
// Copy 64bytes from DRAM to PIF_RAM
//*****************************************************************************
void DMA_SI_CopyFromDRAM( )
{
	u32 mem = Memory_SI_GetRegister(SI_DRAM_ADDR_REG) & 0x1fffffff;
	u8 * p_dst = (u8 *)g_pMemoryBuffers[MEM_PIF_RAM] + 0x7C0;
	u8 * p_src = g_pu8RamBase + mem;

	DPF( DEBUG_MEMORY_PIF, "DRAM (0x%08x) -> PIF Transfer ", mem );

	memcpy_vfpu(p_dst, p_src, 64);

#ifndef DAEDALUS_PUBLIC_RELEASE
	u8 control_byte = p_dst[ 63 ^ U8_TWIDDLE ];

	if ( control_byte > 0x01 )
	{
		DBGConsole_Msg(0, "[WTransfer wrote 0x%02x to the status reg]", control_byte );
	}
#endif

	Memory_SI_SetRegisterBits(SI_STATUS_REG, SI_STATUS_INTERRUPT);
	Memory_MI_SetRegisterBits(MI_INTR_REG, MI_INTR_SI);
	R4300_Interrupt_UpdateCause3();
}

//*****************************************************************************
// Copy 64bytes to DRAM from PIF_RAM
//*****************************************************************************
void DMA_SI_CopyToDRAM( )
{
	u32 mem = Memory_SI_GetRegister(SI_DRAM_ADDR_REG) & 0x1fffffff;
	u8 * p_src = (u8 *)g_pMemoryBuffers[MEM_PIF_RAM] + 0x7C0;
	u8 * p_dst = g_pu8RamBase + mem;

	DPF( DEBUG_MEMORY_PIF, "PIF -> DRAM (0x%08x) Transfer ", mem );

	// Check controller status!
	CController::Get()->Process();

	memcpy_vfpu(p_dst, p_src, 64);

	Memory_SI_SetRegisterBits(SI_STATUS_REG, SI_STATUS_INTERRUPT);
	Memory_MI_SetRegisterBits(MI_INTR_REG, MI_INTR_SI);
	R4300_Interrupt_UpdateCause3();
}



/*
#define PI_DOM2_ADDR1		0x05000000	// to 0x05FFFFFF
#define PI_DOM1_ADDR1		0x06000000	// to 0x07FFFFFF
#define PI_DOM2_ADDR2		0x08000000	// to 0x0FFFFFFF
#define PI_DOM1_ADDR2		0x10000000	// to 0x1FBFFFFF
#define PI_DOM1_ADDR3		0x1FD00000	// to 0x7FFFFFFF
*/

#define IsDom1Addr1( x )		( (x) >= PI_DOM1_ADDR1 && (x) < PI_DOM2_ADDR2 )
#define IsDom1Addr2( x )		( (x) >= PI_DOM1_ADDR2 && (x) < 0x1FBFFFFF )
#define IsDom1Addr3( x )		( (x) >= PI_DOM1_ADDR3 && (x) < 0x7FFFFFFF )
#define IsDom2Addr1( x )		( (x) >= PI_DOM2_ADDR1 && (x) < PI_DOM1_ADDR1 )
#define IsDom2Addr2( x )		( (x) >= PI_DOM2_ADDR2 && (x) < PI_DOM1_ADDR2 )

//*****************************************************************************
//
//*****************************************************************************
bool DMA_HandleTransfer( u8 * p_dst, u32 dst_offset, u32 dst_size, const u8 * p_src, u32 src_offset, u32 src_size, u32 length )
{
	if( ( s32( length ) < 0 ) ||
		(src_offset + length) > src_size ||
		(dst_offset + length) > dst_size )
	{
		return false;
	}

	//Todo:Try to optimize futher Little Endian code
	//Big Endian
	//memcpy(&p_dst[dst_offset],  &p_src[src_offset], length);

	//Little Endian
	// We only have to fiddle the bytes when
	// a) the src is not word aligned
	// b) the dst is not word aligned
	// c) the length is not a multiple of 4 (although we can copy most directly)
	// If the source/dest are word aligned, we can simply copy most of the
	// words using memcpy. Any remaining bytes are then copied individually
	if((dst_offset & 0x3) == 0 && (src_offset & 0x3) == 0)
	{
		// Optimise for u32 alignment - do multiple of four using memcpy
		u32 block_length(length & ~0x3);

		// Might be 0 if xref is less than 4 bytes in total
		// memcpy_vfpu seems to fail here, maybe still needs some tweak for border copy cases to work properly //Corn
		//if(block_length)
			memcpy(&p_dst[dst_offset],  (void*)&p_src[src_offset], block_length);

		// Do remainder - this is only 0->3 bytes
		for(u32 i = block_length; i < length; ++i)
		{
			p_dst[(i + dst_offset)^U8_TWIDDLE] = p_src[(i + src_offset)^U8_TWIDDLE];
		}
	}
	else
	{
		for(u32 i = 0; i < length; ++i)
		{
			p_dst[(i + dst_offset)^U8_TWIDDLE] = p_src[(i + src_offset)^U8_TWIDDLE];
		}
	}
	return true;
}

//*****************************************************************************
//
//*****************************************************************************
void DMA_PI_CopyToRDRAM()
{
	u32 mem_address  = Memory_PI_GetRegister(PI_DRAM_ADDR_REG) & 0x00FFFFFF;
	u32 cart_address = Memory_PI_GetRegister(PI_CART_ADDR_REG)  & 0xFFFFFFFF;
	u32 pi_length_reg = (Memory_PI_GetRegister(PI_WR_LEN_REG) & 0xFFFFFFFF) + 1;

	DPF( DEBUG_MEMORY_PI, "PI: Copying %d bytes of data from 0x%08x to 0x%08x", pi_length_reg, cart_address, mem_address );

	bool copy_succeeded;

	if(pi_length_reg & 0x1)
	{
		DBGConsole_Msg(0, "PI Copy CART to RDRAM %db from %08X to %08X", pi_length_reg, cart_address|0xA0000000, mem_address);
		DBGConsole_Msg(0, "Warning, PI DMA, odd length");

		//This makes Doraemon 3 work !

		pi_length_reg ++;
	}

	if ( IsDom2Addr1( cart_address ) )
	{
		//DBGConsole_Msg(0, "[YReading from Cart domain 2/addr1]");
		const u8 *	p_src( (const u8 *)g_pMemoryBuffers[MEM_SAVE] );
		u32			src_size( MemoryRegionSizes[MEM_SAVE] );
		cart_address -= PI_DOM2_ADDR1;

		copy_succeeded = DMA_HandleTransfer( g_pu8RamBase, mem_address, gRamSize, p_src, cart_address, src_size, pi_length_reg );
	}
	else if ( IsDom1Addr1( cart_address ) )
	{
		//DBGConsole_Msg(0, "[YReading from Cart domain 1/addr1]");
		cart_address -= PI_DOM1_ADDR1;
		CPU_InvalidateICacheRange( 0x80000000 | mem_address, pi_length_reg );
		copy_succeeded = RomBuffer::CopyToRam( g_pu8RamBase, mem_address, gRamSize, cart_address, pi_length_reg );
	}
	else if ( IsDom2Addr2( cart_address ) )
	{
		//DBGConsole_Msg(0, "[YReading from Cart domain 2/addr2]");
		//DBGConsole_Msg(0, "PI: Copying %d bytes of data from 0x%08x to 0x%08x",
		//	pi_length_reg, cart_address, mem_address);

		const u8 *	p_src( (const u8 *)g_pMemoryBuffers[MEM_SAVE] );
		u32			src_size( ( MemoryRegionSizes[MEM_SAVE] ) );
		cart_address -= PI_DOM2_ADDR2;

		if (g_ROM.settings.SaveType != SAVE_TYPE_FLASH)
			copy_succeeded = DMA_HandleTransfer( g_pu8RamBase, mem_address, gRamSize, p_src, cart_address, src_size, pi_length_reg );
		else
			copy_succeeded = DMA_FLASH_CopyToDRAM(mem_address, cart_address, pi_length_reg);
	}
	else if ( IsDom1Addr2( cart_address ) )
	{
		//DBGConsole_Msg(0, "[YReading from Cart domain 1/addr2]");
		cart_address -= PI_DOM1_ADDR2;
		CPU_InvalidateICacheRange( 0x80000000 | mem_address, pi_length_reg );
		copy_succeeded = RomBuffer::CopyToRam( g_pu8RamBase, mem_address, gRamSize, cart_address, pi_length_reg );
	}
	else if ( IsDom1Addr3( cart_address ))
	{
		//DBGConsole_Msg(0, "[YReading from Cart domain 1/addr3]");
		cart_address -= PI_DOM1_ADDR3;
		CPU_InvalidateICacheRange( 0x80000000 | mem_address, pi_length_reg );
		copy_succeeded = RomBuffer::CopyToRam( g_pu8RamBase, mem_address, gRamSize, cart_address, pi_length_reg );
	}
	else
	{
		DBGConsole_Msg(0, "[YUnknown PI Address 0x%08x]", cart_address);

		Memory_PI_ClrRegisterBits(PI_STATUS_REG, PI_STATUS_DMA_BUSY);
		Memory_MI_SetRegisterBits(MI_INTR_REG, MI_INTR_PI);
		R4300_Interrupt_UpdateCause3();
		return;
	}

	if(copy_succeeded)
	{
		s_nTotalDmaTransferSize += pi_length_reg;

#ifdef DAEDALUS_ENABLE_OS_HOOKS
		// Note if dwRescanCount is 0, the rom is only scanned when the
		// ROM jumps to the game boot address
		if (s_nNumDmaTransfers == 0 || s_nNumDmaTransfers == g_ROM.settings.RescanCount)
		{
			// Try to reapply patches - certain roms load in more of the OS after
			// a number of transfers
			Patch_ApplyPatches();
		}
		s_nNumDmaTransfers++;
#endif

		//CDebugConsole::Get()->Stats( STAT_PI, "PI: C->R %d %dMB", s_nNumDmaTransfers, s_nTotalDmaTransferSize/(1024*1024));
	}
// XXX irrelevant to end user
#ifndef DAEDALUS_PUBLIC_RELEASE
	else
	{
		// Road Rash triggers this !
		DBGConsole_Msg(0, "PI: Copying 0x%08x bytes of data from 0x%08x to 0x%08x",
			Memory_PI_GetRegister(PI_WR_LEN_REG),
			Memory_PI_GetRegister(PI_CART_ADDR_REG),
			Memory_PI_GetRegister(PI_DRAM_ADDR_REG));
		DBGConsole_Msg(0, "PIXFer: Copy overlaps RAM/ROM boundary");
		DBGConsole_Msg(0, "PIXFer: Not copying, but issuing interrupt");
	}
#endif
	// Is this a hack?
	if (!gDMAUsed)
	{ 
		gDMAUsed = true;
		
		if (g_ROM.cic_chip != CIC_6105)
			Write32Bits(0x80000318, gRamSize);
		else
			Write32Bits(0x800003F0, gRamSize);
	}

	Memory_PI_ClrRegisterBits(PI_STATUS_REG, PI_STATUS_DMA_BUSY);
	Memory_MI_SetRegisterBits(MI_INTR_REG, MI_INTR_PI);
	R4300_Interrupt_UpdateCause3();
}

//*****************************************************************************
//
//*****************************************************************************
void DMA_PI_CopyFromRDRAM()
{
	u32 mem_address  = Memory_PI_GetRegister(PI_DRAM_ADDR_REG) & 0xFFFFFFFF;
	u32 cart_address = Memory_PI_GetRegister(PI_CART_ADDR_REG)  & 0xFFFFFFFF;
	u32 pi_length_reg = (Memory_PI_GetRegister(PI_RD_LEN_REG)  & 0xFFFFFFFF) + 1;

	DPF(DEBUG_MEMORY_PI, "PI: Copying %d bytes of data from 0x%08x to 0x%08x", pi_length_reg, mem_address, cart_address );

	bool copy_succeeded;

	if(pi_length_reg & 0x1)
	{
		DBGConsole_Msg(0, "PI Copy RDRAM to CART %db from %08X to %08X", pi_length_reg, cart_address|0xA0000000, mem_address);
		DBGConsole_Msg(0, "Warning, PI DMA, odd length");

		// Tonic Trouble triggers this !

		pi_length_reg ++;
	}

	if ( IsDom2Addr1( cart_address ) ) //  0x05000000
	{
		//DBGConsole_Msg(0, "[YWriting to Cart domain 2/addr1]");
		u8 *	p_dst( (u8 *)g_pMemoryBuffers[MEM_SAVE] );
		u32		dst_size( MemoryRegionSizes[MEM_SAVE] );
		cart_address -= PI_DOM2_ADDR1;

		copy_succeeded = DMA_HandleTransfer( p_dst, cart_address, dst_size, g_pu8RamBase, mem_address, gRamSize, pi_length_reg );
		Save::MarkSaveDirty();
	}
	else if ( IsDom1Addr1( cart_address ) )
	{
		//DBGConsole_Msg(0, "[YWriting to Cart domain 1/addr1]");
		cart_address -= PI_DOM1_ADDR1;

		copy_succeeded = RomBuffer::CopyFromRam( cart_address, g_pu8RamBase, mem_address, gRamSize, pi_length_reg );
	}
	else if ( IsDom2Addr2( cart_address ) )
	{
		//DBGConsole_Msg(0, "[YWriting to Cart domain 2/addr2]");
		//DBGConsole_Msg(0, "PI: Copying %d bytes of data from 0x%08x to 0x%08x",
		//	pi_length_reg, mem_address, cart_address);
		u8 *	p_dst( (u8 *)g_pMemoryBuffers[MEM_SAVE] );
		u32		dst_size( MemoryRegionSizes[MEM_SAVE] );
		cart_address -= PI_DOM2_ADDR2;

		if (g_ROM.settings.SaveType != SAVE_TYPE_FLASH)
			copy_succeeded = DMA_HandleTransfer( p_dst, cart_address, dst_size, g_pu8RamBase, mem_address, gRamSize, pi_length_reg );
		else
			copy_succeeded = DMA_FLASH_CopyFromDRAM(mem_address, cart_address, pi_length_reg);
		Save::MarkSaveDirty();
	}
	else if ( IsDom1Addr2( cart_address ) )
	{
		//DBGConsole_Msg(0, "[YWriting to Cart domain 1/addr2]");
		cart_address -= PI_DOM1_ADDR2;
		copy_succeeded = RomBuffer::CopyFromRam( cart_address, g_pu8RamBase, mem_address, gRamSize, pi_length_reg );
	}
	else if ( IsDom1Addr3( cart_address ) )
	{
		//DBGConsole_Msg(0, "[YWriting to Cart domain 1/addr3]");
		cart_address -= PI_DOM1_ADDR3;
		copy_succeeded = RomBuffer::CopyFromRam( cart_address, g_pu8RamBase, mem_address, gRamSize, pi_length_reg );
	}
	else
	{
		DBGConsole_Msg(0, "[YUnknown PI Address 0x%08x]", cart_address);
		Memory_PI_ClrRegisterBits(PI_STATUS_REG, PI_STATUS_DMA_BUSY);
		Memory_MI_SetRegisterBits(MI_INTR_REG, MI_INTR_PI);
		R4300_Interrupt_UpdateCause3();
		return;
	}
// XXX irrelevant to end user
#ifndef DAEDALUS_PUBLIC_RELEASE
	if (copy_succeeded)
	{
		// Nothing to do
	}
	else
	{
		DBGConsole_Msg(0, "PI: Copying %d bytes of data from 0x%08x to 0x%08x",
			pi_length_reg, mem_address, cart_address);
		DBGConsole_Msg(0, "PIXFer: Copy overlaps RAM/ROM boundary");
		DBGConsole_Msg(0, "PIXFer: Not copying, but issuing interrupt");
	}
#endif

	Memory_PI_ClrRegisterBits(PI_STATUS_REG, PI_STATUS_DMA_BUSY);
	Memory_MI_SetRegisterBits(MI_INTR_REG, MI_INTR_PI);
	R4300_Interrupt_UpdateCause3();
}

