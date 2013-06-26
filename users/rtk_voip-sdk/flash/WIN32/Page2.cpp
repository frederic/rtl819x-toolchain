// Page2.cpp : implementation file
//

#include "stdafx.h"
#undef interface	// mib use 'interface' as a name 
#include "cvvoip.h"
#include "Page2.h"
#include "voip_flash.h"
#include "voip_flash_mib.h"
#include "voip_flash_tool.h"
#include "cvcfg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPage2 property page

IMPLEMENT_DYNCREATE(CPage2, CPropertyPage)

CPage2::CPage2() : CPropertyPage(CPage2::IDD)
{
	//{{AFX_DATA_INIT(CPage2)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPage2::~CPage2()
{
}

void CPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPage2)
	DDX_Control(pDX, IDC_EDIT3, m_edtKey);
	DDX_Control(pDX, IDC_EDIT2, m_edtOutFile);
	DDX_Control(pDX, IDC_EDIT1, m_edtInFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CPage2)
	ON_BN_CLICKED(IDC_BUTTON1, OnInFIle)
	ON_BN_CLICKED(IDC_BUTTON2, OnOutFile)
	ON_BN_CLICKED(IDC_BUTTON3, OnFileConvert)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPage2 message handlers

BOOL CPage2::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPage2::OnInFIle() 
{
	char szFilters[]= "*.dat,*.txt|*.dat;*.txt||";
	CFileDialog fileDlg(TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST, szFilters, this);

	fileDlg.m_ofn.lpstrInitialDir = ((CCVVoIPApp *) AfxGetApp())->m_curdir;
	if (fileDlg.DoModal() == IDOK)
	{
		m_edtInFile.SetWindowText(fileDlg.GetPathName());
	}	
}

void CPage2::OnOutFile() 
{
	char szFilters[]= "*.dat,*.txt|*.dat;*.txt||";
	CFileDialog fileDlg(FALSE, NULL, NULL,
		OFN_HIDEREADONLY, szFilters, this);

	fileDlg.m_ofn.lpstrInitialDir = ((CCVVoIPApp *) AfxGetApp())->m_curdir;
	if (fileDlg.DoModal() == IDOK)
	{
		m_edtOutFile.SetWindowText(fileDlg.GetPathName());
	}	
}

void CPage2::OnFileConvert() 
{
	char in_filename[256];
	char out_filename[256];
	int inFileType, outFileType=0, flag=0;
	struct all_config tmpCfg;

	memset(in_filename, 0, sizeof(in_filename));
	memset(out_filename, 0, sizeof(out_filename));
	m_edtInFile.GetWindowText(in_filename, sizeof(in_filename));
	m_edtOutFile.GetWindowText(out_filename, sizeof(out_filename));

	inFileType=checkFileType_win32(in_filename);
	if (inFileType == 0) {
		MessageBox("Invalid input file!");
		return;
	}
	memset(&tmpCfg, '\0', sizeof(struct all_config) );
	switch (inFileType & MODE_MASK) {
		case PC_MODE:
		case TARGET_MODE:
			outFileType = TXT_MODE;
			if ( parseBinConfig_win32(inFileType, in_filename, &tmpCfg) < 0) {
				MessageBox("Parse binary file error!");
				return;
			}
			break;

		case TXT_MODE:
			outFileType = TARGET_MODE;
			flag=1;
			if ( parseTxtConfig_win32(in_filename, &tmpCfg) < 0) {
				MessageBox("Parse text file error!");
				return;
			}
			break;
	}
	
	switch (outFileType & MODE_MASK) {
		case PC_MODE:
		case TARGET_MODE:
			if ( generateBinFile_win32(outFileType, out_filename, flag, &tmpCfg) < 0) {
				MessageBox("Generate binary output file error!");
				return;
			}
			break;

		case TXT_MODE:
			if ( generateTxtFile_win32(out_filename, &tmpCfg) < 0) {
				MessageBox("Generate text output file error!");
				return;
			}

			break;
	}
	MessageBox("Convert to text file OK!");
}
