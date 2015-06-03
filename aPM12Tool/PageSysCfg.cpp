// PageSysCfg.cpp : 实现文件
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageSysCfg.h"
#include "afxdialogex.h"
#include <iostream>

extern CSerialProtocol *g_pSerialProtocol;

// CPageSysCfg 对话框

IMPLEMENT_DYNAMIC(CPageSysCfg, CPropertyPage)

CPageSysCfg::CPageSysCfg()
	: CPropertyPage(CPageSysCfg::IDD)
{

}

CPageSysCfg::~CPageSysCfg()
{
}

void CPageSysCfg::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CBO_SERIAL_NUM, m_SerialNumbSel);
    DDX_Control(pDX, IDC_CBO_SERIAL_BAUD, m_SerialBaudSel);
    DDX_Control(pDX, IDC_BTN_SERIAL_OPEN, m_SerialOpenCtrl);
    DDX_Control(pDX, IDC_CBO_BOARD_SEL, m_BoardSel);
    DDX_Control(pDX, IDC_CHECK_ID_VALID, m_btnCheckID);
}


BEGIN_MESSAGE_MAP(CPageSysCfg, CPropertyPage)
    ON_BN_CLICKED(IDC_BTN_SERIAL_OPEN, &CPageSysCfg::OnBnClickedBtnSerialOpen)
    ON_CBN_SELCHANGE(IDC_CBO_BOARD_SEL, &CPageSysCfg::OnCbnSelchangeCboBoardSel)
    ON_BN_CLICKED(IDC_CHECK_ID_VALID, &CPageSysCfg::OnBnClickedCheckIdValid)
END_MESSAGE_MAP()


// CPageSysCfg 消息处理程序


BOOL CPageSysCfg::OnInitDialog()
{
    int i;

    CPropertyPage::OnInitDialog();
    
	char SerialPortName[10][80];
    int count = g_pSerialProtocol->getSerialPortsReg(SerialPortName);
    for (i = 0; i < count; i++)
    {
        m_SerialNumbSel.InsertString(i,SerialPortName[i]);
    }
    if (count > 0)
    {
        m_SerialNumbSel.SetCurSel(0);
    }

    m_SerialBaudSel.InsertString(0, "19200");
    m_SerialBaudSel.InsertString(1, "115200");
    m_SerialBaudSel.InsertString(2, "230400");
    m_SerialBaudSel.InsertString(3, "460800");
    m_SerialBaudSel.SetCurSel(2);

    m_BoardSel.InsertString(0,"AIO-DSP");
    m_BoardSel.InsertString(1,"单板SPO2");
    m_BoardSel.InsertString(2,"背板");
    m_BoardSel.InsertString(3,"记录仪");
    m_BoardSel.InsertString(4,"扩展模块");
    m_BoardSel.SetCurSel(0);

    return TRUE;
}


void CPageSysCfg::OnBnClickedBtnSerialOpen()
{
    int index = 0, offset = 0;
    UINT portNr = 0;
	CString str,strPortNr;
    int nBaudrate = 0;
    
    if (NULL == g_pSerialProtocol) return;

    str.Empty();
	index = m_SerialBaudSel.GetCurSel();
	m_SerialBaudSel.GetLBText( index, str); 
	nBaudrate = atoi(str);
    
    str.Empty();
	index = m_SerialNumbSel.GetCurSel();
	m_SerialNumbSel.GetLBText( index, str);
    offset = str.ReverseFind('M');// 从右往左边开始查找第一个'\\'，获取左边字符串的长度
    strPortNr = str.Right(str.GetLength() - offset -1);
    portNr = atoi(strPortNr);

    if(g_pSerialProtocol->isSerialOpen())    
    {
        MSG("关闭串口");
        if (0 == g_pSerialProtocol->closeDevice())
        {
            INFO(":OK\r\n");
            m_SerialOpenCtrl.SetWindowText("打开串口");
        }
        else
        {
            INFO(":FAIL\r\n");
        }
    }   
    else    
    { 
        MSG("打开串口");
        if (0 == g_pSerialProtocol->openDevice(this, portNr, nBaudrate))
        {
            INFO(":OK\r\n");
            m_SerialOpenCtrl.SetWindowText("关闭串口");
        }
        else
        {
            INFO(":FAIL\r\n");
        }
    }
}


void CPageSysCfg::OnCbnSelchangeCboBoardSel()
{
    int index = 0;
	CString str;
	index = m_BoardSel.GetCurSel();
	m_BoardSel.GetLBText( index, str);
    MSG("通信板卡选择:%s\r\n",str);
    switch(index)
    {
    case 0:
        g_pSerialProtocol->setRxPacketAddr(UART_AIO_ADDR, UART_MCU_ADDR);
        break;
    case 1:
        g_pSerialProtocol->setRxPacketAddr(UART_SpO2_ADDR, UART_AIO_ADDR);
        break;
    case 2:
        g_pSerialProtocol->setRxPacketAddr(UART_BACK_ADDR, UART_MCU_ADDR);
        break;
    case 3:
        g_pSerialProtocol->setRxPacketAddr(UART_RECORD_ADDR, UART_MCU_ADDR);
        break;
    case 4:
        g_pSerialProtocol->setRxPacketAddr(UART_EXPAND_ADDR, UART_MCU_ADDR);
        break;
    default:
        break;
    }
}


void CPageSysCfg::OnBnClickedCheckIdValid()
{
    if (m_btnCheckID.GetCheck())
    {
        g_pSerialProtocol->setPacketNumValidCheck(true);
    }
    else
    {
        g_pSerialProtocol->setPacketNumValidCheck(false);
    }
}
