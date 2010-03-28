/*
Copyright (C) 2010 Salvy6735

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

#include <pspdebug.h>
#include <psppower.h>

#ifndef LOWBATTERY_H_
#define LOWBATTERY_H_

// ToDO : Improve futher this
//
///////////////////////////////////////////////////////////////////////////////
//																			 //
///////////////////////////////////////////////////////////////////////////////
void low_battery_warning()
{
	//if( !gGlobalPreferences.BatteryWarning || scePowerIsBatteryCharging() ) return;
	if( scePowerIsBatteryCharging() ) return;
	int bat = scePowerGetBatteryLifePercent();
	if (bat > 9) return;

	u32 counter = 0;

	counter++;
	if ((counter % 120) < 80)
	{
		const u32 red	=	0x000000ff;	//  Red..
		const u32 white =	0xffffffff;	//  White..

		pspDebugScreenSetXY(51, 0);		// Allign to the left, becareful not touch the edges
		pspDebugScreenSetBackColor( red );
		pspDebugScreenSetTextColor( white );
		pspDebugScreenPrintf( " Battery Low: %d ", bat);	// it's defined as a single character...
	}
}
///////////////////////////////////////////////////////////////////////////////
//																			 //
///////////////////////////////////////////////////////////////////////////////

#endif // LOWBATTERY_H_

