// PageDebug.cpp : 实现文件
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageDebug.h"
#include "afxdialogex.h"

// CPageDebug 对话框
extern CSerialProtocol *g_pSerialProtocol;

IMPLEMENT_DYNAMIC(CPageDebug, CPropertyPage)

CPageDebug::CPageDebug()
	: CPropertyPage(CPageDebug::IDD)
{
}

CPageDebug::~CPageDebug()
{
}

void CPageDebug::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageDebug, CPropertyPage)
    ON_BN_CLICKED(IDC_BTN_DEB_SEND, &CPageDebug::OnBnClickedBtnDebSend)
    ON_BN_CLICKED(IDC_BTN_CAL_CRC, &CPageDebug::OnBnClickedBtnCalCrc)
    ON_BN_CLICKED(IDC_BTN_DRAW, &CPageDebug::OnBnClickedBtnDraw)
END_MESSAGE_MAP()


// CPageDebug 消息处理程序

BOOL CPageDebug::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    GetDlgItem(IDC_STATIC_DEB_SEND_INFO)->SetWindowTextA(_T("格式:ID+DATA[0...N] 数据间加空格  如:打开心电校准,填写:29 01"));

    return TRUE;
}


void    CPageDebug::initApplication(void)
{
    g_pSerialProtocol->bindPaktFuncByID(AIO_TX_ECG_LEAD_INFO_ID ,this, CPageDebug::PktHandleEcgProbeInfo);

}

//这是一个将字符转换为相应的十六进制值的函数
//好多C语言书上都可以找到
//功能：若是在0-F之间的字符，则转换为相应的十六进制字符，否则返回-1
char CPageDebug::ConvertHexChar(char ch) 
{
	if((ch>='0')&&(ch<='9'))
	{
		return ch-0x30;
	}
	else if((ch>='A')&&(ch<='F'))
	{
		return ch-'A'+10;
	}
	else if((ch>='a')&&(ch<='f'))
	{
		return ch-'a'+10;
	}
	else 
	{
		return (-1);
	}
}

//由于这个转换函数的格式限制，在发送框中的十六制字符应该每两个字符之间插入一个空隔
//如：A1 23 45 0B 00 29
//CByteArray是一个动态字节数组，可参看MSDN帮助
int CPageDebug::String2Hex(CString str, CByteArray &sendData)
{
	int hexdata,lowhexdata;
	int hexdatalen = 0;
	int len = str.GetLength();
	sendData.SetSize(len/2);

	for(int i = 0; i < len;)
	{
		char lstr,hstr=str[i];
		if(hstr == ' ')
		{
			i++;
			continue;
		}
		i++;
		if(i>len)
		{
			break;
		}
		lstr=str[i];
		hexdata = ConvertHexChar(hstr);
		lowhexdata = ConvertHexChar(lstr);
		if((hexdata==16)||(lowhexdata==16))
		{
			break;
		}
		else
		{
			hexdata = hexdata*16+lowhexdata;
		}
		i++;
		sendData[hexdatalen] = (char)hexdata;
		hexdatalen++;
	}

	sendData.SetSize(hexdatalen);
	return hexdatalen;
}

unsigned char CPageDebug::CRC8(unsigned char *ptr,unsigned char len)
{
#if 0//CRC-8算法1
	unsigned char i;
	unsigned char crc = 0;
	while(len-- != 0)
	{
		for(i = 1; i != 0; i *= 2)
		{
			if((crc&1) != 0)
			{
				crc /= 2;
				crc ^= 0x8c;
			}
			else
			{
				crc /= 2;
			}
			
			if((*ptr&i) != 0)
			{
				crc ^= 0x8c;
			}
		}
		ptr++;
	}
	return(crc);
#else//CRC-8算法2
	unsigned char crc;
	unsigned char i;
	crc = 0;
	while(len--)
	{
		crc ^= *ptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc&0x01)
			{
				crc = (crc >> 1) ^ 0x8C;
			}	
			else
			{
				crc >>= 1;
			}
		}	
	}
	return crc;
#endif
}


unsigned short CPageDebug::CRC16(unsigned char *ptr, unsigned short len)
{
#if 0//算法1
    unsigned short thecrc=0x00;
    unsigned char da;
    while(len--!=0) 
    {
		da=thecrc/256;    // 以8位二进制数的形式暂存CRC的高8位 
		thecrc<<=8;     // 左移8位，相当于CRC的低8位乘以256
		thecrc^=Table_CRC16[da^*ptr]; // 高8位和当前字节相加后再查表求CRC ，再加上以前的CRC 
		ptr++;
    }
    return (thecrc);
#else//算法2
//以下是另一个CRC16的查表算法，该表较小，采用按半字体计算CRC的方法，可以用于RAM较小的单片机中，当然会牺牲一点速度：
const unsigned short crc_ta[16]=
{ /* CRC余式表 */
    0x0000,0x1021,0x2042,0x3063,
    0x4084,0x50a5,0x60c6,0x70e7,
    0x8108,0x9129,0xa14a,0xb16b,
    0xc18c,0xd1ad,0xe1ce,0xf1ef
};
    unsigned short crc;
    unsigned char da;
    crc=0;
    while(len--!=0) 
    {
        da = (crc>>12)&0x0f;  /* 暂存CRC的高四位 */
        crc<<=4;     /* CRC右移4位，相当于取CRC的低12位）*/
        crc^=crc_ta[da^(*ptr/16)]; /* CRC的高4位和本字节的前半字节相加后查表计算CRC， 然后加上上一次CRC的余数 */
        da = (crc>>12)&0x0f;  /* 暂存CRC的高4位 */
        crc<<=4;     /* CRC右移4位， 相当于CRC的低12位） */
        crc^=crc_ta[da^(*ptr&0x0f)];/* CRC的高4位和本字节的后半字节相加后查表计算CRC，然后再加上上一次CRC的余数 */
        ptr++;
    }
    return(crc);
#endif
}

// 构造 32 位 CRC 表 
void CPageDebug::BuildTableCRC32( unsigned long aPoly ) 
{ 
    unsigned long i, j; 
    unsigned long nData; 
    unsigned long nAccum; 
    for ( i = 0; i < 256; i++ ) 
    {
        nData = ( unsigned long )( i << 24 ); 
        nAccum = 0; 
        for( j = 0; j < 8; j++ ) 
        { 
            if ( ( nData ^ nAccum ) & 0x80000000 ) 
                nAccum = ( nAccum << 1 ) ^ aPoly; 
            else 
                nAccum <<= 1; 
            nData <<= 1; 
        } 
        Table_CRC32[i] = nAccum; 
    } 
} 
unsigned long CPageDebug::CRC32( unsigned char * aData, unsigned long aSize )
{
    unsigned long i; 
    unsigned long nAccum = 0; 

    for ( i = 0; i < aSize; i++ ) 
    nAccum = ( nAccum << 8 ) ^ Table_CRC32[( nAccum >> 24 ) ^ *aData++]; 
    return nAccum; 
}

int WINAPI CPageDebug::PktHandleEcgProbeInfo(LPVOID pParam, UartProtocolPacket *pPacket)
{
    CPageDebug *pPageDebug = (CPageDebug *)pParam;
//    printf("ECG Probe\r\n");
    return 0;
}
















void CPageDebug::OnBnClickedBtnCalCrc()
{
	CString inputStr;//输入的原始数据
	CByteArray inputData;//存放输入转换成十六进制的数据
	unsigned char crc8Result = 0;
	unsigned short crc16Result = 0;
	unsigned long crc32Result = 0;

	UpdateData(TRUE); //读取编辑框内容
	GetDlgItem(IDC_EDIT_CRC_VAL)->GetWindowText(inputStr);
	String2Hex(inputStr,inputData);
    
    INFO("Get Count = %d, Size=%d",inputData.GetCount(), inputData.GetSize());

    crc8Result = CRC8((unsigned char *)inputData.GetData(),inputData.GetCount());
	crc16Result = CRC16((unsigned char *)inputData.GetData(),inputData.GetCount());
	crc32Result = CRC32((unsigned char *)inputData.GetData(),inputData.GetCount());

    UpdateData(FALSE); //更新编辑框内容
    INFO("\r\n=========== CRC Result Begin: ================");
    INFO("\r\n=========== CRC8   : 0X%02X",crc8Result);
    INFO("\r\n=========== CRC16  : 0X%04X",crc16Result);
    INFO("\r\n=========== CRC32  : 0X%08X",crc32Result);
    INFO("\r\n=========== CRC Result End:   ================\r\n");
}


void CPageDebug::OnBnClickedBtnDebSend()
{
    int nDataLen;
	CString inputStr;       //输入的原始数据
	CByteArray inputData;   //存放输入转换成十六进制的数据
    BYTE *pBuf = NULL;

	UpdateData(TRUE);       //读取编辑框内容
	GetDlgItem(IDC_EDIT_DEB_SEND)->GetWindowText(inputStr);
	String2Hex(inputStr,inputData);
    UpdateData(FALSE); //更新编辑框内容

    pBuf    = inputData.GetData();
    nDataLen= inputData.GetCount() -1;
    if (nDataLen < 0)
    {
        MSG("请确输入正确数据\r\n");
        return;
    }

    if (g_pSerialProtocol->isSerialOpen())
    {
        g_pSerialProtocol->sendOnePacket(pBuf[0], 0, &pBuf[1], nDataLen);
    }
    else
    {
        MSG("请确保正确配置串口\r\n");
    }
}


void CPageDebug::OnBnClickedBtnDraw()
{
}
