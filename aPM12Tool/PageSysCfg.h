#pragma once
#include "afxwin.h"


// CPageSysCfg �Ի���

class CPageSysCfg : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageSysCfg)

public:
	CPageSysCfg();
	virtual ~CPageSysCfg();

// �Ի�������
	enum { IDD = IDD_DLG_CFG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
