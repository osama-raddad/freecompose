#pragma once

#include "OptionsData.h"

enum SORTSTATE { ssUnsorted, ssAscending, ssDescending, ssMaximumValue };

class CKeySequences: public CPropertyPage {
	DECLARE_DYNAMIC( CKeySequences )
	DECLARE_MESSAGE_MAP( )

public:
	enum { IDD = IDD_KEYSEQUENCES };

	CKeySequences( COptionsData& Options );
	virtual ~CKeySequences( );

	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog( );

	afx_msg void OnListColumnClick( NMHDR* pnmhdr, LRESULT* pResult );
	afx_msg void OnListDoubleClick( NMHDR* pnmhdr, LRESULT* pResult );
	afx_msg void OnListItemChanged( NMHDR* pnmhdr, LRESULT* pResult );

	afx_msg void OnBnClickedAdd( );
	afx_msg void OnBnClickedEdit( );
	afx_msg void OnBnClickedRemove( );

private:
	static int const ResultCodePointColumn = 0;
	static int const ResultCharacterColumn = 1;
	static int const SequenceColumn = 2;
	static int const NumberOfColumns = 3;

	using sortcallbackfunc = int CALLBACK ( LPARAM, LPARAM, LPARAM );
	static sortcallbackfunc* ResultColumnsSortFuncMap[3];  // 3 is the number of SORTSTATEs, NOT the number of columns!
	static sortcallbackfunc* SequenceColumnSortFuncMap[3]; // 3 is the number of SORTSTATEs, NOT the number of columns!
	static sortcallbackfunc** ColumnSortFuncMap[NumberOfColumns];

private:
	COptionsData& m_Options;

	int m_nColumnWidths[NumberOfColumns];

	CListCtrl m_List;
	CHeaderCtrl* m_pListHeader;
	CButton m_btnAdd;
	CButton m_btnEdit;
	CButton m_btnRemove;

	SORTSTATE m_SortState;
	int m_nSortColumn;
	int* m_pnSortIndices;

private:
	CString _FormatResultString( ComposeSequence const& sequence );
	void _AddOneKeySequence( const INT_PTR n );
	void _UpdateOneKeySequence( const INT_PTR n );
	void _AdjustColumns( void );
	void _FillKeyComboList( void );
	void _AddNewKeySequence( const INT_PTR n );
	void _SetColumnSortState( int nColumn, SORTSTATE state );

private:
	static int CALLBACK _ListComparer_Unsorted( LPARAM index1, LPARAM index2, LPARAM );
	static int CALLBACK _ListComparer_Ascending_Result( LPARAM index1, LPARAM index2, LPARAM );
	static int CALLBACK _ListComparer_Descending_Result( LPARAM index1, LPARAM index2, LPARAM );
	static int CALLBACK _ListComparer_Ascending_Sequence( LPARAM index1, LPARAM index2, LPARAM );
	static int CALLBACK _ListComparer_Descending_Sequence( LPARAM index1, LPARAM index2, LPARAM );

};
