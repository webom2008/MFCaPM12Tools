#pragma once

#include "2DPushGraph.h"
// CPageNIBP �Ի���

class CPageNIBP : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageNIBP)

public:
	CPageNIBP();
	virtual ~CPageNIBP();

    void        initApplication(void);
// �Ի�������
	enum { IDD = IDD_DLG_NIBP };
private:
    
    int                 m_NibpValueCur;
    C2DPushGraph*       m_pNibpPushGraph;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
};
