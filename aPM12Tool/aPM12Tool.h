
// aPM12Tool.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


//Add by QWB
#include "logfile.h"
#include "DefPrintf.h"
#include "SerialProtocol.h"

// CaPM12ToolApp:
// �йش����ʵ�֣������ aPM12Tool.cpp
//

class CaPM12ToolApp : public CWinApp
{
public:
	CaPM12ToolApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CaPM12ToolApp theApp;