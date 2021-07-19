#include "stdafx.h"
#include "process.h"
#include "delay.h"
#include "dsp_util.h"
#include "pitshift.h"
#include "vcremove.h"
#include "vibrato.h"
#include "pdetect.h"
#include "reverb.h"

#include <math.h>
#include <mmsystem.h>
#include <stdio.h>

#define	PORT_ON		1
#define PORT_OFF	0

//Audio Format//
#define		DEF_SAMRATE		44100
#define		DEF_BITS		16
#define		DEF_CHS			2
#define		DEF_TH			1
#define		DEF_VOCAL_LO	39
#define		DEF_VOCAL_HI	80
#define		DEF_VOCAL_NO	100
#define		DEF_PITCH_FRQ	10
#define		DEF_LOW_VOL		-40
#define	    DEF_ECHO_DRY	1.0
#define		DEF_ECHO_WET	0.3
#define		DEF_REV_TIME	2.0
#define		DEF_REV_DRY		0.7
#define		DEF_REV_WET		0.3
	

CWaveIn			*pWaveIn;
CWaveOut		*pWaveOut;
CWaveFile		*pWaveFile;
CWaveFile		*pSaveFile;

PITCHSHIFT		Shift;
VIBRATO			Vib;
ACPITCH			Pitch;
HPSPITCH		Hps;
PRCREVERB		PRC;
JCREVERB		JC;
DELAYDATA		Echo;

WAVEFORMATEX	WaveFmt;

float			depth = 5,speed = 5;
float			LoVol = DEF_LOW_VOL;
float			echodry = DEF_ECHO_DRY;
float			echowet = DEF_ECHO_WET;
int				HOLDS	 = 5;
int				HoldNote = 100;
int				HoldSeg  = HOLDS;

int				DEV_BUFFERSIZE  = 256;
int				DEV_BUFFERCOUNT = 100;
int				DEV_SAMS		= 441;
float			shiftamt		= 0.0;	
float			vcamt			= 0.0;
HWND			UIWnd;
bool			MsgProc			= false;
bool			ReverbReady		= true;
bool			EchoReady		= false;
float			RoomSize		= DEF_REV_TIME;

int VocalVibratoTrigger()
{
	float tmpnote;
	float power;
	
	if (Pitch.Count != 0)
					return TRUE;

	tmpnote = PitchToMIDINote(Pitch.CurrentPitch);
	power   = Pitch.power;

	if ((tmpnote <= DEF_VOCAL_LO)||(tmpnote >= DEF_VOCAL_HI))
	{
		HoldSeg   =  HOLDS;
		HoldNote  =  DEF_VOCAL_NO;
		Vib.lfosw =  false;
	}else if ((fabs(tmpnote-HoldNote) < DEF_TH) && (power > LoVol))
	{	
		if (HoldSeg != 0)
			HoldSeg--;
		else
			HoldSeg=0;
	}else{
		HoldSeg   =  HOLDS;
		HoldNote  =  tmpnote;
		Vib.lfosw =  false;
	}
	
	if (HoldSeg == 0)
		Vib.lfosw = true;
	else
		Vib.lfosw = false;

	return TRUE;
}
int CalcSegLen(int SamRate, int nSam, int Freq)
{
	int nSamPermilisec;
	int ChunkDuration;
	int T;

	T = 1000*1/(float)Freq;
	nSamPermilisec = SamRate/1000;
	ChunkDuration  = nSam/nSamPermilisec;
	
	if (ChunkDuration != 0)
		return T / ChunkDuration;
	else
		return T;
}

void OnWaveDataDone(WAVEBUFFER *pBuffer, int Len, CWnd *pParent)
{
}

void OnWaveData(WAVEBUFFER *pBuffer, int Len, CWnd *pParent)
{
	MMRESULT CHK;
	int nSam;
	
	MsgProc = true;
	
	nSam = GetnSam(Len,WaveFmt.wBitsPerSample,WaveFmt.nChannels);
	
	ACDetectPitch(&Pitch,pBuffer,nSam);
	VocalVibratoTrigger();
	ProcVibrato(&Vib,pBuffer,nSam);
	ProcPRCRev(&PRC,pBuffer,nSam);
	pWaveOut->SetHDR(pBuffer,Len,&CHK);
	
	MsgProc = false;
}

void SetVCamount(float amt)
{
	vcamt = amt;
}

void OpenInPort(int ID)
{
	if (pWaveIn->GetDeviceStatus()==DEV_CLOSE)
	{
		pWaveIn->SetDeviceID(ID);
		pWaveIn->SetDeviceStatus(DEV_OPEN);
	}else{
		while(pWaveIn->GetDeviceStatus()==DEV_OPEN)
		{
			pWaveIn->SetDeviceStatus(DEV_CLOSE);
			DoMsgProc();
		}
		pWaveIn->SetDeviceID(ID);
		pWaveIn->SetDeviceStatus(DEV_OPEN);
	}
}
void SetFullFmt()
{
	WaveFmt = pWaveIn->GetWaveFormat();
}

void OpenOutPort(int ID)
{
	WaveFmt = pWaveIn->GetWaveFormat();
	pWaveOut->SetWaveFormat(WaveFmt);

	if (pWaveOut->GetDeviceStatus()==DEV_CLOSE)
	{
		pWaveOut->SetDeviceID(ID);
		pWaveOut->SetDeviceStatus(DEV_OPEN);
	}else{
		while(pWaveOut->GetDeviceStatus()==DEV_OPEN)
		{
			pWaveOut->SetDeviceStatus(DEV_CLOSE);
			DoMsgProc();
		}
		pWaveOut->SetDeviceID(ID);
		pWaveOut->SetDeviceStatus(DEV_OPEN);
	}
}

bool CloseInPort()
{
	pWaveIn->SetDeviceStatus(DEV_CLOSE);
	while(pWaveIn->GetDeviceStatus() == DEV_OPEN)
	{	
		DoMsgProc();
	}
	return true;
}

bool CloseOutPort()
{
	pWaveOut->SetDeviceStatus(DEV_CLOSE);

	while(pWaveOut->GetDeviceStatus() == DEV_OPEN)
	{
		DoMsgProc();
	}
	return true;
}

void InitAudioDev(CWnd *pParent)
{
	pWaveIn	  = new CWaveIn;
	pWaveIn->Create(pParent);
	
	pWaveOut  = new CWaveOut; 
	pWaveOut->Create(pParent);

	pWaveFile = new CWaveFile(pParent);
	pSaveFile = new CWaveFile(pParent);	
	
	pWaveOut->OnWaveDataDone  = OnWaveDataDone;
	pWaveIn->OnWaveData       = OnWaveData; 

	WaveFmt.cbSize			= 0;
	WaveFmt.nChannels		= DEF_CHS;
	WaveFmt.nSamplesPerSec	= DEF_SAMRATE;
	WaveFmt.wBitsPerSample	= DEF_BITS;

	WaveFmt.nAvgBytesPerSec = DEF_SAMRATE*DEF_BITS*DEF_CHS / 8;
	WaveFmt.nBlockAlign		= DEF_CHS*DEF_BITS/8;
	WaveFmt.wFormatTag		= WAVE_FORMAT_PCM;

	pWaveIn->SetWaveFormat(WaveFmt);
	pWaveIn->SetProcPriority(NORMAL);
	pWaveIn->SetHandleType(HANDLE_WINDOW);	
	pWaveOut->SetWaveFormat(WaveFmt);
	pWaveOut->SetProcPriority(HIGH);
	pWaveOut->SetHandleType(HANDLE_WINDOW);	
}

void ClearAudioDev()
{
	while(pWaveIn->GetDeviceStatus()==DEV_OPEN)
	{
		pWaveIn->SetDeviceStatus(DEV_CLOSE);
		DoMsgProc();
	}
	delete pWaveIn;
	DoMsgProc();
	InitPitchShift(&Shift);
	DoMsgProc();
	InitVibrato(&Vib);
	DoMsgProc();
	InitACPitchDetect(&Pitch);
	DoMsgProc();
	InitHPSPitchDetect(&Hps);
	DoMsgProc();
	InitPRCRev(&PRC);
	DoMsgProc();
	InitJCRev(&JC);
	DoMsgProc();
	InitDelay(&Echo);
	DoMsgProc();

	while(pWaveOut->GetDeviceStatus()==DEV_OPEN)
	{
		pWaveOut->SetDeviceStatus(DEV_CLOSE);
		DoMsgProc();
	}
	delete pWaveOut;
	DoMsgProc();
	delete pWaveFile;
	delete pSaveFile;
	DoMsgProc();
}

int GetDevInNum()
{
	 return pWaveIn->GetDeviceCount();
}

int GetDevOutNum()
{
	return pWaveOut->GetDeviceCount();
}

void GetDevInName(int ID, char *pBuff)
{
	strcpy(pBuff,pWaveIn->GetDeviceName(ID));	
}

void GetDevOutName(int ID, char *pBuff)
{
	strcpy(pBuff,pWaveOut->GetDeviceName(ID));	
}

int	GetInPortStatus()
{
	if (pWaveIn->GetDeviceStatus()==DEV_OPEN)
		return PORT_ON;
	else
		return PORT_OFF;
}

int GetOutPortStatus()
{
	if (pWaveOut->GetDeviceStatus()==DEV_OPEN)
		return PORT_ON;
	else
		return PORT_OFF;

}

bool SetInPort(int ID, int nSams, int nBuffs)
{
	CloseInPort();

	while(pWaveIn->GetDeviceStatus()==DEV_OPEN)
	{
		pWaveIn->SetDeviceStatus(DEV_CLOSE);
		DoMsgProc();
	}
   
	DEV_SAMS		= nSams;
    DEV_BUFFERCOUNT = nBuffs;
    DEV_BUFFERSIZE	= DEF_BITS/8*DEF_CHS*DEV_SAMS;
	
	pWaveIn->SetBufferSize(DEV_BUFFERSIZE);
	pWaveIn->SetInitBufferNum(DEV_BUFFERCOUNT);

	InitPitchShift(&Shift);
	Shift.nCh			= DEF_CHS;
	Shift.nBit			= DEF_BITS;
	Shift.nSamplePerSec = DEF_SAMRATE;
	MakePitchShift(&Shift);

	InitVibrato(&Vib);
	Vib.nCh				= DEF_CHS;
	Vib.nBit			= DEF_BITS;
	Vib.nSamplePerSec	= DEF_SAMRATE;
	SetVibratoParam(&Vib, 5, 2);
	MakeVibrato(&Vib);
	
	InitACPitchDetect(&Pitch);
	Pitch.nCh			= DEF_CHS;
	Pitch.nBit			= DEF_BITS;
	Pitch.nSamplePerSec = DEF_SAMRATE;
	Pitch.Seg			= CalcSegLen(DEF_SAMRATE,DEV_SAMS,DEF_PITCH_FRQ);
	MakeACPitchDetect(&Pitch);

	InitHPSPitchDetect(&Hps);
	Hps.nCh				= DEF_CHS;
	Hps.nBit			= DEF_BITS;
	Hps.nSamplePerSec	= DEF_SAMRATE;
	Hps.Seg				= CalcSegLen(DEF_SAMRATE,DEV_SAMS,DEF_PITCH_FRQ);
	MakeHPSPitchDetect(&Hps);

	InitPRCRev(&PRC);
	PRC.nCh				= DEF_CHS;
	PRC.nBit			= DEF_BITS;
	PRC.nSamplePerSec	= DEF_SAMRATE;
	PRC.WetGain			= DEF_REV_WET;
	PRC.DryGain			= DEF_REV_DRY;
	PRC.PreGain			= 0.1;
	PRC.Diffuse			= 0.5;
	MakePRCRev(&PRC,RoomSize);

	InitJCRev(&JC);
	JC.nCh				= DEF_CHS;
	JC.nBit				= DEF_BITS;
	JC.nSamplePerSec	= DEF_SAMRATE;
	JC.WetGain			= 0.3;
	JC.DryGain			= 0.7;
	JC.PreGain			= 0.05;
	JC.Diffuse			= 0.5;
	MakeJCRev(&JC,RoomSize);

	InitDelay(&Echo);
	Echo.nCh			= DEF_CHS;
	Echo.nBit			= DEF_BITS;
	Echo.nSamplePerSec  = DEF_SAMRATE;
	Echo.DryGain		= DEF_ECHO_DRY;
	Echo.WetGain		= DEF_ECHO_WET;
	Echo.MixDryGain		= DEF_ECHO_DRY;
	MakeDelay(&Echo,DEV_SAMS);
	
	DoMsgProc();
	OpenInPort(ID);
	return true;
}

bool SetOutPort(int ID)
{
	CloseOutPort();

	while(pWaveOut->GetDeviceStatus()==DEV_OPEN)
	{
		pWaveOut->SetDeviceStatus(DEV_CLOSE);
		DoMsgProc();
	}
	
	OpenOutPort(ID);


	return true;
}

int GetnSamps()
{
	return DEV_SAMS;
}

int GetnBuffs()
{
	return DEV_BUFFERCOUNT;
}

void SetnSamps(int num)
{
	DEV_SAMS = num;
}

void SetnBuffs(int num)
{
	DEV_BUFFERCOUNT	= num;
}

void SetPitch(float amt)
{
	SetShiftAmount(&Shift,amt);
	shiftamt = amt;
}

void SetPitchParam(float thr,float snr)
{
	SetACPitchSNR(&Pitch,snr);
	LoVol = thr;
}

void SetVib(int dep, float rate, int moment)
{
	depth = dep;
	speed = rate;
	SetVibratoParam(&Vib, dep, rate);
	HOLDS = moment;
}

void SetVibSw(bool vibsw)
{
	Vib.lfosw = vibsw;
}

bool ProcNonRealTime(char *inName, char*outName)
{
	int nBytes, readbytes,i, nSam, inID, outID,nbuf;
	WAVEBUFFER *pBuffer;
	WAVEFORMATEX fmt;
	
	inID = pWaveIn->GetDeviceID();
	nbuf = pWaveIn->GetBufferCount(); 
	outID= pWaveOut->GetDeviceID();

	while (CloseInPort() != true);
						DoMsgProc();
	while (CloseOutPort() != true);
						DoMsgProc();

	if (pWaveFile->SetFile(inName,FILE_READ) == true)
	{
		fmt = pWaveFile->GetWaveFormat();
		pSaveFile->SetWaveFormat(fmt);
        pSaveFile->SetFile(outName,FILE_WRITE);

		InitACPitchDetect(&Pitch);
		Pitch.nCh			= fmt.nChannels;
		Pitch.nBit			= fmt.wBitsPerSample;
		Pitch.nSamplePerSec	= fmt.nSamplesPerSec;
		Pitch.Seg			= CalcSegLen(Pitch.nSamplePerSec,DEV_SAMS,DEF_PITCH_FRQ);
		MakeACPitchDetect(&Pitch);

		InitVibrato(&Vib);
		Vib.nCh				= fmt.nChannels;
		Vib.nBit			= fmt.wBitsPerSample;
		Vib.nSamplePerSec	= fmt.nSamplesPerSec;
		SetVibratoParam(&Vib, depth, speed);
		MakeVibrato(&Vib);

		InitPRCRev(&PRC);
		PRC.nCh				= fmt.nChannels;
		PRC.nBit			= fmt.wBitsPerSample;
		PRC.nSamplePerSec	= fmt.nSamplesPerSec;
		PRC.WetGain			= DEF_REV_WET;
		PRC.DryGain			= DEF_REV_DRY;
		PRC.PreGain			= 0.1;
		PRC.Diffuse			= 0.5;
		MakePRCRev(&PRC,RoomSize);

		InitJCRev(&JC);
		JC.nCh				= fmt.nChannels;
		JC.nBit				= fmt.wBitsPerSample;
		JC.nSamplePerSec	= fmt.nSamplesPerSec;
		JC.WetGain			= 0.3;
		JC.DryGain			= 0.7;
		JC.PreGain			= 0.05;
		JC.Diffuse			= 0.5;
		MakeJCRev(&JC,RoomSize);
		
		nBytes  = fmt.nBlockAlign * DEV_SAMS;
		pBuffer = (char*)calloc(nBytes,sizeof(char));
		readbytes = pWaveFile->Read(pBuffer,nBytes);	
				
		while(readbytes != 0)
		{
			if (readbytes != nBytes)
			{
				for (i = readbytes ; i < nBytes ; i++)
				{
					switch(fmt.wBitsPerSample)
					{
						case 8:
							pBuffer[i] = 0x80;
							break;
						case 16:
							pBuffer[i] = 0x00;
							break;
					}				
				}
			}
			nSam = GetnSam(readbytes,fmt.wBitsPerSample,fmt.nChannels);			

			ACDetectPitch(&Pitch,pBuffer,nSam);
			VocalVibratoTrigger();
			ProcVibrato(&Vib,pBuffer,nSam);
			ProcPRCRev(&PRC,pBuffer,nSam);

			pSaveFile->Write(pBuffer,nBytes);
			readbytes = pWaveFile->Read(pBuffer,nBytes);
			DoMsgProc();
		}
		
		free(pBuffer);
		pSaveFile->SetFile(NULL,FILE_CLOSE);
		pWaveFile->SetFile(NULL,FILE_CLOSE);
	}
	SetOutPort(outID);
	SetInPort(inID,DEV_SAMS,nbuf);

	return true;
}

float GetHPSPitch(char *str)
{
	float	out;
	int		note;

	out = Pitch.CurrentPitch;

	note = PitchToMIDINote(out);
	VocalMIDINoteToString(note,str);
	
	return out;
}

float GetHPSPower()
{
	return Pitch.power;
}

int GetVocalInfo(float *power, float *pitch)
{
	int tmpnote;
	STRING	tmp;
	
	while(MsgProc == true)
					DoMsgProc();

	*pitch = Pitch.CurrentPitch;
	*power = Pitch.power;

	return TRUE;
}

void SetProcHWND(HWND hWnd)
{
	UIWnd = hWnd;
}

void MakeReverb(float time, float dry, float wet)
{
	ReverbReady = false;
	EchoReady   = false;

	InitPRCRev(&PRC);
	PRC.nCh				= DEF_CHS;
	PRC.nBit			= DEF_BITS;
	PRC.nSamplePerSec	= DEF_SAMRATE;
	PRC.WetGain			= wet;
	PRC.DryGain			= dry;
	PRC.PreGain			= 0.1;
	PRC.Diffuse			= 0.5;
	MakePRCRev(&PRC,time);

	ReverbReady = true;
	EchoReady   = false; 
}

void MakeEcho()
{
	InitDelay(&Echo);
	Echo.nCh			= DEF_CHS;
	Echo.nBit			= DEF_BITS;
	Echo.nSamplePerSec  = DEF_SAMRATE;
	Echo.DryGain		= DEF_ECHO_DRY;
	Echo.WetGain		= DEF_ECHO_WET;
	Echo.MixDryGain		= DEF_ECHO_DRY;
	MakeDelay(&Echo,DEV_SAMS);
}

void SetReverbGains(float d, float w)
{
	if (d > 1.0)
		d = 1.0;
	else if (d < 0.0)
				d = 0;

	if (w > 1.0)
		w = 1.0;
	else if (w < 0.0)
				w = 0;

	SetPRCRevDryWetGain(&PRC,d,w);
	SetJCRevDryWetGain(&JC,d,w);
}

void SetRoomSize(float rm)
{
	if (rm > 5)
		rm = 5.0;
	else if (rm < 0.0)
			rm = 0.0;

	RoomSize = rm;
}