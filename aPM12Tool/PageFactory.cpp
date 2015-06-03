// PageFactory.cpp : 实现文件
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageFactory.h"
#include "afxdialogex.h"


extern CSerialProtocol *g_pSerialProtocol;
// CPageFactory 对话框

IMPLEMENT_DYNAMIC(CPageFactory, CPropertyPage)

CPageFactory::CPageFactory()
	: CPropertyPage(CPageFactory::IDD)
{

}

CPageFactory::~CPageFactory()
{
}

void CPageFactory::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CMB_NIBP_DSP_VERIFY, m_verify_dsp_sel);
    DDX_Control(pDX, IDC_CMB_NIBP_STM_VERIFY, m_verify_stm_sel);
    DDX_Control(pDX, IDC_CMB_AIO_EEPROM, m_cmb_aio_eeprom_func);
}


BEGIN_MESSAGE_MAP(CPageFactory, CPropertyPage)
    ON_BN_CLICKED(IDC_BTN_NIBP_VERIFY_STATUS, &CPageFactory::OnBnClickedBtnNibpVerifyStatus)
    ON_BN_CLICKED(IDC_BTN_NIBP_VERIFY, &CPageFactory::OnBnClickedBtnNibpVerify)
    ON_BN_CLICKED(IDC_BTN_NIBP_DSP_VERIFY, &CPageFactory::OnBnClickedBtnNibpDspVerify)
    ON_BN_CLICKED(IDC_BTN_NIBP_STM_VERIFY, &CPageFactory::OnBnClickedBtnNibpStmVerify)
    ON_BN_CLICKED(IDC_BTN_AIO_EEPROM, &CPageFactory::OnBnClickedBtnAioEeprom)
END_MESSAGE_MAP()


void CPageFactory::initApplication(void)
{
    g_pSerialProtocol->bindPaktFuncByID(AIO_NIBP_VERIFY_STATE_ID ,this, CPageFactory::PktHandleNIBPVerifyStatus);
    g_pSerialProtocol->bindPaktFuncByID(AIO_NIBP_VERIFY_ID ,this, CPageFactory::PktHandleNIBPVerifyEnterExit);
    g_pSerialProtocol->bindPaktFuncByID(AIO_NIBP_VERIFYING_ID ,this, CPageFactory::PktHandleNIBPVerifyDSP);
    g_pSerialProtocol->bindPaktFuncByID(AIO_NIBP_STM32_PRESS_ID ,this, CPageFactory::PktHandleNIBPVerifySTM);

}

int CPageFactory::PktHandleNIBPVerifyStatus(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CPageFactory *pDlgFactory = (CPageFactory*)pParam;

    if ((pPacket->DataAndCRC[0]) >> 3 & 0x01) //DSP verify mask
    {
        pDlgFactory->GetDlgItem(IDC_STATIC_DSP_VERIFY)->SetWindowTextA("1");
    }
    else
    {
        pDlgFactory->GetDlgItem(IDC_STATIC_DSP_VERIFY)->SetWindowTextA("0");
    }

    if ((pPacket->DataAndCRC[0]) >> 1 & 0x01) //STM32 verify mask
    {
        pDlgFactory->GetDlgItem(IDC_STATIC_STM_VERIFY)->SetWindowTextA("1");
    }
    else
    {
        pDlgFactory->GetDlgItem(IDC_STATIC_STM_VERIFY)->SetWindowTextA("0");
    }
    pDlgFactory->GetDlgItem(IDC_BTN_NIBP_VERIFY_STATUS)->SetWindowTextA("当前状态");
    return 0;
}

int CPageFactory::PktHandleNIBPVerifyEnterExit(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CPageFactory *pDlgFactory = (CPageFactory*)pParam;
    if (VERIFY_ENTER_GET == pDlgFactory->enum_VerifyEnterExitState)
    {
        pDlgFactory->GetDlgItem(IDC_BTN_NIBP_VERIFY)->SetWindowTextA("保存退出");
        pDlgFactory->enum_VerifyEnterExitState = VERIFY_EXIT_SEND;
    }
    else if (VERIFY_EXIT_GET == pDlgFactory->enum_VerifyEnterExitState)
    {
        pDlgFactory->GetDlgItem(IDC_BTN_NIBP_VERIFY)->SetWindowTextA("进入校准");
        pDlgFactory->enum_VerifyEnterExitState = VERIFY_ENTER_SEND;
    }
    return 0;
}

int CPageFactory::PktHandleNIBPVerifyDSP(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CPageFactory *pDlgFactory = (CPageFactory*)pParam;
    BYTE mmHg = 0;
    int adc = 0;

    pDlgFactory->GetDlgItem(IDC_BTN_NIBP_DSP_VERIFY)->SetWindowTextA("校准");
    mmHg = pPacket->DataAndCRC[0];
    adc = (pPacket->DataAndCRC[1] << 16) | (pPacket->DataAndCRC[2] << 8) | pPacket->DataAndCRC[3];
    MSG("DSP mmHg = %d, adc = %d\r\n", mmHg, adc);
    return 0;
}

int CPageFactory::PktHandleNIBPVerifySTM(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CPageFactory *pDlgFactory = (CPageFactory*)pParam;
    BYTE index = 0;
    int adc = 0;

    pDlgFactory->GetDlgItem(IDC_BTN_NIBP_STM_VERIFY)->SetWindowTextA("校准");
    index = pPacket->DataAndCRC[0];
    adc = (pPacket->DataAndCRC[1] << 8) | pPacket->DataAndCRC[2];
    MSG("index = %d (0: 315mmHg, 1: 162mmHg 255:0mmHg), adc = %d\r\n", index, adc);
    return 0;
}
// CPageFactory 消息处理程序


BOOL CPageFactory::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    enum_VerifyEnterExitState = VERIFY_ENTER_SEND;
    
    m_verify_dsp_sel.InsertString(0,_T("140"));
    m_verify_dsp_sel.InsertString(1,_T("141"));
    m_verify_dsp_sel.InsertString(2,_T("142"));
    m_verify_dsp_sel.InsertString(3,_T("143"));
    m_verify_dsp_sel.InsertString(4,_T("144"));
    m_verify_dsp_sel.InsertString(5,_T("145"));
    m_verify_dsp_sel.InsertString(6,_T("146"));
    m_verify_dsp_sel.InsertString(7,_T("147"));
    m_verify_dsp_sel.InsertString(8,_T("148"));
    m_verify_dsp_sel.InsertString(9,_T("149"));
    m_verify_dsp_sel.InsertString(10,_T("150"));
    m_verify_dsp_sel.InsertString(11,_T("151"));
    m_verify_dsp_sel.InsertString(12,_T("152"));
    m_verify_dsp_sel.InsertString(13,_T("153"));
    m_verify_dsp_sel.InsertString(14,_T("154"));
    m_verify_dsp_sel.InsertString(15,_T("155"));
    m_verify_dsp_sel.InsertString(16,_T("156"));
    m_verify_dsp_sel.InsertString(17,_T("157"));
    m_verify_dsp_sel.InsertString(18,_T("158"));
    m_verify_dsp_sel.InsertString(19,_T("159"));
    m_verify_dsp_sel.InsertString(20,_T("160"));
    m_verify_dsp_sel.SetCurSel(10);

    m_verify_stm_sel.InsertString(0,_T("315"));
    m_verify_stm_sel.InsertString(1,_T("162"));
    m_verify_stm_sel.SetCurSel(0);

    m_cmb_aio_eeprom_func.InsertString(0,_T("清空所有内容"));
    m_cmb_aio_eeprom_func.InsertString(1,_T("恢复出厂设置"));
    m_cmb_aio_eeprom_func.SetCurSel(0);
    return TRUE;
}

void CPageFactory::OnBnClickedBtnNibpVerifyStatus()
{
    BYTE id, data[1];

    // TODO: 在此添加控件通知处理程序代码
    if (!g_pSerialProtocol->isSerialOpen())
    {
        MSG("请确保正确配置串口\r\n");
        return; 
    }
    
    id = (BYTE)AIO_NIBP_VERIFY_STATE_ID;
    data[0] = 0x01;
    g_pSerialProtocol->sendOnePacket(id, 0, data, 1);
    GetDlgItem(IDC_BTN_NIBP_VERIFY_STATUS)->SetWindowTextA("获取中");
}

void CPageFactory::OnBnClickedBtnNibpVerify()
{
    // TODO: 在此添加控件通知处理程序代码
    BYTE id, data[1];
    if (!g_pSerialProtocol->isSerialOpen())
    {
        MSG("请确保正确配置串口\r\n");
        return; 
    }
    
    if (VERIFY_ENTER_SEND == enum_VerifyEnterExitState)
    {
        id = (BYTE)AIO_NIBP_VERIFY_ID;
        data[0] = 0xCC;
        g_pSerialProtocol->sendOnePacket(id, 0, data, 1);

        GetDlgItem(IDC_BTN_NIBP_VERIFY)->SetWindowTextA("进入中");
        enum_VerifyEnterExitState = VERIFY_ENTER_GET;
    }
    else if (VERIFY_EXIT_SEND == enum_VerifyEnterExitState)
    {
        id = (BYTE)AIO_NIBP_VERIFY_ID;
        data[0] = 0xFF;
        g_pSerialProtocol->sendOnePacket(id, 0, data, 1);

        GetDlgItem(IDC_BTN_NIBP_VERIFY)->SetWindowTextA("保存中");
        enum_VerifyEnterExitState = VERIFY_EXIT_GET;
    }
}

void CPageFactory::OnBnClickedBtnNibpDspVerify()
{
    // TODO: 在此添加控件通知处理程序代码
    BYTE id, data[1];
    int VerifyPress;
    int index = m_verify_dsp_sel.GetCurSel();
    CString press;

    if (!g_pSerialProtocol->isSerialOpen())
    {
        MSG("请确保正确配置串口\r\n");
        return; 
    }

    m_verify_dsp_sel.GetLBText(index, press); 
    VerifyPress = atoi(press);
    
    id = (BYTE)AIO_NIBP_VERIFYING_ID;
    data[0] = (BYTE)(VerifyPress & 0xFF);
    g_pSerialProtocol->sendOnePacket(id, 0, data, 1);
    GetDlgItem(IDC_BTN_NIBP_DSP_VERIFY)->SetWindowTextA("校准中");
}

void CPageFactory::OnBnClickedBtnNibpStmVerify()
{
    // TODO: 在此添加控件通知处理程序代码
    BYTE id, data[1];
    int index = m_verify_stm_sel.GetCurSel();
    if (!g_pSerialProtocol->isSerialOpen())
    {
        MSG("请确保正确配置串口\r\n");
        return; 
    }

    MSG("index=%d (0: 315mmHg, 1: 162mmHg 255:0mmHg)\r\n",index);
    id = (BYTE)AIO_NIBP_STM32_PRESS_ID;
    data[0] = (BYTE)(index & 0xFF);
    g_pSerialProtocol->sendOnePacket(id, 0, data, 1);
    GetDlgItem(IDC_BTN_NIBP_STM_VERIFY)->SetWindowTextA("校准中");
}


void CPageFactory::OnBnClickedBtnAioEeprom()
{
    // TODO: 在此添加控件通知处理程序代码
    BYTE id, data[1];
    int index = m_cmb_aio_eeprom_func.GetCurSel();
    int msgboxID;
    
    if (!g_pSerialProtocol->isSerialOpen())
    {
        MSG("请确保正确配置串口\r\n");
        return; 
    }

    switch (index)
    {
    case 0: // 清楚所有内容
        msgboxID = MessageBoxW(this->m_hWnd, (LPCWSTR)L"确认清空EEPROM操作 ?",(LPCWSTR)L"警告",MB_YESNO);
        if (IDYES == msgboxID)
        {
            id = (BYTE)AIO_EEPROM_DEBUG_ID;
            data[0] = 0x02;
            g_pSerialProtocol->sendOnePacket(id, 0, data, 1);
            MSG("AIO-EEPROM 所有内容已清空!!!!!\r\n");
        }
        break;
    case 1: // 恢复出厂设置
        //TODO
        break;
    default:
        break;
    }
}
