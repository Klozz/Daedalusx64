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

#include "../../stdafx.h"
#include "SavestateSelectorComponent.h"

#include "UIContext.h"
#include "UIScreen.h"
#include "UIElement.h"
#include "UICommand.h"

#include "../../Core/ROM.h"
#include "../../Core/SaveState.h"

#include "../../Utility/Stream.h"
#include "../../Utility/IO.h"
#include "../../Utility/Translate.h"

#include "../../Math/Vector2.h"

#include "../Graphics/DrawText.h"

#include <pspctrl.h>
#include <pspgu.h>

namespace
{
	//const char *			INSTRUCTIONS_TEXT = "Select a game from the list"; //ToDo - Not Currently Used

	const char * const		SAVING_STATUS_TEXT  = "Saving...";
	const char * const		LOADING_STATUS_TEXT = "Loading...";

	const u32				TEXT_AREA_TOP = 272 / 2;
	const u32				TEXT_AREA_LEFT = 20;
	const u32				TEXT_AREA_RIGHT = 460;

	const s32				DESCRIPTION_AREA_TOP = 272-20;		// We render text aligned from the bottom, so this is largely irrelevant
	const s32				DESCRIPTION_AREA_BOTTOM = 272-10;
	const s32				DESCRIPTION_AREA_LEFT = 16;
	const s32				DESCRIPTION_AREA_RIGHT = 480-16;

	const u32				NUM_SAVESTATE_SLOTS = 64;

	const u32				INVALID_SLOT = u32( -1 );
}

//*************************************************************************************
//
//*************************************************************************************
class ISavestateSelectorComponent : public CSavestateSelectorComponent
{
	public:

		ISavestateSelectorComponent( CUIContext * p_context, EAccessType access_type, CFunctor1< const char * > * on_slot_selected, const char *running_rom );
		~ISavestateSelectorComponent();

		// CUIScreen
		virtual void				Update( float elapsed_time, const v2 & stick, u32 old_buttons, u32 new_buttons );
		virtual void				Render();
		virtual bool				IsFinished() const									{ return mIsFinished; }
	public:
		char 					current_slot_path[MAX_PATH];
		bool					isGameRunning;

	private:
		void				OnSlotSelected( u32 slot_idx );
		void				OnFolderSelected( u32 index );
		void				LoadFolders();
		void				LoadSlots();
		void				deleteSlot(u32 id_ss);
		bool					isDeletionAllowed;

	private:
		EAccessType				mAccessType;
		CFunctor1< const char * > *		mOnSlotSelected;

		u32					mSelectedSlot;
		bool					mIsFinished;
		bool	deleteButtonTriggered;

		CUIElementBag				mElements;
		std::vector<std::string> 		mElementTitle;
		bool					mSlotEmpty[ NUM_SAVESTATE_SLOTS ];
};

//*************************************************************************************
//
//*************************************************************************************
CSavestateSelectorComponent::~CSavestateSelectorComponent()
{
}

//*************************************************************************************
//
//*************************************************************************************
CSavestateSelectorComponent::CSavestateSelectorComponent( CUIContext * p_context )
:	CUIComponent( p_context )
{
}

//*************************************************************************************
//
//*************************************************************************************
CSavestateSelectorComponent *	CSavestateSelectorComponent::Create( CUIContext * p_context, EAccessType access_type, CFunctor1< const char * > * on_slot_selected, const char *running_rom )
{
	return new ISavestateSelectorComponent( p_context, access_type, on_slot_selected, running_rom );
}

//*************************************************************************************
//
//*************************************************************************************
namespace
{
	void MakeSaveSlotPath(char * path, u32 slot_idx, char *slot_path )
	{
		char	filename[ MAX_PATH ];
		char    sub_path[ MAX_PATH ];
		sprintf( filename, "saveslot%u.ss", slot_idx );
		sprintf( sub_path, "SaveStates/%s", slot_path);
		if(!IO::Directory::IsDirectory( "ms0:/n64/SaveStates/" ))
		{
			IO::Path::Combine( path, gDaedalusExePath, sub_path );
			IO::Directory::EnsureExists( path );		// Ensure this dir exists
		}
		else
		{
			IO::Path::Combine( path, "ms0:/n64/", sub_path );
			IO::Directory::EnsureExists( path );		// Ensure this dir exists
		}
		IO::Path::Append( path, filename );
	}
}

//*************************************************************************************
//
//*************************************************************************************
ISavestateSelectorComponent::ISavestateSelectorComponent( CUIContext * p_context, EAccessType access_type, CFunctor1< const char * > * on_slot_selected, const char *running_rom )
:	CSavestateSelectorComponent( p_context )
,	mAccessType( access_type )
,	mOnSlotSelected( on_slot_selected )
,	mSelectedSlot( INVALID_SLOT )
,	mIsFinished( false )
,	deleteButtonTriggered(false)
{


	if(running_rom){
		isGameRunning = true;
		strcpy(current_slot_path, running_rom);
		LoadSlots();
		isDeletionAllowed=true;
	} else {
		isGameRunning = false;
		LoadFolders();
		isDeletionAllowed=false;
	}
}

void ISavestateSelectorComponent::LoadFolders(){
	IO::FindHandleT		find_handle;
	IO::FindDataT		find_data;
	u32 i=0;
	const char * description_text( mAccessType == AT_SAVING ? "Select the slot in which to save" : "Select the slot from which to load" );
	char full_path[MAX_PATH];
	// We're using the same vector for directory names and slots, so we have to clear it
	mElements.Clear();
	if (IO::FindFileOpen( "ms0:/n64/SaveStates" , &find_handle, find_data))
	{
		do 
		{
			IO::Path::Combine( full_path, "ms0:/n64/SaveStates", find_data.Name);
			if(IO::Directory::IsDirectory(full_path) && strlen( find_data.Name ) > 2 )
			{
				COutputStringStream             str;
				CUIElement *    		element;
				str << find_data.Name;
				CFunctor1< u32 > *	functor_1( new CMemberFunctor1< ISavestateSelectorComponent, u32 >( this, &ISavestateSelectorComponent::OnFolderSelected ) );
				CFunctor *		curried( new CCurriedFunctor< u32 >( functor_1, i++ ) );

				element = new CUICommandImpl( curried, str.c_str(), description_text );
				mElements.Add( element );
				mElementTitle.push_back(find_data.Name);
			}
		}
		while(IO::FindFileNext( find_handle, find_data ));
		IO::FindFileClose( find_handle );
	}
	else if(IO::FindFileOpen( "SaveStates" , &find_handle, find_data))
	{
		do 
		{
			IO::Path::Combine( full_path, "SaveStates", find_data.Name);
			if(IO::Directory::IsDirectory(full_path) && strlen( find_data.Name ) > 2 )
			{
				COutputStringStream             str;
				CUIElement *    		element;
				str << find_data.Name;
				CFunctor1< u32 > *	functor_1( new CMemberFunctor1< ISavestateSelectorComponent, u32 >( this, &ISavestateSelectorComponent::OnFolderSelected ) );
				CFunctor *		curried( new CCurriedFunctor< u32 >( functor_1, i++ ) );

				element = new CUICommandImpl( curried, str.c_str(), description_text );
				mElements.Add( element );
				mElementTitle.push_back(find_data.Name);
			}
		}
		while(IO::FindFileNext( find_handle, find_data ));
		IO::FindFileClose( find_handle );
	}
	else
	{
		CUIElement *                    element;
		element = new CUICommandDummy( "There are no Savestates to load", "There are no Savestates to load" );
		mElements.Add( element );
	}

}

void ISavestateSelectorComponent::LoadSlots(){
	const char * description_text( mAccessType == AT_SAVING ? "Select the slot in which to save [X:save O:back]" : "Select the slot from which to load [X:load O:back []:delete]" );
	SceIoStat file_stat;
	char date_string[30];
	// We're using the same vector for directory names and slots, so we have to clear it
	mElements.Clear();

	for( u32 i = 0; i < NUM_SAVESTATE_SLOTS; ++i )
	{
		COutputStringStream		str;
		str << Translate_String("Slot ") << (i+1) << ": ";

		char filename[ MAX_PATH ];
		MakeSaveSlotPath( filename, i, current_slot_path);

		RomID			rom_id( SaveState_GetRomID( filename ) );
		RomSettings		settings;

		CUIElement *	element;
		if( !rom_id.Empty() && CRomSettingsDB::Get()->GetSettings( rom_id, &settings ) )
		{
			IO::File::Stat(filename, &file_stat);
			sprintf(date_string, "%02d/%02d/%d %02d:%02d:%02d", file_stat.st_ctime.month,  file_stat.st_ctime.day, file_stat.st_ctime.year, file_stat.st_ctime.hour, file_stat.st_ctime.minute, file_stat.st_ctime.second); // settings.GameName.c_str();
			str << date_string;
			mSlotEmpty[ i ] = false;
		}
		else
		{
			str << Translate_String("<empty>");
			mSlotEmpty[ i ] = true;
		}

		// 
		//	Don't allow empty slots to be loaded
		//
		if( mAccessType == AT_LOADING && mSlotEmpty[ i ] )
		{
			element = new CUICommandDummy( str.c_str(), description_text );
		}
		else
		{
			CFunctor1< u32 > *		functor_1( new CMemberFunctor1< ISavestateSelectorComponent, u32 >( this, &ISavestateSelectorComponent::OnSlotSelected ) );
			CFunctor *				curried( new CCurriedFunctor< u32 >( functor_1, i ) );

			element = new CUICommandImpl( curried, str.c_str(), description_text );
		}

		mElements.Add( element );
	}
		

}

//*************************************************************************************
//
//*************************************************************************************
ISavestateSelectorComponent::~ISavestateSelectorComponent()
{
	delete mOnSlotSelected;
}

//*************************************************************************************
//
//*************************************************************************************
void	ISavestateSelectorComponent::Update( float elapsed_time, const v2 & stick, u32 old_buttons, u32 new_buttons )
{
	//
	//	Trigger the save on the first update AFTER mSelectedSlot was set.
	//	This ensures we get at least one frame where we can display "Saving..." etc.
	//
	if( mSelectedSlot != INVALID_SLOT && !mIsFinished )
	{
		mIsFinished = true;

		char filename[ MAX_PATH ];
		MakeSaveSlotPath( filename, mSelectedSlot, current_slot_path );

		(*mOnSlotSelected)( filename );
	}

	if(old_buttons != new_buttons)
	{
	  if(mAccessType == AT_LOADING && deleteButtonTriggered)
		{
		  if( new_buttons & PSP_CTRL_TRIANGLE )
			{
				deleteSlot(mElements.GetSelectedIndex());
			}
		}
		
		if( new_buttons & PSP_CTRL_UP )
		{
			mElements.SelectPrevious();
			if(mAccessType ==AT_LOADING)
			    deleteButtonTriggered=false;
		}
		if( new_buttons & PSP_CTRL_DOWN )
		{
			mElements.SelectNext();
			if(mAccessType == AT_LOADING)
			    deleteButtonTriggered=false;
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
				// Commit settings
				element->OnSelected();
			}
			if( new_buttons & PSP_CTRL_SQUARE)
			{
				//delete savestate
				if(mAccessType == AT_LOADING && isDeletionAllowed)
				{  
				  if ((mElements.GetSelectedElement())->IsSelectable())
				    deleteButtonTriggered=true;
				}
				
			}
			if( new_buttons & (PSP_CTRL_CIRCLE|PSP_CTRL_SELECT) )
			{
				// Discard settings
				deleteButtonTriggered=false;
				if(isGameRunning == false)
				{
				  LoadFolders();
				  isDeletionAllowed=false;
				}
				else
					mIsFinished = true;
			}
		}
	}
}


void	ISavestateSelectorComponent::deleteSlot(u32 id_ss)
{
    char	ss_path[ MAX_PATH ];
    char	filename[ MAX_PATH ];
    char	sub_path[ MAX_PATH ];
    sprintf( filename, "saveslot%u.ss", id_ss );
    sprintf( sub_path, "SaveStates/%s", current_slot_path);
	if(!IO::Directory::IsDirectory( "ms0:/n64/SaveStates/" ))
	{
		IO::Path::Combine( ss_path, gDaedalusExePath, sub_path );
	}
	else
	{
		IO::Path::Combine( ss_path, "ms0:/n64/", sub_path );
	}
	IO::Path::Append( ss_path, filename );

	if (IO::File::Exists(ss_path))
    {
      remove(ss_path);
      deleteButtonTriggered=false;
      LoadSlots();
    }
}


//*************************************************************************************
//
//*************************************************************************************
void	ISavestateSelectorComponent::Render()
{
	const u32	font_height( mpContext->GetFontHeight() );

	if( mSelectedSlot == INVALID_SLOT )
	{
		mElements.Draw( mpContext, TEXT_AREA_LEFT, TEXT_AREA_RIGHT, AT_LEFT, TEXT_AREA_TOP - mElements.GetSelectedIndex()*(font_height+2) );

		/*if( mElements.GetSelectedIndex() > 0 )
			mElements.Draw( mpContext, TEXT_AREA_LEFT, TEXT_AREA_RIGHT, AT_LEFT, TEXT_AREA_TOP - mElements.GetSelectedIndex()*11 );
		else
			mElements.Draw( mpContext, TEXT_AREA_LEFT, TEXT_AREA_RIGHT, AT_LEFT, TEXT_AREA_TOP);*/

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
	else
	{
		const char * title_text( mAccessType == AT_SAVING ? SAVING_STATUS_TEXT : LOADING_STATUS_TEXT );

		s32 y( ( mpContext->GetScreenHeight() - font_height ) / 2 + font_height );
		mpContext->DrawTextAlign( 0, mpContext->GetScreenWidth(), AT_CENTRE, y, title_text, mpContext->GetDefaultTextColour() );
	}
	
	if(deleteButtonTriggered)
	  mpContext->DrawTextAlign(0,480,AT_CENTRE,135,"Press Triangle to delete this savestate",DrawTextUtilities::TextRed,DrawTextUtilities::TextWhite);
}

//*************************************************************************************
//
//*************************************************************************************
void	ISavestateSelectorComponent::OnSlotSelected( u32 slot_idx )
{
	if( slot_idx >= NUM_SAVESTATE_SLOTS )
		return;

	// Don't allow empty slots to be loaded
	if( mAccessType == AT_LOADING && mSlotEmpty[ slot_idx ] )
		return;

	mSelectedSlot = slot_idx;
}

void	ISavestateSelectorComponent::OnFolderSelected( u32 index )
{
	strcpy( current_slot_path, mElementTitle[index].c_str());
	mElementTitle.clear();
	LoadSlots();
	isDeletionAllowed=true;
}