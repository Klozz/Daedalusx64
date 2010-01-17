/*
Copyright (C) 2006,2007 StrmnNrmn

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

#include <pspkernel.h>
#include <pspdebug.h>
#include <stdlib.h>
#include <stdio.h>

#include <pspctrl.h>
#include <psprtc.h>
#include <psppower.h>
#include <pspsdk.h>
#include <pspdisplay.h>
#include <kubridge.h>

#include "System.h"
#include "Debug/DBGConsole.h"
#include "Debug/DebugLog.h"
#include "Interface/RomDB.h"
#include "Core/RomSettings.h"
#include "Core/Memory.h"
#include "Core/CPU.h"
#include "Core/PIF.h"
#include "Core/CPU.h"
#include "Core/Save.h"
#include "Input/InputManager.h"
#include "Utility/Profiler.h"
#include "Utility/IO.h"
#include "Utility/Preferences.h"
#include "Utility/Timer.h"

#include "Graphics/VideoMemoryManager.h"
#include "Graphics/GraphicsContext.h"

#include "SysPSP/UI/UIContext.h"
#include "SysPSP/UI/MainMenuScreen.h"
#include "SysPSP/UI/PauseScreen.h"
#include "SysPSP/UI/SplashScreen.h"
#include "SysPSP/Graphics/DrawText.h"
#include "SysPSP/Utility/PathsPSP.h"

#include "HLEGraphics/TextureCache.h"

#include "Test/BatchTest.h"

#include "ConfigOptions.h"

//#define DAEDALUS_KERNEL_MODE

char						gDaedalusExePath[MAX_PATH+1] = DAEDALUS_PSP_PATH( "" );

extern "C" { void _DisableFPUExceptions(); }
extern void initExceptionHandler();

/* Video Manager functions */
extern "C" {
int pspDveMgrCheckVideoOut();
int pspDveMgrSetVideoOut(int, int, int, int, int, int, int);
}

extern int HAVE_DVE;
extern int PSP_TV_CABLE;
extern int PSP_TV_LACED;

bool PSP_IS_SLIM = false;

//*************************************************************************************
//Set up our initial eviroment settings for the PSP
//*************************************************************************************
#ifdef DAEDALUS_KERNEL_MODE

PSP_MODULE_INFO( DaedalusX64 Alpha, 0x1000, 1, 1 );
PSP_MAIN_THREAD_ATTR( 0 | PSP_THREAD_ATTR_VFPU );

#else

PSP_MODULE_INFO( DaedalusX64 Alpha, 0, 1, 1 );
PSP_MAIN_THREAD_ATTR( PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU );
//PSP_HEAP_SIZE_KB(20000);// Set Heapsize to 18.5mb
PSP_HEAP_SIZE_KB(-256);
#endif

#ifdef DAEDALUS_KERNEL_MODE
//*************************************************************************************
//Set up our exception handler for Kernel mode
//*************************************************************************************
static void DaedalusExceptionHandler( PspDebugRegBlock * regs )
{
	pspDebugScreenPrintf("\n\n");
	pspDebugDumpException(regs);
}
#endif

#ifdef DAEDALUS_KERNEL_MODE
//*************************************************************************************
// Function that is called from _init in kernelmode before the
// main thread is started in usermode.
//*************************************************************************************
__attribute__ ((constructor))
void loaderInit()
{
    pspKernelSetKernelPC();
	pspDebugScreenInit();
	pspDebugInstallKprintfHandler(NULL);
	pspDebugInstallErrorHandler(DaedalusExceptionHandler);
	pspSdkInstallNoDeviceCheckPatch();
	pspSdkInstallNoPlainModuleCheckPatch();
}
#endif

//*************************************************************************************
//Set up the Exit Callback (Used to allow the Home Button to work)
//*************************************************************************************
static int ExitCallback( int arg1, int arg2, void * common )
{
	sceKernelExitGame();
	return 0;
}

//*************************************************************************************
//Initialise the Exit Callback (Also used to allow the Home Button to work)
//*************************************************************************************
static int CallbackThread( SceSize args, void * argp )
{
	int cbid;

	cbid = sceKernelCreateCallback( "Exit Callback", ExitCallback, NULL );
	sceKernelRegisterExitCallback( cbid );

	sceKernelSleepThreadCB();

	return 0;
}

//*************************************************************************************
// Sets up the callback thread and returns its thread id
//*************************************************************************************
static int SetupCallbacks()
{
	int thid = 0;

	thid = sceKernelCreateThread( "CallbackThread", CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0 );
	if(thid >= 0)
	{
		sceKernelStartThread(thid, 0, 0);
	}
	return thid;
}

extern void InitialiseJobManager();

//*************************************************************************************
//
//*************************************************************************************
static bool	Initialize()
{
	printf( "Cpu was: %dMHz, Bus: %dMHz\n", scePowerGetCpuClockFrequency(), scePowerGetBusClockFrequency() );
	if (scePowerSetClockFrequency(333, 333, 166) != 0)
	{
		printf( "Could not set CPU to 333MHz\n" );
	}
	printf( "Cpu now: %dMHz, Bus: %dMHz\n", scePowerGetCpuClockFrequency(), scePowerGetBusClockFrequency() );

	InitialiseJobManager();

// Disable for profiling
//	srand(time(0));

	//Set the debug output to default
	pspDebugScreenInit();

	initExceptionHandler();

	_DisableFPUExceptions();

	//Set up callback for our thread
	SetupCallbacks();

	//Set up the DveMgr (TV Display) and Detect PSP Slim
	if ( kuKernelGetModel() == PSP_MODEL_SLIM_AND_LITE )
	{
		PSP_IS_SLIM = true;
		HAVE_DVE = pspSdkLoadStartModule("dvemgr.prx", PSP_MEMORY_PARTITION_KERNEL);
		if (HAVE_DVE >= 0)
			PSP_TV_CABLE = pspDveMgrCheckVideoOut();
		if (PSP_TV_CABLE == 1)
			PSP_TV_LACED = 1; // composite cable => interlaced
	}
	HAVE_DVE = (HAVE_DVE < 0) ? 0 : 1; // 0 == no dvemgr, 1 == dvemgr

    //setup Pad
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

	// Init the savegame directory
	strcpy( g_DaedalusConfig.szSaveDir, DAEDALUS_PSP_PATH( "SaveGames/" ) );

	if (!System_Init())
		return false;

	if(!CVideoMemoryManager::Create())
		return false;

	if(!CGraphicsContext::Create())
		return false;

	return true;
}

//*************************************************************************************
//
//*************************************************************************************
static void Finalise()
{
	System_Finalize();
}

//*************************************************************************************
//
//*************************************************************************************
#ifdef DAEDALUS_KERNEL_MODE
bool	gProfileNextFrame = false;
bool	gWasProfilingFrame = false;
bool	gShowProfilerResults = false;
#endif

#ifdef DAEDALUS_KERNEL_MODE
//*************************************************************************************
//
//*************************************************************************************
static void	DisplayProfilerResults()
{
    SceCtrlData pad;

	static u32 oldButtons = 0;

	if(gShowProfilerResults)
	{
		bool menu_button_pressed( false );

		pspDebugScreenSetTextColor( 0xffffffff );

		CGraphicsContext::Get()->SetDebugScreenTarget( CGraphicsContext::TS_FRONTBUFFER );

		// Remain paused until the Select button is pressed again
		while(!menu_button_pressed)
		{
			sceDisplayWaitVblankStart();
			pspDebugScreenSetXY(0, 0);
			pspDebugScreenPrintf( "                                 Paused\n" );
			pspDebugScreenPrintf( "                                 ------\n" );
			pspDebugScreenPrintf( "                   Press Select to resume emulation\n" );
			pspDebugScreenPrintf( "\n\n" );

			pspDebugProfilerPrint();

			pspDebugScreenPrintf( "\n" );

			sceCtrlPeekBufferPositive(&pad, 1);
			if(oldButtons != pad.Buttons)
			{
				if(pad.Buttons & PSP_CTRL_SELECT)
				{
					menu_button_pressed = true;
				}
			}

			oldButtons = pad.Buttons;
		}
	}

	CGraphicsContext::Get()->SetDebugScreenTarget( CGraphicsContext::TS_BACKBUFFER );

}
#endif


#ifdef DAEDALUS_PROFILE_EXECUTION
//*************************************************************************************
//
//*************************************************************************************
static void	DumpDynarecStats( float elapsed_time )
{
	// Temp dynarec stats
	extern u64 gTotalInstructionsEmulated;
	extern u64 gTotalInstructionsExecuted;
	extern u32 gTotalRegistersCached;
	extern u32 gTotalRegistersUncached;
	extern u32 gFragmentLookupSuccess;
	extern u32 gFragmentLookupFailure;

	u32		dynarec_ratio( 0 );

	if(gTotalInstructionsExecuted + gTotalInstructionsEmulated > 0)
	{
		float fRatio = float(gTotalInstructionsExecuted * 100.0f / float(gTotalInstructionsEmulated+gTotalInstructionsExecuted));

		dynarec_ratio = u32( fRatio );

		//gTotalInstructionsExecuted = 0;
		//gTotalInstructionsEmulated = 0;
	}

	u32		cached_regs_ratio( 0 );
	if(gTotalRegistersCached + gTotalRegistersUncached > 0)
	{
		float fRatio = float(gTotalRegistersCached * 100.0f / float(gTotalRegistersCached+gTotalRegistersUncached));

		cached_regs_ratio = u32( fRatio );
	}

	const char * const TERMINAL_SAVE_CURSOR			= "\033[s";
	const char * const TERMINAL_RESTORE_CURSOR		= "\033[u";
//	const char * const TERMINAL_TOP_LEFT			= "\033[2A\033[2K";
	const char * const TERMINAL_TOP_LEFT			= "\033[H\033[2K";

	printf( TERMINAL_SAVE_CURSOR );
	printf( TERMINAL_TOP_LEFT );

	printf( "Frame: %dms, DynaRec %d%%, Regs cached %d%%, Lookup success %d/%d", u32(elapsed_time * 1000.0f), dynarec_ratio, cached_regs_ratio, gFragmentLookupSuccess, gFragmentLookupFailure );

	printf( TERMINAL_RESTORE_CURSOR );
	fflush( stdout );

	gFragmentLookupSuccess = 0;
	gFragmentLookupFailure = 0;
}
#endif

#ifdef DAEDALUS_DEBUG_DISPLAYLIST
extern bool gDebugDisplayList;
#include "HLEGraphics/DLParser.h"
#endif

//*************************************************************************************
//
//*************************************************************************************
static CTimer		gTimer;

void HandleEndOfFrame()
{
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	if(gDebugDisplayList)
		return;
#endif

	DPF( DEBUG_FRAME, "********************************************" );

	bool		activate_pause_menu( false );

#ifdef DAEDALUS_KERNEL_MODE
	//
	//	Update the profiling stats
	//
	if(gWasProfilingFrame)
	{
		// Show results
		pspDebugProfilerDisable();
		gShowProfilerResults = true;
		gWasProfilingFrame = false;

		// Drop back into the menu
		activate_pause_menu = true;
	}
#endif

	//
	//	Figure out how long the last frame took
	//
	float		elapsed_time( gTimer.GetElapsedSeconds() );
	float		framerate( 0.0f );
	if(elapsed_time > 0)
	{
		framerate = 1.0f / elapsed_time;
	}

#ifdef DAEDALUS_PROFILE_EXECUTION
	DumpDynarecStats( elapsed_time );
#endif
	//
	//	Enter the debug menu as soon as select is newly pressed
	//
    SceCtrlData pad;

	static u32 oldButtons = 0;

	sceCtrlPeekBufferPositive(&pad, 1);
	if(oldButtons != pad.Buttons)
	{
		if(pad.Buttons & PSP_CTRL_SELECT)
		{
			activate_pause_menu = true;
		}
	}
	oldButtons = pad.Buttons;

	if(activate_pause_menu)
	{
		// See how much texture memory we're using
		//CTextureCache::Get()->DropTextures();
		//CVideoMemoryManager::Get()->DisplayDebugInfo();

		Save::Flush(true);
		// switch back to the LCD display
		CGraphicsContext::Get()->SwitchToLcdDisplay();

		// Call this initially, to tidy up any state set by the emulator
		CGraphicsContext::Get()->ClearAllSurfaces();

		CDrawText::Initialise();

		CUIContext *	p_context( CUIContext::Create() );

		if(p_context != NULL)
		{
			p_context->SetBackgroundColour( c32( 94, 188, 94 ) );		// Nice green :)

			CPauseScreen *	pause( CPauseScreen::Create( p_context ) );
			pause->Run();
			delete pause;
			delete p_context;
		}

		CDrawText::Destroy();

		//
		// Commit the preferences database before starting to run
		//
		CPreferences::Get()->Commit();
	}

	//
	//	Reset the elapsed time to avoid glitches when we restart
	//
	gTimer.Reset();

	//
	//	Check whether to profile the next frame
	//
#ifdef DAEDALUS_KERNEL_MODE
	gShowProfilerResults = false;
	if(gProfileNextFrame)
	{
		gProfileNextFrame = false;
		gWasProfilingFrame = true;

		pspDebugProfilerClear();
		pspDebugProfilerEnable();
	}
#endif
}

//*************************************************************************************
// Here's where we load up the GUI
//*************************************************************************************
static void DisplayRomsAndChoose()
{
	// switch back to the LCD display
	CGraphicsContext::Get()->SwitchToLcdDisplay();

	CDrawText::Initialise();

	CUIContext *	p_context( CUIContext::Create() );

	if(p_context != NULL)
	{
		//const c32		BACKGROUND_COLOUR = c32( 107, 188, 255 );		// blue
		//const c32		BACKGROUND_COLOUR = c32( 92, 162, 219 );		// blue
		const c32		BACKGROUND_COLOUR = c32( 1, 1, 127 );			// dark blue
		//const c32		BACKGROUND_COLOUR = c32( 1, 127, 1 );			// dark green

		p_context->SetBackgroundColour( BACKGROUND_COLOUR );

		CSplashScreen *		p_splash( CSplashScreen::Create( p_context ) );
		p_splash->Run();
		delete p_splash;

		CMainMenuScreen *	p_main_menu( CMainMenuScreen::Create( p_context ) );
		p_main_menu->Run();
		delete p_main_menu;
	}

	delete p_context;

	CDrawText::Destroy();
}

//*************************************************************************************
// This is our main loop
//*************************************************************************************
extern "C"
{
int main(int argc, char* argv[])
{
	if( Initialize() )
	{
#ifdef DAEDALUS_BATCH_TEST_ENABLED
		if( argc > 1 )
		{
			BatchTestMain( argc, argv );
		}
#endif

		for(;;)
		{
			DisplayRomsAndChoose();

			//
			// Commit the preferences and roms databases before starting to run
			//
			CRomDB::Get()->Commit();
			CPreferences::Get()->Commit();

			CPU_Run();
			System_Close();
		}

		Finalise();
	}

	sceKernelExitGame();
	return 0;
}
}
