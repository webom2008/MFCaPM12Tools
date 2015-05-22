#pragma once

#include "2DPushGraph.h"
// CPageNIBP 对话框

class CPageNIBP : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageNIBP)

public:
	CPageNIBP();
	virtual ~CPageNIBP();

    void        initApplication(void);
// 对话框数据
	enum { IDD = IDD_DLG_NIBP };
private:
    
    int                 m_NibpValueCur;
    C2DPushGraph*       m_pNibpPushGraph;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
};
