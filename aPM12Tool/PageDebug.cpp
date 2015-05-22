// PageDebug.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "aPM12Tool.h"
#include "PageDebug.h"
#include "afxdialogex.h"

// CPageDebug �Ի���
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


// CPageDebug ��Ϣ�������

BOOL CPageDebug::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    GetDlgItem(IDC_STATIC_DEB_SEND_INFO)->SetWindowTextA(_T("��ʽ:ID+DATA[0...N] ���ݼ�ӿո�  ��:���ĵ�У׼,��д:29 01"));

    return TRUE;
}


void    CPageDebug::initApplication(void)
{
    g_pSerialProtocol->bindPaktFuncByID(AIO_TX_ECG_LEAD_INFO_ID ,this, CPageDebug::PktHandleEcgProbeInfo);

}

//����һ�����ַ�ת��Ϊ��Ӧ��ʮ������ֵ�ĺ���
//�ö�C�������϶������ҵ�
//���ܣ�������0-F֮����ַ�����ת��Ϊ��Ӧ��ʮ�������ַ������򷵻�-1
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

//�������ת�������ĸ�ʽ���ƣ��ڷ��Ϳ��е�ʮ�����ַ�Ӧ��ÿ�����ַ�֮�����һ���ո�
//�磺A1 23 45 0B 00 29
//CByteArray��һ����̬�ֽ����飬�ɲο�MSDN����
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
#if 0//CRC-8�㷨1
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
#else//CRC-8�㷨2
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
#if 0//�㷨1
    unsigned short thecrc=0x00;
    unsigned char da;
    while(len--!=0) 
    {
		da=thecrc/256;    // ��8λ������������ʽ�ݴ�CRC�ĸ�8λ 
		thecrc<<=8;     // ����8λ���൱��CRC�ĵ�8λ����256
		thecrc^=Table_CRC16[da^*ptr]; // ��8λ�͵�ǰ�ֽ���Ӻ��ٲ����CRC ���ټ�����ǰ��CRC 
		ptr++;
    }
    return (thecrc);
#else//�㷨2
//��������һ��CRC16�Ĳ���㷨���ñ��С�����ð����������CRC�ķ�������������RAM��С�ĵ�Ƭ���У���Ȼ������һ���ٶȣ�
const unsigned short crc_ta[16]=
{ /* CRC��ʽ�� */
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
        da = (crc>>12)&0x0f;  /* �ݴ�CRC�ĸ���λ */
        crc<<=4;     /* CRC����4λ���൱��ȡCRC�ĵ�12λ��*/
        crc^=crc_ta[da^(*ptr/16)]; /* CRC�ĸ�4λ�ͱ��ֽڵ�ǰ���ֽ���Ӻ������CRC�� Ȼ�������һ��CRC������ */
        da = (crc>>12)&0x0f;  /* �ݴ�CRC�ĸ�4λ */
        crc<<=4;     /* CRC����4λ�� �൱��CRC�ĵ�12λ�� */
        crc^=crc_ta[da^(*ptr&0x0f)];/* CRC�ĸ�4λ�ͱ��ֽڵĺ���ֽ���Ӻ������CRC��Ȼ���ټ�����һ��CRC������ */
        ptr++;
    }
    return(crc);
#endif
}

// ���� 32 λ CRC �� 
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
	CString inputStr;//�����ԭʼ����
	CByteArray inputData;//�������ת����ʮ�����Ƶ�����
	unsigned char crc8Result = 0;
	unsigned short crc16Result = 0;
	unsigned long crc32Result = 0;

	UpdateData(TRUE); //��ȡ�༭������
	GetDlgItem(IDC_EDIT_CRC_VAL)->GetWindowText(inputStr);
	String2Hex(inputStr,inputData);
    
    INFO("Get Count = %d, Size=%d",inputData.GetCount(), inputData.GetSize());

    crc8Result = CRC8((unsigned char *)inputData.GetData(),inputData.GetCount());
	crc16Result = CRC16((unsigned char *)inputData.GetData(),inputData.GetCount());
	crc32Result = CRC32((unsigned char *)inputData.GetData(),inputData.GetCount());

    UpdateData(FALSE); //���±༭������
    INFO("\r\n=========== CRC Result Begin: ================");
    INFO("\r\n=========== CRC8   : 0X%02X",crc8Result);
    INFO("\r\n=========== CRC16  : 0X%04X",crc16Result);
    INFO("\r\n=========== CRC32  : 0X%08X",crc32Result);
    INFO("\r\n=========== CRC Result End:   ================\r\n");
}


void CPageDebug::OnBnClickedBtnDebSend()
{
    int nDataLen;
	CString inputStr;       //�����ԭʼ����
	CByteArray inputData;   //�������ת����ʮ�����Ƶ�����
    BYTE *pBuf = NULL;

	UpdateData(TRUE);       //��ȡ�༭������
	GetDlgItem(IDC_EDIT_DEB_SEND)->GetWindowText(inputStr);
	String2Hex(inputStr,inputData);
    UpdateData(FALSE); //���±༭������

    pBuf    = inputData.GetData();
    nDataLen= inputData.GetCount() -1;
    if (nDataLen < 0)
    {
        MSG("��ȷ������ȷ����\r\n");
        return;
    }

    if (g_pSerialProtocol->isSerialOpen())
    {
        g_pSerialProtocol->sendOnePacket(pBuf[0], 0, &pBuf[1], nDataLen);
    }
    else
    {
        MSG("��ȷ����ȷ���ô���\r\n");
    }
}


void CPageDebug::OnBnClickedBtnDraw()
{
}
