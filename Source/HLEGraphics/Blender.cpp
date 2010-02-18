/*
Copyright (C) 2010 StrmnNrmn

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

#include "stdafx.h"

#include "RDP.h"
#include "DebugDisplayList.h"
#include "OSHLE/ultra_gbi.h"

#include <pspgu.h>


/*
// P or M
#define	G_BL_CLR_IN	0
#define	G_BL_CLR_MEM	1
#define	G_BL_CLR_BL	2
#define	G_BL_CLR_FOG	3

// A inputs
#define	G_BL_A_IN	0
#define	G_BL_A_FOG	1
#define	G_BL_A_SHADE	2
#define	G_BL_0		3

// B inputs
#define	G_BL_1MA	0
#define	G_BL_A_MEM	1
#define	G_BL_1		2
#define	G_BL_0		3

#define	GBL_c1(m1a, m1b, m2a, m2b)	\
	(m1a) << 30 | (m1b) << 26 | (m2a) << 22 | (m2b) << 18
#define	GBL_c2(m1a, m1b, m2a, m2b)	\
	(m1a) << 28 | (m1b) << 24 | (m2a) << 20 | (m2b) << 16

#define	RM_AA_ZB_OPA_SURF(clk)					\
	AA_EN | Z_CMP | Z_UPD | IM_RD | CVG_DST_CLAMP |		\
	ZMODE_OPA | ALPHA_CVG_SEL |				\
	GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)

#define	G_RM_FOG_SHADE_A		GBL_c1(G_BL_CLR_FOG, G_BL_A_SHADE, G_BL_CLR_IN, G_BL_1MA)
#define	G_RM_FOG_PRIM_A			GBL_c1(G_BL_CLR_FOG, G_BL_A_FOG, G_BL_CLR_IN, G_BL_1MA)
#define	G_RM_PASS				GBL_c1(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)
#define	RM_AA_ZB_OPA_SURF(clk)	GBL_c##clk(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_A_MEM)
*/

//*****************************************************************************
//
//*****************************************************************************

// Todo : Refactored futher this, add notes and instruction to add new blendings.

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
const char * sc_szBlClr[4] = { "In",  "Mem",  "Bl",     "Fog" };
const char * sc_szBlA1[4]  = { "AIn", "AFog", "AShade", "0" };
const char * sc_szBlA2[4]  = { "1-A", "AMem", "1",      "?" };
#endif

#define MAKE_BLEND_MODE( a, b )			( (a) | (b) )
#define BLEND_NOOP1				0x00000000		//GBL_c1(G_BL_CLR_IN, G_BL_1MA, G_BL_CLR_IN, G_BL_1MA)
#define BLEND_NOOP2				0x00000000

#define BLEND_FOG_ASHADE1		0xc8000000		// Fog * AShade + In * 1-A
#define BLEND_FOG_ASHADE2		0xc8000000		// Fog * AShade + In * 1-A
#define BLEND_FOG_APRIM1		0xc4000000

#define BLEND_PASS1				0x0c080000		//GBL_c1(G_BL_CLR_IN, G_BL_0, G_BL_CLR_IN, G_BL_1)
#define BLEND_PASS2				0x03020000

#define BLEND_OPA1				0x00440000
#define BLEND_OPA2				0x00110000

#define BLEND_XLU1				0x00400000
#define BLEND_XLU2				0x00100000

#define BLEND_ADD1				0x04400000		//GBL_c##clk(G_BL_CLR_IN, G_BL_A_FOG, G_BL_CLR_MEM, G_BL_1)
#define BLEND_ADD2				0x01100000

#define BLEND_BI_AFOG			0x84000000		// Bl * AFog + In * 1-A

#define BLEND_MEM1				0x4c400000		// Mem*0 + Mem*(1-0)?!
#define BLEND_MEM2				0x13100000		// Mem*0 + Mem*(1-0)?!

#define BLEND_NOOP3				0x0c480000		// In * 0 + Mem * 1
#define BLEND_NOOP4				0xcc080000		// Fog * 0 + In * 1


//*****************************************************************************
//
//*****************************************************************************

void InitBlenderMode()					// Set Alpha Blender mode
{


	u32 blendmode = u32( gRDPOtherMode._u64 & 0xffff0000 );
	u32 blendmode_1 = u32( gRDPOtherMode.blender & 0xcccc );
	u32 blendmode_2 = u32( gRDPOtherMode.blender & 0x3333 );
	u32 cycletype = gRDPOtherMode.cycle_type;

	int		blend_op = GU_ADD;
	int		blend_src = GU_SRC_ALPHA;
	int		blend_dst = GU_ONE_MINUS_SRC_ALPHA;
	bool	enable_blend( false );

	if ( gRDPOtherMode.cycle_type == G_CYC_FILL )
	{
		enable_blend = false;
	}
	else
	{
		switch( blendmode )
		{
		case MAKE_BLEND_MODE( BLEND_PASS1, BLEND_PASS2 ):
		case MAKE_BLEND_MODE( BLEND_FOG_ASHADE1, BLEND_OPA2 ):
		case MAKE_BLEND_MODE( BLEND_OPA1, BLEND_OPA2 ):
		case MAKE_BLEND_MODE( BLEND_OPA1, BLEND_NOOP2 ):
		case MAKE_BLEND_MODE( BLEND_PASS1, BLEND_OPA2 ):
		case MAKE_BLEND_MODE( BLEND_MEM1, BLEND_MEM2 ):
			// Used on SSV - TV window
		case MAKE_BLEND_MODE( BLEND_NOOP4, BLEND_NOOP2 ):
			// Blender: c800 - Cycle1:  Fog * AShade + In * 1-A 3200 - Cycle2:  Fog * AShade + In * 1-A
			// Used on the F-Zero - Tracks Correction
		case MAKE_BLEND_MODE( BLEND_FOG_ASHADE1, 0x32000000 ):
			// Blender: c800 - Cycle1:  Fog * AShade + In * 1-A 0302 - Cycle2:  In * 0 + In * 1
			// Used on Hey You Pikachu ! - Body and Face Correction, Automobili
		case MAKE_BLEND_MODE( BLEND_FOG_ASHADE1, BLEND_PASS2 ):
			// Blender: c800 - Cycle1:  Fog * AShade + In * 1-A 0000 - Cycle2:  In * AIn + In * 1-A
			// Used on the F-Zero - Cars Correction
		case MAKE_BLEND_MODE( BLEND_FOG_ASHADE1, BLEND_NOOP1 ):
			enable_blend = false;
			break;
		case MAKE_BLEND_MODE( BLEND_NOOP1, BLEND_NOOP2 ):
		case MAKE_BLEND_MODE( BLEND_XLU1, BLEND_XLU2 ):
		case MAKE_BLEND_MODE( BLEND_XLU1, BLEND_NOOP2 ):
		case MAKE_BLEND_MODE( BLEND_PASS1, BLEND_XLU2 ):
		case MAKE_BLEND_MODE( BLEND_FOG_ASHADE1, BLEND_XLU2 ):
			// Blender: 0000 - Cycle1:  In * AIn + In * 1-A 0010 - Cycle2:  In * AIn + Mem * 1-A
			// Used on Hey You Pikachu ! - Shade Correction
		case MAKE_BLEND_MODE( BLEND_NOOP1, BLEND_XLU2 ):
			blend_op = GU_ADD; blend_src = GU_SRC_ALPHA; blend_dst = GU_ONE_MINUS_SRC_ALPHA;
			enable_blend = true;
			break;
			// XXXX
			// Force Transparency
			blend_op = GU_ADD; blend_src = GU_SRC_COLOR; blend_dst = GU_DST_COLOR;
			enable_blend = true;
			break;
		default:
#ifdef DAEDALUS_DEBUG_DISPLAYLIST	// Might need to make my own tag?
			u32 m1A_1 = (gRDPOtherMode.blender>>14) & 0x3;
			u32 m1B_1 = (gRDPOtherMode.blender>>10) & 0x3;
			u32 m2A_1 = (gRDPOtherMode.blender>>6) & 0x3;
			u32 m2B_1 = (gRDPOtherMode.blender>>2) & 0x3;

			u32 m1A_2 = (gRDPOtherMode.blender>>12) & 0x3;
			u32 m1B_2 = (gRDPOtherMode.blender>>8) & 0x3;
			u32 m2A_2 = (gRDPOtherMode.blender>>4) & 0x3;
			u32 m2B_2 = (gRDPOtherMode.blender   ) & 0x3;

			DL_PF( "		 Blend: SRCALPHA/INVSRCALPHA (default: 0x%04x)", gRDPOtherMode.blender );
			DAEDALUS_ERROR( "Blender:%04x - Cycle1:%s * %s + %s * %s || %04x - Cycle2:%s * %s + %s * %s", blendmode_1,
					sc_szBlClr[m1A_1], sc_szBlA1[m1B_1], sc_szBlClr[m2A_1], sc_szBlA2[m2B_1], blendmode_2,
					sc_szBlClr[m1A_2], sc_szBlA1[m1B_2], sc_szBlClr[m2A_2], sc_szBlA2[m2B_2]);


#endif
			blend_op = GU_ADD; blend_src = GU_SRC_ALPHA; blend_dst = GU_ONE_MINUS_SRC_ALPHA;
			enable_blend = true;
			break;
		}
			
	}

	if( enable_blend )
	{
		sceGuBlendFunc( blend_op, blend_src, blend_dst, 0, 0);
		sceGuEnable( GU_BLEND );
	}
	else
	{
		sceGuDisable( GU_BLEND );
	}
}	
	
//*****************************************************************************
//
//*****************************************************************************