#if !defined(AFX_WAVECTRL_H__FA160AF1_5492_40A6_81D1_9CAD1516964D__INCLUDED_)
#define AFX_WAVECTRL_H__FA160AF1_5492_40A6_81D1_9CAD1516964D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mmsystem.h>

#define		DEF_DEV_CLOSE		0
#define		DEF_DEV_OPEN		1	
#define		DEF_CH_MONO			1
#define		DEF_CH_STEREO		2
#define		DEF_SAM_PHONE		8000
#define		DEF_SAM_AM			11025
#define		DEF_SAM_FM			22050
#define		DEF_SAM_SYNTH		32000
#define		DEF_SAM_CD			44100
#define		DEF_RES_LOW			8
#define		DEF_RES_HIGH		16

enum		CHANNEL			{CH_MONO    = DEF_CH_MONO,   CH_STEREO = DEF_CH_STEREO};
enum		STATUS			{DEV_CLOSE  = DEF_DEV_CLOSE, DEV_OPEN  = DEF_DEV_OPEN};
enum		RESOLUTION		{RES_LOW	= DEF_RES_LOW,   
							 RES_HIGH   = DEF_RES_HIGH,
							 RES_USER						};
enum		SAMPLE			{SAM_PHONE	= DEF_SAM_PHONE,
							 SAM_AM		= DEF_SAM_AM,
							 SAM_FM		= DEF_SAM_FM,
							 SAM_SYNTH	= DEF_SAM_SYNTH,
							 SAM_CD		= DEF_SAM_CD,
							 SAM_USER						};
enum		HANDLETYPE		{HANDLE_CALLBACK, HANDLE_WINDOW};
enum		MEMERROR		{ERROR_FREE, ERROR_PREPARE};
enum		PROCPRIORITY	{NORMAL = 0, REALTIME = 1, HIGH = 2};

typedef char			WAVEBUFFER;
typedef char			STRING[256];

// WaveCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWaveIn window

class CWaveIn : public CWnd
{

// Construction
public:
	CWaveIn();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveIn)
	public:

	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	WAVEFORMATEX GetWaveFormat();
	void (*OnWaveData)(WAVEBUFFER *pBuffer, int Len, CWnd *pParent);
	void (*OnWaveError)(MMRESULT ErrorCode, CWnd *pParent);
	void (*OnMemError)(enum MEMERROR Err, CWnd *pParent);
	void Create(CWnd* pParent);
	void ResetDevice();
	void SetWaveFormat(WAVEFORMATEX fmt);
	void SetWaveFormat();
	void SetUserSampleRate(int sam);
	void SetUserResolution(int res);	
	void SetDeviceStatus(enum STATUS DevStatus);
	void SetInitBufferNum(int num);
	void SetDeviceID(int DevID);
	void SetChannel(enum CHANNEL ch);
	void SetSampleRate(enum SAMPLE sam);
	void SetSampleResolution(enum RESOLUTION res);
	void SetBufferSize(int size);
	void SetHandleType(enum HANDLETYPE proc);
	int	 GetBufferSize();
	int  GetBufferCount();
	int  GetDeviceID();
	int  GetDeviceCount();
	int  GetUserSampleRate();
	int  GetUserResolution();
	enum PROCPRIORITY	GetProcPriority();
	enum RESOLUTION		GetSampleResolution();
	enum CHANNEL		GetChannel();
	enum STATUS			GetDeviceStatus();
	enum HANDLETYPE		GetHandleType();
	enum SAMPLE			GetSampleRate();
	bool GetCurrentPosition(MMTIME *pMMTime);
	bool SetProcPriority(enum PROCPRIORITY priority);
	char * GetDeviceErrorComment(MMRESULT ErrCode);
	char * GetDeviceName(int DevID);
	virtual ~CWaveIn();
	// Generated message map functions
protected:
	bool FreeHDR(WAVEHDR *pHdr,MMRESULT *ChkCode);
	bool PrepareHDR(MMRESULT *ChkCode);
	void ProcWaveError(MMRESULT ErrorCode);
	void ProcWaveData(WAVEBUFFER *pBuffer, int Len);
	void ProcMemError(enum MEMERROR Err);
	void ProcCallBack(UINT uMsg, DWORD dwParam1, DWORD dwParam2);
	//{{AFX_MSG(CWaveIn)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int					m_DeviceID;
	int					m_BufferSize;
	int					m_BufferCount;
	int					m_InitBufferNum;
	int					m_UserSampleRate;
	int					m_UserResolution;
	bool				m_AttemptClose;
	enum HANDLETYPE		m_HandleType;
	enum STATUS			m_DeviceStatus;
	enum RESOLUTION		m_Resolution;
	enum SAMPLE			m_Sample;
	enum CHANNEL		m_Channel;
	enum PROCPRIORITY	m_ProcPriority;
	WAVEFORMATEX		m_RecWaveFormat;
	HWAVEIN				m_hDevice;
	CWnd				*m_pParent;
	DWORD				m_OriginalPriority;
    
	static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CWaveOut window

class CWaveOut : public CWnd
{
// Construction
public:
	CWaveOut();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveOut)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	WAVEFORMATEX GetWaveFormat();
	void (*OnWaveDataDone)(WAVEBUFFER *pBuffer, int Len, CWnd *pParent);
	void (*OnWaveError)(MMRESULT ErrorCode, CWnd *pParent);
	void (*OnMemError)(enum MEMERROR Err, CWnd *pParent);
	void Create(CWnd* pParent);	
	void SetWaveFormat();
	void SetWaveFormat(WAVEFORMATEX fmt);
	void SetDeviceStatus(enum STATUS DevStatus);
	void SetSampleResolution(enum RESOLUTION res);	
	void SetSampleRate(enum SAMPLE sam);
	void SetHandleType(enum HANDLETYPE proc);
	void SetChannel(enum CHANNEL ch);
	void SetDeviceID(int DevID);
	void ResetDevice();
	void SetUserSampleRate(int sam);
	void SetUserResolution(int res);	
	bool SetHDR(WAVEBUFFER *pBuffer, int Len, MMRESULT *ChkCode);
	bool GetCurrentPosition(MMTIME *pMMTime);
	bool SetProcPriority(enum PROCPRIORITY priority);
	enum PROCPRIORITY	GetProcPriority();	
	enum RESOLUTION		GetSampleResolution();
	enum SAMPLE			GetSampleRate();	
	enum HANDLETYPE		GetHandleType();
	enum STATUS			GetDeviceStatus();	
	enum CHANNEL		GetChannel();
	int  GetUserSampleRate();
	int  GetUserResolution();
	int  GetDeviceID();
	int  GetDeviceCount();
	int  GetBufferCount();	
	char * GetDeviceName(int DevID);
	char * GetDeviceErrorComment(MMRESULT ErrCode);
	virtual ~CWaveOut();

	// Generated message map functions
protected:
	void ProcessWaveDataDone(WAVEBUFFER *pBuffer, int Len);
	bool FreeHDR(WAVEHDR *pHdr,MMRESULT *ChkCode);
	void ProcWaveError(MMRESULT ErrorCode);
	void ProcMemError(enum MEMERROR Err);
	void ProcCallBack(UINT uMsg, DWORD dwParam1, DWORD dwParam2);
	//{{AFX_MSG(CWaveOut)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int					m_UserResolution;
	int					m_UserSampleRate;
	int					m_DeviceID;
	int					m_BufferCount;
	bool				m_AttemptClose;
	enum HANDLETYPE		m_HandleType;
	enum STATUS			m_DeviceStatus;
	enum RESOLUTION		m_Resolution;
	enum SAMPLE			m_Sample;
	enum CHANNEL		m_Channel;
	enum PROCPRIORITY	m_ProcPriority;
	WAVEFORMATEX		m_PlayWaveFormat;
	HWAVEOUT			m_hDevice;
	DWORD				m_OriginalPriority;
	CWnd				*m_pParent;

	static void CALLBACK waveOutProc(HWAVEOUT hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
};

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVECTRL_H__FA160AF1_5492_40A6_81D1_9CAD1516964D__INCLUDED_)
