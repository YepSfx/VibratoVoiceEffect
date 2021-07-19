#include "stdafx.h"
#include "dsp_def.h"
#include "dsp_util.h"
#include "delay.h"
#include <math.h>

#define		DEF_CH			1
#define		DEF_BIT			16
#define		DEF_SAMRATE		22050

#define		DEF_DEPTH		5
#define		DEF_RATE		1
#define		DEF_TICK		0

#define		TH				1

int Compute(VIBRATO *pVib, bool sw)
{
	float M = 0.0;
	float MD ;
	
	MD = floor(pVib->depth / 2);

	switch(sw){
		case true:
			M = MD*cos(2*PI*pVib->lfotick*pVib->rate/(pVib->nSamplePerSec) + PI) 
				+ MD;
			pVib->lfotick += 1;		
			pVib->lfoloc   = M;
			break;
		case false:
			M = MD*cos(2*PI*pVib->lfotick*pVib->rate/(pVib->nSamplePerSec) + PI) 
				+ MD;
			if (fabs(M) < TH)
			{
				pVib->lfotick = 0;		
				pVib->lfoloc  = 0.0;			
			}else{
				pVib->lfotick += 1;		
				pVib->lfoloc   = M;			
			}
			break;
	}
	return TRUE;
}

int	InitVibrato(VIBRATO *pVib)
{
	InitDelay(&(pVib->Buffer));
	DoMsgProc();

	pVib->depth		= DEF_DEPTH;
	pVib->lfotick	= DEF_TICK;
	pVib->lfoloc	= 0;
	pVib->nBit		= DEF_BIT;
	pVib->nCh		= DEF_CH;
	pVib->rate		= DEF_RATE;
	pVib->lfosw		= false;
	pVib->isProc	= false;
	
	return TRUE;
}

int MakeVibrato(VIBRATO *pVib)
{
	pVib->Buffer.nCh		= pVib->nCh;
	pVib->Buffer.nBit		= pVib->nBit;
	pVib->Buffer.nSamplePerSec 
							= pVib->nSamplePerSec;
	pVib->isProc			= false;
	pVib->Buffer.DryGain	= 1.0;
	pVib->Buffer.WetGain	= 1.0;

	MakeDelayLine(&(pVib->Buffer),1,(int)(pVib->nSamplePerSec*0.05));
	DoMsgProc();
	return TRUE;
}

int SetVibratoParam(VIBRATO *pVib, float dep, float rate)
{
	pVib->depth = (dep * pVib->nSamplePerSec/1000);
	pVib->rate  = rate;
	return TRUE;
}

int ProcVibrato(VIBRATO *pVib, char *pData, int nSam)
{
	int		i;
	INT16	*p16BitData, Tmp16, Tmp16s[2];
	INT8	*p8BitData,   Tmp8,  Tmp8s[2];
	bool	sw;
	
	while(pVib->isProc == true)
							DoMsgProc();

	pVib->isProc = true;

	sw = pVib->lfosw;

	switch(pVib->nCh)
	{
		case 1:
			if (pVib->nBit == 8)
			{
				p8BitData = pData;
				for(i=0 ; i < nSam ; i++)
				{
					Compute(pVib,sw);
					PutDelay(&(pVib->Buffer),&p8BitData[i],1);
					GetFracDelay(&(pVib->Buffer),&Tmp8,1,pVib->lfoloc);
					p8BitData[i] = Tmp8;
				}
			}else if (pVib->nBit == 16)
			{
				p16BitData = (INT16*)pData;
				for(i=0 ; i < nSam ; i++)
				{
					Compute(pVib,sw);
					PutDelay(&(pVib->Buffer),(char*)&p16BitData[i],1);
					GetFracDelay(&(pVib->Buffer),(char*)&Tmp16,1,pVib->lfoloc);
					p16BitData[i] = Tmp16;
				}
			}else{
				pVib->isProc = false;
				return FALSE;
			}
			break;
		case 2:
			if (pVib->nBit == 8)
			{
				p8BitData = pData;
				for(i=0 ; i < nSam ; i++)
				{
					Compute(pVib,sw);
					Tmp8s[0] = p8BitData[i*2];
					Tmp8s[1] = p8BitData[i*2+1];
					PutDelay(&(pVib->Buffer),(char*)Tmp8s,1);
					GetFracDelay(&(pVib->Buffer),(char*)Tmp8s,1,pVib->lfoloc);					
					p8BitData[i*2]   = Tmp8s[0];
					p8BitData[i*2+1] = Tmp8s[1];
				}
			}else if (pVib->nBit == 16)
			{
				p16BitData = (INT16*)pData;
				for(i=0 ; i < nSam ; i++)
				{
					Compute(pVib,sw);
					Tmp16s[0] = p16BitData[i*2];
					Tmp16s[1] = p16BitData[i*2+1];
					PutDelay(&(pVib->Buffer),(char*)Tmp16s,1);
					GetFracDelay(&(pVib->Buffer),(char*)Tmp16s,1,pVib->lfoloc);					
					p16BitData[i*2]   = Tmp16s[0];
					p16BitData[i*2+1] = Tmp16s[1];
				}
			}else{
				pVib->isProc = false;
				return FALSE;
			}
			break;
	}
	pVib->isProc = false;	
	return TRUE;
}