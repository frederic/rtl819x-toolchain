// CVVoIP.h : main header file for the CVVOIP application
//

#if !defined(AFX_CVVOIP_H__DAEDAD83_9883_4C02_B04D_1014CFA12321__INCLUDED_)
#define AFX_CVVOIP_H__DAEDAD83_9883_4C02_B04D_1014CFA12321__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCVVoIPApp:
// See CVVoIP.cpp for the implementation of this class
//

class CCVVoIPApp : public CWinApp
{
public:
	CCVVoIPApp();
	char m_curdir[MAX_PATH];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCVVoIPApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCVVoIPApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CVVOIP_H__DAEDAD83_9883_4C02_B04D_1014CFA12321__INCLUDED_)
