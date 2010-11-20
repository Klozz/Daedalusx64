/*
Copyright (C) 2010 Salvy6735 / psppwner300 / Corn

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

#include <psppower.h>
#include <psprtc.h>

#ifndef BATTERYINFOS_H_
#define BATTERYINFOS_H_

//*****************************************************************************
//
//*****************************************************************************
void battery_infos()
{	
    pspTime time;
    sceRtcGetCurrentClockLocalTime(&time);
	s32 bat = scePowerGetBatteryLifePercent();

	CDrawText::IntrPrintf( 22, 43, 0.9f, DrawTextUtilities::TextWhite,"Time:  %d:%02d%c%02d", time.hour, time.minutes, (time.seconds&1?':':' '), time.seconds );

	if(!scePowerIsBatteryCharging())
	{
		s32 batteryLifeTime = scePowerGetBatteryLifeTime();

		CDrawText::IntrPrintf( 140, 43, 0.9f, DrawTextUtilities::TextWhite,"Battery:  %d%% | %0.2fV | %dC", bat, (f32) scePowerGetBatteryVolt() / 1000.0f, scePowerGetBatteryTemp());
		CDrawText::IntrPrintf( 332, 43, 0.9f, DrawTextUtilities::TextWhite,"Remaining: %2dh %2dm", batteryLifeTime / 60, batteryLifeTime - 60 * (batteryLifeTime / 60));
	}
	else
	{
		CDrawText::IntrPrintf( 22, 43, 0.9f, DrawTextUtilities::TextWhite,"Battery:  %d%% | %0.2fV | %dC", bat, (f32) scePowerGetBatteryVolt() / 1000.0f, scePowerGetBatteryTemp());
		CDrawText::IntrPrintf( 140, 43, 0.9f, DrawTextUtilities::TextWhite,"Charging...");
		CDrawText::IntrPrintf( 332, 43, 0.9f, DrawTextUtilities::TextWhite,"Remaining: --h--m");
	}
}
//*****************************************************************************
//
//*****************************************************************************

#endif // BATTERYINFOS_H_

