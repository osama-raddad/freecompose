#pragma once

//==============================================================================
// Types
//==============================================================================

enum class ApplicationSound: int {
	none = -1,
	Activated = static_cast<int>( CompositionSound::max ),
	Deactivated,
	max
};

//==============================================================================
// Constants
//==============================================================================

extern UINT const APP_ACTIVATE;
extern UINT const APP_RECONFIGURE;
extern UINT const APP_NOTIFYICON;

extern UINT const FCM_KEY;
extern UINT const FCM_COMPOSITION_SOUND;

//==============================================================================
// Classes
//==============================================================================

class CFreeComposeApp: public CWinApp {
	DECLARE_DYNAMIC( CFreeComposeApp )
	DECLARE_MESSAGE_MAP( )

public:
	CFreeComposeApp( );

	virtual BOOL InitInstance( );
	virtual int ExitInstance( );

protected:
	HANDLE m_hInstanceMutex;

	bool IsAlreadyRunning( );

};

//==============================================================================
// Global variables
//==============================================================================

extern CFreeComposeApp theApp;
