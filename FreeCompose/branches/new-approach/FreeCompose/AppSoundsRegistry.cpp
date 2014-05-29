#include "stdafx.h"

#include "AppSoundsRegistry.h"
#include "Utils.h"

#ifdef _DEBUG
#	ifndef DBG_NEW
#		define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#		define new DBG_NEW
#	endif
#endif

static int CompositionDisplayNameIds[] = {
	IDS_APPSOUND_COMPOSITION_STARTING,
	IDS_APPSOUND_COMPOSITION_KEYPRESSED,
	IDS_APPSOUND_COMPOSITION_SUCCEEDED,
	IDS_APPSOUND_COMPOSITION_FAILED,
	IDS_APPSOUND_COMPOSITION_CANCELLED,
	IDS_APPSOUND_COMPOSITION_ESCAPE,
};

static wchar_t const* CompositionDefaultSoundSchemeFileName[] = {
	/* Starting   */ L"C:/Windows/Media/Windows Notify Calendar.wav",
	/* KeyPressed */ L"",
	/* Succeeded  */ L"",
	/* Failed     */ L"",
	/* Cancelled  */ L"",
	/* Escape     */ L"C:/Windows/Media/Windows Notify Email.wav",
};

int const NumberOfCompositionSounds = _countof( CompositionDisplayNameIds );

//
// Implementation
//

void CAppSoundsRegistry::_RegisterEventLabels( wchar_t const* pwzExeName ) {
	LSTATUS ls;
	CRegKey EventLabels;
	CString tmp;
	DWORD dwDisposition;

	ls = EventLabels.Open( HKEY_CURRENT_USER, L"AppEvents\\EventLabels" );
	for ( int n = 0; n < NumberOfCompositionSounds; n++ ) {
		CRegKey key;
		tmp.Format( L"@%s,%d", pwzExeName, -CompositionDisplayNameIds[n] );
		dwDisposition = 0;
		ls = key.Create( EventLabels, CompositionSoundNames[n], nullptr, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, nullptr, &dwDisposition );
		if ( ( ERROR_SUCCESS == ls ) && ( REG_CREATED_NEW_KEY == dwDisposition ) ) {
			ls = key.SetStringValue( nullptr, CompositionSoundNames[n] );
			ls = key.SetStringValue( L"DispFileName", tmp );
		}
		ls = key.Close( );
	}

	ls = EventLabels.Close( );
}

void CAppSoundsRegistry::_RegisterApp( wchar_t const* pwzExeName ) {
	CRegKey FreeCompose;
	CString tmp;
	LSTATUS ls;
	DWORD dwDisposition;

	CString currentSoundScheme;
	bool silentScheme = false;
	{
		CRegKey schemeskey;
		ls = schemeskey.Open( HKEY_CURRENT_USER, L"AppEvents\\Schemes", KEY_READ );

		DWORD dwType = 0;
		ULONG cbValue = 0;
		ls = schemeskey.QueryValue( nullptr, &dwType, nullptr, &cbValue );
		if ( ERROR_SUCCESS == ls || ERROR_INSUFFICIENT_BUFFER == ls ) {
			wchar_t* pbuf = tmp.GetBufferSetLength( cbValue + 1 );
			ls = schemeskey.QueryStringValue( nullptr, pbuf, &cbValue );
			if ( ERROR_SUCCESS == ls ) {
				debug( L"CAppSoundsRegistry::_RegisterApp: Current sound scheme is '%s'\n", static_cast<LPCWSTR>( currentSoundScheme ) );
				if ( 0 == currentSoundScheme.CompareNoCase( L".None" ) ) {
					debug( L"CAppSoundsRegistry::_RegisterApp: Silent sound scheme in effect!\n" );
					silentScheme = true;
				}
			} else {
				debug( L"CAppSoundsRegistry::_RegisterApp: Couldn't query value of current sound scheme: error=%lu\n", ls );
			}
		} else {
			debug( L"CAppSoundsRegistry::_RegisterApp: Couldn't determine length of current sound scheme name: error=%lu\n", ls );
		}
	}

	tmp.Format( L"@%s,%d", pwzExeName, -AFX_IDS_APP_TITLE );
	dwDisposition = 0;
	ls = FreeCompose.Create( HKEY_CURRENT_USER, L"AppEvents\\Schemes\\Apps\\FreeCompose", nullptr, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, nullptr, &dwDisposition );
	if ( ( ERROR_SUCCESS == ls ) && ( REG_CREATED_NEW_KEY == dwDisposition ) ) {
		ls = FreeCompose.SetStringValue( nullptr, LoadFromStringTable( AFX_IDS_APP_TITLE ) );
		ls = FreeCompose.SetStringValue( L"DispFileName", tmp );

		for ( int n = 0; n < NumberOfCompositionSounds; n++ ) {
			CRegKey soundkey;
			ls = soundkey.Create( FreeCompose, CompositionSoundNames[n] );
			ls = soundkey.SetStringValue( nullptr, CompositionSoundNames[n] );

			CRegKey dotdefaultkey;
			ls = dotdefaultkey.Create( soundkey, L".Default" );
			ls = dotdefaultkey.SetStringValue( nullptr, CompositionDefaultSoundSchemeFileName[n] );
			ls = dotdefaultkey.Close( );

			CRegKey dotcurrentkey;
			ls = dotcurrentkey.Create( soundkey, L".Current" );
			ls = dotcurrentkey.SetStringValue( nullptr, silentScheme ? L"" : CompositionDefaultSoundSchemeFileName[n] );
			ls = dotcurrentkey.Close( );

			CRegKey dotnonekey;
			ls = dotnonekey.Create( soundkey, L".None" );
			ls = dotnonekey.SetStringValue( nullptr, L"" );
			ls = dotnonekey.Close( );

			ls = soundkey.Close( );
		}
	}

	ls = FreeCompose.Close( );
}

void CAppSoundsRegistry::_UnregisterEventLabels( void ) {
	CRegKey EventLabels;
	LSTATUS ls = EventLabels.Open( HKEY_CURRENT_USER, L"AppEvents\\EventLabels" );
	if ( ERROR_SUCCESS != ls ) {
		debug( L"CAppSoundsRegistry::_UnregisterEventLabels: EventLabels.Open failed: %ld\n", ls );
		return;
	}

	for ( int n = 0; n < NumberOfCompositionSounds; n++ ) {
		ls = EventLabels.RecurseDeleteKey( CompositionSoundNames[n] );
		debug( L"CAppSoundsRegistry::_UnregisterEventLabels: delete of '%s': %ld\n", CompositionSoundNames[n], ls );
	}
	ls = EventLabels.Close( );
}

void CAppSoundsRegistry::_UnregisterApp( void ) {
	CRegKey Apps;
	LSTATUS ls = Apps.Open( HKEY_CURRENT_USER, L"AppEvents\\Schemes\\Apps" );
	if ( ERROR_SUCCESS != ls ) {
		debug( L"CAppSoundsRegistry::_UnregisterApp: open of HKCU\\AppEvents\\Schemes\\Apps failed: %ld\n", ls );
		return;
	}

	ls = Apps.RecurseDeleteKey( L"FreeCompose" );
	debug( L"CAppSoundsRegistry::_UnregisterApp: delete of 'FreeCompose': %ld\n", ls );
	ls = Apps.Close( );
}

//
// Interface
//

void CAppSoundsRegistry::RegisterFcAppSounds( void ) {
	wchar_t wzExeName[1024];
	GetModuleFileName( AfxGetInstanceHandle( ), wzExeName, 1024 );

	_RegisterEventLabels( wzExeName );
	_RegisterApp( wzExeName );
}

void CAppSoundsRegistry::UnregisterFcAppSounds( void ) {
	_UnregisterEventLabels( );
	_UnregisterApp( );
}
