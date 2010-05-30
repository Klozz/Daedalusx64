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
#include "AboutComponent.h"

#include "UIContext.h"
#include "UIScreen.h"
#include "SysPSP/Graphics/DrawText.h"
#include "Graphics/ColourValue.h"
#include "Graphics/NativeTexture.h"
#include "Math/Vector2.h"

#include "Math/MathUtil.h"

#include "SysPSP/Utility/PathsPSP.h"

#include "svnversion.h"

#include <pspctrl.h>
#include <pspgu.h>

namespace
{
	const u32				TEXT_AREA_LEFT = 40;
	const u32				TEXT_AREA_RIGHT = 480-40;

	const char * const DAEDALUS_VERSION_TEXT = "DaedalusX64 Revision "SVNVERSION"";

	const char * const		DATE_TEXT = "Built " __DATE__;

	const char * const		INFO_TEXT[] =
	{
		"Copyright (C) 2008-2010 DaedalusX64 Team",
		"Copyright (C) 2001-2009 StrmnNrmn",
		"Audio HLE code by Azimer",
		"",
		"For news and updates visit:",
	};

	const char * const		URL_TEXT_1 = "http://sf.net/projects/DaedalusX64/";
	const char * const		URL_TEXT_2 = "http://DaedalusX64.com/";

	const char * const		LOGO_FILENAME = DAEDALUS_PSP_PATH( "Resources/logo.png" );
}

//*************************************************************************************
//
//*************************************************************************************
class IAboutComponent : public CAboutComponent
{
	public:

		IAboutComponent( CUIContext * p_context );
		~IAboutComponent();

		// CUIComponent
		virtual void				Update( f32 elapsed_time, const v2 & stick, u32 old_buttons, u32 new_buttons );
		virtual void				Render();

	private:
		CRefPtr<CNativeTexture>		mpTexture;
};


//*************************************************************************************
//
//*************************************************************************************
CAboutComponent::CAboutComponent( CUIContext * p_context )
:	CUIComponent( p_context )
{
}

//*************************************************************************************
//
//*************************************************************************************
CAboutComponent::~CAboutComponent()
{
}

//*************************************************************************************
//
//*************************************************************************************
CAboutComponent *	CAboutComponent::Create( CUIContext * p_context )
{
	return new IAboutComponent( p_context );
}

//*************************************************************************************
//
//*************************************************************************************
IAboutComponent::IAboutComponent( CUIContext * p_context )
:	CAboutComponent( p_context )
,	mpTexture( CNativeTexture::CreateFromPng( LOGO_FILENAME, TexFmt_8888 ) )
{
}

//*************************************************************************************
//
//*************************************************************************************
IAboutComponent::~IAboutComponent()
{
}

//*************************************************************************************
//
//*************************************************************************************
void	IAboutComponent::Update( float elapsed_time, const v2 & stick, u32 old_buttons, u32 new_buttons )
{
}

//*************************************************************************************
//
//*************************************************************************************
void	IAboutComponent::Render()
{
	u32		text_top( 38 );

	if(mpTexture != NULL)
	{
		u32		w( mpTexture->GetWidth() );
		u32		h( mpTexture->GetHeight() );

		f32		desired_height = 60.0f;
		f32		scale( desired_height / f32( h ) );

		v2		wh( f32( w ) * scale, f32( h ) * scale );
		v2		tl( f32( (480 - wh.x)/2 ), f32( text_top ) );

		mpContext->RenderTexture( mpTexture, tl, wh, c32::White );

		text_top += u32( wh.y + 10.0f );
	}

	s32			y;
	const u32	line_height( mpContext->GetFontHeight() + 2 );

	y = text_top;

	CFixedString<128>	version( DAEDALUS_VERSION_TEXT );
#ifndef DAEDALUS_PUBLIC_RELEASE
	version += " (";
	version += DAEDALUS_CONFIG_VERSION;
	version += ")";
#endif

	mpContext->DrawTextAlign( TEXT_AREA_LEFT, TEXT_AREA_RIGHT, AT_CENTRE, y, version, DrawTextUtilities::TextWhite ); y += line_height;
	mpContext->DrawTextAlign( TEXT_AREA_LEFT, TEXT_AREA_RIGHT, AT_CENTRE, y, DATE_TEXT, DrawTextUtilities::TextWhite ); y += line_height;

	// Spacer
	y += line_height;

	for( u32 i = 0; i < ARRAYSIZE( INFO_TEXT ); ++i )
	{
		const char * str( INFO_TEXT[ i ] );

		mpContext->DrawTextAlign( TEXT_AREA_LEFT, TEXT_AREA_RIGHT, AT_CENTRE, y, str, DrawTextUtilities::TextWhite );
		y += line_height;
	}

	mpContext->DrawTextAlign( TEXT_AREA_LEFT, TEXT_AREA_RIGHT, AT_CENTRE, y, URL_TEXT_1, DrawTextUtilities::TextRed, c32( 255,255,255,160 ) );	y += line_height;
	mpContext->DrawTextAlign( TEXT_AREA_LEFT, TEXT_AREA_RIGHT, AT_CENTRE, y, URL_TEXT_2, DrawTextUtilities::TextRed, c32( 255,255,255,255 ) );	y += line_height;
}

