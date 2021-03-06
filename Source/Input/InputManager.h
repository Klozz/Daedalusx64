#ifndef __INPUTMANAGER_H__
#define __INPUTMANAGER_H__

#include "OSHLE/ultra_os.h"
#include "Utility/Singleton.h"

#include "Math/Vector2.h"

class CInputManager : public CSingleton< CInputManager >
{
	public:
		virtual ~CInputManager() {}

#ifdef DAEDALUS_W32
		virtual void Unaquire() = 0;
		virtual void Configure(HWND hWndParent) = 0;

		// Nasty - should really be hidden in implementation
		virtual BOOL InputSelectDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
		virtual BOOL InputConfigDialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
#endif

#ifdef DAEDALUS_PSP
		virtual u32				GetNumConfigurations() const = 0;
		virtual const char *	GetConfigurationName( u32 configuration_idx ) const = 0;
		virtual const char *	GetConfigurationDescription( u32 configuration_idx ) const = 0;
		virtual void			SetConfiguration( u32 configuration_idx ) = 0;

		virtual u32				GetConfigurationFromName( const char * name ) const = 0;
#endif
		virtual bool Initialise() = 0;
		virtual void Finalise() = 0;

		virtual bool GetState( OSContPad pPad[4] ) = 0;

		static void Init() { CInputManager::Get()->Initialise();}
		static void Fini() { CInputManager::Get()->Finalise();}
};

#ifdef DAEDALUS_PSP
v2	ApplyDeadzone( const v2 & in, f32 min_deadzone, f32 max_deadzone );
#endif

#endif // __INPUTMANAGER_H__
