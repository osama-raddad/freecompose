#include "stdafx.h"

#include <psapi.h>

#import <msxml6.dll>

#include "FreeCompose.h"
#include "OptionsData.h"

#include "ComposeDefaults.h"
#include "Utils.h"

COptionsData::COptionsData( ) {
	Load( );
}

COptionsData::COptionsData( const COptionsData& options ) {
	operator=( options );
}

COptionsData::~COptionsData( ) {
	m_ComposeKeyEntries.RemoveAll( );
}

COptionsData& COptionsData::operator=( const COptionsData& options ) {
	m_fStartActive       = options.m_fStartActive;
	m_fStartWithWindows  = options.m_fStartWithWindows;

	m_fSwapCapsLock      = options.m_fSwapCapsLock;
	m_CapsLockToggleMode = options.m_CapsLockToggleMode;
	m_CapsLockSwapMode   = options.m_CapsLockSwapMode;

	m_vkCompose          = options.m_vkCompose;
	m_vkSwapCapsLock     = options.m_vkSwapCapsLock;

	m_ComposeKeyEntries.RemoveAll( );
	m_ComposeKeyEntries.Copy( options.m_ComposeKeyEntries );

	return *this;
}

bool COptionsData::operator==( const COptionsData& options ) {
	if ( m_fStartActive       != options.m_fStartActive       ) return false;
	if ( m_fStartWithWindows  != options.m_fStartWithWindows  ) return false;
	if ( m_fSwapCapsLock      != options.m_fSwapCapsLock      ) return false;
	if ( m_CapsLockToggleMode != options.m_CapsLockToggleMode ) return false;
	if ( m_CapsLockSwapMode   != options.m_CapsLockSwapMode   ) return false;
	if ( m_vkCompose          != options.m_vkCompose          ) return false;
	if ( m_vkSwapCapsLock     != options.m_vkSwapCapsLock     ) return false;

	if ( m_ComposeKeyEntries.GetCount( ) != options.m_ComposeKeyEntries.GetCount( ) )
		return false;
	for ( int n = 0; n < m_ComposeKeyEntries.GetCount( ); n++ ) {
		if ( m_ComposeKeyEntries[n] != options.m_ComposeKeyEntries[n] )
			return false;
	}

	return true;
}

bool COptionsData::operator!=( const COptionsData& options ) {
	return ! operator==( options );
}

bool COptionsData::_FcValidateCke( const COMPOSE_KEY_ENTRY& cke ) {
	if ( 0 == ( cke.vkFirst &  0xFFUL ) )
		return false;
	if ( 0 != ( cke.vkFirst & ~0x800000FFUL ) )
		return false;

	if ( 0 == ( cke.vkSecond &  0xFFUL ) )
		return false;
	if ( 0 != ( cke.vkSecond & ~0x800000FFUL ) )
		return false;

	if ( 0 == cke.u32Composed )
		return false;
	if ( cke.u32Composed >= 0xD800U && cke.u32Composed < 0xE000U )
		return false;
	if ( cke.u32Composed > 0x10FFFFU )
		return false;

	return true;
}

void COptionsData::_FcLoadKeys( void ) {
	m_ComposeKeyEntries.RemoveAll( );

	int count = theApp.GetProfileInt( _T("Mapping"), _T("Count"), 0 );
	if ( count < 1 ) {
		count = _countof( DefaultComposeKeyEntries );
		m_ComposeKeyEntries.SetSize( count );
		for ( int n = 0; n < count; n++ ) {
			m_ComposeKeyEntries[n] = DefaultComposeKeyEntries[n];
		}
		return;
	}

	COMPOSE_KEY_ENTRY cke;
	CString section;
	m_ComposeKeyEntries.SetSize( count );
	int index = 0;
	for ( int n = 0; n < count; n++ ) {
		section.Format( _T("Mapping\\%d"), n );
		cke.vkFirst     = (DWORD)    theApp.GetProfileInt( section, _T("First"),    0 );
		cke.vkSecond    = (DWORD)    theApp.GetProfileInt( section, _T("Second"),   0 );
		cke.u32Composed = (unsigned) theApp.GetProfileInt( section, _T("Composed"), 0 );
		if ( _FcValidateCke( cke ) ) {
			m_ComposeKeyEntries[index++] = cke;
		} else {
			debug( _T("Bad mapping, #%d\n"), n );
		}
	}
}

void COptionsData::_FcSaveKeys( void ) {
	theApp.DelRegTree( theApp.GetAppRegistryKey( ), CString( _T("Mapping") ) );

	CString section;
	int count = 0;
	for ( int n = 0; n < (int) m_ComposeKeyEntries.GetSize( ); n++ ) {
		if ( !m_ComposeKeyEntries[n].vkFirst && !m_ComposeKeyEntries[n].vkSecond && !m_ComposeKeyEntries[n].u32Composed ) {
			continue;
		}
		section.Format( _T("Mapping\\%d"), count++ );
		theApp.WriteProfileInt( section, _T("First"),    (int) m_ComposeKeyEntries[n].vkFirst     );
		theApp.WriteProfileInt( section, _T("Second"),   (int) m_ComposeKeyEntries[n].vkSecond    );
		theApp.WriteProfileInt( section, _T("Composed"), (int) m_ComposeKeyEntries[n].u32Composed );
	}

	theApp.WriteProfileInt( _T("Mapping"), _T("Count"), count );
}

void COptionsData::_UpdateRunKey( void ) {
	LSTATUS rc;
	HKEY hk;
	
	rc = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_SET_VALUE, &hk );
	if ( ERROR_SUCCESS != rc ) {
		debug( _T("COptionsData::_UpdateRunKey: RegOpenKeyEx failed: %d\n"), rc );
		return;
	}

	if ( m_fStartWithWindows ) {
		wchar_t lpszImageFilename[1024];
		if ( GetModuleFileNameEx( GetCurrentProcess( ), AfxGetApp( )->m_hInstance, lpszImageFilename, countof(lpszImageFilename) ) > 0 ) {
			rc = RegSetValueEx( hk, _T("FreeCompose"), 0, REG_SZ, (LPBYTE) lpszImageFilename, (DWORD) ( sizeof(wchar_t) * ( wcslen( lpszImageFilename ) + 1 ) ) );
			if ( ERROR_SUCCESS != rc ) {
				debug( _T("COptionsData::_UpdateRunKey: RegSetValueEx failed: %d\n"), rc );
			}
		} else {
			debug( _T("COptionsData::_UpdateRunKey: GetModuleFileNameEx failed: %d\n"), GetLastError( ) );
		}
	} else {
		rc = RegDeleteValue( hk, _T("FreeCompose") );
		if ( ERROR_SUCCESS != rc ) {
			debug( _T("COptionsData::_UpdateRunKey: RegDeleteValue failed: %d\n"), rc );
		}
	}

	RegCloseKey( hk );
}

void COptionsData::Load( void ) {
	m_fStartActive       = (BOOL)  theApp.GetProfileInt( _T("Startup"),  _T("StartActive"),        TRUE );
	m_fStartWithWindows  = (BOOL)  theApp.GetProfileInt( _T("Startup"),  _T("StartWithWindows"),   FALSE );

	m_fSwapCapsLock      = (BOOL)  theApp.GetProfileInt( _T("Keyboard"), _T("SwapCapsLock"),       FALSE );

	m_CapsLockToggleMode =
		   (CAPS_LOCK_TOGGLE_MODE) theApp.GetProfileInt( _T("Keyboard"), _T("CapsLockToggleMode"), 
		   (CAPS_LOCK_TOGGLE_MODE) theApp.GetProfileInt( _T("Keyboard"), _T("CapsLockMode"),       CLTM_NORMAL ) );
	m_CapsLockSwapMode   =
			 (CAPS_LOCK_SWAP_MODE) theApp.GetProfileInt( _T("Keyboard"), _T("CapsLockSwapMode"),   CLSM_SWAP );

	m_vkCompose          = (DWORD) theApp.GetProfileInt( _T("Keyboard"), _T("ComposeKey"),         VK_APPS );
	m_vkSwapCapsLock     = (DWORD) theApp.GetProfileInt( _T("Keyboard"), _T("SwapCapsLockKey"),    VK_LCONTROL );

	_FcLoadKeys( );

	// HACK HACK HACK HACK HACK
	// yes it should be LoadFromXml here, but it's SaveToXml for testing purposes!!
	SaveToXml( );
}

void COptionsData::Save( void ) {
	SaveToXml( );

	theApp.WriteProfileInt( _T("Startup"), _T("StartActive"),         (int) m_fStartActive );
	theApp.WriteProfileInt( _T("Startup"), _T("StartWithWindows"),    (int) m_fStartWithWindows );

	theApp.WriteProfileInt( _T("Keyboard"), _T("SwapCapsLock"),       (int) m_fSwapCapsLock );
	theApp.WriteProfileInt( _T("Keyboard"), _T("CapsLockToggleMode"), (int) m_CapsLockToggleMode );
	theApp.WriteProfileInt( _T("Keyboard"), _T("CapsLockSwapMode"),   (int) m_CapsLockSwapMode );

	theApp.WriteProfileInt( _T("Keyboard"), _T("ComposeKey"),         (int) m_vkCompose );
	theApp.WriteProfileInt( _T("Keyboard"), _T("SwapCapsLockKey"),    (int) m_vkSwapCapsLock );

	_FcSaveKeys( );

	_UpdateRunKey( );
}

// =============================================================================
// New-fangled XML stuff starts here!
// =============================================================================

typedef MSXML2::IXMLDOMDocumentPtr XDocument;
typedef MSXML2::IXMLDOMProcessingInstructionPtr XProcessingInstruction;
typedef MSXML2::IXMLDOMElementPtr XElement;
typedef MSXML2::IXMLDOMAttributePtr XAttribute;

static const _bstr_t mapBoolToString[ ] = {
	L"false",
	L"true",
};

static const _bstr_t mapCapsLockToggleModeToString[ ] = {
	(LPCWSTR) NULL,
	L"normal",
	L"pressTwice",
	L"disabled",
};

static const _bstr_t mapCapsLockSwapModeToString[ ] = {
	(LPCWSTR) NULL,
	L"swap",
	L"replace",
};

inline _bstr_t strFromBool( const bool _ ) {
	return mapBoolToString[_];
}

inline _bstr_t strFromCapsLockToggleMode( const CAPS_LOCK_TOGGLE_MODE _ ) {
	return mapCapsLockToggleModeToString[_];
}

inline _bstr_t strFromCapsLockSwapMode( const CAPS_LOCK_SWAP_MODE _ ) {
	return _bstr_t( mapCapsLockSwapModeToString[_] );
}

inline _bstr_t strFromInt( const int _ ) {
	return _bstr_t( _variant_t( _ ) );
}

void COptionsData::LoadFromXml( void ) {
}

void COptionsData::SaveToXml( void ) {
	CString pathname;
	if ( ! GetAppDataFolder( pathname ) ) {
		debug( L"COptionsData::SaveToXml: GetAppDataFolder failed\n" );
		return;
	}

	XDocument doc;
	HRESULT hr = doc.CreateInstance( __uuidof( MSXML2::DOMDocument60 ), NULL, CLSCTX_INPROC_SERVER );
	if ( FAILED(hr) ) {
		debug( L"NewFangledStuff: Can't create instance of DOMDocument: hr=0x%08x\n", hr );
		return;
	}

	try {
		doc->async = VARIANT_FALSE;
		doc->validateOnParse = VARIANT_FALSE;
		doc->resolveExternals = VARIANT_FALSE;
		
		XProcessingInstruction pi = doc->createProcessingInstruction( L"xml", L"version='1.0' encoding='utf-8'" );
        doc->appendChild( pi );

		XElement FreeCompose = doc->createElement( L"FreeCompose" );
		
			XElement Options = doc->createElement( L"Options" );

				XElement Startup = doc->createElement( L"Startup" );

					XElement StartActive = doc->createElement( L"StartActive" );
					StartActive->text = strFromBool( !!m_fStartActive );

					XElement StartWithWindows = doc->createElement( L"StartWithWindows" );
					StartWithWindows->text = strFromBool( !!m_fStartWithWindows );

				Startup->appendChild( StartActive );
				Startup->appendChild( StartWithWindows );

				XElement Keyboard = doc->createElement( L"Keyboard" );

					XElement SwapCapsLock = doc->createElement( L"SwapCapsLock" );
					SwapCapsLock->text = strFromBool( !!m_fSwapCapsLock );

					XElement CapsLockToggleMode = doc->createElement( L"CapsLockToggleMode" );
					CapsLockToggleMode->text = strFromCapsLockToggleMode( m_CapsLockToggleMode );

					XElement CapsLockSwapMode = doc->createElement( L"CapsLockSwapMode" );
					CapsLockSwapMode->text = strFromCapsLockSwapMode( m_CapsLockSwapMode );

					XElement ComposeKey = doc->createElement( L"ComposeKey" );
					ComposeKey->text = strFromInt( m_vkCompose );

					XElement SwapCapsLockKey = doc->createElement( L"SwapCapsLockKey" );
					SwapCapsLockKey->text = strFromInt( m_vkSwapCapsLock );

				Keyboard->appendChild( SwapCapsLock );
				Keyboard->appendChild( CapsLockToggleMode );
				Keyboard->appendChild( CapsLockSwapMode );
				Keyboard->appendChild( ComposeKey );
				Keyboard->appendChild( SwapCapsLockKey );

			Options->appendChild( Startup );
			Options->appendChild( Keyboard );

		FreeCompose->appendChild( Options );

		doc->appendChild( FreeCompose );

		doc->save( L"C:/Users/jsc/AppData/Local/Temp/FreeCompose.xml" );
	}
	catch ( _com_error e ) {
		debug( L"NewFangledStuff: caught exception. HRESULT=0x%08x\n", e.Error( ) );
	}

	doc = NULL;
}
