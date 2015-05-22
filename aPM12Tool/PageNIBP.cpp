// PageNIBP.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageNIBP.h"
#include "afxdialogex.h"


// CPageNIBP �Ի���

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

// CPageNIBP ��Ϣ�������


BOOL CPageNIBP::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    
	// TODO: �ڴ���Ӷ���ĳ�ʼ������
    if (NULL != m_pNibpPushGraph)
    {
        m_pNibpPushGraph->CreateFromStatic(IDC_NIBP_WAVE, this);      //���IDC_REALCTRL�����Ǹ�Picture Control�ؼ���ID�š�
        m_pNibpPushGraph->ModifyStyle(0, WS_THICKFRAME);              //���÷��
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
