#if !defined(AFX_PAGE2_H__BE446233_75A8_412C_98C2_61A376670097__INCLUDED_)
#define AFX_PAGE2_H__BE446233_75A8_412C_98C2_61A376670097__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Page2.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPage2 dialog

class CPage2 : public CPropertyPage
{
	DECLARE_DYNCREATE(CPage2)

// Construction
public:
	CPage2();
	~CPage2();

// Dialog Data
	//{{AFX_DATA(CPage2)
	enum { IDD = IDD_DIALOG2 };
	CEdit	m_edtKey;
	CEdit	m_edtOutFile;
	CEdit	m_edtInFile;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPage2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPage2)
	virtual BOOL OnInitDialog();
	afx_msg void OnInFIle();
	afx_msg void OnOutFile();
	afx_msg void OnFileConvert();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGE2_H__BE446233_75A8_412C_98C2_61A376670097__INCLUDED_)
