#pragma once
#include "afxwin.h"

typedef enum 
{
    VERIFY_ENTER_GET,
    VERIFY_ENTER_SEND,
    VERIFY_EXIT_GET,
    VERIFY_EXIT_SEND,
} NIBP_VerifyState_Typedef;

// CPageFactory 对话框

class CPageFactory : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageFactory)

public:
	CPageFactory();
	virtual ~CPageFactory();
  
    static int WINAPI   PktHandleNIBPVerifyStatus(LPVOID pParam, UartProtocolPacket *pPacket);
    static int WINAPI   PktHandleNIBPVerifyEnterExit(LPVOID pParam, UartProtocolPacket *pPacket);
    static int WINAPI   PktHandleNIBPVerifyDSP(LPVOID pParam, UartProtocolPacket *pPacket);
    static int WINAPI   PktHandleNIBPVerifySTM(LPVOID pParam, UartProtocolPacket *pPacket);

    void        initApplication(void);
// 对话框数据
	enum { IDD = IDD_DLG_FACTORY };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
    
    NIBP_VerifyState_Typedef enum_VerifyEnterExitState;
public:
    virtual BOOL OnInitDialog();
    CComboBox m_verify_dsp_sel;
    CComboBox m_verify_stm_sel;
    afx_msg void OnBnClickedBtnNibpVerifyStatus();
    afx_msg void OnBnClickedBtnNibpVerify();
    afx_msg void OnBnClickedBtnNibpDspVerify();
    afx_msg void OnBnClickedBtnNibpStmVerify();
    afx_msg void OnBnClickedBtnAioEeprom();
    CComboBox m_cmb_aio_eeprom_func;
};
