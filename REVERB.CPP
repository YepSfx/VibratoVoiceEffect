#include "stdafx.h"
#include "dsp_def.h"
#include "dsp_util.h"
#include "delay.h"
#include "reverb.h"
#include <math.h>

#define		DEF_CH			1
#define		DEF_BIT			16
#define		DEF_SAMRATE		44100
#define		DEF_AP_GAIN		0.7
#define		DEF_PRE_GAIN	0.1

int	PRCRevLen[4] = {513, 1763, 1323, 2693};	//Delay Length at FS=44100 {353, 1097, 1777, 2137}
int JCRevLen[9]	 = {1777, 1847, 1993, 2137, 353, 1097, 1401, 211, 179};//{1777, 1847, 1993, 2137, 389, 127, 43, 211, 179};

bool CheckLen(int Len)
{
	int i;

	if (Len == 2) 
		return true;
  
	if (Len & 1)	
	{
		for (i = 3 ; i < (int)sqrt((double)Len)+1 ; i += 2)
		  if ( (Len % i) == 0) 
					return false;
		return true; 
	}else 
		return false; 
}

int InitPRCRev(PRCREVERB *pRev)
{
	InitDelay(&(pRev->AllPass[0]));
	DoMsgProc();
	InitDelay(&(pRev->AllPass[1]));
	DoMsgProc();
	InitDelay(&(pRev->Comb[0]));
	DoMsgProc();
	InitDelay(&(pRev->Comb[1]));
	DoMsgProc();

	pRev->ComGain[0]	= 0;
	pRev->ComGain[1]	= 0;
	pRev->DecaySec		= 0;
	pRev->Diffuse		= 0;
	pRev->DryGain		= 1;
	pRev->WetGain		= 0;
	pRev->nBit			= DEF_BIT;
	pRev->nCh			= DEF_CH;
	pRev->nSamplePerSec = DEF_SAMRATE;
	pRev->PreGain		= DEF_PRE_GAIN;
	pRev->isProc		= false;

	return TRUE;
}

int InitJCRev(JCREVERB *pRev)
{
	InitDelay(&(pRev->AllPass[0]));
	DoMsgProc();
	InitDelay(&(pRev->AllPass[1]));
	DoMsgProc();
	InitDelay(&(pRev->AllPass[2]));
	DoMsgProc();

	InitDelay(&(pRev->Comb[0]));
	DoMsgProc();
	InitDelay(&(pRev->Comb[1]));
	DoMsgProc();
	InitDelay(&(pRev->Comb[2]));
	DoMsgProc();
	InitDelay(&(pRev->Comb[3]));
	DoMsgProc();

	InitDelay(&(pRev->Out[0]));
	DoMsgProc();
	InitDelay(&(pRev->Out[1]));
	DoMsgProc();

	pRev->ComGain[0]	= 0;
	pRev->ComGain[1]	= 0;
	pRev->DecaySec		= 0;
	pRev->Diffuse		= 0;
	pRev->DryGain		= 1;
	pRev->WetGain		= 0;
	pRev->nBit			= DEF_BIT;
	pRev->nCh			= DEF_CH;
	pRev->nSamplePerSec = DEF_SAMRATE;
	pRev->PreGain		= DEF_PRE_GAIN;
	pRev->isProc		= false;

	return TRUE;
}

int MakePRCRev(PRCREVERB *pRev, float sec)
{
	int		i, delay, lengths[4];
	float	scaler = pRev->nSamplePerSec/44100.0;
	
	for(i = 0 ; i < 4 ; i++)
	{
		lengths[i] = PRCRevLen[i];
	}
	
	if ( scaler != 1.0 ) {
		for (i = 0 ; i < 4 ; i++)	
		{
			delay = (int)floor(scaler * lengths[i]);
			
			if ( (delay & 1) == 0) 
								delay++;
			
			while ( !CheckLen(delay) ) 
								delay += 2;
			
			lengths[i] = delay;
		}
	}

	for(i = 0 ; i < 2 ; i++)
	{
		pRev->AllPass[i].nCh			= pRev->nCh;
		pRev->AllPass[i].nSamplePerSec	= pRev->nSamplePerSec;
		pRev->AllPass[i].nBit			= pRev->nBit;
		pRev->AllPass[i].DryGain		= 1.0;
		pRev->AllPass[i].WetGain		= 1.0;
		pRev->Comb[i].nCh				= pRev->nCh;
		pRev->Comb[i].nSamplePerSec		= pRev->nSamplePerSec;
		pRev->Comb[i].nBit				= pRev->nBit;
		pRev->Comb[i].DryGain			= 1.0;
		pRev->Comb[i].WetGain			= 1.0;
		if (pRev->nBit < DEF_BIT) 
		{
			pRev->Comb[i].nBit				= DEF_BIT;			
			pRev->AllPass[i].nBit			= DEF_BIT;
		}
		MakeDelayLine(&(pRev->AllPass[i]), 1, lengths[i]);
		DoMsgProc();
		MakeDelayLine(&(pRev->Comb[i]), 1, lengths[i+2]);
		DoMsgProc();
		pRev->ComGain[i] = pow(10.0,(-3 * lengths[i+2] / (sec * pRev->nSamplePerSec)));
	}
	pRev->isProc  = false;
	pRev->DecaySec = sec;
	return TRUE;
}

int MakeJCRev(JCREVERB *pRev, float sec)
{
	int		i, delay, lengths[9];
	float	scaler = pRev->nSamplePerSec/44100.0;
	
	for(i = 0 ; i < 9 ; i++)
	{
		lengths[i] = JCRevLen[i];
	}
	
	if ( scaler != 1.0 ) {
		for (i = 0 ; i < 9 ; i++)	
		{
			delay = (int)floor(scaler * lengths[i]);
			
			if ( (delay & 1) == 0) 
								delay++;
			
			while ( !CheckLen(delay) ) 
								delay += 2;
			
			lengths[i] = delay;
		}
	}

	for(i = 0 ; i < 3 ; i++)
	{
		pRev->AllPass[i].nCh			= pRev->nCh;
		pRev->AllPass[i].nBit			= pRev->nBit;
		pRev->AllPass[i].nSamplePerSec	= pRev->nSamplePerSec;
		pRev->AllPass[i].DryGain		= 1.0;
		pRev->AllPass[i].WetGain		= 1.0;
		
		if (pRev->nBit < DEF_BIT)
					pRev->AllPass[i].nBit = DEF_BIT;
		
		MakeDelayLine(&(pRev->AllPass[i]), 1, lengths[i+4]);
		DoMsgProc();
	}
	for(i = 0 ; i < 4 ; i++)
	{
		pRev->Comb[i].nCh				= pRev->nCh;
		pRev->Comb[i].nBit				= pRev->nBit;
		pRev->Comb[i].nSamplePerSec		= pRev->nSamplePerSec;
		pRev->Comb[i].DryGain			= 1.0;
		pRev->Comb[i].WetGain			= 1.0;

		if (pRev->nBit < DEF_BIT)
					pRev->Comb[i].nBit = DEF_BIT;

		MakeDelayLine(&(pRev->Comb[i]), 1, lengths[i]);
		DoMsgProc();
		pRev->ComGain[i] = pow(10.0,(-3 * lengths[i] / (sec * pRev->nSamplePerSec)));	
	}
	for(i = 0 ; i < 2 ; i++)
	{
		pRev->Out[i].nCh				= pRev->nCh;
		pRev->Out[i].nBit				= pRev->nBit;
		pRev->Out[i].nSamplePerSec		= pRev->nSamplePerSec;
		pRev->Out[i].DryGain			= 1.0;
		pRev->Out[i].WetGain			= 1.0;

		if (pRev->nBit < DEF_BIT)
					pRev->Out[i].nBit = DEF_BIT;

		MakeDelayLine(&(pRev->Out[i]), 1, lengths[i+7]);
		DoMsgProc();
	}
	pRev->isProc   = false;
	pRev->DecaySec = sec;
	return TRUE;
}

int SetPRCRevDryWetGain(PRCREVERB *pRev, float dry, float wet)
{
	while(pRev->isProc == true)
							DoMsgProc();

	pRev->DryGain = dry;
	pRev->WetGain = wet;

	return TRUE;
}

int SetPRCRevPreGain(PRCREVERB *pRev, float pre)
{
	while(pRev->isProc == true)
							DoMsgProc();

	pRev->PreGain = pre;	

	return TRUE;
}

int SetJCRevDryWetGain(JCREVERB *pRev, float dry, float wet)
{
	while(pRev->isProc == true)
							DoMsgProc();

	pRev->DryGain = dry;
	pRev->WetGain = wet;

	return TRUE;
}

int SetJCRevDryWetGain(JCREVERB *pRev, float pre)
{
	while(pRev->isProc == true)
							DoMsgProc();

	pRev->PreGain = pre;

	return TRUE;
}

int ProcPRCRev(PRCREVERB *pRev, char *pBuff, int nSam)
{
	int		i, lengths[4];
	INT16	 *p16BitData, Tmp16[7], Tmp16s[7][2], Wrk16, Wrk16s[2];
	INT8	 *p8BitData;

	while(pRev->isProc == true)
							DoMsgProc();

	pRev->isProc = true;
	
	lengths[0] = pRev->AllPass[0].nDelay;
	lengths[1] = pRev->AllPass[1].nDelay;
	lengths[2] = pRev->Comb[0].nDelay;
	lengths[3] = pRev->Comb[1].nDelay;

	switch(pRev->nCh)
	{
		case 1:
			if (pRev->nBit == 8)
			{
				p8BitData = pBuff;
				for(i=0 ; i < nSam ; i++)
				{
					Wrk16 = Set8Bit(p8BitData[i]);

					GetDelay(&(pRev->AllPass[0]),(char*)(&Tmp16[4]),1,lengths[0]);
					Tmp16[0] = (DEF_AP_GAIN * Tmp16[4]);
					Tmp16[0] += (Wrk16 * pRev->PreGain);
					PutDelay(&(pRev->AllPass[0]),(char*)(&Tmp16[0]),1);
					Tmp16[0] = -(DEF_AP_GAIN * Tmp16[0]) + Tmp16[4];
									
					GetDelay(&(pRev->AllPass[1]),(char*)(&Tmp16[4]),1,lengths[1]);
					Tmp16[1] = (DEF_AP_GAIN * Tmp16[4]);
					Tmp16[1] += Tmp16[0];
					PutDelay(&(pRev->AllPass[1]),(char*)(&Tmp16[1]),1);
					Tmp16[1] = -(DEF_AP_GAIN * Tmp16[1]) + Tmp16[4];

					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16[5]),1,lengths[2]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16[6]),1,lengths[3]);
					Tmp16[2] = (Tmp16[1] + (Tmp16[5] * pRev->ComGain[0]));					
					Tmp16[3] = (Tmp16[1] + (Tmp16[6] * pRev->ComGain[1]));

					PutDelay(&(pRev->Comb[0]),(char*)(&Tmp16[2]),1);
					PutDelay(&(pRev->Comb[1]),(char*)(&Tmp16[3]),1);
					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16[5]),1,lengths[2]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16[6]),1,lengths[3]);

					Tmp16[5] = (pRev->WetGain*Tmp16[5]);
					Tmp16[6] = (pRev->WetGain*Tmp16[6]);

					Wrk16 = Return16Bit(((Tmp16[5] + Tmp16[6]) * 0.5) + (Wrk16 * pRev->DryGain));
					
					p8BitData[i] = Return8Bit(Wrk16);
				}
			}else if (pRev->nBit == 16)
			{
				p16BitData = (INT16*)pBuff;
				for(i=0 ; i < nSam ; i++)
				{
					GetDelay(&(pRev->AllPass[0]),(char*)(&Tmp16[4]),1,lengths[0]);
					Tmp16[0] = (DEF_AP_GAIN * Tmp16[4]);
					Tmp16[0] += (p16BitData[i] * pRev->PreGain);
					PutDelay(&(pRev->AllPass[0]),(char*)(&Tmp16[0]),1);
					Tmp16[0] = -(DEF_AP_GAIN * Tmp16[0]) + Tmp16[4];
									
					GetDelay(&(pRev->AllPass[1]),(char*)(&Tmp16[4]),1,lengths[1]);
					Tmp16[1] = (DEF_AP_GAIN * Tmp16[4]);
					Tmp16[1] += Tmp16[0];
					PutDelay(&(pRev->AllPass[1]),(char*)(&Tmp16[1]),1);
					Tmp16[1] = -(DEF_AP_GAIN * Tmp16[1]) + Tmp16[4];

					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16[5]),1,lengths[2]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16[6]),1,lengths[3]);
					Tmp16[2] = (Tmp16[1] + (Tmp16[5] * pRev->ComGain[0]));					
					Tmp16[3] = (Tmp16[1] + (Tmp16[6] * pRev->ComGain[1]));

					PutDelay(&(pRev->Comb[0]),(char*)(&Tmp16[2]),1);
					PutDelay(&(pRev->Comb[1]),(char*)(&Tmp16[3]),1);
					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16[5]),1,lengths[2]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16[6]),1,lengths[3]);

					Tmp16[5] = (pRev->WetGain*Tmp16[5]);
					Tmp16[6] = (pRev->WetGain*Tmp16[6]);

					p16BitData[i] = Return16Bit(((Tmp16[5] + Tmp16[6]) * 0.5) + (p16BitData[i] * pRev->DryGain));

				}
			}else{
				pRev->isProc = false;
				return FALSE;
			}
			break;
		case 2:
			if (pRev->nBit == 8)
			{
				p8BitData = pBuff;
				for(i=0 ; i < nSam ; i++)
				{
					Wrk16s[0] = Set8Bit(p8BitData[i*2]);
					Wrk16s[1] = Set8Bit(p8BitData[i*2+1]);

					GetDelay(&(pRev->AllPass[0]),(char*)(&Tmp16s[4]),1,lengths[0]);
					Tmp16s[0][0] = (DEF_AP_GAIN * Tmp16s[4][0]);
					Tmp16s[0][1] = (DEF_AP_GAIN * Tmp16s[4][1]);
					Tmp16s[0][0] += (Wrk16s[0] * pRev->PreGain);
					Tmp16s[0][1] += (Wrk16s[1] * pRev->PreGain);
					PutDelay(&(pRev->AllPass[0]),(char*)(&Tmp16s[0]),1);
					Tmp16s[0][0] = -(DEF_AP_GAIN * Tmp16s[0][0]) + Tmp16s[4][0];
					Tmp16s[0][1] = -(DEF_AP_GAIN * Tmp16s[0][1]) + Tmp16s[4][1];
					
					GetDelay(&(pRev->AllPass[1]),(char*)(&Tmp16s[4]),1,lengths[1]);
					Tmp16s[1][0] = (DEF_AP_GAIN * Tmp16s[4][0]);
					Tmp16s[1][1] = (DEF_AP_GAIN * Tmp16s[4][1]);
					Tmp16s[1][0] += Tmp16s[0][0];
					Tmp16s[1][1] += Tmp16s[0][1];
					PutDelay(&(pRev->AllPass[1]),(char*)(&Tmp16s[1]),1);
					Tmp16s[1][0] = -(DEF_AP_GAIN * Tmp16s[1][0]) + Tmp16s[4][0];
					Tmp16s[1][1] = -(DEF_AP_GAIN * Tmp16s[1][1]) + Tmp16s[4][1];

					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[5]),1,lengths[2]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[6]),1,lengths[3]);
					Tmp16s[2][0] = (Tmp16s[1][0] + (Tmp16s[5][0] * pRev->ComGain[0]));					
					Tmp16s[2][1] = (Tmp16s[1][1] + (Tmp16s[5][1] * pRev->ComGain[0]));					
					Tmp16s[3][0] = (Tmp16s[1][0] + (Tmp16s[6][0] * pRev->ComGain[1]));
					Tmp16s[3][1] = (Tmp16s[1][1] + (Tmp16s[6][1] * pRev->ComGain[1]));

					PutDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[2]),1);
					PutDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[3]),1);
					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[5]),1,lengths[2]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[6]),1,lengths[3]);

					Tmp16s[5][0] = (pRev->WetGain*Tmp16s[5][0] + pRev->Diffuse*Tmp16s[5][1]);
					Tmp16s[5][1] = (pRev->WetGain*Tmp16s[5][1] + pRev->Diffuse*Tmp16s[5][0]);
					
					Tmp16s[6][0] = (pRev->WetGain*Tmp16s[6][0] + pRev->Diffuse*Tmp16s[6][1]);
					Tmp16s[6][1] = (pRev->WetGain*Tmp16s[6][1] + pRev->Diffuse*Tmp16s[6][0]);

					Wrk16s[0] = Return16Bit(((Tmp16s[5][0] + Tmp16s[6][0]) * 0.5) + (Wrk16s[0] * pRev->DryGain));
					Wrk16s[1] = Return16Bit(((Tmp16s[5][1] + Tmp16s[6][1]) * 0.5) + (Wrk16s[1] * pRev->DryGain));
					
					p8BitData[i*2]   = Return8Bit(Wrk16s[0]);
					p8BitData[i*2+1] = Return8Bit(Wrk16s[1]);
				}
			}else if (pRev->nBit == 16)
			{
				p16BitData = (INT16*)pBuff;
				for(i=0 ; i < nSam ; i++)
				{
					GetDelay(&(pRev->AllPass[0]),(char*)(&Tmp16s[4]),1,lengths[0]);
					Tmp16s[0][0] = (DEF_AP_GAIN * Tmp16s[4][0]);
					Tmp16s[0][1] = (DEF_AP_GAIN * Tmp16s[4][1]);
					Tmp16s[0][0] += (p16BitData[i*2]   * pRev->PreGain);
					Tmp16s[0][1] += (p16BitData[i*2+1] * pRev->PreGain);
					PutDelay(&(pRev->AllPass[0]),(char*)(&Tmp16s[0]),1);
					Tmp16s[0][0] = -(DEF_AP_GAIN * Tmp16s[0][0]) + Tmp16s[4][0];
					Tmp16s[0][1] = -(DEF_AP_GAIN * Tmp16s[0][1]) + Tmp16s[4][1];
					
					GetDelay(&(pRev->AllPass[1]),(char*)(&Tmp16s[4]),1,lengths[1]);
					Tmp16s[1][0] = (DEF_AP_GAIN * Tmp16s[4][0]);
					Tmp16s[1][1] = (DEF_AP_GAIN * Tmp16s[4][1]);
					Tmp16s[1][0] += Tmp16s[0][0];
					Tmp16s[1][1] += Tmp16s[0][1];
					PutDelay(&(pRev->AllPass[1]),(char*)(&Tmp16s[1]),1);
					Tmp16s[1][0] = -(DEF_AP_GAIN * Tmp16s[1][0]) + Tmp16s[4][0];
					Tmp16s[1][1] = -(DEF_AP_GAIN * Tmp16s[1][1]) + Tmp16s[4][1];

					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[5]),1,lengths[2]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[6]),1,lengths[3]);
					Tmp16s[2][0] = (Tmp16s[1][0] + (Tmp16s[5][0] * pRev->ComGain[0]));					
					Tmp16s[2][1] = (Tmp16s[1][1] + (Tmp16s[5][1] * pRev->ComGain[0]));					
					Tmp16s[3][0] = (Tmp16s[1][0] + (Tmp16s[6][0] * pRev->ComGain[1]));
					Tmp16s[3][1] = (Tmp16s[1][1] + (Tmp16s[6][1] * pRev->ComGain[1]));

					PutDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[2]),1);
					PutDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[3]),1);
					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[5]),1,lengths[2]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[6]),1,lengths[3]);

					Tmp16s[5][0] = (pRev->WetGain*Tmp16s[5][0] + pRev->Diffuse*Tmp16s[5][1]);
					Tmp16s[5][1] = (pRev->WetGain*Tmp16s[5][1] + pRev->Diffuse*Tmp16s[5][0]);
					
					Tmp16s[6][0] = (pRev->WetGain*Tmp16s[6][0] + pRev->Diffuse*Tmp16s[6][1]);
					Tmp16s[6][1] = (pRev->WetGain*Tmp16s[6][1] + pRev->Diffuse*Tmp16s[6][0]);

					p16BitData[i*2]   = Return16Bit(((Tmp16s[5][0] + Tmp16s[6][0]) * 0.5) + (p16BitData[i*2]   * pRev->DryGain));
					p16BitData[i*2+1] = Return16Bit(((Tmp16s[5][1] + Tmp16s[6][1]) * 0.5) + (p16BitData[i*2+1] * pRev->DryGain));
				}
			}else{
				pRev->isProc = false;
				return FALSE;
			}
			break;
	}
	pRev->isProc = false;	
	return TRUE;
}

int ProcJCRev(JCREVERB *pRev, char *pBuff, int nSam)
{
	int		i, APLen[3], CombLen[4], OutLen[2];
	INT16	*p16BitData, Tmp16[12], Tmp16s[12][2], Wrk16, Wrk16s[2];
	INT8	*p8BitData;

	while(pRev->isProc == true)
							DoMsgProc();

	pRev->isProc = true;

	for(i = 0 ; i < 3 ; i++)
		APLen[i] = pRev->AllPass[i].nDelay;

	for(i = 0 ; i < 4 ; i++)
		CombLen[i] = pRev->AllPass[i].nDelay;

	for(i = 0 ; i < 2 ; i++)
		OutLen[i] = pRev->Out[i].nDelay;

	switch(pRev->nCh)
	{
		case 1:
			if (pRev->nBit == 8)
			{
				p8BitData = pBuff;
				for(i=0 ; i < nSam ; i++)
				{
					Wrk16 = Set8Bit(p8BitData[i]);
					
					GetDelay(&(pRev->AllPass[0]),(char*)(&Tmp16[7]),1,APLen[0]);
					Tmp16[0] = DEF_AP_GAIN * Tmp16[7];
					Tmp16[0] += Wrk16 * pRev->PreGain;
					PutDelay(&(pRev->AllPass[0]),(char*)(&Tmp16[0]),1);
					Tmp16[0] = -(DEF_AP_GAIN * Tmp16[0]) + Tmp16[7];

					GetDelay(&(pRev->AllPass[1]),(char*)(&Tmp16[7]),1,APLen[1]);
					Tmp16[1] = DEF_AP_GAIN * Tmp16[7];
					Tmp16[1] += Tmp16[0];
					PutDelay(&(pRev->AllPass[1]),(char*)(&Tmp16[1]),1);
					Tmp16[1] = -(DEF_AP_GAIN * Tmp16[1]) + Tmp16[7];

					GetDelay(&(pRev->AllPass[2]),(char*)(&Tmp16[7]),1,APLen[2]);
					Tmp16[2] = DEF_AP_GAIN * Tmp16[7];
					Tmp16[2] += Tmp16[1];
					PutDelay(&(pRev->AllPass[2]),(char*)(&Tmp16[2]),1);
					Tmp16[2] = -(DEF_AP_GAIN * Tmp16[2]) + Tmp16[7];
					
					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16[8]),1,CombLen[0]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16[9]),1,CombLen[1]);
					GetDelay(&(pRev->Comb[2]),(char*)(&Tmp16[10]),1,CombLen[2]);
					GetDelay(&(pRev->Comb[3]),(char*)(&Tmp16[11]),1,CombLen[3]);
					Tmp16[3] = (Tmp16[2] + (Tmp16[8] * pRev->ComGain[0]));					
					Tmp16[4] = (Tmp16[2] + (Tmp16[9] * pRev->ComGain[1]));
					Tmp16[5] = (Tmp16[2] + (Tmp16[10] * pRev->ComGain[2]));					
					Tmp16[6] = (Tmp16[2] + (Tmp16[11] * pRev->ComGain[3]));
					PutDelay(&(pRev->Comb[0]),(char*)(&Tmp16[3]),1);
					PutDelay(&(pRev->Comb[1]),(char*)(&Tmp16[4]),1);
					PutDelay(&(pRev->Comb[2]),(char*)(&Tmp16[5]),1);
					PutDelay(&(pRev->Comb[3]),(char*)(&Tmp16[6]),1);
					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16[8]),1,CombLen[0]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16[9]),1,CombLen[1]);
					GetDelay(&(pRev->Comb[2]),(char*)(&Tmp16[10]),1,CombLen[2]);
					GetDelay(&(pRev->Comb[3]),(char*)(&Tmp16[11]),1,CombLen[3]);

					Tmp16[7] = Tmp16[8] + Tmp16[9] + Tmp16[10] + Tmp16[11];
					PutDelay(&(pRev->Out[0]),(char*)(&Tmp16[7]),1);
					PutDelay(&(pRev->Out[1]),(char*)(&Tmp16[7]),1);
					GetDelay(&(pRev->Out[0]),(char*)(&Tmp16[8]),1,OutLen[0]);
					GetDelay(&(pRev->Out[1]),(char*)(&Tmp16[9]),1,OutLen[1]);

					Tmp16[8] = (pRev->WetGain*Tmp16[8]);
					Tmp16[9] = (pRev->WetGain*Tmp16[9]);

					Wrk16 = Return16Bit(((Tmp16[8] + Tmp16[9]) * 0.5) + (Wrk16 * pRev->DryGain));

					p8BitData[i] = Return8Bit(Wrk16);
				}
			}else if (pRev->nBit == 16)
			{
				p16BitData = (INT16*)pBuff;
				for(i=0 ; i < nSam ; i++)
				{
					GetDelay(&(pRev->AllPass[0]),(char*)(&Tmp16[7]),1,APLen[0]);
					Tmp16[0] = DEF_AP_GAIN * Tmp16[7];
					Tmp16[0] += p16BitData[i] * pRev->PreGain;
					PutDelay(&(pRev->AllPass[0]),(char*)(&Tmp16[0]),1);
					Tmp16[0] = -(DEF_AP_GAIN * Tmp16[0]) + Tmp16[7];
									
					GetDelay(&(pRev->AllPass[1]),(char*)(&Tmp16[7]),1,APLen[1]);
					Tmp16[1] = DEF_AP_GAIN * Tmp16[7];
					Tmp16[1] += Tmp16[0];
					PutDelay(&(pRev->AllPass[1]),(char*)(&Tmp16[1]),1);
					Tmp16[1] = -(DEF_AP_GAIN * Tmp16[1]) + Tmp16[7];

					GetDelay(&(pRev->AllPass[2]),(char*)(&Tmp16[7]),1,APLen[2]);
					Tmp16[2] = DEF_AP_GAIN * Tmp16[7];
					Tmp16[2] += Tmp16[1];
					PutDelay(&(pRev->AllPass[2]),(char*)(&Tmp16[2]),1);
					Tmp16[2] = -(DEF_AP_GAIN * Tmp16[2]) + Tmp16[7];

					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16[8]),1,CombLen[0]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16[9]),1,CombLen[1]);
					GetDelay(&(pRev->Comb[2]),(char*)(&Tmp16[10]),1,CombLen[2]);
					GetDelay(&(pRev->Comb[3]),(char*)(&Tmp16[11]),1,CombLen[3]);
					Tmp16[3] = (Tmp16[2] + (Tmp16[8] * pRev->ComGain[0]));					
					Tmp16[4] = (Tmp16[2] + (Tmp16[9] * pRev->ComGain[1]));
					Tmp16[5] = (Tmp16[2] + (Tmp16[10] * pRev->ComGain[2]));					
					Tmp16[6] = (Tmp16[2] + (Tmp16[11] * pRev->ComGain[3]));
					PutDelay(&(pRev->Comb[0]),(char*)(&Tmp16[3]),1);
					PutDelay(&(pRev->Comb[1]),(char*)(&Tmp16[4]),1);
					PutDelay(&(pRev->Comb[2]),(char*)(&Tmp16[5]),1);
					PutDelay(&(pRev->Comb[3]),(char*)(&Tmp16[6]),1);
					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16[8]),1,CombLen[0]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16[9]),1,CombLen[1]);
					GetDelay(&(pRev->Comb[2]),(char*)(&Tmp16[10]),1,CombLen[2]);
					GetDelay(&(pRev->Comb[3]),(char*)(&Tmp16[11]),1,CombLen[3]);

					Tmp16[7] = Tmp16[8] + Tmp16[9] + Tmp16[10] + Tmp16[11];
					PutDelay(&(pRev->Out[0]),(char*)(&Tmp16[7]),1);
					PutDelay(&(pRev->Out[1]),(char*)(&Tmp16[7]),1);
					GetDelay(&(pRev->Out[0]),(char*)(&Tmp16[8]),1,OutLen[0]);
					GetDelay(&(pRev->Out[1]),(char*)(&Tmp16[9]),1,OutLen[1]);

					Tmp16[8] = (pRev->WetGain*Tmp16[8]);
					Tmp16[9] = (pRev->WetGain*Tmp16[9]);

					p16BitData[i] = Return16Bit(((Tmp16[8] + Tmp16[9]) * 0.5) + (p16BitData[i] * pRev->DryGain));

				}
			}else{
				pRev->isProc = false;
				return FALSE;
			}
			break;
		case 2:
			if (pRev->nBit == 8)
			{
				p8BitData = pBuff;
				for(i=0 ; i < nSam ; i++)
				{
					Wrk16s[0] = Set8Bit(p8BitData[i*2]);
					Wrk16s[1] = Set8Bit(p8BitData[i*2+1]);

					GetDelay(&(pRev->AllPass[0]),(char*)(&Tmp16s[7]),1,APLen[0]);
					Tmp16s[0][0] = DEF_AP_GAIN * Tmp16s[7][0];
					Tmp16s[0][1] = DEF_AP_GAIN * Tmp16s[7][1];
					Tmp16s[0][0] += Wrk16s[0] * pRev->PreGain;
					Tmp16s[0][1] += Wrk16s[1] * pRev->PreGain;
					PutDelay(&(pRev->AllPass[0]),(char*)(&Tmp16s[0]),1);
					Tmp16s[0][0] = -(DEF_AP_GAIN * Tmp16s[0][0]) + Tmp16s[7][0];
					Tmp16s[0][1] = -(DEF_AP_GAIN * Tmp16s[0][1]) + Tmp16s[7][1];				

					GetDelay(&(pRev->AllPass[1]),(char*)(&Tmp16s[7]),1,APLen[1]);
					Tmp16s[1][0] = DEF_AP_GAIN * Tmp16s[7][0];
					Tmp16s[1][1] = DEF_AP_GAIN * Tmp16s[7][1];					
					Tmp16s[1][0] += Tmp16s[0][0];
					Tmp16s[1][1] += Tmp16s[0][1];
					PutDelay(&(pRev->AllPass[1]),(char*)(&Tmp16s[1]),1);
					Tmp16s[1][0] = -(DEF_AP_GAIN * Tmp16s[1][0]) + Tmp16s[7][0];
					Tmp16s[1][1] = -(DEF_AP_GAIN * Tmp16s[1][1]) + Tmp16s[7][1];

					GetDelay(&(pRev->AllPass[2]),(char*)(&Tmp16s[7]),1,APLen[2]);
					Tmp16s[2][0] = DEF_AP_GAIN * Tmp16s[7][0];
					Tmp16s[2][1] = DEF_AP_GAIN * Tmp16s[7][1];
					Tmp16s[2][0] += Tmp16s[1][0];
					Tmp16s[2][1] += Tmp16s[1][1];
					PutDelay(&(pRev->AllPass[2]),(char*)(&Tmp16s[2]),1);
					Tmp16s[2][0] = -(DEF_AP_GAIN * Tmp16s[2][0]) + Tmp16s[7][0];
					Tmp16s[2][1] = -(DEF_AP_GAIN * Tmp16s[2][1]) + Tmp16s[7][1];

					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[8]),1,CombLen[0]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[9]),1,CombLen[1]);
					GetDelay(&(pRev->Comb[2]),(char*)(&Tmp16s[10]),1,CombLen[2]);
					GetDelay(&(pRev->Comb[3]),(char*)(&Tmp16s[11]),1,CombLen[3]);
					Tmp16s[3][0] = (Tmp16s[2][0] + (Tmp16s[8][0]  * pRev->ComGain[0]));					
					Tmp16s[3][1] = (Tmp16s[2][1] + (Tmp16s[8][1]  * pRev->ComGain[0]));										
					Tmp16s[4][0] = (Tmp16s[2][0] + (Tmp16s[9][0]  * pRev->ComGain[1]));
					Tmp16s[4][1] = (Tmp16s[2][1] + (Tmp16s[9][1]  * pRev->ComGain[1]));
					Tmp16s[5][0] = (Tmp16s[2][0] + (Tmp16s[10][0] * pRev->ComGain[2]));										
					Tmp16s[5][1] = (Tmp16s[2][1] + (Tmp16s[10][1] * pRev->ComGain[2]));					
					Tmp16s[6][0] = (Tmp16s[2][0] + (Tmp16s[11][0] * pRev->ComGain[3]));
					Tmp16s[6][1] = (Tmp16s[2][1] + (Tmp16s[11][1] * pRev->ComGain[3]));
					PutDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[3]),1);
					PutDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[4]),1);
					PutDelay(&(pRev->Comb[2]),(char*)(&Tmp16s[5]),1);
					PutDelay(&(pRev->Comb[3]),(char*)(&Tmp16s[6]),1);
					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[8]),1,CombLen[0]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[9]),1,CombLen[1]);
					GetDelay(&(pRev->Comb[2]),(char*)(&Tmp16s[10]),1,CombLen[2]);
					GetDelay(&(pRev->Comb[3]),(char*)(&Tmp16s[11]),1,CombLen[3]);

					Tmp16s[7][0] = Tmp16s[8][0] + Tmp16s[9][0] + Tmp16s[10][0] + Tmp16s[11][0];
					Tmp16s[7][1] = Tmp16s[8][1] + Tmp16s[9][1] + Tmp16s[10][1] + Tmp16s[11][1];
					
					PutDelay(&(pRev->Out[0]),(char*)(&Tmp16s[7]),1);
					PutDelay(&(pRev->Out[1]),(char*)(&Tmp16s[7]),1);
					GetDelay(&(pRev->Out[0]),(char*)(&Tmp16s[8]),1,OutLen[0]);
					GetDelay(&(pRev->Out[1]),(char*)(&Tmp16s[9]),1,OutLen[1]);

					Tmp16s[8][0] = (pRev->WetGain*Tmp16s[8][0] + pRev->Diffuse*Tmp16s[8][1]);
					Tmp16s[8][1] = (pRev->WetGain*Tmp16s[8][1] + pRev->Diffuse*Tmp16s[8][0]);
					Tmp16s[9][0] = (pRev->WetGain*Tmp16s[9][0] + pRev->Diffuse*Tmp16s[9][1]);
					Tmp16s[9][1] = (pRev->WetGain*Tmp16s[9][1] + pRev->Diffuse*Tmp16s[9][0]);

					Wrk16s[0] = Return16Bit(((Tmp16s[8][0] + Tmp16s[9][0]) * 0.5) + (Wrk16s[0] * pRev->DryGain));
					Wrk16s[1] = Return16Bit(((Tmp16s[8][1] + Tmp16s[9][1]) * 0.5) + (Wrk16s[1] * pRev->DryGain));
					
					p8BitData[i*2]   = Return8Bit(Wrk16s[0]);
					p8BitData[i*2+1] = Return8Bit(Wrk16s[1]);				
				}
			}else if (pRev->nBit == 16)
			{
				p16BitData = (INT16*)pBuff;
				for(i=0 ; i < nSam ; i++)
				{
					GetDelay(&(pRev->AllPass[0]),(char*)(&Tmp16s[7]),1,APLen[0]);
					Tmp16s[0][0] = DEF_AP_GAIN * Tmp16s[7][0];
					Tmp16s[0][1] = DEF_AP_GAIN * Tmp16s[7][1];
					Tmp16s[0][0] += p16BitData[i*2]   * pRev->PreGain;
					Tmp16s[0][1] += p16BitData[i*2+1] * pRev->PreGain;
					PutDelay(&(pRev->AllPass[0]),(char*)(&Tmp16s[0]),1);
					Tmp16s[0][0] = -(DEF_AP_GAIN * Tmp16s[0][0]) + Tmp16s[7][0];
					Tmp16s[0][1] = -(DEF_AP_GAIN * Tmp16s[0][1]) + Tmp16s[7][1];				

					GetDelay(&(pRev->AllPass[1]),(char*)(&Tmp16s[7]),1,APLen[1]);
					Tmp16s[1][0] = DEF_AP_GAIN * Tmp16s[7][0];
					Tmp16s[1][1] = DEF_AP_GAIN * Tmp16s[7][1];					
					Tmp16s[1][0] += Tmp16s[0][0];
					Tmp16s[1][1] += Tmp16s[0][1];
					PutDelay(&(pRev->AllPass[1]),(char*)(&Tmp16s[1]),1);
					Tmp16s[1][0] = -(DEF_AP_GAIN * Tmp16s[1][0]) + Tmp16s[7][0];
					Tmp16s[1][1] = -(DEF_AP_GAIN * Tmp16s[1][1]) + Tmp16s[7][1];

					GetDelay(&(pRev->AllPass[2]),(char*)(&Tmp16s[7]),1,APLen[2]);
					Tmp16s[2][0] = DEF_AP_GAIN * Tmp16s[7][0];
					Tmp16s[2][1] = DEF_AP_GAIN * Tmp16s[7][1];
					Tmp16s[2][0] += Tmp16s[1][0];
					Tmp16s[2][1] += Tmp16s[1][1];
					PutDelay(&(pRev->AllPass[2]),(char*)(&Tmp16s[2]),1);
					Tmp16s[2][0] = -(DEF_AP_GAIN * Tmp16s[2][0]) + Tmp16s[7][0];
					Tmp16s[2][1] = -(DEF_AP_GAIN * Tmp16s[2][1]) + Tmp16s[7][1];

					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[8]),1,CombLen[0]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[9]),1,CombLen[1]);
					GetDelay(&(pRev->Comb[2]),(char*)(&Tmp16s[10]),1,CombLen[2]);
					GetDelay(&(pRev->Comb[3]),(char*)(&Tmp16s[11]),1,CombLen[3]);
					Tmp16s[3][0] = (Tmp16s[2][0] + (Tmp16s[8][0]  * pRev->ComGain[0]));					
					Tmp16s[3][1] = (Tmp16s[2][1] + (Tmp16s[8][1]  * pRev->ComGain[0]));										
					Tmp16s[4][0] = (Tmp16s[2][0] + (Tmp16s[9][0]  * pRev->ComGain[1]));
					Tmp16s[4][1] = (Tmp16s[2][1] + (Tmp16s[9][1]  * pRev->ComGain[1]));
					Tmp16s[5][0] = (Tmp16s[2][0] + (Tmp16s[10][0] * pRev->ComGain[2]));										
					Tmp16s[5][1] = (Tmp16s[2][1] + (Tmp16s[10][1] * pRev->ComGain[2]));					
					Tmp16s[6][0] = (Tmp16s[2][0] + (Tmp16s[11][0] * pRev->ComGain[3]));
					Tmp16s[6][1] = (Tmp16s[2][1] + (Tmp16s[11][1] * pRev->ComGain[3]));
					PutDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[3]),1);
					PutDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[4]),1);
					PutDelay(&(pRev->Comb[2]),(char*)(&Tmp16s[5]),1);
					PutDelay(&(pRev->Comb[3]),(char*)(&Tmp16s[6]),1);
					GetDelay(&(pRev->Comb[0]),(char*)(&Tmp16s[8]),1,CombLen[0]);
					GetDelay(&(pRev->Comb[1]),(char*)(&Tmp16s[9]),1,CombLen[1]);
					GetDelay(&(pRev->Comb[2]),(char*)(&Tmp16s[10]),1,CombLen[2]);
					GetDelay(&(pRev->Comb[3]),(char*)(&Tmp16s[11]),1,CombLen[3]);

					Tmp16s[7][0] = Tmp16s[8][0] + Tmp16s[9][0] + Tmp16s[10][0] + Tmp16s[11][0];
					Tmp16s[7][1] = Tmp16s[8][1] + Tmp16s[9][1] + Tmp16s[10][1] + Tmp16s[11][1];
					
					PutDelay(&(pRev->Out[0]),(char*)(&Tmp16s[7]),1);
					PutDelay(&(pRev->Out[1]),(char*)(&Tmp16s[7]),1);
					GetDelay(&(pRev->Out[0]),(char*)(&Tmp16s[8]),1,OutLen[0]);
					GetDelay(&(pRev->Out[1]),(char*)(&Tmp16s[9]),1,OutLen[1]);

					Tmp16s[8][0] = (pRev->WetGain*Tmp16s[8][0] + pRev->Diffuse*Tmp16s[8][1]);
					Tmp16s[8][1] = (pRev->WetGain*Tmp16s[8][1] + pRev->Diffuse*Tmp16s[8][0]);
					Tmp16s[9][0] = (pRev->WetGain*Tmp16s[9][0] + pRev->Diffuse*Tmp16s[9][1]);
					Tmp16s[9][1] = (pRev->WetGain*Tmp16s[9][1] + pRev->Diffuse*Tmp16s[9][0]);

					p16BitData[i*2]   = Return16Bit(((Tmp16s[8][0] + Tmp16s[9][0]) * 0.5) + (p16BitData[i*2]   * pRev->DryGain));
					p16BitData[i*2+1] = Return16Bit(((Tmp16s[8][1] + Tmp16s[9][1]) * 0.5) + (p16BitData[i*2+1] * pRev->DryGain));
				}
			}else{
				pRev->isProc = false;
				return FALSE;
			}
			break;
	}
	pRev->isProc = false;	
	return TRUE;
}
