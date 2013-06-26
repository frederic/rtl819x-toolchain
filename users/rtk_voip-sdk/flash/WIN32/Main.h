#if !defined(AFX_MAIN_H__CA0C87F9_EDC1_401B_B040_B8AD4074801A__INCLUDED_)
#define AFX_MAIN_H__CA0C87F9_EDC1_401B_B040_B8AD4074801A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Main.h : header file
//

#include "Page1.h"
#include "Page2.h"

/////////////////////////////////////////////////////////////////////////////
// CMain

class CMain : public CPropertySheet
{
	DECLARE_DYNAMIC(CMain)

// Construction
public:
	CMain(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CMain(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
	CPage1 m_page1;
	CPage2 m_page2;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMain)
	public:
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMain();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMain)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAIN_H__CA0C87F9_EDC1_401B_B040_B8AD4074801A__INCLUDED_)
