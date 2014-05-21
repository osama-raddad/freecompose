#pragma once

#include "CompositeCharacter.h"

enum SEQUENCE_EDITOR_MODE {
	semAdd,
	semEdit,
};

class CComposeSequenceEditor: public CDialog {
	DECLARE_DYNAMIC( CComposeSequenceEditor )
	DECLARE_MESSAGE_MAP( )

public:
	CComposeSequenceEditor( ComposeSequence& sequence, SEQUENCE_EDITOR_MODE mode, CWnd* pParent = nullptr );
	virtual ~CComposeSequenceEditor( );

	// Dialog Data
	enum { IDD = IDD_COMPOSE_SEQUENCE_EDITOR };

protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog( );
	virtual void OnOK( );
	virtual void OnOKAddAnother( );

	void OnDrawItem( int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct );
	void OnUpdateComposeSequence( );
	void OnUpdateComposeResult( );
	void OnResultModeClicked( UINT uID );

private:
	ComposeSequence& m_sequence;
	SEQUENCE_EDITOR_MODE m_mode;
	CString m_strResultInput;
	CFont* m_pFont;

	CompositeCharacter m_CompositeCharacter;

	CButton m_buttonAddAnother;
	CEdit   m_editComposeSequence;
	CEdit   m_editComposeResult;
	CStatic m_staticPreview;

	CString m_strComposeSequence;
	CString m_strComposeResult;
	int     m_nResultMode;
	BOOL    m_fEnabled;
	BOOL    m_fCaseInsensitive;
	BOOL    m_fReversible;

	bool _ParseCodePointList( CString const& str, int const base, CArray<UChar32>& output );
	void _SetResultFromInput( void );
	void _SetInputFromResult( void );

	void _DDV_MinMaxCompositeCharacters( CDataExchange* pDX, unsigned uID, CString& strComposeSequence, int const cchMin, int const cchMax );

};
