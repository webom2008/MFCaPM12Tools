// ToolSheet.cpp : 实现文件
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "ToolSheet.h"

#include <io.h>
#include <fcntl.h>
#include <stdio.h>

const char VERSION[] = "V1.0.6 Beta";

extern CSerialProtocol *g_pSerialProtocol;

IMPLEMENT_DYNAMIC(CToolSheet, CPropertySheet)

CToolSheet::CToolSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{

}

CToolSheet::CToolSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
    g_pSerialProtocol = new CSerialProtocol;
    g_pSerialProtocol->initApplication();
    
    this->m_psh.dwFlags |= PSH_NOAPPLYNOW;
    this->m_psh.dwFlags &= ~(PSH_HASHELP);
    
    m_PageSysCfg.m_psp.dwFlags &= ~(PSP_HASHELP);
    m_PageUpdate.m_psp.dwFlags &= ~(PSP_HASHELP);
    m_PageDebug.m_psp.dwFlags &= ~(PSP_HASHELP);
    m_PageNIBP.m_psp.dwFlags &= ~(PSP_HASHELP);
    m_PageWave.m_psp.dwFlags &= ~(PSP_HASHELP);
    m_PageFactory.m_psp.dwFlags &= ~(PSP_HASHELP);
	m_PageFileMaker.m_psp.dwFlags &= ~(PSP_HASHELP);
	m_PageSmartUpdate.m_psp.dwFlags &= ~(PSP_HASHELP);

    AddPage(&m_PageSysCfg);
    AddPage(&m_PageUpdate);
    AddPage(&m_PageNIBP);
    AddPage(&m_PageWave);
    AddPage(&m_PageFactory);
    AddPage(&m_PageDebug);
    AddPage(&m_PageSmartUpdate);

    AddPage(&m_PageFileMaker);
}

CToolSheet::~CToolSheet()
{
    Log2File("Close aPM12Tool \r\n");
    if (NULL != g_pSerialProtocol)
    {
        delete g_pSerialProtocol;
        g_pSerialProtocol = NULL;
    }
}


BEGIN_MESSAGE_MAP(CToolSheet, CPropertySheet)
END_MESSAGE_MAP()


// CToolSheet 消息处理程序
void CToolSheet::initApplication(void)
{
    m_PageDebug.initApplication();
	m_PageUpdate.initApplication();
	m_PageWave.initApplication();
	m_PageNIBP.initApplication();
	m_PageFactory.initApplication();
	m_PageFileMaker.initApplication();
	m_PageSmartUpdate.initApplication();
}

void InitConsoleWindow(void)
{
    INFO("<!-- ========================================= -->\r\n");
    INFO("// Copyright (C) 2015 QiuWeibo <qiuweibo@cvte.com>\r\n");
    INFO("// 功能描述 : 调试打印窗口\r\n");
    INFO("// 软件版本 : %s\r\n",VERSION);
    INFO("// 作    者 : 邱伟波\r\n");
    INFO("// 时    间 : %s %s\r\n",__DATE__, __TIME__);
    INFO("<!-- ========================================= -->\r\n");
}


BOOL CToolSheet::OnInitDialog()
{
    BOOL bResult = CPropertySheet::OnInitDialog();

    Logconsole_open();
    InitConsoleWindow();
    Log2File("Open aPM12Tool \r\n");

    //去掉“确定”和“取消”按钮
    CWnd *pWnd = GetDlgItem(IDOK);
    if(pWnd && pWnd->GetSafeHwnd())
    {
        pWnd->ShowWindow(false);
    }
    pWnd = GetDlgItem(IDCANCEL); 
    if(pWnd && pWnd->GetSafeHwnd())
    {
        pWnd->ShowWindow(false); 
    }

    //去掉四个按钮后消除界面下方的空白
    GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
    GetDlgItem(IDHELP)->ShowWindow(SW_HIDE);
    GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
    GetDlgItem(ID_APPLY_NOW)->ShowWindow(SW_HIDE);
    
    //获取窗体尺寸 
    CRect btnRect; 
    GetDlgItem(IDCANCEL)->GetWindowRect(&btnRect);
    CRect wdnRect; 
    GetWindowRect(&wdnRect); 
    //调整窗体大小 
    ::SetWindowPos(this->m_hWnd, HWND_TOP, 0,0,wdnRect.Width(),wdnRect.Height() - btnRect.Height(), SWP_NOMOVE | SWP_NOZORDER);

    initApplication();
    return bResult;
}
