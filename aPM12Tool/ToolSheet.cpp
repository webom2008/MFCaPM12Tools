// ToolSheet.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "ToolSheet.h"

#include <io.h>
#include <fcntl.h>
#include <stdio.h>

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

    m_PageDebug.initApplication();
	m_PageUpdate.initApplication();
	m_PageWave.initApplication();

    this->m_psh.dwFlags |= PSH_NOAPPLYNOW;
    this->m_psh.dwFlags &= ~(PSH_HASHELP);
    
    m_PageSysCfg.m_psp.dwFlags &= ~(PSP_HASHELP);
    m_PageUpdate.m_psp.dwFlags &= ~(PSP_HASHELP);
    m_PageDebug.m_psp.dwFlags &= ~(PSP_HASHELP);
    m_PageNIBP.m_psp.dwFlags &= ~(PSP_HASHELP);
    m_PageWave.m_psp.dwFlags &= ~(PSP_HASHELP);

    AddPage(&m_PageSysCfg);
    AddPage(&m_PageUpdate);
    AddPage(&m_PageDebug);
    AddPage(&m_PageNIBP);
    AddPage(&m_PageWave);
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


// CToolSheet ��Ϣ��������


void InitConsoleWindow(void)
{
    INFO("<!-- ========================================= -->\r\n");
    INFO("// Copyright (C) 2015 QiuWeibo <qiuweibo@cvte.com>\r\n");
    INFO("// �������� : ���Դ�ӡ����\r\n");
    INFO("// �����汾 : V1.0.2\r\n");
    INFO("// ��    �� : ��ΰ��\r\n");
    INFO("// ʱ    �� : %s %s\r\n",__DATE__, __TIME__);
    INFO("<!-- ========================================= -->\r\n");
}


BOOL CToolSheet::OnInitDialog()
{
    BOOL bResult = CPropertySheet::OnInitDialog();

    Logconsole_open();
    InitConsoleWindow();
    Log2File("Open aPM12Tool \r\n");

    //ȥ����ȷ�����͡�ȡ������ť
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

    //ȥ���ĸ���ť�����������·��Ŀհ�
    GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
    GetDlgItem(IDHELP)->ShowWindow(SW_HIDE);
    GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
    GetDlgItem(ID_APPLY_NOW)->ShowWindow(SW_HIDE);
    
    //��ȡ����ߴ� 
    CRect btnRect; 
    GetDlgItem(IDCANCEL)->GetWindowRect(&btnRect);
    CRect wdnRect; 
    GetWindowRect(&wdnRect); 
    //���������С 
    ::SetWindowPos(this->m_hWnd, HWND_TOP, 0,0,wdnRect.Width(),wdnRect.Height() - btnRect.Height(), SWP_NOMOVE | SWP_NOZORDER);

    return bResult;
}