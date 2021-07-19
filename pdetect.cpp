#include "stdafx.h"
#include "dsp_def.h"
#include "dsp_util.h"
#include "delay.h"
#include "dsp_fft.h"
#include "pdetect.h"
#include <math.h>

#define		DEF_CH			1
#define		DEF_BIT			16
#define		DEF_SAMRATE		22050
#define		DEF_SEG			0
#define		DEF_LO_HPS		110
#define		DEF_UP_HPS		980
#define		DEF_ACR_BOTTOM 	76
#define		DEF_ACR_UPPER	950
#define		DEF_ACR_PERIOD	4
#define		DEF_ACR_CUTOFF	77
#define		DEF_MIN_POW		-40.0
#define		DEF_NOISE_THR	0.4

int Windowing(HPSPITCH *pPitch, float *pData, int nSam)
{
	int i ;
	
	for (i = 0 ; i < nSam ; i++)
	{
		pData[i*2] = pData[i*2] * pPitch->pWindow[i];
	}
	
	return TRUE;
}

float MeasureRMS(char *pData, int nBit, int nSam)
{
	int		i;
	float	Out = 0.0;
	INT8	*p8BitData;
	INT16	*p16BitData;

	switch(nBit){
		case 8:
			p8BitData = (INT8*)pData;
			for (i = 0 ; i < nSam ; i++)
			{
				Out += Set8Bit(p8BitData[i]) * Set8Bit(p8BitData[i]);
			}
			break;
		case 16:
			p16BitData = (INT16*)pData;
			for (i = 0 ; i < nSam ; i++)
			{
				Out += p16BitData[i] * p16BitData[i];
			}
			break;
	}
	
	Out = Out/(float)nSam;
	Out = sqrt(Out);

	return Out;
}

int MakeFFTBuffer(HPSPITCH *pPitch, char *pData, float *pOut, int nSam)
{
	int i;
	INT8	*p8BitData;
	INT16	*p16BitData;

	if (pPitch->nBit == 8){
		p8BitData = pData;
		for(i = 0 ; i < nSam ; i++)
		{
			pOut[i*2] = (float)Set8Bit(p8BitData[i]);
		}
	}else if (pPitch->nBit == 16){
		p16BitData = (INT16*)pData;
		for(i = 0 ; i < nSam ; i++)
		{
			pOut[i*2] = (float)Return16Bit(p16BitData[i]);
		}
	}else
		return FALSE;

	return TRUE;
}

int	HPSCompute(HPSPITCH *pPitch, float *pOut, int nSam)
{	
	float *pY1, *pY2, *pY3, *pY4, *pYP, m = 0, tmp_m, ans;
	int   i, Len, max_idx;

	Len = nSam/2;

	pY1 = (float*)calloc(Len,sizeof(float));
	pY2 = (float*)calloc(Len,sizeof(float));
	pY3 = (float*)calloc(Len,sizeof(float));
	pY4 = (float*)calloc(Len,sizeof(float));
	pYP = (float*)calloc(Len,sizeof(float));


	Windowing(pPitch, pOut, nSam);
	dspfft(pOut,nSam,-1);
	
	for(i = 0 ; i < Len ; i++)
	{
		pY1[i] = sqrt(pOut[i*2]*pOut[i*2] + pOut[i*2+1]*pOut[i*2+1]);
	}

	for(i = 0 ; i < Len ; i++)
	{
		if (i >= Len/2)
					pY2[i] = 1;
		else
			pY2[i] = 0.5*(pY1[i*2] + pY1[i*2+1]);

	}

	for(i = 0 ; i < Len ; i++)
	{
		if (i >= Len/3)
					pY3[i] = 1;
		else
			pY3[i] = 0.3333*(pY1[i*3] + pY1[i*3+1] + pY1[i*3+2]);

	}

	for(i = 0 ; i < Len ; i++)
	{
		if (i >= Len/4)
					pY4[i] = 1;
		else
			pY4[i] = 0.25*(pY1[i*4] + pY1[i*4+1] + pY1[i*4+2] + pY1[i*4+3]);

	}

	m = 0;
	for(i = 0 ; i < Len ; i++)
	{
		pYP[i] = pY1[i] * pY2[i] * pY3[i] * pY4[i];
		tmp_m  = pYP[i];

		if (tmp_m >= m)
		{
			max_idx = i;
			m       = tmp_m;
		}
	}
	
	ans = ((max_idx+1)/(float)nSam) * pPitch->nSamplePerSec;
	if ((ans < DEF_LO_HPS)||(ans > DEF_UP_HPS))
		ans = 0;
	
	pPitch->CurrentPitch = ans;
	
	free(pY1);
	free(pY2);
	free(pY3);
	free(pY4);
	free(pYP);
	return TRUE;
}

int	InitHPSPitchDetect(HPSPITCH *pPitch)
{
	InitDelay(&(pPitch->Buffer));
	DoMsgProc();
	
	if (pPitch->pWindow != NULL)
	{		
		free(pPitch->pWindow);
		pPitch->pWindow = NULL;
	}

	pPitch->Count			= DEF_SEG;
	pPitch->CurrentPitch	= 0.0;
	pPitch->power			= -250;
	pPitch->nBit			= DEF_BIT;
	pPitch->nCh				= DEF_CH;
	pPitch->nSamplePerSec	= DEF_SAMRATE;
	pPitch->Seg				= DEF_SEG;
	pPitch->BufferLen		= 0;
	pPitch->isProc			= false;

	return TRUE;
}

int MakeHPSPitchDetect(HPSPITCH *pPitch)
{
	int   DelayLen;
	
	switch(pPitch->nSamplePerSec)
	{
		case 44100:
			DelayLen = 4096;			
			break;
		case 22050:
			DelayLen = 2048;						
			break;
		case 11025:
			DelayLen = 1024;
			break;
		case 8000:
			DelayLen = 512;
			break;
	}

	pPitch->Buffer.nCh		= 1;
	pPitch->Buffer.nBit		= pPitch->nBit;
	pPitch->Buffer.nSamplePerSec
							= pPitch->nSamplePerSec;
	pPitch->Buffer.DryGain	= 1.0;
	pPitch->Buffer.WetGain	= 1.0;
	pPitch->BufferLen		= DelayLen;

	MakeDelayLine(&(pPitch->Buffer),1,DelayLen);
	DoMsgProc();

	pPitch->pWindow = (float*)calloc(DelayLen,sizeof(float));
	MakeWindow(DelayLen, DEF_WIN_HAM, pPitch->pWindow);
	DoMsgProc();

	return TRUE;
}

int HPSDetectPitch(HPSPITCH *pPitch, char *pData, int nSam)
{
	float	*pReturn, temp;
	int		Len, i;
	INT8	*p8BitData, *pBuffer;
	INT16   *p16BitData; 

	Len = pPitch->BufferLen;
	pPitch->Count++;	

	switch(pPitch->nCh){
		case 1:
			if (pPitch->nBit == 8)
			{
				p8BitData = pData;
				for(i = 0 ; i < nSam ; i++)
				{
					PutDelay(&(pPitch->Buffer),&p8BitData[i],1);
				}
				pBuffer = (INT8*)calloc(Len, sizeof(INT8));
			}else if (pPitch->nBit == 16)
			{
				p16BitData = (INT16*)pData;
				for(i = 0 ; i < nSam ; i++)
				{
					PutDelay(&(pPitch->Buffer),(char*)(&p16BitData[i]),1);				
				}
				pBuffer = (INT8*)calloc(Len, sizeof(INT16));
			}else{
				return FALSE;
			}
			break;
		case 2:
			if (pPitch->nBit == 8)
			{
				p8BitData = pData;
				for(i = 0 ; i < nSam ; i++)
				{
					PutDelay(&(pPitch->Buffer),(char*)&p8BitData[i*2],1);
				}
				pBuffer = (INT8*)calloc(Len, sizeof(INT8));	
			}else if (pPitch->nBit == 16)
			{
				p16BitData = (INT16*)pData;
				for(i = 0 ; i < nSam ; i++)
				{
					PutDelay(&(pPitch->Buffer),(char*)(&p16BitData[i*2]),1);
				}
				pBuffer = (INT8*)calloc(Len, sizeof(INT16));	
			}else{
				return FALSE;
			}
			break;
	}
	
	pReturn = (float*)calloc(Len*2,sizeof(float));		
	
	if (pPitch->Count > pPitch->Seg)
	{
		pPitch->isProc = true;

		GetDelay(&(pPitch->Buffer),pBuffer,Len,Len);
		switch(pPitch->nBit)
		{
			case 8:
				temp = MeasureRMS(pBuffer, pPitch->nBit, Len);
				pPitch->power = 20*log10(temp/DEF_8BIT_MAX);
				break;
			case 16:
				temp = MeasureRMS(pBuffer, pPitch->nBit, Len);
				pPitch->power = 20*log10(temp/DEF_16BIT_MAX);
				break;
		}

		if (MakeFFTBuffer(pPitch,pBuffer,pReturn,Len)==FALSE)
		{
			free(pBuffer);
			free(pReturn);
			
			return FALSE;			
		}
		
		pPitch->Count = 0;
		HPSCompute(pPitch,pReturn,Len);
		pPitch->isProc = false;
	}
	free(pBuffer);
	free(pReturn);
	return TRUE;
}

int	ACCompute(ACPITCH *pPitch, char *pData, int nSam, float *pOut, int nUpper)
{	
	float	temp,norm;
	int		i, j, k, Len, Bottomidx, Upperidx;
	INT8	*p8BitData;
	INT16	*p16BitData;

	Len			= nSam;
	Bottomidx	= pPitch->nSamplePerSec/DEF_ACR_UPPER;
	Upperidx	= pPitch->nSamplePerSec/DEF_ACR_CUTOFF;
	pPitch->isProc = true;

	if (pPitch->nBit == 8)
	{
		p8BitData = pData;
		for(i = 0 ; i < nUpper ; i++)
		{
			if ((i > Bottomidx)||(i == 0)){
				pOut[i] = 0;				
				for(j = 0 ; j < (Len - i) ; j++)
				{
					pOut[i] += (float)Set8Bit(p8BitData[i+j])*Set8Bit(p8BitData[j]);	
				}
			}else{
				pOut[i] = 0; 
			}
		}			
	}else if (pPitch->nBit == 16)
	{
		p16BitData = (INT16*)pData;
		for(i = 0 ; i < nUpper ; i++)
		{
			if ((i > Bottomidx)||(i == 0)){
				pOut[i] = 0;
				for(j = 0 ; j < (Len - i) ; j++)
				{
					pOut[i] += (float)(p16BitData[i+j]*p16BitData[j]);	
				}
			}else{
				pOut[i] = 0;
			}
		}			
	}else{
		return 0;
	}

	temp = pOut[0];
	while((pOut[Bottomidx] < temp) && (Bottomidx < nUpper))
	{
		temp = pOut[Bottomidx];
		Bottomidx += 1;
	}
  	
	temp = 0.0;
	j    = 0;
	for(i = Bottomidx ; i < nUpper ; i++)
	{
		if (pOut[i] > temp){
			j		= i;
			temp	= pOut[i];
		}
	}
	
	temp = pOut[0];
	norm = 1.0/Len;
	k    = Len/2;
	for(i = 0 ; i < nUpper ; i++)
	{
		pOut[i] *= (k -  i)*norm;
	}

	if (pOut[j] == 0) 
		j = 0;
	else if ((pOut[j] / pOut[0]) < pPitch->NoiseTHR) 
		j = 0;
	else if (j <= Bottomidx)
		j = 0;
	else if (j >= nUpper)
		j = 0;

	pOut[0] = temp/Len;
	
	pPitch->isProc = false;

	return j;
}

int	InitACPitchDetect(ACPITCH *pPitch)
{
	InitDelay(&(pPitch->Buffer));
	DoMsgProc();

	pPitch->Count			= DEF_SEG;
	pPitch->CurrentPitch	= 0.0;
	pPitch->power			= -250;
	pPitch->nBit			= DEF_BIT;
	pPitch->nCh				= DEF_CH;
	pPitch->nSamplePerSec	= DEF_SAMRATE;
	pPitch->Seg				= DEF_SEG;
	pPitch->BufferLen		= 0;
	pPitch->NoiseTHR		= DEF_NOISE_THR;
	pPitch->isProc			= false;
	return TRUE;
}

int MakeACPitchDetect(ACPITCH *pPitch)
{
	float Len;
	int   DelayLen;

	Len = (float)pPitch->nSamplePerSec * DEF_ACR_PERIOD / (DEF_ACR_BOTTOM);
	DelayLen = (int)Len + (int)Len%2;

	pPitch->Buffer.nCh		= 1;
	pPitch->Buffer.nBit		= pPitch->nBit;
	pPitch->Buffer.nSamplePerSec
							= pPitch->nSamplePerSec;
	pPitch->Buffer.DryGain	= 1.0;
	pPitch->Buffer.WetGain	= 1.0;
	pPitch->BufferLen		= DelayLen;

	MakeDelayLine(&(pPitch->Buffer),1,DelayLen);
	DoMsgProc();

	return TRUE;
}

int ACDetectPitch(ACPITCH *pPitch, char *pData, int nSam)
{
	float	*pReturn, rms;
	int		Len, i, idx, nMax;
	INT8	*p8BitData,  *pBuffer;
	INT16   *p16BitData; 

	Len = pPitch->BufferLen;
	pPitch->Count++;	
	
	switch(pPitch->nCh){
		case 1:
			if (pPitch->nBit == 8)
			{
				p8BitData = pData;
				for(i = 0 ; i < nSam ; i++)
				{
					PutDelay(&(pPitch->Buffer),&p8BitData[i],1);
				}
				pBuffer = (INT8*)calloc(Len, sizeof(INT8));
			}else if (pPitch->nBit == 16)
			{
				p16BitData = (INT16*)pData;
				for(i = 0 ; i < nSam ; i++)
				{
					PutDelay(&(pPitch->Buffer),(char*)(&p16BitData[i]),1);				
				}
				pBuffer = (INT8*)calloc(Len, sizeof(INT16));
			}else{
				return FALSE;
			}
			break;
		case 2:
			if (pPitch->nBit == 8)
			{
				p8BitData = pData;
				for(i = 0 ; i < nSam ; i++)
				{
					PutDelay(&(pPitch->Buffer),&p8BitData[i*2],1);
				}
				pBuffer = (INT8*)calloc(Len, sizeof(INT8));
			}else if (pPitch->nBit == 16)
			{
				p16BitData = (INT16*)pData;
				for(i = 0 ; i < nSam ; i++)
				{
					PutDelay(&(pPitch->Buffer),(char*)&p16BitData[i*2],1);
				}
				pBuffer = (INT8*)calloc(Len, sizeof(INT16));
			}else{
				return FALSE;
			}
			break;
	}
	
	pReturn = (float*)calloc(Len,sizeof(float));		
	
	if (pPitch->Count > pPitch->Seg)
	{
		GetDelay(&(pPitch->Buffer),pBuffer,Len,Len);
		nMax = Len / DEF_ACR_PERIOD;

		idx = ACCompute(pPitch, pBuffer, Len, pReturn, nMax);
	
		switch(pPitch->nBit)
		{
			case 8:
				pPitch->power = 20*log10( sqrt(pReturn[0])/DEF_8BIT_MAX );			
				break;
			case 16:
				pPitch->power = 20*log10( sqrt(pReturn[0])/DEF_16BIT_MAX );
				break;
		}

		if ((idx > 0)&&(pPitch->power > DEF_MIN_POW))
			pPitch->CurrentPitch = pPitch->nSamplePerSec/idx;
		else
			pPitch->CurrentPitch = 0;
		
		pPitch->Count = 0;
	}else{
//		GetDelay(&(pPitch->Buffer),pBuffer,Len,Len);
//		pPitch->power = MeasureRMS(pBuffer, pPitch->nBit, Len);	
//		switch(pPitch->nBit)
//		{
//			case 8:
//				pPitch->power = 20*log10( pPitch->power/DEF_8BIT_MAX );			
//				break;
//			case 16:
//				pPitch->power = 20*log10( pPitch->power/DEF_16BIT_MAX );
//				break;
//		}	
	}
	free(pBuffer);
	free(pReturn);
	return TRUE;
}

int SetACPitchSNR(ACPITCH *pPitch, float snr)
{
	while(pPitch->isProc == true)
							DoMsgProc();

	pPitch->NoiseTHR = snr;

	return TRUE;
}
