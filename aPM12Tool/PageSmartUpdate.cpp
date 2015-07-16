// PageSmartUpdate.cpp : 实现文件
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageSmartUpdate.h"
#include "afxdialogex.h"


// CPageSmartUpdate 对话框

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


// CPageSmartUpdate 消息处理程序

void CPageSmartUpdate::initApplication(void)
{

}