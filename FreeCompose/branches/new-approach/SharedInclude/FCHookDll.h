#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef FCHOOKDLL_EXPORTS
#define FCHOOKDLL_API __declspec(dllexport)
#else
#define FCHOOKDLL_API __declspec(dllimport)
#endif

#define FCHOOKDLL_API_VERSION 0x0030

//==============================================================================
// Types
//==============================================================================

struct ComposeSequence {
	ComposeSequence( ) { }
	ComposeSequence( CString& sequence, CString& result ): Sequence( sequence ), Result( result ) { }

	CString Sequence;
	CString Result;
};

enum CAPS_LOCK_TOGGLE_MODE {
	CLTM_NORMAL     = 1,
	CLTM_PRESSTWICE,
	CLTM_DISABLED,
};

enum CAPS_LOCK_SWAP_MODE {
	CLSM_NORMAL  = 1,
	CLSM_SWAP,
	CLSM_REPLACE,
};

//==============================================================================
// Constants
//==============================================================================

WPARAM const PIP_OK_1  = 1;
WPARAM const PIP_OK_2  = 2;
WPARAM const PIP_OK_3  = 3;
WPARAM const PIP_ABORT = 4;
WPARAM const PIP_ERROR = 5;
WPARAM const PIP_FAIL  = 6;

int const MAXIMUM_SEQUENCE_LENGTH = 16;

//==============================================================================
// Function prototypes
//==============================================================================

//
// Suite 0: API version
//

FCHOOKDLL_API DWORD FcGetApiVersion( void );

//
// Suite 1: Lifetime management
//

FCHOOKDLL_API BOOL FcInitialize( void );
FCHOOKDLL_API BOOL FcUninitialize( void );

//
// Suite 2: Hook control
//

FCHOOKDLL_API BOOL FcEnableHook( void );
FCHOOKDLL_API BOOL FcDisableHook( void );
FCHOOKDLL_API BOOL FcIsHookEnabled( void );

FCHOOKDLL_API BOOL FcSetNotifyWindowHandle( HWND hwndNotifyWindow );
FCHOOKDLL_API HWND FcGetNotifyWindowHandle( void );

//
// Suite 3: Compose key and sequences
//

FCHOOKDLL_API BOOL FcSetComposeKey( DWORD vkCompose );
FCHOOKDLL_API DWORD FcGetComposeKey( void );

FCHOOKDLL_API BOOL FcSetComposeSequences( ComposeSequence* pSequences, DWORD cSequences );

//
// Suite 4: Caps Lock
//

FCHOOKDLL_API BOOL FcSetSwapCapsLockKey( DWORD vkCapsLockSwap );
FCHOOKDLL_API DWORD FcGetSwapCapsLockKey( void );

FCHOOKDLL_API BOOL FcSetCapsLockToggleMode( CAPS_LOCK_TOGGLE_MODE mode );
FCHOOKDLL_API CAPS_LOCK_TOGGLE_MODE FcGetCapsLockToggleMode( void );

FCHOOKDLL_API BOOL FcSetCapsLockSwapMode( CAPS_LOCK_SWAP_MODE mode );
FCHOOKDLL_API CAPS_LOCK_SWAP_MODE FcGetCapsLockSwapMode( void );

#ifdef __cplusplus
};
#endif
