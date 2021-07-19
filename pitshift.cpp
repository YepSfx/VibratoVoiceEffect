#include "stdafx.h"
#include "dsp_def.h"
#include "dsp_util.h"
#include "delay.h"
#include "pitshift.h"
#include <math.h>

#define		DEF_CH			1
#define		DEF_BIT			16
#define		DEF_SAMRATE		22050
#define		DEF_FADER		1
#define		DEF_LOW			24
#define		DEF_MID			1000
#define		DEF_UPPER		2024
#define		DEF_SHIFT		0
#define		DEF_VOL			1

#define		DEF_DELAYLEN	2048
#define		DEF_DELAYGAP	2000
#define		DEF_NOTCHGAIN	0.001
#define		DEF_SHIFTGAIN	1024

int Compute(PITCHSHIFT *pShift)
{
	
	pShift->Delay[0] += pShift->Shift[0];
	while (pShift->Delay[0] > DEF_UPPER)
	{
		pShift->Delay[0] -= DEF_DELAYGAP;
		
		if (pShift->Shift[0] != pShift->Shift[1])
				pShift->Shift[0] = pShift->Shift[1];
	}
		
	while (pShift->Delay[0] < DEF_LOW)
	{
		pShift->Delay[0] += DEF_DELAYGAP;

		if (pShift->Shift[0] != pShift->Shift[1])
				pShift->Shift[0] = pShift->Shift[1];
	}

	pShift->Delay[1] = pShift->Delay[0] + DEF_MID;
	while (pShift->Delay[1] > DEF_UPPER)
	{
		pShift->Delay[1] -= DEF_DELAYGAP;

		if (pShift->Shift[0] != pShift->Shift[1])
				pShift->Shift[0] = pShift->Shift[1];
	}
	
	while (pShift->Delay[1] < DEF_LOW)
	{
		pShift->Delay[1] += DEF_DELAYGAP;

		if (pShift->Shift[0] != pShift->Shift[1])
				pShift->Shift[0] = pShift->Shift[1];
	}

	pShift->Fader[1] = float(fabs(pShift->Delay[0] - DEF_SHIFTGAIN) * DEF_NOTCHGAIN);
	pShift->Fader[0] = 1 - pShift->Fader[1];
	
	return TRUE;
}


int InitPitchShift(PITCHSHIFT *pShift)
{
	InitDelay(&(pShift->Buffer[0]));
	DoMsgProc();
	InitDelay(&(pShift->Buffer[1]));
	DoMsgProc();

	pShift->nCh				= DEF_CH;
	pShift->nBit			= DEF_BIT;
	pShift->nSamplePerSec	= DEF_SAMRATE; 
	pShift->Fader[0]		= DEF_FADER;
	pShift->Fader[1]		= DEF_FADER;
	pShift->Delay[0]		= DEF_LOW;
	pShift->Delay[1]		= DEF_UPPER;	
	pShift->Shift[0]		= DEF_SHIFT;
	pShift->Shift[1]		= DEF_SHIFT;
	pShift->Vol[0]			= DEF_VOL;
	pShift->Vol[1]			= DEF_VOL;
	pShift->isProc			= false;

	return TRUE;
}

int MakePitchShift(PITCHSHIFT *pShift)
{
	pShift->Buffer[0].nCh			= pShift->nCh;
	pShift->Buffer[1].nCh			= pShift->nCh;
	pShift->Buffer[0].nBit			= pShift->nBit;
	pShift->Buffer[1].nBit			= pShift->nBit;
	pShift->Buffer[0].nSamplePerSec = pShift->nSamplePerSec;
	pShift->Buffer[1].nSamplePerSec = pShift->nSamplePerSec;
	pShift->Buffer[0].WetGain		= 1;
	pShift->Buffer[1].WetGain		= 1;
	pShift->Buffer[0].DryGain		= 1;
	pShift->Buffer[1].DryGain		= 1;
	pShift->isProc					= false;

	MakeDelayLine(&(pShift->Buffer[0]), 1, DEF_DELAYLEN);
	DoMsgProc();
	MakeDelayLine(&(pShift->Buffer[1]), 1, DEF_DELAYLEN);
	DoMsgProc();

	return TRUE;
}

int SetShiftAmount(PITCHSHIFT *pShift, float amt)
{
	pShift->Shift[1] = amt;
	pShift->Shift[0] = pShift->Shift[1];
	
	return TRUE;
}

int ShiftPitch(PITCHSHIFT *pShift, char *pData, int nSam)
{
	int   i;
	INT16 *p16BitBuff, Tmp16M1,Tmp16M2, s16M, 
		   Tmp16S1[2], Tmp16S2[2],s16S[2];
	INT8  *p8BitBuff,  Tmp8M1,  Tmp8M2,  s8M, 
		   Tmp8S1[2],  Tmp8S2[2], s8S[2];

	while(pShift->isProc == true)
							DoMsgProc();

	pShift->isProc = true;

	p8BitBuff	= pData;
	p16BitBuff	= (INT16*)pData;

	switch(pShift->nCh)
	{
		case 1:
			if (pShift->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					Compute(pShift);

					PutDelay(&(pShift->Buffer[0]),&p8BitBuff[i],1);
					PutDelay(&(pShift->Buffer[1]),&p8BitBuff[i],1);
					GetFracDelay(&(pShift->Buffer[0]),&Tmp8M1,1,pShift->Delay[0]);
					GetFracDelay(&(pShift->Buffer[1]),&Tmp8M2,1,pShift->Delay[1]);
					s8M = Return8Bit(Set8Bit(Tmp8M1)*pShift->Fader[0] + 
						             Set8Bit(Tmp8M2)*pShift->Fader[1]);
					p8BitBuff[i] = s8M;
				}
			}else if (pShift->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					Compute(pShift);	
					PutDelay(&(pShift->Buffer[0]),(char*)(&p16BitBuff[i]),1);
					PutDelay(&(pShift->Buffer[1]),(char*)(&p16BitBuff[i]),1);

					GetFracDelay(&(pShift->Buffer[0]),(char*)(&Tmp16M1),1,pShift->Delay[0]);
					GetFracDelay(&(pShift->Buffer[1]),(char*)(&Tmp16M2),1,pShift->Delay[1]);
					s16M =Return16Bit((Tmp16M1*pShift->Fader[0]) + (Tmp16M2*pShift->Fader[1]));

					p16BitBuff[i] = s16M;
				}

			}else{
				pShift->isProc = false;
				return FALSE;
			}
			break;
		case 2:
			if (pShift->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					Compute(pShift);
					
					Tmp8S1[0] = p8BitBuff[i*2];
					Tmp8S1[1] = p8BitBuff[i*2+1];
					
					PutDelay(&(pShift->Buffer[0]),(char*)Tmp8S1,1);
					PutDelay(&(pShift->Buffer[1]),(char*)Tmp8S1,1);
					
					GetFracDelay(&(pShift->Buffer[0]),(char*)Tmp8S1,1,pShift->Delay[0]);
					GetFracDelay(&(pShift->Buffer[1]),(char*)Tmp8S2,1,pShift->Delay[1]);
					
					s8S[0] = Return8Bit(Set8Bit(Tmp8S1[0])*pShift->Fader[0] + 
						                Set8Bit(Tmp8S2[0])*pShift->Fader[1]);
					s8S[1] = Return8Bit(Set8Bit(Tmp8S1[1])*pShift->Fader[0] + 
						                Set8Bit(Tmp8S2[1])*pShift->Fader[1]);
					
					p8BitBuff[i*2]   = s8S[0];
					p8BitBuff[i*2+1] = s8S[1];

				}
			}else if (pShift->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					Compute(pShift);		
					
					Tmp16S1[0] =  p16BitBuff[i*2];
					Tmp16S1[1] =  p16BitBuff[i*2+1];

					PutDelay(&(pShift->Buffer[0]),(char*)Tmp16S1,1);
					PutDelay(&(pShift->Buffer[1]),(char*)Tmp16S1,1);

					GetFracDelay(&(pShift->Buffer[0]),(char*)Tmp16S1,1,pShift->Delay[0]);
					GetFracDelay(&(pShift->Buffer[1]),(char*)Tmp16S2,1,pShift->Delay[1]);

					s16S[0] = Return16Bit(Tmp16S1[0]*pShift->Fader[0] + 
						                  Tmp16S2[0]*pShift->Fader[1]);
					s16S[1] = Return16Bit(Tmp16S1[1]*pShift->Fader[0] + 
						                  Tmp16S2[1]*pShift->Fader[1]);
					
					p16BitBuff[i*2]   = s16S[0];
					p16BitBuff[i*2+1] = s16S[1];
				}

			}else{
				pShift->isProc = false;
				return FALSE;
			}
			break;
	}
	pShift->isProc = false;
	return TRUE;
}