#pragma once

// CPageDebug 对话框

class CPageDebug : public CPropertyPage
{
	DECLARE_DYNAMIC(CPageDebug)

public:
	CPageDebug();
	virtual ~CPageDebug();

    void    initApplication(void);
    static int WINAPI    PktHandleEcgProbeInfo(LPVOID pParam, UartProtocolPacket *pPacket);

// 对话框数据
	enum { IDD = IDD_DLG_DEBUG };

protected:

    unsigned long Table_CRC32[256];
    int String2Hex(CString str, CByteArray &sendData);
    char ConvertHexChar(char ch) ;
	unsigned char CRC8(unsigned char *ptr,unsigned char len);
	unsigned short CRC16(unsigned char *ptr, unsigned short len);
    void BuildTableCRC32( unsigned long aPoly );
	unsigned long CRC32( unsigned char * aData, unsigned long aSize );

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnDebSend();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtnCalCrc();
    afx_msg void OnBnClickedBtnDraw();
};
