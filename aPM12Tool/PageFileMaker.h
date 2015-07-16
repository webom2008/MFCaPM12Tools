#pragma once
#include "afxwin.h"


// CPageFileMaker 对话框

class CPageFileMaker : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageFileMaker)
public:
	CPageFileMaker();
	virtual ~CPageFileMaker();

// 对话框数据
	enum { IDD = IDD_DLG_FILES };
    
public:
    void        initApplication(void);
private:
    CString         m_strFilePath1;
    CString         m_strFilePath2;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnFile1();
    afx_msg void OnBnClickedBtnFile2();
    CEdit m_EditFilePath1;
    CEdit m_EditFilePath2;
    CComboBox m_CBoxFile2Offset;
    CComboBox m_CBoxFile1Offset;
    virtual BOOL OnInitDialog();
    CEdit m_EditMergeName;
    afx_msg void OnBnClickedBtnFilesMake();
};
