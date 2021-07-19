/*
*************************************************************************
		Win32S/WinNT Wave File IO Handling Routine
			Dec 06, 2002
		
		  Copyright Reserved to
		 
		  Chulwoong Jeon (chulwoongjeon@hotmail.com)
		  Total Solutions.,LTD

		  Version 1.02	

		  Ver 1.00	Initial Release
		  Ver 1.01  typedef unsigned long   LONGINT -->
		            typedef long		    LONGINT
		  Ver 1.02  Bug Fix at  function    Read	
*************************************************************************
*/
// WaveFile.cpp: implementation of the CWaveFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WaveFile.h"

#include <stdio.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#pragma comment(lib, "winmm.lib")

#define		ZERO8BIT		-128

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWaveFile::CWaveFile()
{

}

CWaveFile::CWaveFile(CWnd *pParent)
{
	m_pParent = pParent;
	m_AccStatus = FILE_CLOSE;
	m_DataOffSet = 0;
	strcpy(m_FileName,"");
	m_hMMIO = 0;
	m_TotalWaveLength = 0;
	OnFileError = NULL;
}

CWaveFile::~CWaveFile()
{
	SetFile(NULL,FILE_CLOSE);
}

void CWaveFile::SetWaveFormat(WAVEFORMATEX fmt)
{
	m_WaveFormat = fmt;
}

WAVEFORMATEX CWaveFile::GetWaveFormat()
{
	return m_WaveFormat;
}

char* CWaveFile::GetFileErrorComment(MMRESULT ErrCode)
{
	STRING tmp;
	if (ErrCode < MMSYSERR_BASE + 20)
	{
		waveInGetErrorText(ErrCode,tmp,sizeof(tmp));
		return tmp;

	}else{
		switch (ErrCode){
			case MMIOERR_CHUNKNOTFOUND:
				return "Chunk cannot be found.";	
			break;
			case MMIOERR_CANNOTWRITE:
				return "The contents of the buffer could not be written to disk.";
			break;
			case MMIOERR_CANNOTSEEK:
				return "There was an error while seeking to the end of the chunk.";
			break;
			case MMIOERR_CANNOTEXPAND:
				return "The specified memory file cannot be expanded";
			break;
			case MMIOERR_OUTOFMEMORY:
				return "There was not enough memory to expand a memory file for further writing.";
			break;
			case MMIOERR_UNBUFFERED:
				return "The specified file is not opened for buffered I/O.";
			break;
			default: return "Unkown File Access Error.";
		}
	}
}

bool CWaveFile::SetFile(char *FileName, FILEACCESS FileAccess)
{
	MMRESULT ExResult;
	DWORD	 ByteRead;
	
	switch(FileAccess){
		case FILE_WRITE :
			if (m_AccStatus == FILE_WRITE)
					return true;
			
			m_hMMIO = mmioOpen(FileName, NULL, MMIO_CREATE|MMIO_READWRITE);
			strcpy(m_FileName,FileName);

			if (m_hMMIO == NULL)
			{
				m_AccStatus = FILE_CLOSE;
				ProcFileError(MMSYSERR_NOERROR,"Cannot Open Write File.");
				return false;
			}
			
			m_ChkParentInfo.fccType = mmioFOURCC('W','A','V','E');
			m_ChkParentInfo.cksize  = 0;

			ExResult = mmioCreateChunk(m_hMMIO, &m_ChkParentInfo, MMIO_CREATERIFF);
			if (ExResult != MMSYSERR_NOERROR)
			{
				ProcFileError(ExResult,"Error in Creating Parent Chunk(WAVE)." );

				if (SetFile(NULL,FILE_CLOSE) != true)
					ProcFileError(ExResult,"Fail to Close Write File.");

				m_AccStatus = FILE_CLOSE;				
				return false;
			}

			m_ChkChildInfo.ckid   = mmioFOURCC('f', 'm', 't', ' ');
			m_ChkChildInfo.cksize = sizeof(m_WaveFormat);
			
			ExResult = mmioCreateChunk(m_hMMIO,&m_ChkChildInfo,0);
			if (ExResult != MMSYSERR_NOERROR)
			{
				ProcFileError(ExResult,"Error in Creating Child Chunk(fmt)." );

				if (SetFile(NULL,FILE_CLOSE) != true)
					ProcFileError(ExResult,"Fail to Close Write File.");

				m_AccStatus = FILE_CLOSE;				
				return false;
			}
			
			if (mmioWrite(m_hMMIO,(HPSTR)&m_WaveFormat,sizeof(m_WaveFormat)) != sizeof(m_WaveFormat))
			{
				ProcFileError(MMSYSERR_NOERROR,"Error in writing Child Chunk(fmt)." );

				if (SetFile(NULL,FILE_CLOSE) != true)
					ProcFileError(ExResult,"Fail to Close Write File.");

				m_AccStatus = FILE_CLOSE;				
				return false;

			}

			ExResult = mmioAscend(m_hMMIO,&m_ChkChildInfo,0);
			if (ExResult != MMSYSERR_NOERROR)
			{
				ProcFileError(ExResult,"Error in Ascending Child Chunk(fmt)." );

				if (SetFile(NULL,FILE_CLOSE) != true)
					ProcFileError(ExResult,"Fail to Close Write File.");

				m_AccStatus = FILE_CLOSE;				
				return false;
			}

		    m_ChkChildInfo.ckid   = mmioFOURCC('d', 'a', 't', 'a');
			m_ChkChildInfo.cksize = 0;

			ExResult = mmioCreateChunk(m_hMMIO,&m_ChkChildInfo,0);
			if (ExResult != MMSYSERR_NOERROR)
			{
				ProcFileError(ExResult,"Error in Creating Child Chunk(data)." );

				if (SetFile(NULL,FILE_CLOSE) != true)
					ProcFileError(ExResult,"Fail to Close Write File.");

				m_AccStatus = FILE_CLOSE;				
				return false;
			}
			m_AccStatus = FILE_WRITE;
			break;

		case FILE_READ :
			
			if (m_AccStatus == FILE_READ)
					return true;

			m_hMMIO = mmioOpen(FileName, NULL, MMIO_READ);	
			strcpy(m_FileName,FileName);
			if (m_hMMIO == NULL)
			{
				m_AccStatus = FILE_CLOSE;
				ProcFileError(MMSYSERR_NOERROR,"Cannot Open Read File.");
				return false;
			}
			
			m_ChkParentInfo.fccType = mmioFOURCC('W','A','V','E');
			ExResult = mmioDescend(m_hMMIO,&m_ChkParentInfo,NULL,MMIO_FINDRIFF);
			if (ExResult != MMSYSERR_NOERROR)
			{
				ProcFileError(ExResult,"Error in Checking Parent Chunk(WAVE).");

				ExResult = mmioClose(m_hMMIO,0);
				if (ExResult != MMSYSERR_NOERROR)
					ProcFileError(ExResult,"Fail to Close File.");
				m_AccStatus = FILE_CLOSE;				
				return false;
			}
			
			m_ChkChildInfo.ckid = mmioFOURCC('f','m','t',' ');
			ExResult = mmioDescend(m_hMMIO,&m_ChkChildInfo,&m_ChkParentInfo,MMIO_FINDCHUNK);
			if (ExResult != MMSYSERR_NOERROR)
			{
				ProcFileError(ExResult,"Error in Checking Child Chunk(fmt).");

				ExResult = mmioClose(m_hMMIO,0);
				if (ExResult != MMSYSERR_NOERROR)
					ProcFileError(ExResult,"Fail to Close File.");
				m_AccStatus = FILE_CLOSE;
				return false;
			}
			
			ByteRead = mmioRead(m_hMMIO,(HPSTR)&m_WaveFormat,m_ChkChildInfo.cksize);
			if (ByteRead == 0)
			{
				ProcFileError(MMSYSERR_NOERROR,"Cannot read Wave Format.");

				ExResult = mmioClose(m_hMMIO,0);
				if (ExResult != MMSYSERR_NOERROR)
					ProcFileError(ExResult,"Fail to Close File.");
				m_AccStatus = FILE_CLOSE;				
				return false;
			}

			ExResult = mmioAscend(m_hMMIO,&m_ChkChildInfo,0);
			if (ExResult != MMSYSERR_NOERROR)
			{
				ProcFileError(ExResult,"Error in Ascending Child Chunk.");

				ExResult = mmioClose(m_hMMIO,0);
				if (ExResult != MMSYSERR_NOERROR)
					ProcFileError(ExResult,"Fail to Close File.");
				m_AccStatus = FILE_CLOSE;
				return false;
			}

			m_ChkChildInfo.ckid = mmioFOURCC('d','a','t','a');
			ExResult = mmioDescend(m_hMMIO,&m_ChkChildInfo,&m_ChkParentInfo,MMIO_FINDCHUNK);
			if (ExResult != MMSYSERR_NOERROR)
			{
				ProcFileError(ExResult,"Error in Descending Child Chunk(data).");

				ExResult = mmioClose(m_hMMIO,0);
				if (ExResult != MMSYSERR_NOERROR)
					ProcFileError(ExResult,"Fail to Close File.");
				m_AccStatus = FILE_CLOSE;
				return false;
			}
		
			m_TotalWaveLength	= m_ChkChildInfo.cksize;
			m_DataOffSet		= m_ChkChildInfo.dwDataOffset;

			mmioSeek(m_hMMIO,m_DataOffSet,SEEK_SET);
			m_AccStatus = FILE_READ;

			break;
		case FILE_CLOSE :
			
			if (m_AccStatus == FILE_CLOSE)
				return true;

			m_AccStatus = FILE_CLOSE;	
			
			mmioSeek( m_hMMIO, 0, SEEK_END );

			ExResult = mmioAscend(m_hMMIO, &m_ChkChildInfo, 0);
			if (ExResult != MMSYSERR_NOERROR)
				ProcFileError(ExResult,"Error in Closing(Ascend Child Chunk).");
				    
			ExResult = mmioAscend(m_hMMIO, &m_ChkParentInfo, 0);
			if (ExResult != MMSYSERR_NOERROR)
				ProcFileError(ExResult,"Error in Closing(Ascend Parent Chunk).");

			ExResult = mmioFlush(m_hMMIO,0);
			if (ExResult != MMSYSERR_NOERROR)
				ProcFileError(ExResult,"Flush Error");

			ExResult = mmioClose(m_hMMIO,0);
			if (ExResult != MMSYSERR_NOERROR)
			{
				ProcFileError(ExResult,"Fail to Close File.");
				return false;
			}
			break;
	}

	return true;
}

void CWaveFile::ProcFileError(MMRESULT ErrCode, char *pErr)
{
	if (OnFileError != NULL)
			OnFileError(ErrCode,pErr,m_pParent);

}

LONGINT CWaveFile::GetTotalWaveLength()
{
	return m_TotalWaveLength;
}

LONGINT CWaveFile::GetCurrentPosition()
{
	return Seek(0, SEEK_CUR);
}

long CWaveFile::Read(WAVEBUFFER *pBuffer, LONGINT count)
{
	LONGINT ans;
	int		i;

	ans = mmioRead(m_hMMIO,(HPSTR)pBuffer, count); 
	
	if (ans < count)
	{
		switch(m_WaveFormat.wBitsPerSample)
		{
			case 8:
				for(i = ans ; i < count ; i++)
						pBuffer[i] = ZERO8BIT;
				break;
			case 16:
				for(i = ans ; i < count ; i++)
						pBuffer[i] = 0;
				break;
		}
	}
	
	if (ans == -1)
		ProcFileError(MMSYSERR_NOERROR,"Error in Reading Wave Data.");

	return ans;
}

long CWaveFile::Seek(LONGINT offset, LONG origin)
{
	long ans;

	if (origin == SEEK_SET)
	{
		ans = (mmioSeek(m_hMMIO, offset + m_DataOffSet, origin) - m_DataOffSet);
	}else{
		ans = (mmioSeek(m_hMMIO, offset, origin) - m_DataOffSet);
	}

	if (ans == -1 )
		ProcFileError(MMSYSERR_NOERROR,"Seeking to an invalid location in the file.");

	return ans;
}

void CWaveFile::SetPosition(LONGINT pos)
{
	Seek(pos, SEEK_SET);
}

FILEACCESS CWaveFile::GetAccessStatus()
{
	return m_AccStatus;
}

char * CWaveFile::GetFileName()
{
	return m_FileName;
}

long CWaveFile::Write(WAVEBUFFER *pBuffer, LONGINT count)
{
	long ans;  
	
	ans = mmioWrite(m_hMMIO, (HPSTR)pBuffer, count);

	if (ans == -1)
		ProcFileError(MMSYSERR_NOERROR,"Error in Writing Wave Data.");

	return ans;
}
