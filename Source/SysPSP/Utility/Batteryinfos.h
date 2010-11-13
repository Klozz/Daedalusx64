/*
Copyright (C) 2010 Salvy6735 / psppwner300

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

//#include <time.h>
//#include <psputils.h>
//#include <psputility_sysparam.h>
#include <pspdebug.h>
#include <psppower.h>
#include <psprtc.h>

#ifndef BATTERYINFOS_H_
#define BATTERYINFOS_H_

///////////////////////////////////////////////////////////////////////////////
//Battery info psppwner300 / Corn											 //
///////////////////////////////////////////////////////////////////////////////
void battery_infos()
{	
	const u32 black	=	0x00000000;	//  Black..
	const u32 white =	0xffffffff;	//  White..
	
    pspTime time;
    sceRtcGetCurrentClockLocalTime(&time);

	pspDebugScreenSetXY(0, 0);		// Allign to the left, becareful not touch the edges
	pspDebugScreenSetBackColor( black );
	pspDebugScreenSetTextColor( white );

	if (scePowerIsBatteryExist())
	{		
		s32 bat = scePowerGetBatteryLifePercent();
		s32 batteryLifeTime = scePowerGetBatteryLifeTime();

		pspDebugScreenPrintf("Batt:%d%% %0.2fV %dC", bat, (float) scePowerGetBatteryVolt() / 1000.0f, scePowerGetBatteryTemp());
		pspDebugScreenPrintf(" %2dh%2dm", batteryLifeTime / 60, batteryLifeTime - 60 * (batteryLifeTime / 60));
		pspDebugScreenPrintf(" | %d:%02d%c%02d ", time.hour, time.minutes, (time.seconds&1?':':' '), time.seconds);
	}
	else
	{		
		pspDebugScreenPrintf("Batt:--%% -.--V --C");
		pspDebugScreenPrintf(" --h--m");
		pspDebugScreenPrintf(" | %d:%02d%c%02d ", time.hour, time.minutes, (time.seconds&1?':':' '), time.seconds);
	}
}
///////////////////////////////////////////////////////////////////////////////
//																			 //
///////////////////////////////////////////////////////////////////////////////

#endif // BATTERYINFOS_H_

