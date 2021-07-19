#include "stdafx.h"
#include "dsp_def.h"
#include "dsp_util.h"
#include "delay.h"
#include <math.h>

#define			RINGOFFSET			3

void IncIDX(DELAYDATA *pDelay)
{
	pDelay->in_idx = (pDelay->in_idx%(pDelay->nSamBuff))  + 1;
	pDelay->out_idx= (pDelay->out_idx%(pDelay->nSamBuff)) + 1;
}

int MakeStart(DELAYDATA *pDelay, int offset)
{
	int ans;

	ans  = (pDelay->out_idx + offset )%pDelay->nSamBuff;
	
	return ans;
}

int BoundarySafe(DELAYDATA *pDelay, int idx)
{
	int ans;
	ans = idx % (pDelay->nSamBuff);  
	return ans;
}

int	InitDelay(DELAYDATA *pDelay)
{
	if (pDelay->pBuffer != NULL)
			free(pDelay->pBuffer);

	pDelay->out_idx		= 0;
	pDelay->in_idx		= 0;
	pDelay->nSamBuff	= 0;
	pDelay->WetGain		= 0.3;
	pDelay->DryGain		= 1.0;
	pDelay->MixDryGain	= 1.0;
	pDelay->nDelay		= 0;
	pDelay->nTabs		= 1;
	pDelay->isProc		= false;
	return TRUE;
}

int	MakeDelay(DELAYDATA *pDelay, int nProcSam)
{
	int SamLen;
	int nSamBuff;
	//nProcSam > 1 ======> Chunk Process
	//nProcSam = 1 ======> Single Sample Process
	switch(pDelay->nCh)
	{
		case 1:
			SamLen   = (int)(pDelay->nSamplePerSec + nProcSam + RINGOFFSET);
			nSamBuff = (int)(pDelay->nSamplePerSec + nProcSam + RINGOFFSET);
			break;
		case 2:
			SamLen   = (int)(pDelay->nSamplePerSec + nProcSam + RINGOFFSET) * 2;
			nSamBuff = (int)(pDelay->nSamplePerSec + nProcSam + RINGOFFSET);			
			break;
	}
	
	pDelay->pBuffer = (float*)calloc(SamLen,sizeof(float));	
	
	if (pDelay->pBuffer == NULL)
	{
		pDelay->in_idx		= 0;
		pDelay->out_idx		= 0;
		pDelay->nSamBuff	= 0;
		pDelay->nDelay		= 0;
		pDelay->isProc		= false;
		return FALSE;
	}else{
		pDelay->in_idx		= pDelay->nSamplePerSec + nProcSam + 1;
		pDelay->out_idx		= pDelay->in_idx - (pDelay->nSamplePerSec + nProcSam);
		pDelay->nSamBuff	= nSamBuff;
		pDelay->nDelay		= pDelay->nSamplePerSec;
		pDelay->isProc		= false;
		return TRUE;
	}
}

int MakeDelayLine(DELAYDATA *pDelay, int nProcSam, int nDelay)
{
	int SamLen;
	int nSamBuff;
	//nProcSam > 1 ======> Chunk Process
	//nProcSam = 1 ======> Single Sample Process
	switch(pDelay->nCh)
	{
		case 1:
			SamLen   = (nDelay + nProcSam + RINGOFFSET);
			nSamBuff = (nDelay + nProcSam + RINGOFFSET);
			break;
		case 2:
			SamLen   = (nDelay + nProcSam + RINGOFFSET) * 2;
			nSamBuff = (nDelay + nProcSam + RINGOFFSET);			
			break;
	}
	
	pDelay->pBuffer = (float*)calloc(SamLen,sizeof(float));	
	
	if (pDelay->pBuffer == NULL)
	{
		pDelay->in_idx		= 0;
		pDelay->out_idx		= 0;
		pDelay->nSamBuff	= 0;
		pDelay->nDelay		= 0;
		pDelay->isProc		= false;
		return FALSE;
	}else{
		pDelay->in_idx		= pDelay->nDelay+ nProcSam + 1;
		pDelay->out_idx		= pDelay->in_idx - (pDelay->nDelay + nProcSam);
		pDelay->nSamBuff	= nSamBuff;
		pDelay->nDelay		= nDelay;
		pDelay->isProc		= false;
		return TRUE;
	}
}

int	PutDelay(DELAYDATA *pDelay, char *pData, int nSam)
{
	int i, thisidx;
	short *p16BitBuff;
	char  *p8BitBuff;

	float gain;

	while(pDelay->isProc == true)
					DoMsgProc();
	
	pDelay->isProc = true;
	
	p8BitBuff	= pData;
	p16BitBuff	= (short*)pData;

	gain = pDelay->DryGain;

	switch(pDelay->nCh)
	{
		case 1:
			if (pDelay->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					thisidx = pDelay->in_idx-1;
					pDelay->pBuffer[thisidx] = gain * Set8Bit(p8BitBuff[i]);
					IncIDX(pDelay);
				}

			}else if (pDelay->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					thisidx = pDelay->in_idx-1;
					pDelay->pBuffer[thisidx] = gain * p16BitBuff[i];
					IncIDX(pDelay);
				}

			}else{
				pDelay->isProc = false;
				return FALSE;
			}
			break;
		case 2:
			if (pDelay->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					thisidx = (pDelay->in_idx-1)*2;
					pDelay->pBuffer[thisidx]   = gain * Set8Bit(p8BitBuff[i*2]);
					thisidx = (pDelay->in_idx-1)*2+1;
					pDelay->pBuffer[thisidx] = gain * Set8Bit(p8BitBuff[i*2+1]);
					IncIDX(pDelay);
				}
			}else if (pDelay->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					thisidx = (pDelay->in_idx-1)*2;
					pDelay->pBuffer[thisidx]   = gain * p16BitBuff[i*2];
					thisidx = (pDelay->in_idx-1)*2+1;
					pDelay->pBuffer[thisidx] = gain * p16BitBuff[i*2+1];
					IncIDX(pDelay);
				}
			}else{
				pDelay->isProc = false;
				return FALSE;
			}			
			break;
	}
	pDelay->isProc = false;
	return TRUE;
}

int	GetDelay(DELAYDATA *pDelay, char *pData, int nSam, int nDelaySam)
{
	int i,idx,safe,thisidx,start,offset;
	short *p16BitBuff;
	char  *p8BitBuff;
	float gain;

	while(pDelay->isProc == true)
					DoMsgProc();
	pDelay->isProc = true;
    
	if (nDelaySam > pDelay->nSamBuff)
					nDelaySam = pDelay->nSamBuff;

	offset = pDelay->nDelay - nDelaySam;	
	gain   = pDelay->WetGain;
	
	start = MakeStart(pDelay,offset);

	p8BitBuff	= pData;
	p16BitBuff	= (short*)pData;

	switch(pDelay->nCh)
	{
		case 1:
			if (pDelay->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					idx = start + i;
					safe = BoundarySafe(pDelay,idx);
					
					p8BitBuff[i] = Return8Bit(pDelay->pBuffer[safe]*gain);
				}
			}else if (pDelay->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					idx = start + i;
					safe = BoundarySafe(pDelay,idx);

					p16BitBuff[i] = Return16Bit((pDelay->pBuffer[safe]*gain));						
				}

			}else{
				pDelay->isProc = false;
				return FALSE;
			}
			break;
		case 2:
			if (pDelay->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					idx = start + i;
					safe = BoundarySafe(pDelay,idx);
					
					thisidx = safe*2;
					p8BitBuff[i*2]   = Return8Bit(pDelay->pBuffer[thisidx]*gain);
					thisidx = safe*2 +1;
					p8BitBuff[i*2+1] = Return8Bit(pDelay->pBuffer[thisidx]*gain);
				}
			}else if (pDelay->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					idx = start + i;
					safe = BoundarySafe(pDelay,idx);

					thisidx = safe*2;
					p16BitBuff[i*2]   = Return16Bit(pDelay->pBuffer[thisidx]*gain);
					thisidx = safe*2+1;
					p16BitBuff[i*2+1] = Return16Bit(pDelay->pBuffer[thisidx]*gain);
				}

			}else{
				pDelay->isProc = false;
				return FALSE;
			}
			break;
	}
	pDelay->isProc = false;
	return TRUE;
}
int GetFracDelay(DELAYDATA *pDelay, char *pData, int nSam, float nDelaySam)
{
	int		i, idx		= 0;
	float	frac		= 0.0,  Tmp1 = 0.0, Tmp2 = 0.0;
	float	Tmp1Ch[2],  Tmp2Ch[2];			
	char	*p8BitData	= NULL, Tmp8Bit  = 0, Tmp8Bit2Ch[2];
	INT16	*p16BitData = NULL, Tmp16Bit = 0, Tmp16Bit2Ch[2];
	INT16   TmpOut[2];

	idx  = nDelaySam;
	frac = nDelaySam - idx;

	switch(pDelay->nCh)
	{
		case 1:
			if (pDelay->nBit==8)
			{
				p8BitData = pData;
				for(i = 0 ; i < nSam ; i++)
				{
					GetDelay(pDelay, &Tmp8Bit, 1, (idx + i));
					Tmp2 = (1 - frac) * Set8Bit(Tmp8Bit);
					GetDelay(pDelay, &Tmp8Bit, 1, (idx + 1 + i));
					Tmp1 = frac * Set8Bit(Tmp8Bit);
					p8BitData[i] = Return8Bit((Tmp1 + Tmp2)*pDelay->WetGain);									

				}
			}else if (pDelay->nBit==16)
			{
				p16BitData = (INT16*)pData;
				for(i = 0 ; i < nSam ; i++)
				{
					GetDelay(pDelay, (char*)&Tmp16Bit, 1, (idx + i));
					Tmp2 = Tmp16Bit * (1 - frac);
					GetDelay(pDelay, (char*)&Tmp16Bit, 1, (idx + 1 + i));
					Tmp1 = Tmp16Bit * frac;
					p16BitData[i] = Return16Bit((Tmp1 + Tmp2)*pDelay->WetGain);									
				}

			}
			break;
		case 2:
			if (pDelay->nBit==8)
			{
				p8BitData = pData;
				for(i = 0 ; i < nSam ; i++)
				{
					GetDelay(pDelay, (char*)Tmp8Bit2Ch, 1, (idx + i));
					Tmp2Ch[0] = Set8Bit(Tmp8Bit2Ch[0])*(1-frac);
					Tmp2Ch[1] = Set8Bit(Tmp8Bit2Ch[1])*(1-frac);
					GetDelay(pDelay, (char*)Tmp16Bit2Ch, 1, ((idx + i) + 1));
					Tmp1Ch[0] = Set8Bit(Tmp8Bit2Ch[0])*frac;
					Tmp1Ch[1] = Set8Bit(Tmp8Bit2Ch[1])*frac;
					
					TmpOut[0] = Return8Bit((Tmp1Ch[0]+Tmp2Ch[0])*pDelay->WetGain);
					TmpOut[1] = Return8Bit((Tmp1Ch[1]+Tmp2Ch[1])*pDelay->WetGain);
					
					p8BitData[i*2]   = TmpOut[0];
					p8BitData[i*2+1] = TmpOut[1];
				}
			}else if (pDelay->nBit==16)
			{
				p16BitData = (INT16*)pData;
				for(i = 0 ; i < nSam ; i++)
				{
					GetDelay(pDelay, (char*)Tmp16Bit2Ch, 1, (idx + i));
					Tmp2Ch[0] = Tmp16Bit2Ch[0]*(1-frac);
					Tmp2Ch[1] = Tmp16Bit2Ch[1]*(1-frac);
					GetDelay(pDelay, (char*)Tmp16Bit2Ch, 1, ((idx + i) + 1));
					Tmp1Ch[0] = Tmp16Bit2Ch[0]*frac;
					Tmp1Ch[1] = Tmp16Bit2Ch[1]*frac;
					
					TmpOut[0] = Return16Bit((Tmp1Ch[0]+Tmp2Ch[0])*pDelay->WetGain);
					TmpOut[1] = Return16Bit((Tmp1Ch[1]+Tmp2Ch[1])*pDelay->WetGain);
					
					p16BitData[i*2]   = TmpOut[0];
					p16BitData[i*2+1] = TmpOut[1];
				}

			}
			break;
	}
	return TRUE;
}

int	MixDelayDry(DELAYDATA *pDelay, char *pData, int nSam, int nDelaySam)
{
	int i,idx,safe,thisidx,start,offset;
	short *p16BitBuff;
	char  *p8BitBuff;
	float gain, mix;

	while(pDelay->isProc == true)
					DoMsgProc();

	pDelay->isProc = true;

	if (nDelaySam > pDelay->nSamBuff)
					nDelaySam = pDelay->nSamBuff;

	offset = pDelay->nSamplePerSec - nDelaySam;
	gain   = pDelay->WetGain;
	mix    = pDelay->MixDryGain;

	start = MakeStart(pDelay,offset);

	p8BitBuff	= pData;
	p16BitBuff	= (short*)pData;

	switch(pDelay->nCh)
	{
		case 1:
			if (pDelay->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					idx = start + i;
					safe = BoundarySafe(pDelay,idx);
					
					p8BitBuff[i] = Return8Bit(mix*(float)Set8Bit(p8BitBuff[i]) + (pDelay->pBuffer[safe]*gain));
					
				}
			}else if (pDelay->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					idx = start + i;
					safe = BoundarySafe(pDelay,idx);

					p16BitBuff[i] = Return16Bit(mix*(float)p16BitBuff[i] + ((pDelay->pBuffer[safe])*gain));						
				}

			}else{
				pDelay->isProc = false;
				return FALSE;
			}
			break;
		case 2:
			if (pDelay->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					idx = start + i;
					safe = BoundarySafe(pDelay,idx);
					
					thisidx = safe*2;
					p8BitBuff[i*2]   = Return8Bit(mix*Set8Bit(p8BitBuff[i*2]) + (pDelay->pBuffer[thisidx]*gain));
					thisidx = safe*2 +1;
					p8BitBuff[i*2+1] = Return8Bit(mix*Set8Bit(p8BitBuff[i*2+1]) + (pDelay->pBuffer[thisidx]*gain));
				}
			}else if (pDelay->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					idx = start + i;
					safe = BoundarySafe(pDelay,idx);

					thisidx = safe*2;
					p16BitBuff[i*2]   = Return16Bit(mix*p16BitBuff[i*2] + (pDelay->pBuffer[thisidx]*gain));
					thisidx = safe*2+1;
					p16BitBuff[i*2+1] = Return16Bit(mix*p16BitBuff[i*2+1] + (pDelay->pBuffer[thisidx]*gain));
				}

			}else{
				pDelay->isProc = false;
				return FALSE;
			}
			break;
	}
	pDelay->isProc = false;
	return TRUE;
}

int	MixFracDelayDry(DELAYDATA *pDelay, char *pData, int nSam, float nDelaySam)
{
	int		i, idx		= 0;
	float	frac		= 0.0,  Tmp1 = 0.0, Tmp2 = 0.0, mix = 0.0;
	float	Tmp1Ch[2],  Tmp2Ch[2];			
	char	*p8BitData	= NULL, Tmp8Bit  = 0, Tmp8Bit2Ch[2];
	INT16	*p16BitData = NULL, Tmp16Bit = 0, Tmp16Bit2Ch[2];
	INT16   TmpOut[2];

	idx  = (int)nDelaySam;
	frac = nDelaySam - (float)idx;

	mix    = pDelay->MixDryGain;

	switch(pDelay->nCh)
	{
		case 1:
			if (pDelay->nBit==8)
			{
				p8BitData = pData;
				for(i = 0 ; i < nSam ; i++)
				{
					GetDelay(pDelay, &Tmp8Bit, 1, (idx + i));
					Tmp2 = Set8Bit(Tmp8Bit) * (1 - frac);
					GetDelay(pDelay, &Tmp8Bit, 1, (idx + 1 + i));
					Tmp1 = Set8Bit(Tmp8Bit) * frac;
					p8BitData[i] = Return8Bit(mix*Set8Bit(p8BitData[i]) + 
						                     (Tmp1 + Tmp2)*pDelay->WetGain);									

				}
			}else if (pDelay->nBit==16)
			{
				p16BitData = (INT16*)pData;
				for(i = 0 ; i < nSam ; i++)
				{
					GetDelay(pDelay, (char*)&Tmp16Bit, 1, (idx + i));
					Tmp2 = Tmp16Bit * (1 - frac);
					GetDelay(pDelay, (char*)&Tmp16Bit, 1, (idx + 1 + i));
					Tmp1 = Tmp16Bit * frac;
					p16BitData[i] = Return16Bit(mix*p16BitData[i] + 
						                       (Tmp1 + Tmp2)*pDelay->WetGain);									
				}

			}
			break;
		case 2:
			if (pDelay->nBit==8)
			{
				p8BitData = pData;
				for(i = 0 ; i < nSam ; i++)
				{
					GetDelay(pDelay, (char*)Tmp8Bit2Ch, 1, (idx + i));
					Tmp2Ch[0] = Set8Bit(Tmp8Bit2Ch[0])*(1-frac);
					Tmp2Ch[1] = Set8Bit(Tmp8Bit2Ch[1])*(1-frac);
					GetDelay(pDelay, (char*)Tmp16Bit2Ch, 1, ((idx + i) + 1));
					Tmp1Ch[0] = Set8Bit(Tmp8Bit2Ch[0])*frac;
					Tmp1Ch[1] = Set8Bit(Tmp8Bit2Ch[1])*frac;
					
					TmpOut[0] = Return8Bit(mix*Set8Bit(p8BitData[i*2]) + 
						                  (Tmp1Ch[0]+Tmp2Ch[0])*pDelay->WetGain);
					TmpOut[1] = Return8Bit(mix*Set8Bit(p8BitData[i*2+1]) + 
						                  (Tmp1Ch[1]+Tmp2Ch[1])*pDelay->WetGain);
					
					p8BitData[i*2]   = TmpOut[0];
					p8BitData[i*2+1] = TmpOut[1];
				}
			}else if (pDelay->nBit==16)
			{
				p16BitData = (INT16*)pData;
				for(i = 0 ; i < nSam ; i++)
				{
					GetDelay(pDelay, (char*)Tmp16Bit2Ch, 1, (idx + i));
					Tmp2Ch[0] = Tmp16Bit2Ch[0]*(1-frac);
					Tmp2Ch[1] = Tmp16Bit2Ch[1]*(1-frac);
					GetDelay(pDelay, (char*)Tmp16Bit2Ch, 1, ((idx + i) + 1));
					Tmp1Ch[0] = Tmp16Bit2Ch[0]*frac;
					Tmp1Ch[1] = Tmp16Bit2Ch[1]*frac;
					
					TmpOut[0] = Return16Bit(mix*p16BitData[i*2] +
						                   (Tmp1Ch[0]+Tmp2Ch[0])*pDelay->WetGain);
					TmpOut[1] = Return16Bit(mix*p16BitData[i*2+1] + 
						                   (Tmp1Ch[1]+Tmp2Ch[1])*pDelay->WetGain);
					
					p16BitData[i*2]   = TmpOut[0];
					p16BitData[i*2+1] = TmpOut[1];
				}

			}
			break;
	}
	return TRUE;

}

int	GetTabDelay(DELAYDATA *pDelay, char *pData, int nSam)
{
	int   i,idx,safe,thisidx,jdx;
	int   *start, *offset;
	short *p16BitBuff;
	char  *p8BitBuff;
	float *gain;
	
	while(pDelay->isProc == true)
					DoMsgProc();

	pDelay->isProc = true;
	
	offset = (int*)calloc(pDelay->nTabs,sizeof(int));
	start  = (int*)calloc(pDelay->nTabs,sizeof(int));
	gain   = (float*)calloc(pDelay->nTabs,sizeof(float));	
	
	for(i = 0 ; i < pDelay->nTabs ; i++)
	{
		offset[i] = pDelay->nDelay - pDelay->TabDelaySams[i];	
		gain[i]   = pDelay->TabGain[i];
	
		start[i] = MakeStart(pDelay,offset[i]);

	}

	p8BitBuff	= pData;
	p16BitBuff	= (short*)pData;

	switch(pDelay->nCh)
	{
		case 1:
			if (pDelay->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					for(jdx = 0 ; jdx < pDelay->nTabs ; jdx++)
					{
						idx = start[jdx] + i;
						safe = BoundarySafe(pDelay,idx);
						
						if (jdx==0)
							p8BitBuff[i] = Return8Bit(pDelay->pBuffer[safe]*gain[jdx]);
						else
							p8BitBuff[i] = Return8Bit(Set8Bit(p8BitBuff[i]) + 
							                             pDelay->pBuffer[safe]*gain[jdx]);
					}
				}
			}else if (pDelay->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					for(jdx = 0 ; jdx < pDelay->nTabs ; jdx++)
					{
						idx = start[jdx] + i;
						safe = BoundarySafe(pDelay,idx);

						if (jdx==0)
							p16BitBuff[i] = Return16Bit((pDelay->pBuffer[safe])*gain[jdx]);						
						else
							p16BitBuff[i] = Return16Bit(p16BitBuff[i] + 
							                              pDelay->pBuffer[safe]*gain[jdx]);						
					}
				}

			}else{
				pDelay->isProc = false;
				free(start);
				free(offset);
				free(gain);
				return FALSE;
			}
			break;
		case 2:
			if (pDelay->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					for(jdx = 0 ; jdx < pDelay->nTabs ; jdx++)
					{
						idx = start[jdx] + i;
						safe = BoundarySafe(pDelay,idx);
					
						if (jdx==0)
						{
							thisidx = safe*2;
							p8BitBuff[i*2]   = Return8Bit(pDelay->pBuffer[thisidx]*gain[jdx]);
							thisidx = safe*2 + 1;
							p8BitBuff[i*2+1] = Return8Bit(pDelay->pBuffer[thisidx]*gain[jdx]);
						}else{
							thisidx = safe*2;
							p8BitBuff[i*2]   = Return8Bit(Set8Bit(p8BitBuff[i*2]) + 
								                                 pDelay->pBuffer[thisidx]*gain[jdx]);
							thisidx = safe*2 + 1;
							p8BitBuff[i*2+1] = Return8Bit(Set8Bit(p8BitBuff[i*2+1]) + 
								                                 pDelay->pBuffer[thisidx]*gain[jdx]);						
						}
					}
				}
			}else if (pDelay->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					for(jdx = 0 ; jdx < pDelay->nTabs ; jdx++)
					{
						idx = start[jdx] + i;
						safe = BoundarySafe(pDelay,idx);

						if (jdx==0)
						{
							thisidx = safe*2;
							p16BitBuff[i*2]   = Return16Bit(pDelay->pBuffer[thisidx]*gain[jdx]);
							thisidx = safe*2 + 1;
							p16BitBuff[i*2+1] = Return16Bit(pDelay->pBuffer[thisidx]*gain[jdx]);
						}else{
							thisidx = safe*2;
							p16BitBuff[i*2]   = Return16Bit(p16BitBuff[i*2] + 
								                                   pDelay->pBuffer[thisidx]*gain[jdx]);
							thisidx = safe*2 + 1;
							p16BitBuff[i*2+1] = Return16Bit(p16BitBuff[i*2+1] + 
								                                  pDelay->pBuffer[thisidx]*gain[jdx]);						
						}
					}
				}

			}else{
				pDelay->isProc = false;
				free(start);
				free(offset);
				free(gain);
				return FALSE;
			}
			break;
	}
	pDelay->isProc = false;
	free(start);
	free(offset);
	free(gain);
	return TRUE;
}

int	MixTabDelayDry(DELAYDATA *pDelay, char *pData, int nSam)
{
	int   i,idx,safe,thisidx,jdx;
	int   *start, *offset;
	short *p16BitBuff;
	char  *p8BitBuff;
	float *gain,mix;
	
	while(pDelay->isProc == true)
					DoMsgProc();

	pDelay->isProc = true;
	
	offset = (int*)calloc(pDelay->nTabs,sizeof(int));
	start  = (int*)calloc(pDelay->nTabs,sizeof(int));
	gain   = (float*)calloc(pDelay->nTabs,sizeof(float));	
	mix    = pDelay->MixDryGain;
	
	for(i = 0 ; i < pDelay->nTabs ; i++)
	{
		offset[i] = pDelay->nDelay - pDelay->TabDelaySams[i];	
		gain[i]   = pDelay->TabGain[i];
	
		start[i] = MakeStart(pDelay,offset[i]);

	}

	p8BitBuff	= pData;
	p16BitBuff	= (short*)pData;

	switch(pDelay->nCh)
	{
		case 1:
			if (pDelay->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					for(jdx = 0 ; jdx < pDelay->nTabs ; jdx++)
					{
						idx = start[jdx] + i;
						safe = BoundarySafe(pDelay,idx);
						
						if (jdx==0)
							p8BitBuff[i] = Return8Bit(mix*Set8Bit(p8BitBuff[i]) + 
							                             (pDelay->pBuffer[safe]*gain[jdx]));
						else
							p8BitBuff[i] = Return8Bit(Set8Bit(p8BitBuff[i]) + 
							                             (pDelay->pBuffer[safe]*gain[jdx]));
					}
				}
			}else if (pDelay->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					for(jdx = 0 ; jdx < pDelay->nTabs ; jdx++)
					{
						idx = start[jdx] + i;
						safe = BoundarySafe(pDelay,idx);

						if (jdx==0)
							p16BitBuff[i] = Return16Bit(mix*p16BitBuff[i] + 
							                               ((pDelay->pBuffer[safe])*gain[jdx]));						
						else
							p16BitBuff[i] = Return16Bit(p16BitBuff[i] + 
							                              (pDelay->pBuffer[safe]*gain[jdx]));						
					}
				}

			}else{
				pDelay->isProc = false;
				free(start);
				free(offset);
				free(gain);
				return FALSE;
			}
			break;
		case 2:
			if (pDelay->nBit==8)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					for(jdx = 0 ; jdx < pDelay->nTabs ; jdx++)
					{
						idx = start[jdx] + i;
						safe = BoundarySafe(pDelay,idx);
					
						if (jdx==0)
						{
							thisidx = safe*2;
							p8BitBuff[i*2]   = Return8Bit(mix*Set8Bit(p8BitBuff[i*2]) +
								                              (pDelay->pBuffer[thisidx]*gain[jdx]));
							thisidx = safe*2 + 1;
							p8BitBuff[i*2+1] = Return8Bit(mix*Set8Bit(p8BitBuff[i*2+1]) +
								                              (pDelay->pBuffer[thisidx]*gain[jdx]));
						}else{
							thisidx = safe*2;
							p8BitBuff[i*2]   = Return8Bit(Set8Bit(p8BitBuff[i*2]) + 
								                                 pDelay->pBuffer[thisidx]*gain[jdx]);
							thisidx = safe*2 + 1;
							p8BitBuff[i*2+1] = Return8Bit(Set8Bit(p8BitBuff[i*2+1]) + 
								                                 pDelay->pBuffer[thisidx]*gain[jdx]);						
						}
					}
				}
			}else if (pDelay->nBit==16)
			{
				for(i = 0 ; i < nSam ; i++)
				{
					for(jdx = 0 ; jdx < pDelay->nTabs ; jdx++)
					{
						idx = start[jdx] + i;
						safe = BoundarySafe(pDelay,idx);

						if (jdx==0)
						{
							thisidx = safe*2;
							p16BitBuff[i*2]   = Return16Bit(mix*p16BitBuff[i*2] + 
								                            (pDelay->pBuffer[thisidx]*gain[jdx]));
							thisidx = safe*2 + 1;
							p16BitBuff[i*2+1] = Return16Bit(mix*p16BitBuff[i*2+1] +
								                            (pDelay->pBuffer[thisidx]*gain[jdx]));
						}else{
							thisidx = safe*2;
							p16BitBuff[i*2]   = Return16Bit(p16BitBuff[i*2] + 
								                                   pDelay->pBuffer[thisidx]*gain[jdx]);
							thisidx = safe*2 + 1;
							p16BitBuff[i*2+1] = Return16Bit(p16BitBuff[i*2+1] + 
								                                  pDelay->pBuffer[thisidx]*gain[jdx]);						
						}
					}
				}

			}else{
				pDelay->isProc = false;
				free(start);
				free(offset);
				free(gain);
				return FALSE;
			}
			break;
	}
	pDelay->isProc = false;
	free(start);
	free(offset);
	free(gain);
	return TRUE;
	
}
