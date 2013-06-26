// Main.cpp : implementation file
//

#include "stdafx.h"
#include "CVVoIP.h"
#include "Main.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMain

IMPLEMENT_DYNAMIC(CMain, CPropertySheet)

CMain::CMain(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_page1);
	AddPage(&m_page2);
}

CMain::CMain(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
	AddPage(&m_page1);
	AddPage(&m_page2);
}

CMain::~CMain()
{
}


BEGIN_MESSAGE_MAP(CMain, CPropertySheet)
	//{{AFX_MSG_MAP(CMain)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMain message handlers

BOOL CMain::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	CRect rect;
	
	// sets application icons
	SetIcon(AfxGetApp()->LoadIcon( IDR_MAINFRAME ), FALSE);
	SetIcon(AfxGetApp()->LoadIcon( IDR_MAINFRAME ), TRUE);

	// add the minimize button to the window
	::SetWindowLong(m_hWnd, GWL_STYLE, GetStyle() | WS_MINIMIZEBOX);

	// insert minimize command and separator into system menu
	GetSystemMenu(FALSE)->InsertMenu(1, MF_BYPOSITION | MF_STRING, SC_ICON, 
		"Mi&nimize");
	GetSystemMenu(FALSE)->InsertMenu(2, MF_BYPOSITION | MF_SEPARATOR, SC_ICON);

	// hide some button
	GetDlgItem(IDOK)->SetWindowText("E&xit");
	GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	GetDlgItem(IDHELP)->ShowWindow(SW_HIDE);

	GetDlgItem(IDHELP)->GetWindowRect(&rect); //rect where we want to put button
	ScreenToClient(&rect);
	GetDlgItem(IDOK)->SetWindowPos(NULL, rect.left, rect.top,rect.Width(),
		rect.Height(), SWP_NOZORDER);
	
	return bResult;
}
