#include "stdafx.h"

#include "FreeCompose.h"
#include "MainFrm.h"
#include "AboutDlg.h"
#include "OptionsPropSheet.h"
#include "NTray.h"
#include "Utils.h"

//
// Message map for CMainFrame
//

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP( CMainFrame )
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_REGISTERED_MESSAGE (APP_ACTIVATE,     &CMainFrame::OnActivate)
	ON_REGISTERED_MESSAGE (APP_NOTIFYICON,   &CMainFrame::OnNotifyIcon)
	ON_REGISTERED_MESSAGE (APP_RECONFIGURE,  &CMainFrame::OnReconfigure)
	ON_REGISTERED_MESSAGE (FCM_PIP,          &CMainFrame::OnFcmPip)
	ON_COMMAND            (ID_APP_ABOUT,     &CMainFrame::OnAppAbout)
	ON_COMMAND            (ID_APP_CAPSLOCK,  &CMainFrame::OnAppCapsLock)
	ON_COMMAND            (ID_APP_CONFIGURE, &CMainFrame::OnAppConfigure)
	ON_COMMAND            (ID_APP_EXIT,      &CMainFrame::OnAppExit)
	ON_COMMAND            (ID_APP_TOGGLE,    &CMainFrame::OnAppToggle)
#ifdef _DEBUG
	ON_COMMAND            (ID_APP_ZAPCONF,   &CMainFrame::OnAppZapConf)
#endif
	ON_UPDATE_COMMAND_UI  (ID_APP_CAPSLOCK,  &CMainFrame::OnUpdateAppCapsLock)
	ON_UPDATE_COMMAND_UI  (ID_APP_TOGGLE,    &CMainFrame::OnUpdateAppToggle)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// Local functions
//

static bool IsCapsLockOn( void ) {
	return ( GetKeyState( VK_CAPITAL ) & 1 ) != 0;
}

//
// CMainFrame implementation
//

CMainFrame::CMainFrame( ):
	m_pTrayIcon   ( nullptr ),
	m_pOptions    ( nullptr ),
	m_strTitle    ( LoadFromStringTable( AFX_IDS_APP_TITLE ) ),
	m_strEnabled  ( LoadFromStringTable( IDS_MAINFRAME_ENABLED ) ),
	m_strDisabled ( LoadFromStringTable( IDS_MAINFRAME_DISABLED ) ),
	m_pPropSheet  ( nullptr )
{
}

CMainFrame::~CMainFrame( ) {
	if ( m_pOptions ) {
		delete m_pOptions;
	}
}

void CMainFrame::_Initialize( void ) {
	m_pOptions = new COptionsData( );
	m_pOptions->Load( );
	// HACK HACK HACK HACK HACK
	m_pOptions->StartActive = FALSE;
	// HACK ENDS HACK ENDS HACK ENDS
	_Reconfigure( );

	FcSetNotifyWindowHandle( GetSafeHwnd( ) );
	_SetUpTrayIcon( );
}

void CMainFrame::_Reconfigure( void ) {
	FcSetComposeSequences( m_pOptions->ComposeSequences.GetData( ), static_cast<DWORD>( m_pOptions->ComposeSequences.GetCount( ) ) );

	FcSetCapsLockToggleMode( m_pOptions->CapsLockToggleMode );

	FcSetCapsLockSwapMode( m_pOptions->CapsLockSwapMode );
	FcSetSwapCapsLockKey( m_pOptions->SwapCapsLockVk );

	FcSetComposeKey( m_pOptions->ComposeVk );

	if ( m_pOptions->StartActive ) {
		FcEnableHook( );
	} else {
		FcDisableHook( );
	}
}

void CMainFrame::_SetUpTrayIcon( void ) {
	m_pTrayIcon = new CTrayNotifyIcon( );
	if ( !m_pTrayIcon->Create(
		this,
		IDM_TRAY_MENU,
		FcIsHookEnabled( ) ? m_strEnabled : m_strDisabled,
		AfxGetApp( )->LoadIcon( IDR_MAINFRAME ),
		APP_NOTIFYICON,
		IDM_TRAY_MENU,
		TRUE
	) ) {
		debug( L"CMainFrame::_SetUpTrayIcon: CTrayNotifyIcon::Create failed\n" );
	}

#ifdef _DEBUG
	CMenu& trayMenu = m_pTrayIcon->GetMenu( );
	trayMenu.InsertMenu( ID_APP_CAPSLOCK, MF_BYCOMMAND|MF_STRING, ID_APP_ZAPCONF, L"&Zap configuration file" );
#endif
}

void CMainFrame::_UpdateTooltip( void ) {
	CString& str = FcIsHookEnabled( ) ? m_strEnabled : m_strDisabled;
	m_pTrayIcon->SetTooltipText( str );
	m_pTrayIcon->SetBalloonDetails( str, m_strTitle, CTrayNotifyIcon::Info, 10 );
}

//
// CMainFrame event handlers
//

int CMainFrame::OnCreate( LPCREATESTRUCT lpCreateStruct ) {
	if ( -1 == CFrameWnd::OnCreate( lpCreateStruct ) ) {
		return -1;
	}

	_Initialize( );

	return 0;
}

void CMainFrame::OnClose( ) {
	if ( FcIsHookEnabled( ) ) {
		FcDisableHook( );
	}
	FcSetNotifyWindowHandle( nullptr );
	
	delete m_pTrayIcon;

	CFrameWnd::OnClose( );
}

LRESULT CMainFrame::OnActivate( WPARAM, LPARAM ) {
	_UpdateTooltip( );

	return 0;
}

LRESULT CMainFrame::OnNotifyIcon(WPARAM wParam, LPARAM lParam) {
	return m_pTrayIcon->OnTrayNotification( wParam, lParam );
}

LRESULT CMainFrame::OnReconfigure( WPARAM, LPARAM lparamOptionsPropSheet ) {
	COptionsPropSheet* poptions = reinterpret_cast< COptionsPropSheet* >( lparamOptionsPropSheet );

	*m_pOptions = poptions->GetNewOptions( );
	m_pOptions->Save( );
	_Reconfigure( );

	return 0;
}

LRESULT CMainFrame::OnFcmPip(WPARAM wPip, LPARAM /*lParam*/) {
	debug( L"CMainFrame::OnFcmPip: wPip=%llu\n", wPip );

	switch ( wPip ) {
		case PIP_OK_1:
			Beep( 523, 100 );
			break;

		case PIP_OK_2:
			Beep( 523, 100 );
			break;

		case PIP_OK_3:
			Beep( 1047, 100 );
			break;

		case PIP_ABORT:
			MessageBeep( MB_ICONASTERISK );
			break;

		case PIP_ERROR:
			MessageBeep( MB_ICONHAND );
			break;

		case PIP_FAIL:
			MessageBeep( MB_ICONHAND );
			break;

		default:
			break;
	}

	return 0;
}

void CMainFrame::OnAppAbout(void) {
	CAboutDlg aboutDlg;
	aboutDlg.DoModal( );
}

void CMainFrame::OnAppExit( ) {
	bool fPropSheetOpen;
	LOCK( m_csPropSheet ) {
		fPropSheetOpen = ( nullptr != m_pPropSheet );
	} UNLOCK( m_csPropSheet );
	if ( fPropSheetOpen ) {
		MessageBox( LoadFromStringTable( IDS_MAINFRAME_CLOSEOPTIONSFIRST ), LoadFromStringTable( AFX_IDS_APP_TITLE ), MB_ICONWARNING|MB_OK );
	} else {
		OnClose( );
	}
}

void CMainFrame::OnAppCapsLock( void ) {
	INPUT inputs[2];
	memset( inputs, 0, 2 * sizeof( INPUT ) );

	inputs[0].type = INPUT_KEYBOARD;
	inputs[0].ki.wVk = VK_CAPITAL;
	inputs[1].ki.dwFlags = 0;

	inputs[1].type = INPUT_KEYBOARD;
	inputs[1].ki.wVk = VK_CAPITAL;
	inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

	UINT u = SendInput( 2, inputs, sizeof( INPUT ) );
	if ( u < 2 ) {
		debug( L"CMainFrame::OnAppCapsLock: SendInput failed: sent=%u err=%lu\n", u, GetLastError( ) );
	}
}

void CMainFrame::OnAppConfigure( ) {
	bool reused = false;

	LOCK( m_csPropSheet ) {
		if ( nullptr != m_pPropSheet ) {
			debug( L"CMainFrame::OnAppConfigure: reusing existing prop sheet\n" );
			m_pPropSheet->ShowWindow( SW_SHOW );
			reused = true;
		}
	} UNLOCK( m_csPropSheet );
	if ( reused )
		return;

	COptionsPropSheet options( *m_pOptions, this );
	LOCK( m_csPropSheet ) { m_pPropSheet = &options; } UNLOCK( m_csPropSheet );
	INT_PTR rc = options.DoModal( );
	LOCK( m_csPropSheet ) { m_pPropSheet = nullptr;  } UNLOCK( m_csPropSheet );

	if ( IDOK != rc ) {
		return;
	}

	const COptionsData& newoptions = options.GetNewOptions( );
	if ( *m_pOptions != newoptions ) {
		OnReconfigure( 0, reinterpret_cast<LPARAM>( &options ) );
	}
}

void CMainFrame::OnAppToggle( void ) {
	if ( FcIsHookEnabled( ) ) {
		FcDisableHook();
	} else {
		FcEnableHook( );
	}
	_UpdateTooltip( );
}

#ifdef _DEBUG
void CMainFrame::OnAppZapConf( void ) {
	CString appTitle = LoadFromStringTable( AFX_IDS_APP_TITLE );

	debug( L"CMainFrame::OnAppZapConf: Prompting to confirm deletion\n" );
	int nResult = MessageBox( L"Did you really mean to click the 'Zap configuration file' menu item??", appTitle, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2 );
	if ( IDNO == nResult ) {
		debug( L"CMainFrame::OnAppZapConf: Reponse is NO\n" );
		return;
	}
	if ( IDYES != nResult ) {
		debug( L"CMainFrame::OnAppZapConf: got unexpected value %d from MessageBox()\n", nResult );
		MessageBox( L"Something went wrong: configuration not zapped.", appTitle, MB_OK|MB_ICONWARNING );
		return;
	}
	debug( L"CMainFrame::OnAppZapConf: Response was YES\n" );

	CString str( GetFreeComposeFolderAsCString( ) + L"\\FreeCompose.xml" );
	debug( L"CMainFrame::OnAppZapConf: Deleting configuration file '%s'\n", static_cast<wchar_t const*>( str ) );

	SetLastError( 0 );
	if ( !DeleteFile( str ) ) {
		DWORD dwError = GetLastError( );
		debug( L"CMainFrame::OnAppZapConf: Error %lu while trying to delete file.", dwError );

		wchar_t pwzMessage[1024];
		swprintf_s( pwzMessage, 1024, L"Error %lu occurred while trying to zap the configuration.", dwError );
		MessageBox( pwzMessage, appTitle, MB_OK|MB_ICONERROR );
		return;
	}
	
	debug( L"CMainFrame::OnAppZapConf: Configuration file deleted successfully.\n" );
	MessageBox( L"Configuration file zapped.", appTitle, MB_OK|MB_ICONINFORMATION );
}
#endif

void CMainFrame::OnUpdateAppCapsLock( CCmdUI* pui ) {
	pui->SetCheck( IsCapsLockOn( ) ? 1 : 0 );
}

void CMainFrame::OnUpdateAppToggle( CCmdUI* pui ) {
	if ( FcIsHookEnabled( ) ) {
		pui->SetText( LoadFromStringTable( IDS_MAINFRAME_MENU_DISABLE ) );
	} else {
		pui->SetText( LoadFromStringTable( IDS_MAINFRAME_MENU_ENABLE ) );
	}
}

BOOL CMainFrame::PreTranslateMessage( MSG* pMsg ) {
	if ( IsDialogMessage( pMsg ) ) {
		return TRUE;
	}
	
	return CFrameWnd::PreTranslateMessage( pMsg );
}
