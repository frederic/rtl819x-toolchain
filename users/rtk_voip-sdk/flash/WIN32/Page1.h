#if !defined(AFX_PAGE1_H__440EF622_91F5_4369_9676_7A3059647468__INCLUDED_)
#define AFX_PAGE1_H__440EF622_91F5_4369_9676_7A3059647468__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Page1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPage1 dialog

class CPage1 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPage1)

// Construction
public:
	CPage1();
	~CPage1();

// Dialog Data
	//{{AFX_DATA(CPage1)
	enum { IDD = IDD_DIALOG1 };
	CEdit	m_edtKey;
	CEdit	m_edtCfgFile;
	CEdit	m_edtMACList;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPage1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPage1)
	virtual BOOL OnInitDialog();
	afx_msg void OnMACList();
	afx_msg void OnConfigFile();
	afx_msg void OnBatchConvert();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGE1_H__440EF622_91F5_4369_9676_7A3059647468__INCLUDED_)
