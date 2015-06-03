#pragma once

#include "2DPushGraph.h"
#include "afxwin.h"
// CPageNIBP 对话框

class CPageNIBP : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageNIBP)

public:
	CPageNIBP();
	virtual ~CPageNIBP();
    
    static int WINAPI   PktHandleNIBPStop(LPVOID pParam, UartProtocolPacket *pPacket);
    static int WINAPI   PktHandleNIBPStart(LPVOID pParam, UartProtocolPacket *pPacket);
    static int WINAPI   PktHandleNIBPRealTime(LPVOID pParam, UartProtocolPacket *pPacket);
    static int WINAPI   PktHandleNIBPmmHg(LPVOID pParam, UartProtocolPacket *pPacket);
    static int WINAPI   PktHandleNIBPResult(LPVOID pParam, UartProtocolPacket *pPacket);
    static int WINAPI   PktHandleNIBPAlarm(LPVOID pParam, UartProtocolPacket *pPacket);
    static int WINAPI   PktHandleNIBPCountdown(LPVOID pParam, UartProtocolPacket *pPacket);

    void        initApplication(void);
// 对话框数据
	enum { IDD = IDD_DLG_NIBP };
private:
    bool                m_bNIBPStartFlag;
    int                 m_NibpValueCur;
    C2DPushGraph*       m_pNibpPushGraph;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    CComboBox m_patient_sel;
    CComboBox m_pre_press_sel;
    CComboBox m_venipuncture_sel;
    CComboBox m_mode_sel;
    afx_msg void OnBnClickedBtnNibpStart();
    afx_msg void OnCbnSelchangeCmbNibpPrePressure();
    afx_msg void OnCbnSelchangeCmbNibpPatient();
    afx_msg void OnBnClickedCheckNibpAdc();
    CButton m_btn_realtime;
};
