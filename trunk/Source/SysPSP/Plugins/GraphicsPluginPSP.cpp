/*
Copyright (C) 2007 StrmnNrmn

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
#include "GraphicsPluginPSP.h"

#include "Debug/DBGConsole.h"

#include "HLEGraphics/PSPRenderer.h"
#include "HLEGraphics/TextureCache.h"
#include "HLEGraphics/DLParser.h"

#include "Graphics/GraphicsContext.h"
#include "SysPSP/Graphics/VideoMemoryManager.h"
#include "Graphics/ColourValue.h"

#include "Utility/Profiler.h"
#include "Utility/FramerateLimiter.h"
#include "Utility/Preferences.h"
#include "Utility/Timing.h"

#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspgu.h>
#include <psppower.h>



#ifdef DAEDALUS_DEBUG_DISPLAYLIST
#include "HLEGraphics/DisplayListDebugger.h"
#endif

#include "Core/Memory.h"
#include "OSHLE/ultra_rcp.h"

//#define DAEDALUS_FRAMERATE_ANALYSIS

extern void HandleEndOfFrame();

bool	gFrameskipActive = false;
bool	gTakeScreenshot = false;
bool	gDebugDisplayList = false;

EFrameskipValue		gFrameskipValue = FV_DISABLED;

namespace
{
	u32					gTotalFrames = 0;
	u32					gVblCount = 0;
	u32					gFlipCount = 0;
	float				gCurrentVblrate = 0.0f;
	float				gCurrentFramerate = 0.0f;
	u64					gLastFramerateCalcTime = 0;
	u64					gTicksPerSecond = 0;

#ifdef DAEDALUS_FRAMERATE_ANALYSIS
	u64					gFirstFrameTime = 0;
	FILE *				gFramerateFile = NULL;
#endif

void	UpdateFramerate()
{
	gTotalFrames++;
	gFlipCount++;

	u64			now;
	NTiming::GetPreciseTime( &now );

	if(gLastFramerateCalcTime == 0)
	{
		u64		freq;
		gLastFramerateCalcTime = now;

		NTiming::GetPreciseFrequency( &freq );
		gTicksPerSecond = freq;
	}

#ifdef DAEDALUS_FRAMERATE_ANALYSIS
	if( gFramerateFile == NULL )
	{
		gFirstFrameTime = now;
		gFramerateFile = fopen( "framerate.csv", "w" );
	}
	fprintf( gFramerateFile, "%d,%f\n", gTotalFrames, f32(now - gFirstFrameTime) / f32(gTicksPerSecond) );
#endif

	// If 1 second has elapsed since last recalculation, do it now
	u64		ticks_since_recalc( now - gLastFramerateCalcTime );
	if(ticks_since_recalc > gTicksPerSecond)
	{
		gCurrentVblrate = float( gVblCount * gTicksPerSecond ) / float( ticks_since_recalc );
		gCurrentFramerate = float( gFlipCount * gTicksPerSecond ) / float( ticks_since_recalc );

		gVblCount = 0;
		gFlipCount = 0;
		gLastFramerateCalcTime = now;

	#ifdef DAEDALUS_FRAMERATE_ANALYSIS
		if( gFramerateFile != NULL )
		{
			fflush( gFramerateFile );
		}
	#endif
	}

}
}


//*****************************************************************************
//
//*****************************************************************************
CGraphicsPluginPsp::CGraphicsPluginPsp()
{

}

//*****************************************************************************
//
//*****************************************************************************
CGraphicsPluginPsp::~CGraphicsPluginPsp()
{

}

//*****************************************************************************
//
//*****************************************************************************
bool CGraphicsPluginPsp::Initialise()
{
	if(!PSPRenderer::Create())
	{
		return false;
	}

	if(!CTextureCache::Create())
	{
		return false;
	}

	if (!DLParser_Initialise()) 
	{
		return false;
	}

	return true;
}

//*****************************************************************************
//
//*****************************************************************************
bool CGraphicsPluginPsp::StartEmulation()
{
	return true;
}

//*****************************************************************************
//
//*****************************************************************************
void CGraphicsPluginPsp::ViStatusChanged()
{
}

//*****************************************************************************
//
//*****************************************************************************
void CGraphicsPluginPsp::ViWidthChanged()
{
}

//*****************************************************************************
//
//*****************************************************************************
void CGraphicsPluginPsp::ProcessDList()
{
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	if(!gDebugDisplayList)
#endif
	{
		DLParser_Process();
	}

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	// DLParser_Process may set this flag, so check again after execution
	if(gDebugDisplayList)
	{
		CDisplayListDebugger *	p_debugger( CDisplayListDebugger::Create() );
		p_debugger->Run();
		delete p_debugger;
		gDebugDisplayList = false;
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
//  TODO: Need to move this function out of our graphics plugin...			 //
///////////////////////////////////////////////////////////////////////////////
void low_battery()
{
	//if( !gGlobalPreferences.BatteryWarning || scePowerIsBatteryCharging() ) return;	// Need to double check...
	if( scePowerIsBatteryCharging() ) return;
	int bat = scePowerGetBatteryLifePercent();
	if (bat > 9) return;

	u32 counter = 0;

	counter++;
	if ((counter % 120) < 80)
	{
		const u32 red =  0x000000ff;	//  Red..
		const u32 white = 0xffffffff;	//  White..

		pspDebugScreenSetXY(51, 0);		// Allign to the left, becareful not touch the edges
		pspDebugScreenSetBackColor( red );
		pspDebugScreenSetTextColor( white );
		pspDebugScreenPrintf( " Battery Low: %d ", bat);	// it's defined as a single character...
	}
}
//*****************************************************************************
//
//*****************************************************************************
void CGraphicsPluginPsp::UpdateScreen()
{
	gVblCount++;

	static u32		last_origin = 0;
	u32 current_origin = Memory_VI_GetRegister(VI_ORIGIN_REG);
	if( current_origin != last_origin )
	{
		//printf( "Flip (%08x, %08x)\n", current_origin, last_origin );
		UpdateFramerate();

		if(!gFrameskipActive)
		{		
			if( gGlobalPreferences.DisplayFramerate )
			{
				pspDebugScreenSetTextColor( 0xffffffff );
				pspDebugScreenSetBackColor(0);
				pspDebugScreenSetXY(0, 0);
				pspDebugScreenPrintf( "Frames/sec: %#.1f | Vbls/sec: %d/%d | Sync: %#.2f%%   ", gCurrentFramerate, u32( gCurrentVblrate ), FramerateLimiter_GetTvFrequencyHz(), FramerateLimiter_GetSync() * 100.0f );
			}
			if( gGlobalPreferences.BatteryWarning )
			{
				low_battery();
			}
			if(gTakeScreenshot)
			{
				CGraphicsContext::Get()->DumpNextScreen();
				gTakeScreenshot = false;
			}
			CGraphicsContext::Get()->UpdateFrame( false );
		}

		static u32 current_frame = 0;
		current_frame++;
		if( gFrameskipValue == FV_DISABLED )
		{
			gFrameskipActive = false;
		}
		else
		{
			gFrameskipActive = (current_frame % gFrameskipValue) != 0;
		}

		last_origin = current_origin;
	}

	HandleEndOfFrame();
}

//*****************************************************************************
//
//*****************************************************************************
void CGraphicsPluginPsp::RomClosed()
{
	//
	//	Clean up resources used by the PSP build
	//
	DBGConsole_Msg(0, "Finalising PSPGraphics");
	DLParser_Finalise();
	CTextureCache::Destroy();
	PSPRenderer::Destroy();
}

//*****************************************************************************
//
//*****************************************************************************
CGraphicsPlugin *		CreateGraphicsPlugin()
{
	DBGConsole_Msg( 0, "Initialising Graphics Plugin [CPSP]" );

	CGraphicsPluginPsp *	plugin( new CGraphicsPluginPsp );
	if( !plugin->Initialise() )
	{
		delete plugin;
		plugin = NULL;
	}

	return plugin;
}
