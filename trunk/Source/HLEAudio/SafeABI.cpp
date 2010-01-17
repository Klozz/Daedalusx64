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

extern AudioHLEInstruction ABI[0x20];

extern bool isMKABI;
extern bool isZeldaABI;

void smDetect( AudioHLECommand command )
{
	u32 UCData= ((u32*)dmem)[0xFD8/4];
	u8  *UData = rdram+UCData;

	memset( gAudioHLEState.Segments, 0, sizeof( gAudioHLEState.Segments ) );

	isMKABI = isZeldaABI = false;

	if (((u32*)UData)[0] != 0x1) { // Then it's either ABI 3, 4 or 5
		if (*(u32*)(UData+(0x30)) == 0x0B396696)
			ChangeABI(5);
		else
			ChangeABI(3);
	} else {
		if (*(u32*)(UData+(0x30)) == 0xF0000F00) // Should be common in ABI 1 :)
			ChangeABI(1);
		else {
			ChangeABI(2);  //We will default to ABI 2...
		}
	}
	ABI[command.cmd]( command ); // Do the command which was skipped :)
/*
	u32 Boot  = ((u32*)dmem)[0xFC8/4], BootLen = ((u32*)dmem)[0xFCC/4];
	u32 UC    = ((u32*)dmem)[0xFD0/4], UCLen   = ((u32*)dmem)[0xFD4/4];

	static int runonce = 0;
	static int alist=0;

	dfile = fopen ("d:\\HLEInfo.txt", "wt");

	fprintf (dfile, "--------- Audio List #%i --------\n\n", ++alist);
	fprintf (dfile, "LQV: %04X %04X %04X %04X %04X %04X %04X %04X\n", 
		((u16*)UData)[1], ((u16*)UData)[0], ((u16*)UData)[3], ((u16*)UData)[2],
		((u16*)UData)[5], ((u16*)UData)[4], ((u16*)UData)[7], ((u16*)UData)[6]);

	fprintf (dfile, "Functions: \n");
	for (int xx = 0; xx < 0x20; xx++) {
		fprintf (dfile, "%02X: %02X\n", xx, *(u16*)(UData+((0x10+(xx*2))^2)));
	}
	fclose (dfile);*/
}

AudioHLEInstruction SafeABI[0x20] =  // SafeMode UCode (used for Audio detection/experimentation)
{
    smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect,
    smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect,
    smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect,
    smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect, smDetect
};
