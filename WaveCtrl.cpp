/*
*************************************************************************
		Win32S/WinNT Wave I/O Handling Routine
			Jan 19, 2005
		
		  Copyright Reserved to
		 
		  Chulwoong Jeon (chulwoongjeon@hotmail.com)
		  Total Solutions.,LTD

		  Version 1.08	
*************************************************************************
*/
// WaveCtrl.cpp : implementation file
//
#include "stdafx.h"
#include "WaveCtrl.h"
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment(lib, "winmm.lib")

#define		BUFFERNUM	5;
#define		BUFFERLEN	320;

void AppProcessMessages()
{    
	MSG	uMsg;

	while (PeekMessage(&uMsg, 0, 0, 0, PM_REMOVE))
	{
          TranslateMessage(&uMsg);
          DispatchMessage(&uMsg);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CWaveIn

CWaveIn::CWaveIn()
{
	m_DeviceID		= 0;
	m_DeviceStatus	= DEV_CLOSE;
	m_Channel		= CH_MONO;
	m_Sample		= SAM_PHONE;
	m_Resolution	= RES_LOW;
	m_HandleType	= HANDLE_WINDOW;	
	m_AttemptClose	= false;
	m_InitBufferNum = BUFFERNUM;
	m_BufferSize	= BUFFERLEN;
	m_BufferCount	= 0;
	m_UserSampleRate= SAM_PHONE;
	m_UserResolution= RES_LOW; 
	m_ProcPriority	= NORMAL;

	OnWaveData		= NULL;
	OnWaveError		= NULL;
	OnMemError		= NULL;

	SetWaveFormat();
}

CWaveIn::~CWaveIn()
{
	if (m_DeviceStatus == DEV_OPEN)
	{	
		SetDeviceStatus(DEV_CLOSE);
		AppProcessMessages();
	}

}

BEGIN_MESSAGE_MAP(CWaveIn, CWnd)
	//{{AFX_MSG_MAP(CWaveIn)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWaveIn message handlers

void CALLBACK CWaveIn::waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	((CWaveIn*)dwInstance)->ProcCallBack(uMsg,dwParam1,dwParam2);
}

void CWaveIn::Create(CWnd* pParent) 
{
	// TODO: Add your specialized code here and/or call the base class
	m_pParent = pParent;
	
	CWnd::Create(NULL, NULL, WS_CHILD,CRect(0, 0, 0, 0), pParent, 1234);
}

enum RESOLUTION CWaveIn::GetSampleResolution()
{
	return	m_Resolution;
}

void CWaveIn::SetSampleResolution(enum RESOLUTION res)
{
	m_Resolution = res;
	m_UserResolution = (int)res;
	SetWaveFormat();
}

void CWaveIn::SetSampleRate(enum SAMPLE sam)
{
	m_Sample = sam;
	m_UserSampleRate = (int)sam;
	SetWaveFormat();
}

enum SAMPLE CWaveIn::GetSampleRate()
{
	return m_Sample;
}

void CWaveIn::SetChannel(enum CHANNEL ch)
{
	m_Channel = ch;
	SetWaveFormat();
}

enum CHANNEL CWaveIn::GetChannel()
{
	return m_Channel;
}

void CWaveIn::SetWaveFormat()
{

	m_RecWaveFormat.wFormatTag		= WAVE_FORMAT_PCM;
	m_RecWaveFormat.nChannels		= m_Channel;	
	
	if (m_Sample != SAM_USER)
		m_RecWaveFormat.nSamplesPerSec	= m_Sample;	
	else
		m_RecWaveFormat.nSamplesPerSec	= m_UserSampleRate;	

	if (m_Resolution != RES_USER)

		m_RecWaveFormat.wBitsPerSample	= m_Resolution;
	else
		m_RecWaveFormat.wBitsPerSample	= m_UserResolution;

		m_RecWaveFormat.nAvgBytesPerSec	= m_RecWaveFormat.nSamplesPerSec * 
										  m_RecWaveFormat.wBitsPerSample * 
										  m_RecWaveFormat.nChannels / 8;
		m_RecWaveFormat.nBlockAlign		= m_RecWaveFormat.nChannels * 
										  m_RecWaveFormat.wBitsPerSample /8;
		m_RecWaveFormat.cbSize			= 0;
}

void CWaveIn::SetDeviceID(int DevID)
{
	m_DeviceID = DevID;
}

int CWaveIn::GetDeviceID()
{
	return m_DeviceID;
}

char * CWaveIn::GetDeviceName(int DevID)
{
	WAVEINCAPS	WaveCaps;
	
	waveInGetDevCaps(DevID,&WaveCaps,sizeof(WAVEINCAPS));
		
	return WaveCaps.szPname;
}

char* CWaveIn::GetDeviceErrorComment(MMRESULT ErrCode)
{
	MMRESULT	ExResult;
	STRING		tmp;

	ExResult = waveInGetErrorText(ErrCode,tmp,sizeof(tmp));

	if (ExResult != MMSYSERR_NOERROR)
	{
		switch(ExResult)
		{
			case MMSYSERR_BADERRNUM:
				return  "Specified error number is out of range.";
				break;
			case MMSYSERR_NODRIVER:
				return "No device driver is present.";
				break;
			case MMSYSERR_NOMEM:
				return "Unable to allocate or lock memory.";
				break;
		}
	}
	return tmp;
}

void CWaveIn::SetHandleType(enum HANDLETYPE proc)
{
	m_HandleType = proc;
}

enum HANDLETYPE CWaveIn::GetHandleType()
{
	return m_HandleType;
}

void CWaveIn::ProcWaveData(WAVEBUFFER *pBuffer, int Len)
{
	if (OnWaveData != NULL)
			OnWaveData(pBuffer, Len, m_pParent);
}

void CWaveIn::ProcWaveError(MMRESULT ErrorCode)
{
	if (OnWaveError != NULL)
			OnWaveError(ErrorCode, m_pParent);
}

void CWaveIn::ProcMemError(enum MEMERROR Err)
{
	if (OnMemError != NULL)
			OnMemError(Err,m_pParent);
}

void CWaveIn::SetInitBufferNum(int num)
{
	m_InitBufferNum = num;
}

int CWaveIn::GetBufferCount()
{
	return m_BufferCount;
}

void CWaveIn::SetBufferSize(int size)
{
	m_BufferSize = size;
}

int CWaveIn::GetBufferSize()
{
	return m_BufferSize;
}

bool CWaveIn::PrepareHDR(MMRESULT *ChkCode)
{
	MMRESULT	ExResult;
	WAVEHDR		*pHdr;
	WAVEBUFFER	*pBuffer;

	pHdr	= (WAVEHDR*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*pHdr));
	pBuffer = (WAVEBUFFER*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,m_BufferSize);
			
	if ((pHdr == NULL)||(pBuffer == NULL))
	{
		*ChkCode = MMSYSERR_NOERROR;
		return false;
	}
			
	pHdr->lpData			= pBuffer;
	pHdr->dwBufferLength	= m_BufferSize;
	pHdr->dwFlags			= 0;
			
	ExResult = waveInPrepareHeader(m_hDevice,pHdr,sizeof(*pHdr));
	if (ExResult != MMSYSERR_NOERROR)
	{
		HeapFree(GetProcessHeap(),0,pHdr->lpData);
		HeapFree(GetProcessHeap(),0,pHdr);

		*ChkCode = ExResult;
		return false;
	}
		
	ExResult = waveInAddBuffer(m_hDevice,pHdr,sizeof(*pHdr));
	if (ExResult != MMSYSERR_NOERROR)
	{
		HeapFree(GetProcessHeap(),0,pHdr->lpData);
		HeapFree(GetProcessHeap(),0,pHdr);

		*ChkCode = ExResult;
		return false;
	}
	*ChkCode = ExResult;
			
	m_BufferCount++;

	return true;
}

bool CWaveIn::FreeHDR(WAVEHDR *pHdr,MMRESULT *ChkCode)
{
	MMRESULT	ExResult;

	ExResult = waveInUnprepareHeader(m_hDevice,pHdr,sizeof(*pHdr));
	if (ExResult != MMSYSERR_NOERROR)
	{
		HeapFree(GetProcessHeap(),0,pHdr->lpData);
		HeapFree(GetProcessHeap(),0,pHdr);

		*ChkCode = ExResult;
		return false;
	}
	*ChkCode = ExResult;
			
	HeapFree(GetProcessHeap(),0,pHdr->lpData);
	HeapFree(GetProcessHeap(),0,pHdr);
			
	m_BufferCount--;
	
	return true;
}

enum STATUS CWaveIn::GetDeviceStatus()
{
	return m_DeviceStatus;
}

void CWaveIn::SetDeviceStatus(enum STATUS DevStatus)
{
	MMRESULT	ExResult;
	int			i;
	HANDLE		ThisProcHandle;
	
	switch (DevStatus){
		case DEV_OPEN:
			if (m_DeviceStatus == DEV_OPEN)
				return;

			if (m_AttemptClose == true)
				return;
			
			switch(m_HandleType){
				case HANDLE_CALLBACK:
					ExResult = waveInOpen(&m_hDevice, m_DeviceID, &m_RecWaveFormat, (DWORD)waveInProc,(DWORD)this, CALLBACK_FUNCTION);
					if (ExResult != MMSYSERR_NOERROR)
						ProcWaveError(ExResult);
					break;
				case HANDLE_WINDOW:
					ExResult = waveInOpen(&m_hDevice, m_DeviceID, &m_RecWaveFormat, (DWORD)m_hWnd, 0, CALLBACK_WINDOW);
					if (ExResult != MMSYSERR_NOERROR)
						ProcWaveError(ExResult);
					break;
			}
			AppProcessMessages();
				
			for ( i= 0 ; i < m_InitBufferNum ; i++)
			{
				if (PrepareHDR(&ExResult) != true)
						ProcWaveError(ExResult);
			}
			
			if (m_ProcPriority == REALTIME)
			{
				ThisProcHandle		= OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
				m_OriginalPriority	= GetPriorityClass(ThisProcHandle);
				CloseHandle(ThisProcHandle);	

				ThisProcHandle = OpenProcess(PROCESS_SET_INFORMATION, FALSE, GetCurrentProcessId());			
				SetPriorityClass(ThisProcHandle,REALTIME_PRIORITY_CLASS);
				CloseHandle(ThisProcHandle);			
			}else if (m_ProcPriority == HIGH)
			{
				ThisProcHandle		= OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
				m_OriginalPriority	= GetPriorityClass(ThisProcHandle);
				CloseHandle(ThisProcHandle);	

				ThisProcHandle = OpenProcess(PROCESS_SET_INFORMATION, FALSE, GetCurrentProcessId());			
				SetPriorityClass(ThisProcHandle,HIGH_PRIORITY_CLASS);
				CloseHandle(ThisProcHandle);			
			}

			ExResult = waveInStart(m_hDevice);
			if (ExResult != MMSYSERR_NOERROR)
						ProcWaveError(ExResult);
			break;
		case DEV_CLOSE:
			switch(m_HandleType){
				case HANDLE_WINDOW:
					if (m_DeviceStatus == DEV_CLOSE)
						return;

					if (m_AttemptClose == true)
						return;
	
					m_AttemptClose = true;

					while(m_BufferCount != 0)
					{
						AppProcessMessages();
					}

					ExResult = waveInStop(m_hDevice);
					if (ExResult != MMSYSERR_NOERROR)
							ProcWaveError(ExResult);
					
					ExResult = waveInReset(m_hDevice);
					if (ExResult != MMSYSERR_NOERROR)
								ProcWaveError(ExResult);
					
					while(m_BufferCount != 0)
					{
						AppProcessMessages();
					}

					ExResult = waveInClose(m_hDevice);
					if (ExResult != MMSYSERR_NOERROR)
							ProcWaveError(ExResult);

					break;
				case HANDLE_CALLBACK:
					if (m_DeviceStatus == DEV_CLOSE)
						return;

					if (m_AttemptClose == true)
						return;
	
					m_AttemptClose = true;

					while(m_BufferCount != 0)
					{
						AppProcessMessages();
					}

					ExResult = waveInStop(m_hDevice);
					if (ExResult != MMSYSERR_NOERROR)
						ProcWaveError(ExResult);

					ExResult = waveInReset(m_hDevice);
					if (ExResult != MMSYSERR_NOERROR)
						ProcWaveError(ExResult);

					while(m_BufferCount != 0)
					{
						AppProcessMessages();
					}

					ExResult = waveInClose(m_hDevice);
					if (ExResult != MMSYSERR_NOERROR)
						ProcWaveError(ExResult);

					break;
				}

			if (m_ProcPriority == REALTIME)
			{
				ThisProcHandle = OpenProcess(PROCESS_SET_INFORMATION, FALSE, GetCurrentProcessId());			
				SetPriorityClass(ThisProcHandle, m_OriginalPriority);
				CloseHandle(ThisProcHandle);	
			}else if (m_ProcPriority == HIGH)
			{
				ThisProcHandle = OpenProcess(PROCESS_SET_INFORMATION, FALSE, GetCurrentProcessId());			
				SetPriorityClass(ThisProcHandle, m_OriginalPriority);
				CloseHandle(ThisProcHandle);	
			}
			break;
	}
}

LRESULT CWaveIn::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	MMRESULT	ExResult;
	WAVEBUFFER	*pBuffer;
	WAVEHDR		*pHdr;
	int			Len = 0;
	
	switch(message){
		case MM_WIM_OPEN:
			
			m_DeviceStatus = DEV_OPEN;
			
			break;
		case MM_WIM_CLOSE:
			
			m_DeviceStatus = DEV_CLOSE;
			m_AttemptClose = false;
			
			break;
		case MM_WIM_DATA:
			
			pHdr	= (WAVEHDR*)lParam;
			pBuffer = pHdr->lpData;
			Len		= pHdr->dwBufferLength;
			
			if (m_AttemptClose != true)
			{
				if (PrepareHDR(&ExResult) != true)
				{	
					if (ExResult == MMSYSERR_NOERROR)
						ProcMemError(ERROR_PREPARE);
					else
						ProcWaveError(ExResult);
				}
		
				ProcWaveData(pBuffer,Len);

				if (FreeHDR(pHdr,&ExResult) !=true)
				{	
					if (ExResult == MMSYSERR_NOERROR)
						ProcMemError(ERROR_FREE);					
					else 
						ProcWaveError(ExResult);
				}
			}else{
				if (FreeHDR(pHdr,&ExResult) !=true)
				{	
					if (ExResult == MMSYSERR_NOERROR)
						ProcMemError(ERROR_FREE);					
					else 
						ProcWaveError(ExResult);
				}
			}
		
			break;
	}
	
	return CWnd::WindowProc(message, wParam, lParam);
}

void CWaveIn::ProcCallBack(UINT uMsg, DWORD dwParam1, DWORD dwParam2)
{
	MMRESULT	ExResult;
	WAVEBUFFER	*pBuffer;
	WAVEHDR		*pHdr;
	int			Len = 0;
	
	switch(uMsg){
		case WIM_OPEN:
			
			m_DeviceStatus = DEV_OPEN;
			
			break;
		case WIM_CLOSE:
			
			m_DeviceStatus = DEV_CLOSE;
			m_AttemptClose = false;
			
			break;
		case WIM_DATA:
			
			pHdr	= (WAVEHDR*)dwParam1;
			pBuffer = pHdr->lpData;
			Len		= pHdr->dwBufferLength;
			
			if (m_AttemptClose != true)
			{
				if (PrepareHDR(&ExResult) != true)
				{	
					if (ExResult == MMSYSERR_NOERROR)
						ProcMemError(ERROR_PREPARE);
					else
						ProcWaveError(ExResult);
				}
		
				ProcWaveData(pBuffer,Len);

				if (FreeHDR(pHdr,&ExResult) !=true)
				{	
					if (ExResult == MMSYSERR_NOERROR)
						ProcMemError(ERROR_FREE);					
					else 
						ProcWaveError(ExResult);
				}
			}else{
				if (FreeHDR(pHdr,&ExResult) !=true)
				{	
					if (ExResult == MMSYSERR_NOERROR)
						ProcMemError(ERROR_FREE);					
					else 
						ProcWaveError(ExResult);
				}

			}
			
			break;
	}
}

void CWaveIn::ResetDevice()
{
	MMRESULT	ExResult;

	if (m_DeviceStatus == DEV_CLOSE)
		return;

	if (m_AttemptClose == true)
		return;

	m_AttemptClose = true;

	while(m_BufferCount != 0)
	{
		AppProcessMessages();
	}

	ExResult = waveInStop(m_hDevice);
	if (ExResult != MMSYSERR_NOERROR)
			ProcWaveError(ExResult);

	ExResult = waveInReset(m_hDevice);
	if (ExResult != MMSYSERR_NOERROR)
			ProcWaveError(ExResult);

	while(m_BufferCount != 0)
	{
		AppProcessMessages();
	}

	ExResult = waveInClose(m_hDevice);
	if (ExResult != MMSYSERR_NOERROR)
				ProcWaveError(ExResult);

}

int CWaveIn::GetDeviceCount()
{
	return waveInGetNumDevs();
}

void CWaveIn::SetUserSampleRate(int sam)
{
	m_UserSampleRate = sam;
	SetWaveFormat();
}

void CWaveIn::SetUserResolution(int res)
{
	m_UserResolution = res;
	SetWaveFormat();
}

int CWaveIn::GetUserSampleRate()
{
	return	m_UserSampleRate;
}

int CWaveIn::GetUserResolution()
{
	return	m_UserResolution;
}

void CWaveIn::SetWaveFormat(WAVEFORMATEX fmt)
{
	m_RecWaveFormat	= fmt;
	
	switch(fmt.nChannels){
		case 1:	m_Channel = CH_MONO;
			break;
		case 2:	m_Channel = CH_STEREO;
			break;
	}

	m_Resolution	 = RES_USER;
	m_UserResolution = fmt.wBitsPerSample;

	m_Sample		 = SAM_USER;
	m_UserSampleRate = fmt.nSamplesPerSec;
}

WAVEFORMATEX CWaveIn::GetWaveFormat()
{
	return m_RecWaveFormat;
}

bool CWaveIn::GetCurrentPosition(MMTIME *pMMTime)
{
	MMRESULT	ExResult;

	ExResult = waveInGetPosition(m_hDevice,pMMTime,sizeof(*pMMTime));
	
	if (ExResult != MMSYSERR_NOERROR)
	{
		ProcWaveError(ExResult);
		return false;
	}
	return true;
}

enum PROCPRIORITY CWaveIn::GetProcPriority()
{
	return m_ProcPriority;
}

bool CWaveIn::SetProcPriority(enum PROCPRIORITY priority)
{
	if (m_DeviceStatus == DEV_OPEN)
		return false;

	m_ProcPriority = priority;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CWaveOut

CWaveOut::CWaveOut()
{
	m_DeviceID		= 0;
	m_DeviceStatus	= DEV_CLOSE;
	m_Channel		= CH_MONO;
	m_Sample		= SAM_PHONE;
	m_Resolution	= RES_LOW;
	m_HandleType	= HANDLE_WINDOW;	
	m_AttemptClose	= false;
	m_BufferCount	= 0;
	m_UserSampleRate= SAM_PHONE;	
	m_UserResolution= RES_LOW;
	m_ProcPriority	= NORMAL;

	OnWaveDataDone	= NULL;
	OnWaveError		= NULL;
	OnMemError		= NULL;

	SetWaveFormat();
}

CWaveOut::~CWaveOut()
{
	if (m_DeviceStatus == DEV_OPEN)
	{	
		SetDeviceStatus(DEV_CLOSE);
		AppProcessMessages();
	}
}


BEGIN_MESSAGE_MAP(CWaveOut, CWnd)
	//{{AFX_MSG_MAP(CWaveOut)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWaveOut message handlers

void CALLBACK CWaveOut::waveOutProc(HWAVEOUT hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	((CWaveOut*)dwInstance)->ProcCallBack(uMsg,dwParam1,dwParam2);
}

void CWaveOut::SetWaveFormat()
{
	m_PlayWaveFormat.wFormatTag		= WAVE_FORMAT_PCM;
	m_PlayWaveFormat.nChannels		= m_Channel;	
	
	if (m_Sample != SAM_USER)
		m_PlayWaveFormat.nSamplesPerSec	= m_Sample;	
	else
		m_PlayWaveFormat.nSamplesPerSec	= m_UserSampleRate;	

	if (m_Resolution != RES_USER)

		m_PlayWaveFormat.wBitsPerSample	= m_Resolution;
	else
		m_PlayWaveFormat.wBitsPerSample	= m_UserResolution;

		m_PlayWaveFormat.nAvgBytesPerSec= m_PlayWaveFormat.nSamplesPerSec * 
										  m_PlayWaveFormat.wBitsPerSample * 
										  m_PlayWaveFormat.nChannels / 8;
		m_PlayWaveFormat.nBlockAlign	= m_PlayWaveFormat.nChannels * 
										  m_PlayWaveFormat.wBitsPerSample /8;
		m_PlayWaveFormat.cbSize			= 0;
}

void CWaveOut::Create(CWnd* pParent) 
{
	// TODO: Add your specialized code here and/or call the base class
	m_pParent = pParent;
	
	CWnd::Create(NULL, NULL, WS_CHILD,CRect(0, 0, 0, 0), pParent, 1234);
}

char* CWaveOut::GetDeviceErrorComment(MMRESULT ErrCode)
{
	MMRESULT	ExResult;
	STRING		tmp;

	ExResult = waveOutGetErrorText(ErrCode,tmp,sizeof(tmp));

	if (ExResult != MMSYSERR_NOERROR)
	{
		switch(ExResult)
		{
			case MMSYSERR_BADERRNUM:
				return  "Specified error number is out of range.";
				break;
			case MMSYSERR_NODRIVER:
				return "No device driver is present.";
				break;
			case MMSYSERR_NOMEM:
				return "Unable to allocate or lock memory.";
				break;
		}
	}
	return tmp;
}

char * CWaveOut::GetDeviceName(int DevID)
{
	WAVEOUTCAPS	WaveCaps;
	
	waveOutGetDevCaps(DevID,&WaveCaps,sizeof(WAVEOUTCAPS));
		
	return WaveCaps.szPname;
}

int CWaveOut::GetBufferCount()
{
	return m_BufferCount;
}

enum CHANNEL CWaveOut::GetChannel()
{
	return m_Channel;
}

int CWaveOut::GetDeviceCount()
{
	return waveInGetNumDevs();
}

int CWaveOut::GetDeviceID()
{
	return m_DeviceID;
}

enum STATUS CWaveOut::GetDeviceStatus()
{
	return m_DeviceStatus;
}

enum HANDLETYPE CWaveOut::GetHandleType()
{
	return m_HandleType;
}

enum SAMPLE CWaveOut::GetSampleRate()
{
	return m_Sample;
}

enum RESOLUTION CWaveOut::GetSampleResolution()
{
	return	m_Resolution;
}

void CWaveOut::ResetDevice()
{
	MMRESULT	ExResult;

	if (m_DeviceStatus == DEV_CLOSE)
		return;

	if (m_AttemptClose == true)
		return;

	m_AttemptClose = true;

	while(m_BufferCount != 0)
	{
		AppProcessMessages();
	}

	ExResult = waveOutReset(m_hDevice);
	if (ExResult != MMSYSERR_NOERROR)
			ProcWaveError(ExResult);

	while(m_BufferCount != 0)
	{
		AppProcessMessages();
	}

	ExResult = waveOutClose(m_hDevice);
	if (ExResult != MMSYSERR_NOERROR)
				ProcWaveError(ExResult);
}

void CWaveOut::ProcWaveError(MMRESULT ErrorCode)
{
	if (OnWaveError != NULL)
			OnWaveError(ErrorCode, m_pParent);
}

void CWaveOut::SetChannel(enum CHANNEL ch)
{
	m_Channel = ch;
	SetWaveFormat();
}

void CWaveOut::SetDeviceID(int DevID)
{
	m_DeviceID = DevID;
}

void CWaveOut::SetHandleType(enum HANDLETYPE proc)
{
	m_HandleType = proc;
}

void CWaveOut::SetSampleRate(enum SAMPLE sam)
{
	m_Sample = sam;
	m_UserSampleRate = (int)sam;
	SetWaveFormat();
}

void CWaveOut::SetSampleResolution(enum RESOLUTION res)
{
	m_Resolution = res;
	m_UserResolution = (int)res;
	SetWaveFormat();
}

bool CWaveOut::FreeHDR(WAVEHDR *pHdr,MMRESULT *ChkCode)
{
	MMRESULT	ExResult;

	ExResult = waveOutUnprepareHeader(m_hDevice,pHdr,sizeof(*pHdr));
	if (ExResult != MMSYSERR_NOERROR)
	{
		HeapFree(GetProcessHeap(),0,pHdr->lpData);
		HeapFree(GetProcessHeap(),0,pHdr);

		*ChkCode = ExResult;
		return false;
	}
	*ChkCode = ExResult;

	HeapFree(GetProcessHeap(),0,pHdr->lpData);
	HeapFree(GetProcessHeap(),0,pHdr);

	m_BufferCount--;
	
	return true;
}

void CWaveOut::SetDeviceStatus(enum STATUS DevStatus)
{
	MMRESULT	ExResult;
	HANDLE		ThisProcHandle;	

	switch (DevStatus){
		case DEV_OPEN:
			if (m_DeviceStatus == DEV_OPEN)
				return;

			if (m_AttemptClose == true)
				return;

			switch(m_HandleType){
				case HANDLE_CALLBACK:
					ExResult = waveOutOpen(&m_hDevice, m_DeviceID, &m_PlayWaveFormat, (DWORD)waveOutProc,(DWORD)this, CALLBACK_FUNCTION);
					if (ExResult != MMSYSERR_NOERROR)
						ProcWaveError(ExResult);
					break;
				case HANDLE_WINDOW:
					ExResult = waveOutOpen(&m_hDevice, m_DeviceID, &m_PlayWaveFormat, (DWORD)m_hWnd, 0, CALLBACK_WINDOW);
					if (ExResult != MMSYSERR_NOERROR)
						ProcWaveError(ExResult);
					break;
			}

			if (m_ProcPriority == REALTIME)
			{
				ThisProcHandle		= OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
				m_OriginalPriority	= GetPriorityClass(ThisProcHandle);
				CloseHandle(ThisProcHandle);	

				ThisProcHandle = OpenProcess(PROCESS_SET_INFORMATION, FALSE, GetCurrentProcessId());			
				SetPriorityClass(ThisProcHandle,REALTIME_PRIORITY_CLASS);
				CloseHandle(ThisProcHandle);			
			}else if (m_ProcPriority == HIGH)
			{
				ThisProcHandle		= OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
				m_OriginalPriority	= GetPriorityClass(ThisProcHandle);
				CloseHandle(ThisProcHandle);	

				ThisProcHandle = OpenProcess(PROCESS_SET_INFORMATION, FALSE, GetCurrentProcessId());			
				SetPriorityClass(ThisProcHandle,HIGH_PRIORITY_CLASS);
				CloseHandle(ThisProcHandle);			
			}
			
			AppProcessMessages();
			
			break;
		case DEV_CLOSE:
				
			switch(m_HandleType){
				case HANDLE_WINDOW:
					if (m_DeviceStatus == DEV_CLOSE)
						return;

					if (m_AttemptClose == true)
						return;
	
					m_AttemptClose = true;

					while(m_BufferCount != 0)
					{
						AppProcessMessages();
					}
					
					ExResult = waveOutReset(m_hDevice);
					if (ExResult != MMSYSERR_NOERROR)
								ProcWaveError(ExResult);
					
					AppProcessMessages();

					ExResult = waveOutClose(m_hDevice);
					if (ExResult != MMSYSERR_NOERROR)
							ProcWaveError(ExResult);

				break;
				case HANDLE_CALLBACK:
					if (m_DeviceStatus == DEV_CLOSE)
						return;

					if (m_AttemptClose == true)
						return;
	
					m_AttemptClose = true;

					while(m_BufferCount != 0)
					{
						AppProcessMessages();
					}

					ExResult = waveOutReset(m_hDevice);
					if (ExResult != MMSYSERR_NOERROR)
						ProcWaveError(ExResult);

					AppProcessMessages();

					ExResult = waveOutClose(m_hDevice);
					if (ExResult != MMSYSERR_NOERROR)
						ProcWaveError(ExResult);

					break;
			}

			if (m_ProcPriority == REALTIME)
			{
				ThisProcHandle = OpenProcess(PROCESS_SET_INFORMATION, FALSE, GetCurrentProcessId());			
				SetPriorityClass(ThisProcHandle, m_OriginalPriority);
				CloseHandle(ThisProcHandle);	
			}else if (m_ProcPriority == HIGH)
			{
				ThisProcHandle = OpenProcess(PROCESS_SET_INFORMATION, FALSE, GetCurrentProcessId());			
				SetPriorityClass(ThisProcHandle, m_OriginalPriority);
				CloseHandle(ThisProcHandle);	
			}

			break;
	}
}

void CWaveOut::ProcCallBack(UINT uMsg, DWORD dwParam1, DWORD dwParam2)
{
	MMRESULT	ExResult;
	WAVEBUFFER	*pBuffer;
	WAVEHDR		*pHdr;
	int			Len = 0;
	switch(uMsg){
		case WOM_OPEN:
			
			m_DeviceStatus = DEV_OPEN;
			
			break;
		case WOM_CLOSE:
			
			m_DeviceStatus = DEV_CLOSE;
			m_AttemptClose = false;
			
			break;
		case WOM_DONE:
			
			pHdr	= (WAVEHDR*)dwParam1;
			pBuffer = pHdr->lpData;
			Len		= pHdr->dwBufferLength;

			if (m_AttemptClose != true)
				ProcessWaveDataDone(pBuffer,Len);
			
			if (FreeHDR(pHdr,&ExResult) !=true)
			{	
				if (ExResult == MMSYSERR_NOERROR)
					ProcMemError(ERROR_FREE);					
				else 
					ProcWaveError(ExResult);
			}

			break;
	}
}

LRESULT CWaveOut::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	MMRESULT	ExResult;
	WAVEBUFFER	*pBuffer;
	WAVEHDR		*pHdr;
	int			Len = 0;

	switch(message){
		case MM_WOM_OPEN:
			
			m_DeviceStatus = DEV_OPEN;
			
			break;
		case MM_WOM_CLOSE:
			
			m_DeviceStatus = DEV_CLOSE;
			m_AttemptClose = false;
			
			break;
		case MM_WOM_DONE:
			
			pHdr	= (WAVEHDR*)lParam;
			pBuffer = pHdr->lpData;
			Len		= pHdr->dwBufferLength;

			if (m_AttemptClose != true)
				ProcessWaveDataDone(pBuffer,Len);
			
			if (FreeHDR(pHdr,&ExResult) !=true)
			{	
				if (ExResult == MMSYSERR_NOERROR)
					ProcMemError(ERROR_FREE);					
				else 
					ProcWaveError(ExResult);
			}
			
			break;
	}
	
	return CWnd::WindowProc(message, wParam, lParam);
}

bool CWaveOut::SetHDR(WAVEBUFFER *pBuffer,int Len, MMRESULT *ChkCode)
{
	int			i;
	MMRESULT	ExResult;
	WAVEBUFFER	*pSetBuffer;
	WAVEHDR		*pHdr;

	if (m_AttemptClose == true)
	{
		*ChkCode = MMSYSERR_NOERROR;
		return true;
	}

	pHdr		= (WAVEHDR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*pHdr));
	pSetBuffer	= (WAVEBUFFER*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Len);

	if ((pHdr == NULL)||(pSetBuffer == NULL))
	{
		*ChkCode = MMSYSERR_NOERROR;
		return false;
	}

	for ( i = 0 ; i < Len ; i++ )
		pSetBuffer[i] = pBuffer[i];

	pHdr->lpData			= pSetBuffer;
	pHdr->dwBufferLength	= Len;
	pHdr->dwFlags			= 0;

	ExResult = waveOutPrepareHeader(m_hDevice,pHdr,sizeof(*pHdr));
	if (ExResult != MMSYSERR_NOERROR)
	{
		HeapFree(GetProcessHeap(),0,pHdr->lpData);
		HeapFree(GetProcessHeap(),0,pHdr);
		
		*ChkCode = ExResult;
		return false;
	}

	m_BufferCount++;
	
	ExResult = waveOutWrite(m_hDevice,pHdr,sizeof(*pHdr));
	if (ExResult != MMSYSERR_NOERROR)
	{
		HeapFree(GetProcessHeap(),0,pHdr->lpData);
		HeapFree(GetProcessHeap(),0,pHdr);

		*ChkCode = ExResult;
		return false;
	}
	*ChkCode = ExResult;
	
	return true;
}

void CWaveOut::SetUserSampleRate(int sam)
{
	m_UserSampleRate = sam;
	SetWaveFormat();
}

int CWaveOut::GetUserSampleRate()
{
	return	m_UserSampleRate;
}

void CWaveOut::ProcessWaveDataDone(WAVEBUFFER *pBuffer, int Len)
{
	if (OnWaveDataDone != NULL)
			OnWaveDataDone(pBuffer,Len, m_pParent);
}

void CWaveOut::SetWaveFormat(WAVEFORMATEX fmt)
{
	m_PlayWaveFormat = fmt;

	switch(fmt.nChannels){
		case 1:	m_Channel = CH_MONO;
			break;
		case 2:	m_Channel = CH_STEREO;
			break;
	}

	m_Resolution	 = RES_USER;
	m_UserResolution = fmt.wBitsPerSample;

	m_Sample		 = SAM_USER;
	m_UserSampleRate = fmt.nSamplesPerSec;
}

WAVEFORMATEX CWaveOut::GetWaveFormat()
{
	return m_PlayWaveFormat;
}

bool CWaveOut::GetCurrentPosition(MMTIME *pMMTime)
{
	MMRESULT	ExResult;

	ExResult = waveOutGetPosition(m_hDevice,pMMTime,sizeof(*pMMTime));
	
	if (ExResult != MMSYSERR_NOERROR)
	{
		ProcWaveError(ExResult);
		return false;
	}
	return true;
}

void CWaveOut::ProcMemError(enum MEMERROR Err)
{
	if (OnMemError != NULL)
			OnMemError(Err,m_pParent);
}

int CWaveOut::GetUserResolution()
{
	return	m_UserResolution;
}

void CWaveOut::SetUserResolution(int res)
{
	m_UserResolution = res;
	SetWaveFormat();
}

enum PROCPRIORITY CWaveOut::GetProcPriority()
{
	return m_ProcPriority;
}

bool CWaveOut::SetProcPriority(enum PROCPRIORITY priority)
{
	if (m_DeviceStatus == DEV_OPEN)
		return false;

	m_ProcPriority = priority;

	return true;
}
