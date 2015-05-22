// PageNIBP.cpp : 实现文件
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageNIBP.h"
#include "afxdialogex.h"


// CPageNIBP 对话框

IMPLEMENT_DYNAMIC(CPageNIBP, CPropertyPage)

CPageNIBP::CPageNIBP()
	: CPropertyPage(CPageNIBP::IDD)
{
    m_NibpValueCur = 0;
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
}


BEGIN_MESSAGE_MAP(CPageNIBP, CPropertyPage)
END_MESSAGE_MAP()

void CPageNIBP::initApplication(void)
{

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
        m_pNibpPushGraph->SetLabelForMax( "100" );
        m_pNibpPushGraph->SetLabelForMin( "0" );
        m_pNibpPushGraph->ShowGrid( TRUE );
        m_pNibpPushGraph->ShowLabels( TRUE  );
        m_pNibpPushGraph->ShowAsBar(0, false );
        m_pNibpPushGraph->SetInterval( 4 );
        m_pNibpPushGraph->SetGridSize( (USHORT)12 );
        m_pNibpPushGraph->SetPeekRange( 0, 100 );
    }


    return TRUE;
}
