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

#include "SysPSP/Utility/JobManager.h"

#include "Core/Interrupt.h"
#include "Core/Memory.h"
#include "Core/ROM.h"
#include "Core/CPU.h"
#include "Core/RSP_HLE.h"

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
,	mLastDACRate( 0 )
{
	mAudioCode->SetAdaptFrequency( gAdaptFrequency );
	//gAudioPluginEnabled = APM_ENABLED_SYNC; // for testing
}

//*****************************************************************************
//
//*****************************************************************************
CAudioPluginPsp::~CAudioPluginPsp()
{
	ChangeABI(0);
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
	//PluginInfo->MemoryBswaped = true;
	//PluginInfo->NormalMemory  = false;
	//strcpy (PluginInfo->Name, "Azimer's HLE Audio v");
	//strcat (PluginInfo->Name, PLUGIN_VERSION);
	//PluginInfo->Type = PLUGIN_TYPE_AUDIO;
	//PluginInfo->Version = 0x0101; // Set this to retain backwards compatibility

	ChangeABI(0);
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
	if( gAudioPluginEnabled == APM_DISABLED ) return;

#ifndef DAEDALUS_SILENT
	printf( "DacrateChanged( %d )\n", system_type );
#endif

	mLastDACRate = Memory_AI_GetRegister(AI_DACRATE_REG);

	u32	frequency = DEFAULT_FREQUENCY; // Is this correct? - Salvy
	switch (system_type)
	{
		case ST_NTSC: frequency = VI_NTSC_CLOCK / (mLastDACRate + 1); break;
		case ST_PAL:  frequency = VI_PAL_CLOCK  / (mLastDACRate + 1); break;
		case ST_MPAL: frequency = VI_MPAL_CLOCK / (mLastDACRate + 1); break;
	}

	mAudioCode->SetFrequency( frequency );

}

//*****************************************************************************
//
//*****************************************************************************
void	CAudioPluginPsp::LenChanged()
{
	if( gAudioPluginEnabled > APM_DISABLED )
	{
		mAudioCode->SetAdaptFrequency( gAdaptFrequency );

		u32		address( Memory_AI_GetRegister(AI_DRAM_ADDR_REG) & 0x00FFFFF8 );
		u32		length( Memory_AI_GetRegister(AI_LEN_REG) & 0x3FFF8 );

		u32		result( mAudioCode->AddBuffer( g_pu8RamBase + address, length ) );
		//if (result & SND_IS_FULL)
		//	Memory_AI_GetRegister(AI_STATUS_REG) |= AI_STATUS_FIFO_FULL;
		//Memory_AI_GetRegister(AI_STATUS_REG) |= AI_STATUS_DMA_BUSY;
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
	u32		length( mAudioCode->GetReadStatus() );

	//Memory_AI_SetRegister( AI_LEN_REG, length );

	return length;
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
struct SHLEStartJob : public SJob
{
	SHLEStartJob()
	{
		InitJob = NULL;
		DoJob = &DoHLEStartStatic;
		FiniJob = &DoHLEFinishedStatic;
	}

	static int DoHLEStartStatic( SJob * arg )
	{
		SHLEStartJob *	job( static_cast< SHLEStartJob * >( arg ) );
		return job->DoHLEStart();
	}

	static int DoHLEFinishedStatic( SJob * arg )
	{
		SHLEStartJob *	job( static_cast< SHLEStartJob * >( arg ) );
		return job->DoHLEFinish();
	}

	int DoHLEStart()
	{
		HLEStart();
		return 0;
	}

	int DoHLEFinish()
	{
		CPU_AddEvent(RSP_AUDIO_INTR_CYCLES, CPU_EVENT_AUDIO);
		return 0;
	}
};

EProcessResult	CAudioPluginPsp::ProcessAList()
{
	Memory_SP_SetRegisterBits(SP_STATUS_REG, SP_STATUS_HALT);

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

	return result;
}

//*****************************************************************************
//
//*****************************************************************************
void	CAudioPluginPsp::RomClosed()
{
	ChangeABI(0);
	mAudioCode->StopAudio();
	mLastDACRate = 0;
}

//*****************************************************************************
//
//*****************************************************************************
CAudioPlugin *		CreateAudioPlugin()
{
	return CAudioPluginPsp::Create();
}
