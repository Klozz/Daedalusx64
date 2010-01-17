/*
Copyright (C) 2003 Azimer
Copyright (C) 2001,2006 StrmnNrmn

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
//	N.B. This source code is derived from Azimer's Audio plugin (v0.55?)
//	and modified by StrmnNrmn to work with Daedalus PSP. Thanks Azimer!
//	Drop me a line if you get chance :)
//

#include "stdafx.h"
#include "audiohle.h"
#include "AudioHLEProcessor.h"

// Audio UCode lists
//     Dummy UCode Handler for UCode detection... (Will always assume UCode1 until the nth list is executed)
extern AudioHLEInstruction SafeABI[0x20];
//---------------------------------------------------------------------------------------------
//
//     ABI 1 : Mario64, WaveRace USA, Golden Eye 007, Quest64, SF Rush
//				 60% of all games use this.  Distributed 3rd Party ABI
//
extern AudioHLEInstruction ABI1[0x20];
//---------------------------------------------------------------------------------------------
//
//     ABI 2 : WaveRace JAP, MarioKart 64, Mario64 JAP RumbleEdition, 
//				 Yoshi Story, Pokemon Games, Zelda64, Zelda MoM (miyamoto) 
//				 Most NCL or NOA games (Most commands)
extern AudioHLEInstruction ABI2[0x20];
//---------------------------------------------------------------------------------------------
//
//     ABI 3 : DK64, Perfect Dark, Banjo Kazooi, Banjo Tooie
//				 All RARE games except Golden Eye 007
//
extern AudioHLEInstruction ABI3[0x20];
//---------------------------------------------------------------------------------------------
//
//     ABI 5 : Factor 5 - MoSys/MusyX
//				 Rogue Squadron, Tarzan, Hydro Thunder, and TWINE
//				 Indiana Jones and Battle for Naboo (?)
//---------------------------------------------------------------------------------------------
//
//     ABI ? : Unknown or unsupported UCode
//
extern AudioHLEInstruction ABIUnknown[0x20];
//---------------------------------------------------------------------------------------------

AudioHLEInstruction ABI[0x20];
bool locklistsize = false;

//---------------------------------------------------------------------------------------------
// Set the Current ABI
void ChangeABI (int type) {
	switch (type) {
		case 0x0:
			//MessageBox (NULL, "ABI set to AutoDetect", "Audio ABI Changed", MB_OK);
			memcpy (ABI, SafeABI, 0x20*4);
		break;
		case 0x1:
			//MessageBox (NULL, "ABI set to ABI 1", "Audio ABI Changed", MB_OK);
			memcpy (ABI, ABI1, 0x20*4);
		break;
		case 0x2:
			//MessageBox (NULL, "ABI set to ABI 2", "Audio ABI Changed", MB_OK);
			//MessageBox (NULL, "Mario Kart, Zelda64, Zelda MoM, WaveRace JAP, etc. are not supported right now...", "Audio ABI Changed", MB_OK);
			//memcpy (ABI, ABIUnknown, 0x20*4);
			memcpy (ABI, ABI2, 0x20*4);
		break;
		case 0x3:
			//MessageBox (NULL, "ABI set to ABI 3", "Audio ABI Changed", MB_OK);
			//MessageBox (NULL, "DK64, Perfect Dark, Banjo Kazooi, Banjo Tooie, (RARE), not supported yet...", "Audio ABI Changed", MB_OK);
			//memcpy (ABI, ABIUnknown, 0x20*4);
			memcpy (ABI, ABI3, 0x20*4);
		break;
		/*case 0x4: // Mario Kart, Zelda64 (Demo Version)
			memcpy (ABI, ABI2, 0x20*4);
		break;*/
		case 0x5:
			DAEDALUS_ERROR ("ABI set to ABI 5");
			//MessageBox (NULL, "Rogue Squadron, Tarzan, Hydro Thunder, Indiana Jones and Battle for Naboo, and TWINE not supported yet...", "Audio ABI Not Supported", MB_OK);
//     ABI 5 : Factor 5 - MoSys/MusyX
//				 Rogue Squadron, Tarzan, Hydro Thunder, and TWINE
//				 Indiana Jones and Battle for Naboo (?)
			memcpy (ABI, ABIUnknown, 0x20*4);
			//memcpy (ABI, ABI3, 0x20*4);
		break;
		default:
			DAEDALUS_ERROR("ABI set to ABI Unknown");
			memcpy (ABI, ABIUnknown, 0x20*4);
			return; // Quick out to prevent Dynarec from getting it...
	}
}

//---------------------------------------------------------------------------------------------

u32 UCData, UDataLen;

void HLEStart()
{
	u32 base, dmembase;

	u32 List  = ((u32*)dmem)[0xFF0/4], ListLen = ((u32*)dmem)[0xFF4/4];
	u32 *HLEPtr= (u32 *)(rdram+List);
	
	UCData= ((u32*)dmem)[0xFD8/4];
	UDataLen= ((u32*)dmem)[0xFDC/4];
	base = ((u32*)dmem)[0xFD0/4];
	dmembase = ((u32*)dmem)[0xFD8/4];

	gAudioHLEState.LoopVal = 0;
	memset( gAudioHLEState.Segments, 0, sizeof( gAudioHLEState.Segments ) );

	//memcpy (imem+0x80, rdram+((u32*)dmem)[0xFD0/4], ((u32*)dmem)[0xFD4/4]);

	ListLen = ListLen >> 2;

	if (*(u32*)(rdram+UCData+0x30) == 0x0B396696) {
		ChangeABI (5); // This will be replaced with ProcessMusyX
		return;
	}


	for (u32 x=0; x < ListLen; x+=2)
	{
		AudioHLECommand command;

		command.cmd0 = HLEPtr[x  ];
		command.cmd1 = HLEPtr[x+1];
		ABI[command.cmd]( command );
	}
}

static void SPU( AudioHLECommand command )
{
}

AudioHLEInstruction ABIUnknown [0x20] = { // Unknown ABI
	SPU, SPU, SPU, SPU, SPU, SPU, SPU, SPU,
	SPU, SPU, SPU, SPU, SPU, SPU, SPU, SPU,
	SPU, SPU, SPU, SPU, SPU, SPU, SPU, SPU,
	SPU, SPU, SPU, SPU, SPU, SPU, SPU, SPU
};
