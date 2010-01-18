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

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef PREFERENCES_H_
#define PREFERENCES_H_

#include "ConfigOptions.h"

enum ETextureHashFrequency
{
	THF_DISABLED = 0,
	THF_EVERY_FRAME,
	THF_EVERY_2,
	THF_EVERY_3,
	THF_EVERY_4,
	THF_EVERY_5,
	THF_EVERY_10,
	THF_EVERY_15,
	THF_EVERY_20,
	THF_EVERY_30,

	NUM_THF,
};

enum EFrameskipValue
{
	FV_DISABLED = 0,
	FV_1,
	FV_2,
	FV_3,
	FV_4,
	FV_5,
	FV_6,
	FV_7,
	FV_8,
	FV_9,
	FV_10,
#ifndef DAEDALUS_PUBLIC_RELEASE
	FV_99,
#endif
	NUM_FRAMESKIP_VALUES,
};

enum EForceTextureFilter
{
	FORCE_DEFAULT_FILTER = 0,
	FORCE_POINT_FILTER,
	FORCE_LINEAR_FILTER,
};
const u32 NUM_FILTER_TYPES = FORCE_LINEAR_FILTER+1;

enum EViewportType
{
	VT_UNSCALED_4_3 = 0,
	VT_SCALED_4_3,
	VT_FULLSCREEN,
};
const u32 NUM_VIEWPORT_TYPES = VT_FULLSCREEN+1;

enum ETVType
{
	TT_4_3 = 0,
	TT_WIDESCREEN,
};

struct SGlobalPreferences
{
	bool						DisplayFramerate;
	bool						SoftwareClipping;
	bool						HighlightInexactBlendModes;
	bool						CustomBlendModes;
	bool						BatteryWarning;
	bool						LogMicrocodes;
	bool						LargeROMBuffer;
	EForceTextureFilter			ForceTextureFilter;

	float						StickMinDeadzone;
	float						StickMaxDeadzone;

	EViewportType				ViewportType;

	bool						TVEnable;
	bool						TVLaced;
	ETVType						TVType;

	SGlobalPreferences();

	void		Apply() const;
};

extern SGlobalPreferences	gGlobalPreferences;


struct SRomPreferences
{
	bool						PatchesEnabled;
	bool						SpeedSyncEnabled;
	bool						DynarecEnabled;				// Requires DynarceSupported in RomSettings
	bool						DynarecStackOptimisation;
	bool						DynarecLoopOptimisation;
	bool						TLBHackEnabled;
	bool						DoubleDisplayEnabled;
	bool						SimulateDoubleDisabled;
	bool						ViewPortHackEnabled;
	bool						FlatShadeDisabled;
	bool						CleanSceneEnabled;
	bool						CullingDisabled;
	bool						ForceDepthBuffer;
	bool						FlushTrisHack;

	ETextureHashFrequency		CheckTextureHashFrequency;
	EFrameskipValue				Frameskip;
	EAudioPluginMode			AudioEnabled;
//	bool						AudioAdaptFrequency;
	u32							ControllerIndex;

	SRomPreferences();

	void		Reset();
	void		Apply() const;
};

#include "Utility/Singleton.h"

//*****************************************************************************
//
//*****************************************************************************
class	RomID;

//*****************************************************************************
//
//*****************************************************************************
class CPreferences : public CSingleton< CPreferences >
{
	public:
		virtual					~CPreferences() {}

		virtual bool			OpenPreferencesFile( const char * filename ) = 0;
		virtual void			Commit() = 0;

		virtual bool			GetRomPreferences( const RomID & id, SRomPreferences * preferences ) const = 0;
		virtual void			SetRomPreferences( const RomID & id, const SRomPreferences & preferences ) = 0;
};

u32						ROM_GetTexureHashFrequencyAsFrames( ETextureHashFrequency thf );
ETextureHashFrequency	ROM_GetTextureHashFrequencyFromFrames( u32 frames );
const char *			ROM_GetTextureHashFrequencyDescription( ETextureHashFrequency thf );

u32						ROM_GetFrameskipValueAsInt( EFrameskipValue value );
EFrameskipValue			ROM_GetFrameskipValueFromInt( u32 value );
const char *			ROM_GetFrameskipDescription( EFrameskipValue value );

#endif // PREFERENCES_H_
