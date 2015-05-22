// PageWave.cpp : 实现文件
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageWave.h"
#include "afxdialogex.h"

extern CSerialProtocol *g_pSerialProtocol;


inline int s16_to_s32(int Data16)
{
    int data;
    if (Data16 & 0x00008000)//mask bit 15,negative
    {
        Data16 = (((~Data16) & 0x00007FFF) + 1) ;//先转为正数值
        data =  0 - Data16;//转为负值
        return data;
    }
    else    //positive
    {
        return Data16;
    }
}
    
inline int s24_to_s32(int Data24)
{
    int data;
    if (Data24 & 0x00800000)//mask bit 23,negative
    {
        Data24 = (((~Data24) & 0x007FFFFF) + 1) ;//先转为正数值
        data =  0 - (int)Data24;//转为负值
        return data;
    }
    else    //positive
    {
        return Data24;
    }
}

// CPageWave 对话框

IMPLEMENT_DYNAMIC(CPageWave, CPropertyPage)

CPageWave::CPageWave()
	: CPropertyPage(CPageWave::IDD)
{
    m_bIsEcgFilData         = true;
    m_bIsEcgSaveFile        = false;
    m_pEcgSaveFile          = NULL;
    m_EcgWaveThread         = NULL;
    m_hKillEcgThreadEvent   = NULL;
    m_hGetEcgEvent          = NULL;
    m_pEcgPushGraph         = NULL;

    m_hKillRespThreadEvent  = NULL;
    m_hGetRespEvent         = NULL;
    m_RespWaveThread        = NULL;
    m_pRespPushGraph        = NULL;
    
    m_Spo2WaveThread        = NULL;
    m_hGetSpo2Event         = NULL;
    m_hKillSpo2ThreadEvent  = NULL;
    m_Spo2PushGraph         = NULL;
}

CPageWave::~CPageWave()
{

    if (NULL != m_pEcgPushGraph) delete m_pEcgPushGraph;
    if (NULL != m_hKillEcgThreadEvent) CloseHandle( m_hKillEcgThreadEvent );
    if (NULL != m_hGetEcgEvent) CloseHandle( m_hGetEcgEvent );
    //g_pSerialProtocol->releasePaktFuncByID(AIO_TX_ECG_REALTIME_ID);

    if (NULL != m_pRespPushGraph) delete m_pRespPushGraph;
    if (NULL != m_hKillRespThreadEvent) CloseHandle( m_hKillRespThreadEvent );
    if (NULL != m_hGetRespEvent) CloseHandle( m_hGetRespEvent );
    //g_pSerialProtocol->releasePaktFuncByID(AIO_TX_RESP_REALTIME_ID);

    if (NULL != m_Spo2PushGraph) delete m_Spo2PushGraph;
    if (NULL != m_hKillSpo2ThreadEvent) CloseHandle( m_hKillSpo2ThreadEvent );
    if (NULL != m_hGetSpo2Event) CloseHandle( m_hGetSpo2Event );
    //g_pSerialProtocol->releasePaktFuncByID(SPO2_NORMALIZED_ID);
}

void CPageWave::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CHECK_ECG_DATA, m_CheckEcgData);
    DDX_Control(pDX, IDC_CHECK_ECG_SAVE, m_CheckEcgSave);
}


BEGIN_MESSAGE_MAP(CPageWave, CPropertyPage)
    ON_BN_CLICKED(IDC_BTN_SPO2_MONITOR, &CPageWave::OnBnClickedBtnSpo2Monitor)
    ON_BN_CLICKED(IDC_BTN_RESP_MONITOR, &CPageWave::OnBnClickedBtnRespMonitor)
    ON_BN_CLICKED(IDC_BTN_ECG_MONITOR, &CPageWave::OnBnClickedBtnEcgMonitor)
    ON_BN_CLICKED(IDC_CHECK_ECG_DATA, &CPageWave::OnBnClickedCheckEcgData)
    ON_BN_CLICKED(IDC_CHECK_ECG_SAVE, &CPageWave::OnBnClickedCheckEcgSave)
END_MESSAGE_MAP()


// CPageWave 消息处理程序


BOOL CPageWave::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    
    // create events
    if (NULL != m_hKillEcgThreadEvent) ResetEvent(m_hKillEcgThreadEvent);
    else m_hKillEcgThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL != m_hGetEcgEvent) ResetEvent(m_hGetEcgEvent);
    else m_hGetEcgEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hEcgEventArray[0] = m_hKillEcgThreadEvent;
    m_hEcgEventArray[1] = m_hGetEcgEvent;
    initEcgDrawPicture();
    
    if (NULL != m_hKillRespThreadEvent) ResetEvent(m_hKillRespThreadEvent);
    else m_hKillRespThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL != m_hGetRespEvent) ResetEvent(m_hGetRespEvent);
    else m_hGetRespEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hRespEventArray[0] = m_hKillRespThreadEvent;
    m_hRespEventArray[1] = m_hGetRespEvent;
    initRespDrawPicture();

    if (NULL != m_hKillSpo2ThreadEvent) ResetEvent(m_hKillSpo2ThreadEvent);
    else m_hKillSpo2ThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL != m_hGetSpo2Event) ResetEvent(m_hGetSpo2Event);
    else m_hGetSpo2Event = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hSpo2EventArray[0] = m_hKillSpo2ThreadEvent;
    m_hSpo2EventArray[1] = m_hGetSpo2Event;
    initSpo2DrawPicture();

    return TRUE;
}

void CPageWave::initApplication(void)
{
    g_pSerialProtocol->bindPaktFuncByID(SPO2_NORMALIZED_ID ,this, CPageWave::PktHandleSpo2Wave);
    g_pSerialProtocol->bindPaktFuncByID(AIO_TX_ECG_REALTIME_ID ,this, CPageWave::PktHandleEcgWave);
    g_pSerialProtocol->bindPaktFuncByID(AIO_TX_RESP_REALTIME_ID ,this, CPageWave::PktHandleRespWave);

}

void  CPageWave::initSpo2DrawPicture(void)
{
    m_Spo2PushGraph = new C2DPushGraph;

	// TODO: 在此添加额外的初始化代码
    if (NULL != m_Spo2PushGraph)
    {
        m_Spo2PushGraph->CreateFromStatic(IDC_SPO2_WAVE, this);      //这个IDC_REALCTRL即是那个Picture Control控件的ID号。
        m_Spo2PushGraph->ModifyStyle(0, WS_THICKFRAME);              //设置风格
        m_Spo2PushGraph->AddLine(0,  RGB(255,255,255));
        m_Spo2PushGraph->SetLabelForMax( "100" );
        m_Spo2PushGraph->SetLabelForMin( "0" );
        m_Spo2PushGraph->ShowGrid( TRUE );
        m_Spo2PushGraph->ShowLabels( TRUE  );
        m_Spo2PushGraph->ShowAsBar(0, false );
        m_Spo2PushGraph->SetInterval( 1 );
        m_Spo2PushGraph->SetGridSize( (USHORT)15 );
        m_Spo2PushGraph->SetPeekRange( 0, 100 );
    }
    else
    {
        TRACE("m_Spo2PushGraph = new C2DPushGraph ERROR!\r\n");
    }
}

void CPageWave::initEcgDrawPicture(void)
{
    m_pEcgPushGraph = new C2DPushGraph;

	// TODO: 在此添加额外的初始化代码
    if (NULL != m_pEcgPushGraph)
    {
        m_pEcgPushGraph->CreateFromStatic(IDC_ECG_WAVE, this);      //这个IDC_REALCTRL即是那个Picture Control控件的ID号。
        m_pEcgPushGraph->ModifyStyle(0, WS_THICKFRAME);              //设置风格
        m_pEcgPushGraph->AddLine(0,  RGB(255,0,0));
        //m_pEcgPushGraph->SetLabelForMax( "2000" );
        //m_pEcgPushGraph->SetLabelForMin( "0" );
        m_pEcgPushGraph->ShowGrid( TRUE );
        m_pEcgPushGraph->ShowLabels( TRUE  );
        m_pEcgPushGraph->ShowAsBar(0, false );
        m_pEcgPushGraph->SetInterval( 1 );
        m_pEcgPushGraph->SetGridSize( (USHORT)15 );
        m_pEcgPushGraph->SetPeekRange( ECG_WAVE_VALUE_MIN, ECG_WAVE_VALUE_MAX );
    }
    else
    {
        TRACE("m_pEcgPushGraph = new C2DPushGraph ERROR!\r\n");
    }
}

void CPageWave::initRespDrawPicture(void)
{
    m_pRespPushGraph = new C2DPushGraph;

	// TODO: 在此添加额外的初始化代码
    if (NULL != m_pRespPushGraph)
    {
        m_pRespPushGraph->CreateFromStatic(IDC_RESP_WAVE, this);      //这个IDC_REALCTRL即是那个Picture Control控件的ID号。
        m_pRespPushGraph->ModifyStyle(0, WS_THICKFRAME);              //设置风格
        m_pRespPushGraph->AddLine(0,  RGB(0,255,255));
        //m_pRespPushGraph->SetLabelForMax( "10000" );
        //m_pRespPushGraph->SetLabelForMin( "0" );
        m_pRespPushGraph->ShowGrid( TRUE );
        m_pRespPushGraph->ShowLabels( TRUE  );
        m_pRespPushGraph->ShowAsBar(0, false );
        m_pRespPushGraph->SetInterval( 1 );
        m_pRespPushGraph->SetGridSize( (USHORT)15 );
        m_pRespPushGraph->SetPeekRange( RESP_WAVE_VALUE_MIN, RESP_WAVE_VALUE_MAX );
    }
    else
    {
        TRACE("m_pRespPushGraph = new C2DPushGraph ERROR!\r\n");
    }
}

void CPageWave::DrawEcgWave(int value)
{
    if (NULL == m_pEcgPushGraph) return;
    m_pEcgPushGraph->Push(value, 0);
    m_pEcgPushGraph->Update();
    
}

void CPageWave::DrawRespWave(int value)
{
    if (NULL == m_pRespPushGraph) return;
    m_pRespPushGraph->Push(value, 0);
    TRACE("RESP=%d\r\n",value);
    m_pRespPushGraph->Update();
}

void CPageWave::DrawSpo2Wave(int value)
{
    if (NULL == m_Spo2PushGraph) return;
    m_Spo2PushGraph->Push(value, 0);
    m_Spo2PushGraph->Update();
}

int CPageWave::PktHandleSpo2Wave(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CPageWave *pDlgDraw = (CPageWave*)pParam; 
    pDlgDraw->m_Spo2ValueCur = pPacket->DataAndCRC[0];
    SetEvent(pDlgDraw->m_hGetSpo2Event);
    return 0;
}

int CPageWave::PktHandleEcgWave(LPVOID pParam, UartProtocolPacket *pPacket)
{
    int wDataLent;
    char buffer[20]={0,};
    int data = 0;
    CPageWave *pDlgDraw = (CPageWave*)pParam;
    if (pDlgDraw->m_bIsEcgFilData)
    {
        data = (pPacket->DataAndCRC[4] << 8) | pPacket->DataAndCRC[5]; //II
        data = s16_to_s32(data);
        pDlgDraw->m_EcgValueCur = data + ECG_WAVE_VALUE_OFFSET;
        TRACE("EcgFilData=%d\r\n",pDlgDraw->m_EcgValueCur);
    }
    else
    {
        data = (pPacket->DataAndCRC[5] << 16) | (pPacket->DataAndCRC[6] << 8) | pPacket->DataAndCRC[7];
        data = s24_to_s32(data);
        pDlgDraw->m_EcgValueCur = data;
        TRACE("EcgSrcData=%d\r\n",pDlgDraw->m_EcgValueCur);
    }

    if ((true == pDlgDraw->m_bIsEcgSaveFile) && (NULL != pDlgDraw->m_pEcgSaveFile))
    {
        sprintf(buffer,"%d\r\n", data);
        wDataLent = fwrite(buffer, 1, strlen(buffer), pDlgDraw->m_pEcgSaveFile);
        if (strlen(buffer) != wDataLent)
        {
            TRACE("CPageWave::PktHandleEcgWave fwrite error!\r\n");
        }
    }
    SetEvent(pDlgDraw->m_hGetEcgEvent);
    return 0;
}

int CPageWave::PktHandleRespWave(LPVOID pParam, UartProtocolPacket *pPacket)
{
    int data = 0;
    CPageWave *pDlgDraw = (CPageWave*)pParam; 
    data = (pPacket->DataAndCRC[1] << 16) | (pPacket->DataAndCRC[2] << 8) | pPacket->DataAndCRC[3];
    pDlgDraw->m_RespValueCur = s24_to_s32(data) + RESP_WAVE_VALUE_OFFSET;
    SetEvent(pDlgDraw->m_hGetRespEvent);
    return 0;
}

UINT CPageWave::EcgWaveThread(LPVOID pParam)
{
    CPageWave *pDlgDraw = (CPageWave*)pParam;     
    DWORD Event;
    
    ResetEvent(pDlgDraw->m_hGetEcgEvent);
    ResetEvent(pDlgDraw->m_hKillEcgThreadEvent);

    TRACE("EcgWaveThread started\n");  
    while(1)
    {  
        Event = WaitForMultipleObjects(2, pDlgDraw->m_hEcgEventArray, FALSE, INFINITE); 
        switch (Event)   
        {   
        case 0:   
            { 
                // Kill this thread.  break is not needed, but makes me feel better. 
                pDlgDraw->m_EcgWaveThread = NULL;
                TRACE("EcgWaveThread stoped\n");   
                AfxEndThread(100);
                break;   
            }   
        case 1:
            {   
                ResetEvent(pDlgDraw->m_hGetEcgEvent);
                pDlgDraw->DrawEcgWave(pDlgDraw->m_EcgValueCur);
                break;   
            } 
        default:
            break;
        } // end switch  
    } // end of while(1)

    return 0;
}

UINT CPageWave::RespWaveThread(LPVOID pParam)
{
    CPageWave *pDlgDraw = (CPageWave*)pParam;     
    DWORD Event;
    
    ResetEvent(pDlgDraw->m_hGetRespEvent);
    ResetEvent(pDlgDraw->m_hKillRespThreadEvent);

    TRACE("RespWaveThread started\n");  
    while(1)
    {  
        Event = WaitForMultipleObjects(2, pDlgDraw->m_hRespEventArray, FALSE, INFINITE); 
        switch (Event)   
        {   
        case 0:   
            { 
                // Kill this thread.  break is not needed, but makes me feel better. 
                pDlgDraw->m_RespWaveThread = NULL;
                TRACE("RespWaveThread stoped\n");   
                AfxEndThread(100);
                break;   
            }   
        case 1:
            {   
                ResetEvent(pDlgDraw->m_hGetRespEvent);
                pDlgDraw->DrawRespWave(pDlgDraw->m_RespValueCur);
                break;   
            } 
        default:
            break;
        } // end switch  
    } // end of while(1)

    return 0;
}

UINT CPageWave::Spo2WaveThread(LPVOID pParam)
{
    CPageWave *pDlgDraw = (CPageWave*)pParam;     
    DWORD Event;
    
    ResetEvent(pDlgDraw->m_hGetSpo2Event);
    ResetEvent(pDlgDraw->m_hKillSpo2ThreadEvent);

    TRACE("Spo2WaveThread started\n");  
    while(1)
    {  
        Event = WaitForMultipleObjects(2, pDlgDraw->m_hSpo2EventArray, FALSE, INFINITE); 
        switch (Event)   
        {   
        case 0:   
            { 
                // Kill this thread.  break is not needed, but makes me feel better. 
                pDlgDraw->m_Spo2WaveThread = NULL;
                TRACE("Spo2WaveThread stoped\n");   
                AfxEndThread(100);
                break;   
            }   
        case 1:
            {   
                ResetEvent(pDlgDraw->m_hGetSpo2Event);
                pDlgDraw->DrawSpo2Wave(pDlgDraw->m_Spo2ValueCur);
                break;   
            } 
        default:
            break;
        } // end switch  
    } // end of while(1)

    return 0;
}

void CPageWave::OnBnClickedBtnSpo2Monitor()
{
    // TODO: 在此添加控件通知处理程序代码
    if (NULL == m_Spo2WaveThread)
    {
        m_Spo2WaveThread = AfxBeginThread(Spo2WaveThread, this);
        if (NULL == m_Spo2WaveThread)
        {
            ERROR_INFO("创建线程ERROR");
        }
        else
        {
            GetDlgItem(IDC_BTN_SPO2_MONITOR)->SetWindowTextA("停止");
        }
    }
    else
    {
        SetEvent(m_hKillSpo2ThreadEvent);
        GetDlgItem(IDC_BTN_SPO2_MONITOR)->SetWindowTextA("开始");
    }
}


void CPageWave::OnBnClickedBtnRespMonitor()
{
    // TODO: 在此添加控件通知处理程序代码
    if (NULL == m_RespWaveThread)
    {
        m_RespWaveThread = AfxBeginThread(RespWaveThread, this);
        if (NULL == m_RespWaveThread)
        {
            ERROR_INFO("创建线程ERROR");
        }
        else
        {
            GetDlgItem(IDC_BTN_RESP_MONITOR)->SetWindowTextA("停止");
        }
    }
    else
    {
        SetEvent(m_hKillRespThreadEvent);
        GetDlgItem(IDC_BTN_RESP_MONITOR)->SetWindowTextA("开始");
    }
}


void CPageWave::OnBnClickedBtnEcgMonitor()
{
    // TODO: 在此添加控件通知处理程序代码
    if (NULL == m_EcgWaveThread)
    {
        m_EcgWaveThread = AfxBeginThread(EcgWaveThread, this);
        if (NULL == m_EcgWaveThread)
        {
            ERROR_INFO("创建线程ERROR");
        }
        else
        {
            GetDlgItem(IDC_BTN_ECG_MONITOR)->SetWindowTextA("停止");
        }
    }
    else
    {
        SetEvent(m_hKillEcgThreadEvent);
        GetDlgItem(IDC_BTN_ECG_MONITOR)->SetWindowTextA("开始");
    }
}

void CPageWave::OnBnClickedCheckEcgData()
{
    // TODO: 在此添加控件通知处理程序代码
    if (m_CheckEcgData.GetCheck())
    {
        m_bIsEcgFilData = false;
        m_pEcgPushGraph->SetPeekRange( 10000, 40000 );
    }
    else
    {
        m_bIsEcgFilData = true;
        m_pEcgPushGraph->SetPeekRange( ECG_WAVE_VALUE_MIN, 2*ECG_WAVE_VALUE_MAX );
    }
}


void CPageWave::OnBnClickedCheckEcgSave()
{
    // TODO: 在此添加控件通知处理程序代码
    COleDateTime time = COleDateTime::GetCurrentTime();
    CString s1 = time.Format("%Y%m%d%H%M%S");
    CString info = "创建时间:";
    CString strFileName;
    if (m_CheckEcgSave.GetCheck())
    {
        if (NULL != m_pEcgSaveFile) fclose(m_pEcgSaveFile);
        strFileName = "ECG_"+s1+".txt";
        TRACE("%s\r\n",strFileName);
        m_pEcgSaveFile = fopen(strFileName,"a");
        if (NULL != m_pEcgSaveFile)
        {
            //fseek( m_pEcgSaveFile, 0, SEEK_END); //每次都移动到文件结尾在写入
            fwrite(info,1, info.GetLength(), m_pEcgSaveFile);
            s1 += "\r\n";
            fwrite(s1,1, s1.GetLength(), m_pEcgSaveFile);
            m_bIsEcgSaveFile = true;
        }
        else
        {
            ERROR_INFO("文件%s创建失败\r\n",strFileName);
        }
    }
    else
    {
        if (NULL != m_pEcgSaveFile) fclose(m_pEcgSaveFile);
        m_pEcgSaveFile = NULL;
        m_bIsEcgSaveFile = false;
    }
}
