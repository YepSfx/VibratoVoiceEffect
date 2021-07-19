// WaveFile.h: interface for the CWaveFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVEFILE_H__E6696322_3B40_48F8_A0B9_5A7A56D8A764__INCLUDED_)
#define AFX_WAVEFILE_H__E6696322_3B40_48F8_A0B9_5A7A56D8A764__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mmsystem.h>

enum FILEACCESS			{FILE_READ = 0 , FILE_WRITE = 1 , 
						 FILE_CLOSE	= 2};

typedef char			WAVEBUFFER;
typedef char			STRING[256];
typedef long			LONGINT;


class CWaveFile : public CObject 
{
public:
	FILEACCESS		GetAccessStatus();
	LONGINT			GetCurrentPosition();
	LONGINT			GetTotalWaveLength();
	bool			SetFile(char *FileName, FILEACCESS FileAccess);
	void			SetPosition(LONGINT pos);
	void			(*OnFileError)(MMRESULT ErrCode, char *pErr, CWnd *pParent);
	void			SetWaveFormat(WAVEFORMATEX fmt);
	char*			GetFileName();
	char*			GetFileErrorComment(MMRESULT ErrCode);
	long			Read(WAVEBUFFER *pBuffer, LONGINT count);
	long			Write(WAVEBUFFER *pBuffer, LONGINT count);
	WAVEFORMATEX	GetWaveFormat();
	CWaveFile();
	CWaveFile(CWnd *pParent);
	virtual ~CWaveFile();

private:
	FILEACCESS		m_AccStatus;
	LONGINT			m_DataOffSet;
	LONGINT			m_TotalWaveLength;
	CWnd			*m_pParent;
	HMMIO			m_hMMIO;
	MMCKINFO		m_ChkParentInfo;
	MMCKINFO		m_ChkChildInfo;
	WAVEFORMATEX	m_WaveFormat;
protected:
	STRING			m_FileName;
	long			Seek(LONGINT offset, LONG origin);
	void			ProcFileError(MMRESULT ErrCode,char *pErr);
};

#endif // !defined(AFX_WAVEFILE_H__E6696322_3B40_48F8_A0B9_5A7A56D8A764__INCLUDED_)
