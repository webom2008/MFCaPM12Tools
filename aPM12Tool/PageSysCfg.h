#pragma once
#include "afxwin.h"


// CPageSysCfg 对话框

class CPageSysCfg : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageSysCfg)

public:
	CPageSysCfg();
	virtual ~CPageSysCfg();

// 对话框数据
	enum { IDD = IDD_DLG_CFG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    CComboBox   m_SerialNumbSel;
    CComboBox   m_SerialBaudSel;
    CButton     m_SerialOpenCtrl;
    CComboBox   m_BoardSel;
    afx_msg void OnBnClickedBtnSerialOpen();
    afx_msg void OnCbnSelchangeCboBoardSel();
    afx_msg void OnBnClickedCheckIdValid();
    CButton m_btnCheckID;
};
