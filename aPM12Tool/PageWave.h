#pragma once

#include "2DPushGraph.h"
#include "afxwin.h"

const int ECG_WAVE_VALUE_MAX        = 2000;
const int ECG_WAVE_VALUE_MIN        = 0;
const int ECG_WAVE_VALUE_OFFSET     = (ECG_WAVE_VALUE_MAX - ECG_WAVE_VALUE_MIN)/3;

const int RESP_WAVE_VALUE_MAX        = 300000;
const int RESP_WAVE_VALUE_MIN        = 0;
const int RESP_WAVE_VALUE_OFFSET     = (RESP_WAVE_VALUE_MAX - RESP_WAVE_VALUE_MIN)/2;

const int SPO2_WAVE_VALUE_MAX        = 100;
const int SPO2_WAVE_VALUE_MIN        = 0;

// CPageWave 对话框

class CPageWave : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageWave)

public:
	CPageWave();
	virtual ~CPageWave();

    void        initApplication(void);
    
    void                DrawEcgWave(int value);
    static int WINAPI   PktHandleEcgWave(LPVOID pParam, UartProtocolPacket *pPacket);

    void                DrawRespWave(int value);
    static int WINAPI   PktHandleRespWave(LPVOID pParam, UartProtocolPacket *pPacket);

    void                DrawSpo2Wave(int value);
    static int WINAPI   PktHandleSpo2Wave(LPVOID pParam, UartProtocolPacket *pPacket);
    
// 对话框数据
	enum { IDD = IDD_DLG_WAVE };
private:
    bool                m_bIsEcgFilData;
    bool                m_bIsEcgSaveFile;
    FILE*               m_pEcgSaveFile;
    int                 m_EcgValueCur;
    C2DPushGraph*       m_pEcgPushGraph;
    HANDLE              m_hKillEcgThreadEvent;
    HANDLE              m_hGetEcgEvent;
    HANDLE				m_hEcgEventArray[2]; 
	CWinThread*	        m_EcgWaveThread;
    void                initEcgDrawPicture(void);
    static UINT	        EcgWaveThread(LPVOID pParam);

    int                 m_RespValueCur;
    C2DPushGraph*       m_pRespPushGraph;
    HANDLE              m_hKillRespThreadEvent;
    HANDLE              m_hGetRespEvent;
    HANDLE				m_hRespEventArray[2]; 
	CWinThread*	        m_RespWaveThread;
    void                initRespDrawPicture(void);
    static UINT	        RespWaveThread(LPVOID pParam);

    int                 m_Spo2ValueCur;
    C2DPushGraph*       m_Spo2PushGraph;
    HANDLE              m_hKillSpo2ThreadEvent;
    HANDLE              m_hGetSpo2Event;
    HANDLE				m_hSpo2EventArray[2]; 
	CWinThread*	        m_Spo2WaveThread;
    void                initSpo2DrawPicture(void);
    static UINT	        Spo2WaveThread(LPVOID pParam);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtnSpo2Monitor();
    afx_msg void OnBnClickedBtnRespMonitor();
    afx_msg void OnBnClickedBtnEcgMonitor();
    afx_msg void OnBnClickedCheckEcgData();
    CButton m_CheckEcgData;
    afx_msg void OnBnClickedCheckEcgSave();
    CButton m_CheckEcgSave;
};
