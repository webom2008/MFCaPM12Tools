// PageNIBP.cpp : 实现文件
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageNIBP.h"
#include "afxdialogex.h"


extern CSerialProtocol *g_pSerialProtocol;

typedef enum
{ 
    CUFF_GAS_LEAKAGE     = 0,//NIBP袖带充气管漏气
    CUFF_OFF             = 1,
    PUMP_LEAKAGE         = 2,
    CUFF_TYPE_ERROR      = 3,
    AIR_PRESSURE_ERROR   = 4,
    SIGNAL_TOO_WEAK      = 5,
    SIGNAL_SATURATION    = 6,
    OVER_MEASURE_PRESSURE= 7,
    ARM_EXERCISE         = 8,
    OVER_PROTECT_PRESSURE= 9,
    MODULE_FAILED        = 10,
    MEASURE_TIMEOUT      = 11,
    MEASURE_FAILED       = 12,
    RESET_ERROR          = 13,
} NIBP_ALARM_TypeDef;
#define NIBP_ALARM_TypeDef_MAX      (14)
static char NIBP_ALARM_TypeName[NIBP_ALARM_TypeDef_MAX][100] = 
{
    {"NIBP袖带充气管漏气"},
    {"NIBP袖带太松或没接"},
    {"NIBP泵漏气"},
    {"NIBP袖带类型错"},
    {"NIBP空气压力错"},
    {"NIBP信号太弱"},
    {"NIBP信号饱和"},
    {"NIBP压力超范围"},
    {"NIBP手臂运动"},
    {"NIBP过压保护"},
    {"NIBP系统失败"},
    {"NIBP测量超时"},
    {"NIBP测量失败"},
    {"NIBP复位出错"},
};
// CPageNIBP 对话框

IMPLEMENT_DYNAMIC(CPageNIBP, CPropertyPage)

CPageNIBP::CPageNIBP()
	: CPropertyPage(CPageNIBP::IDD)
{
    m_NibpValueCur = 0;
    m_bNIBPStartFlag = false;
    m_pNibpPushGraph = new C2DPushGraph;
}

CPageNIBP::~CPageNIBP()
{
    if (NULL != m_pNibpPushGraph)
    {
        delete m_pNibpPushGraph;
    }
}

void CPageNIBP::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CMB_NIBP_PATIENT, m_patient_sel);
    DDX_Control(pDX, IDC_CMB_NIBP_PRE_PRESSURE, m_pre_press_sel);
    DDX_Control(pDX, IDC_CMB_NIBP_VEN_PRESS, m_venipuncture_sel);
    DDX_Control(pDX, IDC_CMB_NIBP_ACTION_TYPE, m_mode_sel);
    DDX_Control(pDX, IDC_CHECK_NIBP_ADC, m_btn_realtime);
}


BEGIN_MESSAGE_MAP(CPageNIBP, CPropertyPage)
    ON_BN_CLICKED(IDC_BTN_NIBP_START, &CPageNIBP::OnBnClickedBtnNibpStart)
    ON_CBN_SELCHANGE(IDC_CMB_NIBP_PRE_PRESSURE, &CPageNIBP::OnCbnSelchangeCmbNibpPrePressure)
    ON_CBN_SELCHANGE(IDC_CMB_NIBP_PATIENT, &CPageNIBP::OnCbnSelchangeCmbNibpPatient)
    ON_BN_CLICKED(IDC_CHECK_NIBP_ADC, &CPageNIBP::OnBnClickedCheckNibpAdc)
END_MESSAGE_MAP()

void CPageNIBP::initApplication(void)
{
    g_pSerialProtocol->bindPaktFuncByID(AIO_NIBP_STOP_ID ,this, CPageNIBP::PktHandleNIBPStop);
    g_pSerialProtocol->bindPaktFuncByID(AIO_NIBP_START_ID ,this, CPageNIBP::PktHandleNIBPStart);
    g_pSerialProtocol->bindPaktFuncByID(AIO_NIBP_VENIPUNCTURE_ID ,this, CPageNIBP::PktHandleNIBPStart);
    g_pSerialProtocol->bindPaktFuncByID(AIO_NIBP_GAS_LEAK_ID ,this, CPageNIBP::PktHandleNIBPStart);
    g_pSerialProtocol->bindPaktFuncByID(AIO_NIBP_STATIC_PRESS_ID ,this, CPageNIBP::PktHandleNIBPStart);
    g_pSerialProtocol->bindPaktFuncByID(AIO_TX_NIBP_REALTIME_ID ,this, CPageNIBP::PktHandleNIBPRealTime);
    g_pSerialProtocol->bindPaktFuncByID(AIO_TX_NIBP_MMHG_ID ,this, CPageNIBP::PktHandleNIBPmmHg);
    g_pSerialProtocol->bindPaktFuncByID(AIO_NIBP_RESLULT_ID ,this, CPageNIBP::PktHandleNIBPResult);
    g_pSerialProtocol->bindPaktFuncByID(AIO_TX_NIBP_ALARM_ID ,this, CPageNIBP::PktHandleNIBPAlarm);
    g_pSerialProtocol->bindPaktFuncByID(AIO_TX_NIBP_COUNT_DOWN_S_ID ,this, CPageNIBP::PktHandleNIBPCountdown);
}

int CPageNIBP::PktHandleNIBPStop(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CPageNIBP *pDlgNIBP = (CPageNIBP*)pParam;
    pDlgNIBP->GetDlgItem(IDC_BTN_NIBP_START)->SetWindowTextA("启动测量");
    pDlgNIBP->m_bNIBPStartFlag = false;
    return 0;
}

int CPageNIBP::PktHandleNIBPStart(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CPageNIBP *pDlgNIBP = (CPageNIBP*)pParam;
    if ((AIO_NIBP_START_ID == pPacket->PacketID) && (0xFF == pPacket->DataAndCRC[0])) //End of NIBP
    {
        pDlgNIBP->GetDlgItem(IDC_BTN_NIBP_START)->SetWindowTextA("启动测量");
        pDlgNIBP->m_bNIBPStartFlag = false;
    }
    else
    {
        pDlgNIBP->GetDlgItem(IDC_BTN_NIBP_START)->SetWindowTextA("停止测量");
        pDlgNIBP->m_bNIBPStartFlag = true;
    }
    return 0;
}

int CPageNIBP::PktHandleNIBPRealTime(LPVOID pParam, UartProtocolPacket *pPacket)
{
    int mmHg = 0;
    CString str;
    CPageNIBP *pDlgNIBP = (CPageNIBP*)pParam;
    mmHg =  pPacket->DataAndCRC[1]<<16 \
            | pPacket->DataAndCRC[2]<<8 \
            | pPacket->DataAndCRC[3];
    if (mmHg == 0xFFFFFF) mmHg=-1;
    str.Format("%d",mmHg);
    pDlgNIBP->GetDlgItem(IDC_EDIT_NIBP_REAL_VAL)->SetWindowTextA(str);
    
    if ((NULL == pDlgNIBP->m_pNibpPushGraph) || (-1 == mmHg)) return -1;
    mmHg = mmHg/10000;
    pDlgNIBP->m_pNibpPushGraph->Push(mmHg, 0);
    pDlgNIBP->m_pNibpPushGraph->Update();
    return 0;
}

int CPageNIBP::PktHandleNIBPmmHg(LPVOID pParam, UartProtocolPacket *pPacket)
{
    int mmHg = 0;
    CString str;
    CPageNIBP *pDlgNIBP = (CPageNIBP*)pParam;
    mmHg =  pPacket->DataAndCRC[1]<<8 \
            | pPacket->DataAndCRC[2];
    str.Format("%d",mmHg);
    pDlgNIBP->GetDlgItem(IDC_EDIT_NIBP_VAL)->SetWindowTextA(str);
    return 0;
}

int CPageNIBP::PktHandleNIBPResult(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CString str;
    int index;
    int mmHg = 0;
    CPageNIBP *pDlgNIBP = (CPageNIBP*)pParam;
    int SP = pPacket->DataAndCRC[1]<<8 | pPacket->DataAndCRC[2];
    int MAP = pPacket->DataAndCRC[3]<<8 | pPacket->DataAndCRC[4];
    int DP = pPacket->DataAndCRC[5]<<8 | pPacket->DataAndCRC[6];
    int BPM = pPacket->DataAndCRC[7]<<8 | pPacket->DataAndCRC[8];
    CTime Time = CTime::GetCurrentTime();
    CString TimeCur;

    TimeCur.Format("%02d-%02d %02d:%02d:%02d",
        Time.GetMonth(),Time.GetDay(),
        Time.GetHour(),Time.GetMinute(),Time.GetSecond());
    MSG("[%s]结果如下:\r\n",TimeCur);
    MSG("==========\tSP :%d\r\n",SP);
    MSG("==========\tMAP:%d\r\n",MAP);
    MSG("==========\tDP :%d\r\n",DP);
    MSG("==========\tBPM:%d\r\n",BPM);

    //下次预充压力
    mmHg =  pPacket->DataAndCRC[9]<<8 \
            | pPacket->DataAndCRC[10];
    TRACE("\r\n>>PktHandleNIBPResult prePress=%d",mmHg);
    str.Format("%d",mmHg);
    index = pDlgNIBP->m_pre_press_sel.FindString(0,str);
    if (CB_ERR == index)
    {
        MSG("NIBP预充压力索引不再列表!%d\r\n",mmHg);
    }
    else
    {
        pDlgNIBP->m_pre_press_sel.SetCurSel(index);
    }
    return 0;
}

int CPageNIBP::PktHandleNIBPAlarm(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CPageNIBP *pDlgNIBP = (CPageNIBP*)pParam;
    if (pPacket->DataAndCRC[0] < NIBP_ALARM_TypeDef_MAX)
    {
        MSG("%s\r\n",NIBP_ALARM_TypeName[pPacket->DataAndCRC[0]]);
    }
    return 0;
}

int CPageNIBP::PktHandleNIBPCountdown(LPVOID pParam, UartProtocolPacket *pPacket)
{
    int second;
    CString str;
    CPageNIBP *pDlgNIBP = (CPageNIBP*)pParam;

    second =  pPacket->DataAndCRC[0]<<8 | pPacket->DataAndCRC[1];
    str.Format("%dm:%ds",(second / 60), (second % 60));
    pDlgNIBP->GetDlgItem(IDC_EDIT_NIBP_COUNTDOWN)->SetWindowTextA(str);
    return 0;
}
// CPageNIBP 消息处理程序


BOOL CPageNIBP::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    
	// TODO: 在此添加额外的初始化代码
    if (NULL != m_pNibpPushGraph)
    {
        m_pNibpPushGraph->CreateFromStatic(IDC_NIBP_WAVE, this);      //这个IDC_REALCTRL即是那个Picture Control控件的ID号。
        m_pNibpPushGraph->ModifyStyle(0, WS_THICKFRAME);              //设置风格
        m_pNibpPushGraph->AddLine(0,  RGB(255,255,255));
        m_pNibpPushGraph->SetLabelForMax( "200" );
        m_pNibpPushGraph->SetLabelForMin( "0" );
        m_pNibpPushGraph->ShowGrid( TRUE );
        m_pNibpPushGraph->ShowLabels( TRUE  );
        m_pNibpPushGraph->ShowAsBar(0, false );
        m_pNibpPushGraph->SetInterval( 1 );
        m_pNibpPushGraph->SetGridSize( (USHORT)12 );
        m_pNibpPushGraph->SetPeekRange( 0, 200 );
    }

    m_patient_sel.InsertString(0, "成人");
    m_patient_sel.InsertString(1, "小儿");
    m_patient_sel.InsertString(2, "新生儿");
    m_patient_sel.SetCurSel(0);
    
    m_pre_press_sel.InsertString(0,_T("60"));
    m_pre_press_sel.InsertString(1,_T("70"));
    m_pre_press_sel.InsertString(2,_T("80"));
    m_pre_press_sel.InsertString(3,_T("100"));
    m_pre_press_sel.InsertString(4,_T("120"));
    m_pre_press_sel.InsertString(5,_T("140"));
    m_pre_press_sel.InsertString(6,_T("150"));
    m_pre_press_sel.InsertString(7,_T("160"));
    m_pre_press_sel.InsertString(8,_T("180"));
    m_pre_press_sel.InsertString(9,_T("200"));
    m_pre_press_sel.InsertString(10,_T("220"));
    m_pre_press_sel.InsertString(11,_T("240"));
    m_pre_press_sel.SetCurSel(7);
    
    m_venipuncture_sel.InsertString(0,_T("20"));
    m_venipuncture_sel.InsertString(1,_T("30"));
    m_venipuncture_sel.InsertString(2,_T("40"));
    m_venipuncture_sel.InsertString(3,_T("50"));
    m_venipuncture_sel.InsertString(4,_T("60"));
    m_venipuncture_sel.InsertString(5,_T("70"));
    m_venipuncture_sel.InsertString(6,_T("80"));
    m_venipuncture_sel.InsertString(7,_T("90"));
    m_venipuncture_sel.InsertString(8,_T("100"));
    m_venipuncture_sel.InsertString(9,_T("110"));
    m_venipuncture_sel.InsertString(10,_T("120"));
    m_venipuncture_sel.SetCurSel(6);
    
    m_mode_sel.InsertString(0,_T("单次测量"));
    m_mode_sel.InsertString(1,_T("静脉穿刺"));
    m_mode_sel.InsertString(2,_T("漏气检测"));
    m_mode_sel.InsertString(3,_T("静态压力"));
    m_mode_sel.SetCurSel(0);
    
    return TRUE;
}


void CPageNIBP::OnBnClickedBtnNibpStart()
{
    // TODO: 在此添加控件通知处理程序代码
    int ModeIndex = m_mode_sel.GetCurSel();
    int VemipunctureIndex = m_venipuncture_sel.GetCurSel();
    CString str;
    int VemipunctureValue = 0;
    BYTE id, data[2];

	m_venipuncture_sel.GetLBText( VemipunctureIndex, str); 
	VemipunctureValue = atoi(str);
    TRACE("\r\nVemipunctureValue=%d",VemipunctureValue);
    
    if (!g_pSerialProtocol->isSerialOpen())
    {
        MSG("请确保正确配置串口\r\n");
        return; 
    }

    if (m_bNIBPStartFlag) //正在测量
    {
        id = (BYTE)AIO_NIBP_STOP_ID;
        data[0] = 0x01;
        g_pSerialProtocol->sendOnePacket(id, 0, data, 1);
        return;
    }

    switch(ModeIndex)
    {
    case 0: // 单次测量
    {
        if (m_btn_realtime.GetCheck()) // 原始数据显示
        {
            id = (BYTE)AIO_RX_NIBP_Debug_ID;
            data[0] = 0x02;
            data[1] = 0x01;
            g_pSerialProtocol->sendOnePacket(id, 0, data, 2);
        }
        else
        {
            id = (BYTE)AIO_NIBP_START_ID;
            data[0] = 0xF0;
            g_pSerialProtocol->sendOnePacket(id, 0, data, 1);
        }
        break;  
    }
    case 1: // 静脉穿刺
    {
        id = (BYTE)AIO_NIBP_VENIPUNCTURE_ID;
        data[0] = 0x01;
        data[1] = (BYTE)(VemipunctureValue & 0xFF);
        g_pSerialProtocol->sendOnePacket(id, 0, data, 2);
        break;
    }
    case 2: // 漏气检测
    {
        id = (BYTE)AIO_NIBP_GAS_LEAK_ID;
        data[0] = 0x01;
        g_pSerialProtocol->sendOnePacket(id, 0, data, 1);
        break; 
    }
    case 3: // 静态压力
    {
        id = (BYTE)AIO_NIBP_STATIC_PRESS_ID;
        data[0] = 0x01;
        g_pSerialProtocol->sendOnePacket(id, 0, data, 1);
        break;
    }
    default:
    {
        break;
    }
    } 
}


void CPageNIBP::OnCbnSelchangeCmbNibpPrePressure()
{
    // TODO: 在此添加控件通知处理程序代码
    int PrePressureIndex = m_pre_press_sel.GetCurSel();
    int PrePressureValue = 0;
    CString str;
    BYTE id, data[2];

	m_pre_press_sel.GetLBText( PrePressureIndex, str); 
	PrePressureValue = atoi(str);
    TRACE("\r\nPrePressureValue=%d",PrePressureValue);
    
    id = (BYTE)AIO_NIBP_PREPROCESS_PRESS_ID;
	data[0] = (BYTE)(PrePressureValue>>8);
	data[1] = (BYTE)PrePressureValue;
    if (g_pSerialProtocol->isSerialOpen())
    {
        g_pSerialProtocol->sendOnePacket(id, 0, data, sizeof(data));
    }
    else
    {
        MSG("请确保正确配置串口\r\n");
    }
}


void CPageNIBP::OnCbnSelchangeCmbNibpPatient()
{
    // TODO: 在此添加控件通知处理程序代码
    int PatientIndex = m_patient_sel.GetCurSel();
    BYTE id, data[1];
    
    id = (BYTE)COM_PATIENT_TYPE_ID;
	data[0] = (BYTE)(PatientIndex+1); //1:Adult, 2:Child, 3:Newborn
    if (g_pSerialProtocol->isSerialOpen())
    {
        g_pSerialProtocol->sendOnePacket(id, 0, data, sizeof(data));
    }
    else
    {
        MSG("请确保正确配置串口\r\n");
    }
}


void CPageNIBP::OnBnClickedCheckNibpAdc()
{
    // TODO: 在此添加控件通知处理程序代码
}
