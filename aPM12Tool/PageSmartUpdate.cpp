// PageSmartUpdate.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageSmartUpdate.h"
#include "afxdialogex.h"


// CPageSmartUpdate �Ի���

IMPLEMENT_DYNAMIC(CPageSmartUpdate, CPropertyPage)

CPageSmartUpdate::CPageSmartUpdate()
	: CPropertyPage(CPageSmartUpdate::IDD)
{

}

CPageSmartUpdate::~CPageSmartUpdate()
{
}

void CPageSmartUpdate::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageSmartUpdate, CPropertyPage)
END_MESSAGE_MAP()


// CPageSmartUpdate ��Ϣ�������

void CPageSmartUpdate::initApplication(void)
{

}