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

#include "AudioPluginPSP.h"
#include "HLEAudio/AudioCode.h"
#include "HLEAudio/audiohle.h"

#include "Core/Interrupt.h"
#include "Core/Memory.h"
#include "Core/ROM.h"

#include "ConfigOptions.h"

#include <pspkernel.h>


/* This sets default frequency what is used if rom doesn't want to change it.
   Probably only game that needs this is Zelda: Ocarina Of Time Master Quest 
   *NOTICE* We should try to find out why Demos' frequencies are always wrong
   They tend to rely on a default frequency, apparently, never the same one ;)*/

#define DEFAULT_FREQUENCY 33600	// Taken from Mupen64 : )

//*****************************************************************************
//
//*****************************************************************************
EAudioPluginMode gAudioPluginEnabled( APM_DISABLED );
bool gAdaptFrequency( false );

//*****************************************************************************
//
//*****************************************************************************
CAudioPluginPsp::CAudioPluginPsp()
:	mAudioCode( new AudioCode )
{
	mAudioCode->SetAdaptFrequency( gAdaptFrequency );
	//gAudioPluginEnabled = APM_ENABLED_SYNC; // for testing
}

//*****************************************************************************
//
//*****************************************************************************
CAudioPluginPsp::~CAudioPluginPsp()
{
	delete mAudioCode;
}

//*****************************************************************************
//
//*****************************************************************************
CAudioPluginPsp *	CAudioPluginPsp::Create()
{
	return new CAudioPluginPsp();
}

//*****************************************************************************
//
//*****************************************************************************
void	CAudioPluginPsp::SetAdaptFrequecy( bool adapt )
{
	mAudioCode->SetAdaptFrequency( adapt );
}

//*****************************************************************************
//
//*****************************************************************************
bool		CAudioPluginPsp::StartEmulation()
{
	return true;
}

//*****************************************************************************
//
//*****************************************************************************
void	CAudioPluginPsp::StopEmulation()
{
	mAudioCode->StopAudio();
}

//*****************************************************************************
//
//*****************************************************************************
void	CAudioPluginPsp::DacrateChanged( ESystemType system_type )
{
	//if( gAudioPluginEnabled == APM_DISABLED ) return;
	if( gAudioPluginEnabled > APM_DISABLED )
	{
		printf( "DacrateChanged( %d )\n", system_type );

		u32 dacrate = Memory_AI_GetRegister(AI_DACRATE_REG);

		u32	frequency = DEFAULT_FREQUENCY; // Is this correct? - Salvy
		switch (system_type)
		{
			case ST_NTSC: frequency = VI_NTSC_CLOCK / (dacrate + 1); break;
			case ST_PAL:  frequency = VI_PAL_CLOCK  / (dacrate + 1); break;
			case ST_MPAL: frequency = VI_MPAL_CLOCK / (dacrate + 1); break;
		}

		mAudioCode->SetFrequency( frequency );
	}
	else
	{
		// Do nothing as sound is disabled
	}

}

//*****************************************************************************
//
//*****************************************************************************
void	CAudioPluginPsp::LenChanged()
{
	if( gAudioPluginEnabled > APM_DISABLED )
	{
		mAudioCode->SetAdaptFrequency( gAdaptFrequency );

		u32		address( Memory_AI_GetRegister(AI_DRAM_ADDR_REG) & 0xFFFFFF );
		u32		length(Memory_AI_GetRegister(AI_LEN_REG));

		u32		result( mAudioCode->AddBuffer( g_pu8RamBase + address, length ) );

		use(result);
	}
	else
	{
		mAudioCode->StopAudio();
	}
}

//*****************************************************************************
//
//*****************************************************************************
u32		CAudioPluginPsp::ReadLength()
{
	return 0;
}

//*****************************************************************************
//
//*****************************************************************************
void	CAudioPluginPsp::Update( bool wait )
{
	// if(Wait) WaitMessage();
}

//*****************************************************************************
//
//*****************************************************************************
// Move me?
// We need to figure out this to get async working again..
//

EProcessResult	CAudioPluginPsp::ProcessAList()
{
	// Deprecated ProcessAList, is done directly in the RSP plugin now
	// Remove me
	//

	/*Memory_SP_SetRegisterBits(SP_STATUS_REG, SP_STATUS_HALT);

	EProcessResult	result( PR_NOT_STARTED );

	switch( gAudioPluginEnabled )
	{
		case APM_DISABLED:
			result = PR_COMPLETED;
			break;
		
		case APM_ENABLED_ASYNC:
			{
				SHLEStartJob	job;
				gJobManager.AddJob( &job, sizeof( job ) );
			}
			result = PR_STARTED;
			break;
		case APM_ENABLED_SYNC:
			HLEStart();
			result = PR_COMPLETED;
			break;
	}

	return result;*/
}

//*****************************************************************************
//
//*****************************************************************************
void	CAudioPluginPsp::RomClosed()
{
	mAudioCode->StopAudio();
}

//*****************************************************************************
//
//*****************************************************************************
CAudioPlugin *		CreateAudioPlugin()
{
	return CAudioPluginPsp::Create();
}
