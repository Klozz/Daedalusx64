/*
Copyright (C) 2006 StrmnNrmn

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
#include "GlobalSettingsComponent.h"

#include "UIContext.h"
#include "UIScreen.h"
#include "UISetting.h"
#include "AdjustDeadzoneScreen.h"
#include "SysPSP/Graphics/DrawText.h"
#include "Graphics/ColourValue.h"

#include "SysPSP/Utility/Buttons.h"
#include "SysPSP/Utility/PathsPSP.h"
#include "Utility/Thread.h"
#include "Utility/FramerateLimiter.h"
#include "Utility/Preferences.h"
#include "Utility/IO.h"

#include "Input/InputManager.h"
#include "EasyMsg/easymessage.h"

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspgu.h>

extern u32 HAVE_DVE;
extern u32 PSP_TV_CABLE;
extern bool PSP_IS_SLIM;


namespace
{
	const u32		TEXT_AREA_TOP = 15+16+16;
	const u32		TEXT_AREA_LEFT = 40;
	const u32		TEXT_AREA_RIGHT = 440;

	const s32		DESCRIPTION_AREA_TOP = 0;		// We render text aligned from the bottom, so this is largely irrelevant
	const s32		DESCRIPTION_AREA_BOTTOM = 272-10;
	const s32		DESCRIPTION_AREA_LEFT = 16;
	const s32		DESCRIPTION_AREA_RIGHT = 480-16;

	class CViewPortSetting : public CUISetting
	{
	public:
		CViewPortSetting( const char * name, const char * description )
			:	CUISetting( name, description )
		{
		}

		virtual	void	OnNext()				{ gGlobalPreferences.ViewportType = EViewportType( (gGlobalPreferences.ViewportType+1) % NUM_VIEWPORT_TYPES ); }
		virtual	void	OnPrevious()			{ gGlobalPreferences.ViewportType = EViewportType( (gGlobalPreferences.ViewportType + NUM_VIEWPORT_TYPES - 1) % NUM_VIEWPORT_TYPES ); }

		virtual const char *	GetSettingName() const
		{
			if ( gGlobalPreferences.TVEnable )
			{
				if ( gGlobalPreferences.TVType == TT_4_3 )
				{
					switch( gGlobalPreferences.ViewportType )
					{
					case VT_UNSCALED_4_3:	return "4:3 no overscan (640x448)";
					case VT_SCALED_4_3:		return "4:3 overscan (658x460)";
					case VT_FULLSCREEN:		return "Fullscreen (720x460)";
					}
					DAEDALUS_ERROR( "Unhandled viewport type" );
					return "?";
				}
				else
				{
					switch( gGlobalPreferences.ViewportType )
					{
					case VT_UNSCALED_4_3:	return "4:3 no overscan (528x448)";
					case VT_SCALED_4_3:		return "4:3 overscan (542x460)";
					case VT_FULLSCREEN:		return "Fullscreen (720x460)";
					}
					DAEDALUS_ERROR( "Unhandled viewport type" );
					return "?";
				}
			}
			else
			{
				switch( gGlobalPreferences.ViewportType )
				{
				case VT_UNSCALED_4_3:	return "4:3 unscaled (320x240)";
				case VT_SCALED_4_3:		return "4:3 scaled (362x272)";
				case VT_FULLSCREEN:		return "Fullscreen (480x272)";
				}
				DAEDALUS_ERROR( "Unhandled viewport type" );
				return "?";
			}
		}
	};

	class CTVTypeSetting : public CUISetting
	{
	public:
		CTVTypeSetting( const char * name, const char * description )
			:	CUISetting( name, description )
		{
		}

		virtual	void	OnNext()				{ gGlobalPreferences.TVType = ETVType( gGlobalPreferences.TVType ^ 1 ); }
		virtual	void	OnPrevious()			{ gGlobalPreferences.TVType = ETVType( gGlobalPreferences.TVType ^ 1 ); }

		virtual const char *	GetSettingName() const
		{
			switch( gGlobalPreferences.TVType )
			{
			case TT_4_3:			return "4:3 SDTV";
			case TT_WIDESCREEN:		return "16:9 HDTV";
			}
			DAEDALUS_ERROR( "Unhandled TV type" );
			return "?";
		}
	};

	class CAdjustDeadzoneSetting : public CUISetting
	{
	public:
		CAdjustDeadzoneSetting( CUIContext * p_context, const char * name, const char * description )
			:	CUISetting( name, description )
			,	mpContext( p_context )
		{
		}

		virtual	void			OnSelected()
		{
			CAdjustDeadzoneScreen *	adjust_deadzone( CAdjustDeadzoneScreen::Create( mpContext ) );
			adjust_deadzone->Run();
			delete adjust_deadzone;
		}

		virtual const char *	GetSettingName() const
		{
			f32 min_deadzone( gGlobalPreferences.StickMinDeadzone );
			f32 max_deadzone( gGlobalPreferences.StickMaxDeadzone );

			static char buffer[ 10+10+1 ];
			sprintf( buffer, "%d/%d", s32( 100.0f * min_deadzone ), s32( 100.0f * max_deadzone ) );
			return buffer;
		}

	private:
		CUIContext *			mpContext;
	};

	class CResetSetting : public CUISetting
	{
	public:
		CResetSetting(  const char * name, const char * description )
			:	CUISetting( name, description )
		{
		}

		virtual	void			OnSelected()
		{

			if(ShowMessage("This will guide you to reset the settings in Daedalus\n \nDo you want to reset HLE cache?", 1))
			{
				IO::Path::DeleteRecursive("SaveGames",".hle");
				ThreadSleepMs(1000);	//safety wait for s

				if (ShowMessage("Do you want to reset settings to default?\n \nNote : This will exit the emulator", 1))
				{
					remove(DAEDALUS_PSP_PATH("preferences.ini"));
					remove(DAEDALUS_PSP_PATH("rom.db"));
					ThreadSleepMs(1000);	//safety wait for s

					ShowMessage("Daedalus will exit now",0);

					sceKernelExitGame();
				}
			}
			else if(ShowMessage("Do you want to reset settings to default?\n \nNote : This will exit the emulator", 1))
			{
				remove(DAEDALUS_PSP_PATH("preferences.ini"));
				remove(DAEDALUS_PSP_PATH("rom.db"));
				ThreadSleepMs(1000);	//safety wait for s

				ShowMessage("Daedalus will exit now",0);

				sceKernelExitGame();
			}
		}

		virtual const char *	GetSettingName() const
		{
			return "Press X";
		}
	};

	class CGuiType : public CUISetting
	{
	public:
		CGuiType(  const char * name, const char * description )
			:	CUISetting( name, description )
		{
		}

		virtual	void		OnNext()		{ gGlobalPreferences.GuiType = EGuiType( (gGlobalPreferences.GuiType+1) % NUM_GUI_TYPES ); }
		virtual	void		OnPrevious()	{ gGlobalPreferences.GuiType = EGuiType( (gGlobalPreferences.GuiType + NUM_GUI_TYPES - 1) % NUM_GUI_TYPES ); }

		virtual const char *	GetSettingName() const
		{
			switch ( gGlobalPreferences.GuiType )
			{
				case COVERFLOW:		return "Cover Flow";
				case CLASSIC:		return "Classic";
			}
			DAEDALUS_ERROR( "Unknown Style" );
			return "?";
		}
	};



	class CColorSetting : public CUISetting
	{
	public:
		CColorSetting(  const char * name, const char * description )
			:	CUISetting( name, description )
		{
		}

		virtual	void		OnNext()		{ gGlobalPreferences.GuiColor = EGuiColor( (gGlobalPreferences.GuiColor+1) % NUM_COLOR_TYPES ); }
		virtual	void		OnPrevious()	{ gGlobalPreferences.GuiColor = EGuiColor( (gGlobalPreferences.GuiColor + NUM_COLOR_TYPES - 1) % NUM_COLOR_TYPES ); }

		virtual const char *	GetSettingName() const
		{
			switch ( gGlobalPreferences.GuiColor )
			{
				case BLACK:		return "Black";
				case RED:		return "Red";
				case GREEN:		return "Green";
				case MAGENTA:	return "Magenta";
				case BLUE:		return "Blue";
				case TURQUOISE:	return "Turquoise";
				case ORANGE:	return "Orange";
				case PURPLE:	return "Purple";
				case GREY:		return "Grey";
			}
			DAEDALUS_ERROR( "Unknown Color" );
			return "?";
		}
	};
	
	class CInfoSetting : public CUISetting
	{
	public:
		CInfoSetting(  const char * name, const char * description )
			:	CUISetting( name, description )
		{
		}
 
		virtual	void		OnNext()		{ (gGlobalPreferences.DisplayFramerate >= 3) ? 0 : gGlobalPreferences.DisplayFramerate++; }
		virtual	void		OnPrevious()	{ (gGlobalPreferences.DisplayFramerate <= 0) ? 0 : gGlobalPreferences.DisplayFramerate--; }

		virtual const char *	GetSettingName() const
		{
			switch ( gGlobalPreferences.DisplayFramerate )
			{
				case 0:		return "None";
				case 1:		return "FPS";
				case 2:		return "FPS + VB + SYNC";
				case 3:		return "Render stats";
			}
			return "?";
		}
	};



}

//*************************************************************************************
//
//*************************************************************************************
class IGlobalSettingsComponent : public CGlobalSettingsComponent
{
	public:

		IGlobalSettingsComponent( CUIContext * p_context );
		~IGlobalSettingsComponent();

		// CUIComponent
		virtual void				Update( float elapsed_time, const v2 & stick, u32 old_buttons, u32 new_buttons );
		virtual void				Render();

	private:
		CUIElementBag				mElements;
};

//*************************************************************************************
//
//*************************************************************************************
CGlobalSettingsComponent::CGlobalSettingsComponent( CUIContext * p_context )
:	CUIComponent( p_context )
{
}

//*************************************************************************************
//
//*************************************************************************************
CGlobalSettingsComponent::~CGlobalSettingsComponent()
{
}

//*************************************************************************************
//
//*************************************************************************************
CGlobalSettingsComponent *	CGlobalSettingsComponent::Create( CUIContext * p_context )
{
	return new IGlobalSettingsComponent( p_context );
}

//*************************************************************************************
//
//*************************************************************************************
IGlobalSettingsComponent::IGlobalSettingsComponent( CUIContext * p_context )
:	CGlobalSettingsComponent( p_context )
{
	mElements.Add( new CInfoSetting( "Display Info", "Whether to show additional info while the rom is running. Some modes are only available in DEBUG mode") );
	mElements.Add( new CViewPortSetting( "Viewport Size", "The size of the viewport on the PSP." ) );

	if (HAVE_DVE && PSP_TV_CABLE > 0)
	{
		mElements.Add( new CBoolSetting( &gGlobalPreferences.TVEnable, "TV Output", "Whether to direct the video to the TV out.", "Yes", "No" ) );
		mElements.Add( new CBoolSetting( &gGlobalPreferences.TVLaced, "TV Interlaced", "Whether the TV needs interlaced output.", "Yes", "No" ) );
		mElements.Add( new CTVTypeSetting( "TV Type", "The aspect ratio of the TV." ) );
	}
	else
		gGlobalPreferences.TVEnable = false;
	mElements.Add( new CBoolSetting( &gGlobalPreferences.ForceLinearFilter,"Force Linear Filter", "Enable to force linear filter, this can improve the look of textures", "Yes", "No" ) );
	mElements.Add( new CAdjustDeadzoneSetting( mpContext, "Stick Deadzone", "Adjust the size of the deadzone applied to the PSP stick while playing. Press Start/X to edit." ) );
	if (PSP_IS_SLIM) mElements.Add( new CBoolSetting( &gGlobalPreferences.LargeROMBuffer, "Use Large ROM Buffer", "Disable this for faster loading with a small slowdown during scene changes on large ROMs. Takes effect only when loading ROM.", "Yes", "No" ) );
#ifdef DAEDALUS_DEBUG_DISPLAYLIST
	mElements.Add( new CBoolSetting( &gGlobalPreferences.HighlightInexactBlendModes, "Highlight Inexact Blend Modes",	"Replace inexact blend modes with a placeholder texture.", "Yes", "No" ) );
#endif
	mElements.Add( new CBoolSetting( &gGlobalPreferences.BatteryWarning, "Low Battery Warning",	"Whether to allow Daedalus to notify when the battery is low.", "Yes", "No" ) );
	mElements.Add( new CGuiType( "Gui Style",	"Select Gui Type either CoverFlow Style or Classic Style" ) );
	mElements.Add( new CColorSetting( "GUI Color", "Change GUI Color" ) );
	mElements.Add( new CResetSetting( "Reset Settings", "Will guide you to reset preferences to default, and hle cache files. Note : emulator will exit if resetting settings" ) );

#ifndef DAEDALUS_PUBLIC_RELEASE
	mElements.Add( new CBoolSetting( &gGlobalPreferences.SkipSplash, "Skip Splash Screen",	"Whether or not to skip the logo screen.", "Yes", "No" ) );
	mElements.Add( new CBoolSetting( &gGlobalPreferences.CustomBlendModes, "Use Custom Blend Modes",	"Debugging tool to disable custom blendmodes.", "Yes", "No" ) );
	mElements.Add( new CBoolSetting( &gGlobalPreferences.LogMicrocodes, "Log Microcodes",	"Debugging tool to log Microcodes to ucodes.txt.", "Yes", "No" ) );
#endif

}

//*************************************************************************************
//
//*************************************************************************************
IGlobalSettingsComponent::~IGlobalSettingsComponent()
{
}

//*************************************************************************************
//
//*************************************************************************************
void	IGlobalSettingsComponent::Update( float elapsed_time, const v2 & stick, u32 old_buttons, u32 new_buttons )
{

//if ((!mHleTriggered) && (!mSettingsTriggered) && (!mResetTriggered) && (!mQuitTriggered)) {
	if(old_buttons != new_buttons)
	{
		if( new_buttons & PSP_CTRL_UP )
		{
			mElements.SelectPrevious();
		}
		if( new_buttons & PSP_CTRL_DOWN )
		{
			mElements.SelectNext();
		}
		

		CUIElement *	element( mElements.GetSelectedElement() );
		if( element != NULL )
		{
			if( new_buttons & PSP_CTRL_LEFT )
			{
				element->OnPrevious();
			}
			if( new_buttons & PSP_CTRL_RIGHT )
			{
				element->OnNext();
			}
			if( new_buttons & (PSP_CTRL_CROSS|PSP_CTRL_START) )
			{
				element->OnSelected();
			}
		}
	}
}

//*************************************************************************************
//
//*************************************************************************************
void	IGlobalSettingsComponent::Render()
{
	mElements.Draw( mpContext, TEXT_AREA_LEFT, TEXT_AREA_RIGHT, AT_CENTRE, TEXT_AREA_TOP );

	CUIElement *	element( mElements.GetSelectedElement() );
	if( element != NULL )
	{
		const char *		p_description( element->GetDescription() );
		mpContext->DrawTextArea( DESCRIPTION_AREA_LEFT,
								 DESCRIPTION_AREA_TOP,
								 DESCRIPTION_AREA_RIGHT - DESCRIPTION_AREA_LEFT,
								 DESCRIPTION_AREA_BOTTOM - DESCRIPTION_AREA_TOP,
								 p_description,
								 DrawTextUtilities::TextWhite,
								 VA_BOTTOM );
	}

}

