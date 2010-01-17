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
bool gLogSpDMA = false;


//*****************************************************************************
//
//*****************************************************************************
void DMA_SP_CopyFromRDRAM()
{
	u32 i;
	u32 spmem_address_reg = Memory_SP_GetRegister(SP_MEM_ADDR_REG);
	u32 rdram_address_reg = Memory_SP_GetRegister(SP_DRAM_ADDR_REG);
	u32 rdlen_reg         = Memory_SP_GetRegister(SP_RD_LEN_REG);

	u32 rdram_address = (rdram_address_reg&0x00FFFFFF)	& ~7;	// Align to 8 byte boundary
	u32 spmem_address = (spmem_address_reg&0x1FFF)		& ~7;	// Align to 8 byte boundary
	u32 length = ((rdlen_reg    )&0x0FFF) | 7;					// Round up to 8 bytes
	u32 count  = ((rdlen_reg>>12)&0x00FF);
	u32 skip   = ((rdlen_reg>>20)&0x0FFF);

	u32 rdram_address_end = rdram_address + ( ((length+1)+skip) * (count+1) );
//	u32 spmem_address_end = spmem_address + ( ((length+1)     ) * (count+1) );
	u32 spmem_segment_end = ( spmem_address_reg & 0x0FFF ) + ( (length+1) * (count+1) );

	//
	// Perform a couple of sanity checks to ensure that we don't overflow
	//
	if ( rdram_address_end > gRamSize )
	{
		DBGConsole_Msg( 0, "SP DMA to RDRAM overflows" );
		return;
	}

	if ( spmem_segment_end > 0x1000 )
	{
//		DBGConsole_Msg( 0, "SP DMA to RDRAM crosses memory segment" );
		return;
	}

	if ( gLogSpDMA )
	{
		DBGConsole_Msg( 0, "SP: DMA 0x%08x <- 0x%08x L:%08x C:%02x S:%03x", spmem_address, rdram_address, length+1, count, skip );
	}

//Todo:Try to optimize futher Little Endian code
//Big Endian
/*	for ( int c = 0; c <= count; c++ )
	{
		memcpy( &g_pu8SpMemBase[spmem_address], &g_pu8RamBase[rdram_address], length+1 );

		rdram_address+= length+1;
		spmem_address+= length+1;
		rdram_address += skip;
		// rdram_address has advanced by length + skip. spmem_address has advanced by length+1
	}*/

//Little Endian
	if (skip != 0 || count != 0)
	{
		u32 i, c;

		for ( c = 0; c <= count; c++ )
		{
			for ( i = 0; i <= length; i++ )
			{
				g_pu8SpMemBase[spmem_address^U8_TWIDDLE] = g_pu8RamBase[rdram_address^U8_TWIDDLE];

				rdram_address++;
				spmem_address++;
			}
			rdram_address += skip;
			// rdram_address has advanced by length + skip. spmem_address has advanced by length+1
		}
	}
	else
	{
		length++;

		if ((rdram_address & 0x3) == 0 &&
			(spmem_address & 0x3) == 0)
		{
			// Optimise for u32 alignment

			u32 aligned_length = length & ~0x3;

			//
			// Do multiple of four using memcpy
			//
			if ( aligned_length )		// Might be 0 if xfer is less than 4 bytes in total
			{
				memcpy( &g_pu8SpMemBase[spmem_address], &g_pu8RamBase[rdram_address], aligned_length );
			}

			//
			// Do remainder - this is only 0->3 bytes
			//
			for (i = aligned_length; i < length; i++)
			{
				g_pu8SpMemBase[(i + spmem_address)^U8_TWIDDLE] = g_pu8RamBase[(i + rdram_address)^U8_TWIDDLE];
			}
		}
		else
		{
			for (i = 0; i < length; i++)
			{
				g_pu8SpMemBase[(i + spmem_address)^U8_TWIDDLE] = g_pu8RamBase[(i + rdram_address)^U8_TWIDDLE];
			}
			DBGConsole_Msg(0, "Couldn't optimise: 0x%08x 0x%08x 0x%08x",
				spmem_address, rdram_address, length);
		}
	}

	s_nTotalSPTransferSize += length;
	s_nNumSPTransfers++;

	//CDebugConsole::Get()->Stats( STAT_SP, "SP: S<-R %d %dMB", s_nNumSPTransfers, s_nTotalSPTransferSize/(1024*1024));

	Memory_SP_SetRegister(SP_DMA_BUSY_REG, 0);
	Memory_SP_ClrRegisterBits(SP_STATUS_REG, SP_STATUS_DMA_BUSY);
}


//*****************************************************************************
//
//*****************************************************************************
void DMA_SP_CopyToRDRAM()
{
	u32 spmem_address_reg = Memory_SP_GetRegister(SP_MEM_ADDR_REG);
	u32 rdram_address_reg = Memory_SP_GetRegister(SP_DRAM_ADDR_REG);
	u32 wrlen_reg         = Memory_SP_GetRegister(SP_WR_LEN_REG);

	u32 rdram_address = (rdram_address_reg&0x00FFFFFF)	& ~7;	// Align to 8 byte boundary
	u32 spmem_address = (spmem_address_reg&0x1FFF)		& ~7;	// Align to 8 byte boundary
	u32 length = ((wrlen_reg    )&0x0FFF) | 7;					// Round up to 8 bytes
	u32 count  = ((wrlen_reg>>12)&0x00FF);
	u32 skip   = ((wrlen_reg>>20)&0x0FFF);

	u32 rdram_address_end = rdram_address + ( ((length+1)+skip) * (count+1) );
	u32 spmem_address_end = spmem_address + ( ((length+1)     ) * (count+1) );
	u32 spmem_segment_end = ( spmem_address_reg & 0x0FFF ) + ( (length+1) * (count+1) );


	use(spmem_address_end);

	//
	// Perform a couple of sanity checks to ensure that we don't overflow
	//
	if ( rdram_address_end > gRamSize )
	{
		DBGConsole_Msg( 0, "SP DMA to RDRAM overflows" );
		return;
	}

	if ( spmem_segment_end > 0x1000 )
	{
		DBGConsole_Msg( 0, "SP DMA to RDRAM crosses memory segment" );
		return;
	}

	if ( gLogSpDMA )
	{
		DBGConsole_Msg( 0, "SP: DMA 0x%08x -> 0x%08x L:%08x C:%02x S:%03x", spmem_address, rdram_address, length+1, count, skip );
	}

//Todo:Try to optimize futher Little Endian code
//Big Endian
/*	for ( u32 c = 0; c <= count; c++ )
	{
		memcpy( &g_pu8RamBase[rdram_address], &g_pu8SpMemBase[spmem_address], length+1 );
		rdram_address+= length+1;
		spmem_address+= length+1;
		rdram_address += skip;
		// rdram_address has advanced by length+1 + skip. spmem_address has advanced by length+1
	}*/

//Little Endian
	for ( u32 c = 0; c <= count; c++ )
	{
		for ( u32 i = 0; i <= length; i++ )
		{
			g_pu8RamBase[rdram_address^U8_TWIDDLE] = g_pu8SpMemBase[spmem_address^U8_TWIDDLE];

			rdram_address++;
			spmem_address++;
		}
		rdram_address += skip;
		// rdram_address has advanced by length+1 + skip. spmem_address has advanced by length+1
	}

	DAEDALUS_ASSERT_Q( rdram_address == rdram_address_end );
	DAEDALUS_ASSERT_Q( spmem_address == spmem_address_end );

	Memory_SP_SetRegister(SP_DMA_BUSY_REG, 0);
	Memory_SP_ClrRegisterBits(SP_STATUS_REG, SP_STATUS_DMA_BUSY);
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

	memcpy(p_dst, p_src, 64);

	u8 control_byte = p_dst[ 63 ^ U8_TWIDDLE ];

	if ( control_byte > 0x01 )
	{
		DBGConsole_Msg(0, "[WTransfer wrote 0x%02x to the status reg]", control_byte );
	}

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

	memcpy(p_dst, p_src, 64);

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
//	memcpy(&p_dst[dst_offset],  &p_src[src_offset], length);

//Little Endian
	// We only have to fiddle the bytes when
	// a) the src is not word aligned
	// b) the dst is not word aligned
	// c) the length is not a multiple of 4 (although we can copy most directly)
	// If the source/dest are word aligned, we can simply copy most of the
	// words using memcpy. Any remaining bytes are then copied individually
	if((dst_offset & 0x3) == 0 &&
	   (src_offset & 0x3) == 0)
	{
		// Optimise for u32 alignment - do multiple of four using memcpy
		u32 block_length(length & ~0x3);

		// Might be 0 if xref is less than 4 bytes in total
		if(block_length > 0)
		{
			memcpy(&p_dst[dst_offset],  &p_src[src_offset], block_length);
		}

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
	else
	{
		DBGConsole_Msg(0, "PI: Copying 0x%08x bytes of data from 0x%08x to 0x%08x",
			Memory_PI_GetRegister(PI_WR_LEN_REG),
			Memory_PI_GetRegister(PI_CART_ADDR_REG),
			Memory_PI_GetRegister(PI_DRAM_ADDR_REG));
		DBGConsole_Msg(0, "PIXFer: Copy overlaps RAM/ROM boundary");
		DBGConsole_Msg(0, "PIXFer: Not copying, but issuing interrupt");
	}

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

	Memory_PI_ClrRegisterBits(PI_STATUS_REG, PI_STATUS_DMA_BUSY);
	Memory_MI_SetRegisterBits(MI_INTR_REG, MI_INTR_PI);
	R4300_Interrupt_UpdateCause3();
}

