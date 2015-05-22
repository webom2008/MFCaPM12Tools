#pragma once
class CSerialDrive
{
public:
    CSerialDrive(void);
    ~CSerialDrive(void);
    
	bool            OpenSerialPort(CWnd* pOwner,
                            UINT portNr,
                            unsigned long nBaudRate,
                            unsigned char nByteSize,
                            unsigned char nParity,
                            unsigned char nStopBits);

    bool            CloseSerialPort();
    
    int             getSerialPortsReg(char (*pListStr)[80]);

    int             WriteSerial(const unsigned char* pData, unsigned long nLength);

    int             StartMonitoring(void);
    int             StopMonitoring(void);

    static int      RxPopResult(unsigned char *pChar);
    static int      RxGetDataLen(void);
    static void     RxBuffeerReset(void);

protected:

	static UINT     SerialReceiveThread(LPVOID pParam);
    int             ReadSerial(unsigned char* pData, unsigned long nLength);

	void		    ProcessErrorMessage(char* ErrorText);

private:
    
	CRITICAL_SECTION	m_csDriverSync; 
    CWnd*           m_pDriveOwner;
	bool            m_bSerialOpen;
	HANDLE          m_hSerial;
    CWinThread*     m_Thread;
    bool            m_bThreadRuning;
};
