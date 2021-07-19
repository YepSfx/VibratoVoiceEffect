#include "stdafx.h"
#include "dsp_def.h"
#include "dsp_util.h"
#include "filter.h"
#include <math.h>

#define			DEF_BIQUAD_LEN		3
#define			DEF_1STAP_LEN		2
#define			DEF_2NDAP_LEN		3

INT8 Filter8Bit(INT8 data, float *pBuffer[], float *pCoef[], float gain, int FilterLen[])
{
	int			i, ZeroOrder, PoleOrder;
	float		*pZero, *pPole;
	float		*pZeroCoef, *pPoleCoef;

	pPole		= pBuffer[0];			
	pZero		= pBuffer[1];
	pPoleCoef	= pCoef[0];
	pZeroCoef	= pCoef[1];
	PoleOrder	= FilterLen[0] - 1;
	ZeroOrder	= FilterLen[1] - 1;

	pPole[0] = 0.0;
	
	pZero[0] = gain * Set8Bit(data);

	for (i = ZeroOrder ; i > 0 ; i--) 
	{
		pPole[0] += pZeroCoef[i] * pZero[i];
		pZero[i]  = pZero[i-1];
	}
  
	pPole[0] += pZeroCoef[0] * pZero[0];

	for (i = PoleOrder ; i > 0 ; i--) 
	{
		pPole[0] +=  -pPoleCoef[i] * pPole[i];
		pPole[i]  =  pPole[i-1];
	}

    return Return8Bit(pPole[0]);
}

INT16 Filter16Bit(INT16 data, float *pBuffer[], float *pCoef[], float gain, int FilterLen[])
{
	int			i, ZeroOrder, PoleOrder;
	float		*pZero, *pPole;
	float		*pZeroCoef, *pPoleCoef;

	pPole		= pBuffer[0];			
	pZero		= pBuffer[1];
	pPoleCoef	= pCoef[0];
	pZeroCoef	= pCoef[1];
	PoleOrder	= FilterLen[0] - 1;
	ZeroOrder	= FilterLen[1] - 1;

	pPole[0] = 0.0;
	
	pZero[0] = gain * data;

	for (i = ZeroOrder ; i > 0 ; i--) 
	{
		pPole[0] += pZeroCoef[i] * pZero[i];
		pZero[i]  = pZero[i-1];
	}
  
	pPole[0] += pZeroCoef[0] * pZero[0];

	for (i = PoleOrder ; i > 0 ; i--) 
	{
		pPole[0] +=  -pPoleCoef[i] * pPole[i];
		pPole[i]  =  pPole[i-1];
	}

    return Return16Bit(pPole[0]);
}

int InitFilter(FILTER *pFilter)
{
	int i;

	while(pFilter->isProc == true)
						DoMsgProc();

	pFilter->isProc			= true;

	for(i = 0 ; i < pFilter->nCh ; i++)
	{
		if (pFilter->PoleBuffer[i] != NULL)
		{
			free(pFilter->PoleBuffer[i]);
			pFilter->PoleBuffer[i]	= NULL;
		}
		if (pFilter->ZeroBuffer[i] != NULL)
		{
			free(pFilter->ZeroBuffer[i]);
			pFilter->ZeroBuffer[i]	= NULL;
		}
	}
	
	if (pFilter->PoleCoeff != NULL)
	{
		free(pFilter->PoleCoeff);
		pFilter->PoleCoeff		= NULL;
		pFilter->FilterLen[0]	= 0;
	}

	if (pFilter->ZeroCoeff != NULL)
	{
		free(pFilter->ZeroCoeff);
		pFilter->ZeroCoeff		= NULL;
		pFilter->FilterLen[1]	= 0;
	}

	pFilter->isProc			= false;	
	
	return TRUE;
}


int MakeFilter(FILTER *pFilter, float *pCoef[], int Len[])
{
	int i;
	float	*pPole, *pZero;

	while(pFilter->isProc == true)
							DoMsgProc();

	if ((pFilter->FilterLen[0] == Len[0])&&(pFilter->FilterLen[1] == Len[1]))
	{
		pFilter->isProc			= true;
		
		free(pFilter->PoleCoeff);
		free(pFilter->ZeroCoeff);
		pFilter->PoleCoeff		= pCoef[0];
		pFilter->ZeroCoeff		= pCoef[1];
	}else{
		InitFilter(pFilter);

		pFilter->isProc = true;	

		pFilter->PoleCoeff		= pCoef[0];
		pFilter->FilterLen[0]	= Len[0];	
		pFilter->ZeroCoeff		= pCoef[1];
		pFilter->FilterLen[1]	= Len[1];
	
		for(i = 0 ; i < pFilter->nCh ; i++)
		{
			pZero = (float*)calloc(Len[1],sizeof(&pZero));
			pFilter->ZeroBuffer[i]		= pZero;
			pPole = (float*)calloc(Len[0],sizeof(&pPole));
			pFilter->PoleBuffer[i]		= pPole;	
		}
	}
	pFilter->isProc			= false;		

	return TRUE;
}

int MakeBiQuadFilter(FILTER *pFilter, float freq[2], float damp[2])
{
	int   i;
	float OmegaP, OmegaZ;
	float ResonFreq, AntiResonFreq;
	float rp,rz;
	float *pPole, *pZero;

	ResonFreq		= freq[0];
	AntiResonFreq	= freq[1];
	rp				= damp[0];
	rz				= damp[1];
	
	OmegaP	= (ResonFreq		/ pFilter->nSamplePerSec) * 2 * PI;	//   Cutoff Freq
	OmegaZ	= (AntiResonFreq	/ pFilter->nSamplePerSec) * 2 * PI;	//---------------- = Normalize OmegaC
																	// (Sample Rate/2)

	if (rp >= 1.0)
				rp = 0.99;
	
	pPole	= (float*)calloc(DEF_BIQUAD_LEN,  sizeof(&pPole));
	pZero	= (float*)calloc(DEF_BIQUAD_LEN,  sizeof(&pZero));

	pZero[0] = 1.0;
	pZero[1] = -(float)(2.0 * rz * cos(OmegaZ));				
	pZero[2] = (rz * rz);    									

	pPole[0] = 1.0;
	pPole[1] = -(float)(2.0 * rp * cos(OmegaP));				
	pPole[2] = (rp * rp);		    							

	while(pFilter->isProc == true)
							DoMsgProc();

	if ((pFilter->FilterLen[0] == DEF_BIQUAD_LEN)&&(pFilter->FilterLen[1] == DEF_BIQUAD_LEN))
	{
		pFilter->isProc = true;	
		
		free(pFilter->PoleCoeff);
		free(pFilter->ZeroCoeff);
		pFilter->PoleCoeff		= pPole;
		pFilter->ZeroCoeff		= pZero;
	}else{
		InitFilter(pFilter);	
		
		pFilter->isProc = true;	

		pFilter->PoleCoeff		= pPole;
		pFilter->FilterLen[0]	= DEF_BIQUAD_LEN;	
		pFilter->ZeroCoeff		= pZero;
		pFilter->FilterLen[1]	= DEF_BIQUAD_LEN;
	
		for(i = 0 ; i < pFilter->nCh ; i++)
		{
			pZero = (float*)calloc(DEF_BIQUAD_LEN,sizeof(&pZero));
			pFilter->ZeroBuffer[i]		= pZero;
			pPole = (float*)calloc(DEF_BIQUAD_LEN,sizeof(&pPole));
			pFilter->PoleBuffer[i]		= pPole;	
		}

	}
	pFilter->isProc = false;	
	
	return TRUE;
}

int Make1stAP(FILTER *pFilter, float cutoff)
{
	int   i;
	float c, *pPole, *pZero;

	c = (tan(PI*cutoff/pFilter->nSamplePerSec) - 1)/(tan(PI*cutoff/pFilter->nSamplePerSec) + 1);
	pPole	= (float*)calloc(DEF_1STAP_LEN,  sizeof(&pPole));
	pZero	= (float*)calloc(DEF_1STAP_LEN,  sizeof(&pZero));

	pPole[0] = 1;
	pPole[1] = c;
	pZero[0] = c;
	pZero[1] = 1;
	
	while(pFilter->isProc == true)
							DoMsgProc();

	if ((pFilter->FilterLen[0] == DEF_1STAP_LEN)&&(pFilter->FilterLen[1] == DEF_1STAP_LEN))
	{
		pFilter->isProc = true;	
		free(pFilter->PoleCoeff);
		free(pFilter->ZeroCoeff);
		pFilter->PoleCoeff		= pPole;
		pFilter->ZeroCoeff		= pZero;
	}else{
		InitFilter(pFilter);	

		pFilter->isProc = true;	

		pFilter->PoleCoeff		= pPole;
		pFilter->FilterLen[0]	= DEF_1STAP_LEN;	
		pFilter->ZeroCoeff		= pZero;
		pFilter->FilterLen[1]	= DEF_1STAP_LEN;
	
		for(i = 0 ; i < pFilter->nCh ; i++)
		{
			pZero = (float*)calloc(DEF_1STAP_LEN,sizeof(&pZero));
			pFilter->ZeroBuffer[i]		= pZero;
			pPole = (float*)calloc(DEF_1STAP_LEN,sizeof(&pPole));
			pFilter->PoleBuffer[i]		= pPole;	
		}

	}
	pFilter->isProc = false;	
	
	return TRUE;
}

int Make2ndAP(FILTER *pFilter, float bw, float cutoff)
{
	int	  i;	
	float c, d, *pPole, *pZero;

	c = (tan(PI*bw/pFilter->nSamplePerSec) - 1)/(tan(PI*bw/pFilter->nSamplePerSec) + 1);
	d = -cos(2*PI*cutoff/pFilter->nSamplePerSec);
	pPole	= (float*)calloc(DEF_2NDAP_LEN,  sizeof(&pPole));
	pZero	= (float*)calloc(DEF_2NDAP_LEN,  sizeof(&pZero));

	pPole[0] = 1;
	pPole[1] = d*(1-c);
	pPole[2] = -c; 
	pZero[0] = -c;
	pZero[1] = d*(1-c);
	pZero[2] = 1;
	
	while(pFilter->isProc == true)
							DoMsgProc();

	if ((pFilter->FilterLen[0] == DEF_2NDAP_LEN)&&(pFilter->FilterLen[1] == DEF_2NDAP_LEN))
	{
		pFilter->isProc = true;	
		free(pFilter->PoleCoeff);
		free(pFilter->ZeroCoeff);
		pFilter->PoleCoeff		= pPole;
		pFilter->ZeroCoeff		= pZero;
	}else{
		InitFilter(pFilter);	

		pFilter->isProc = true;	

		pFilter->PoleCoeff		= pPole;
		pFilter->FilterLen[0]	= DEF_2NDAP_LEN;	
		pFilter->ZeroCoeff		= pZero;
		pFilter->FilterLen[1]	= DEF_2NDAP_LEN;
	
		for(i = 0 ; i < pFilter->nCh ; i++)
		{
			pZero = (float*)calloc(DEF_2NDAP_LEN,sizeof(&pZero));
			pFilter->ZeroBuffer[i]		= pZero;
			pPole = (float*)calloc(DEF_2NDAP_LEN,sizeof(&pPole));
			pFilter->PoleBuffer[i]		= pPole;	
		}

	}
	pFilter->isProc = false;	
	
	return TRUE;
}

int ProcFilter(FILTER *pFilter, char *pBuff, int nSam)
{
	int		i;
	INT8	*p8BitData,  Out8Bit[2];
	INT16	*p16BitData, Out16Bit[2];
	float	*pBufCh1[2], *pBufCh2[2], *pCoef[2];

	while(pFilter->isProc == true)
							DoMsgProc();

	pFilter->isProc = true;	

	if ((pFilter->FilterLen[0] == 0)&&(pFilter->FilterLen[1] == 0))
	{
		pFilter->isProc = false;		
		return FALSE;
	}

	switch(pFilter->nCh){
		case 1:
			pBufCh1[0] = pFilter->PoleBuffer[0];
			pBufCh1[1] = pFilter->ZeroBuffer[0];
			pCoef[0]   = pFilter->PoleCoeff;
			pCoef[1]   = pFilter->ZeroCoeff;	
			
			if (pFilter->nBit == 8){
				p8BitData = (INT8*)pBuff;
				for(i = 0 ; i < nSam ; i++)				
				{
					Out8Bit[0] = Filter8Bit(p8BitData[i],pBufCh1,pCoef,
											pFilter->gain,pFilter->FilterLen);
					p8BitData[i] = Out8Bit[0];
				}
			}else if (pFilter->nBit == 16){
				p16BitData = (INT16*)pBuff;
				for(i = 0 ; i < nSam ; i++)
				{
					Out16Bit[0] = Filter16Bit(p16BitData[i],pBufCh1,pCoef,
											  pFilter->gain,pFilter->FilterLen);
					p16BitData[i] = Out16Bit[0];
				}
			}else{
				pFilter->isProc = false;
				return FALSE;
			}
			break;
		case 2:
			pBufCh1[0] = pFilter->PoleBuffer[0];
			pBufCh1[1] = pFilter->ZeroBuffer[0];
			pBufCh2[0] = pFilter->PoleBuffer[1];
			pBufCh2[1] = pFilter->ZeroBuffer[1];
			pCoef[0]   = pFilter->PoleCoeff;
			pCoef[1]   = pFilter->ZeroCoeff;	

			if (pFilter->nBit == 8){
				p8BitData = (INT8*)pBuff;
				for(i = 0 ; i < nSam ; i++)
				{
					Out8Bit[0] = Filter8Bit(p8BitData[i*2],pBufCh1,pCoef,
											pFilter->gain,pFilter->FilterLen);
					Out8Bit[1] = Filter8Bit(p8BitData[i*2+1],pBufCh2,pCoef,
											pFilter->gain,pFilter->FilterLen);						
					p8BitData[i*2]   = Out8Bit[0];
					p8BitData[i*2+1] = Out8Bit[1];
				}
			}else if (pFilter->nBit == 16){
				p16BitData = (INT16*)pBuff;
				for(i = 0 ; i < nSam ; i++)
				{
					Out16Bit[0] = Filter16Bit(p16BitData[i*2],pBufCh1,pCoef,
											  pFilter->gain,pFilter->FilterLen);
					Out16Bit[1] = Filter16Bit(p16BitData[i*2+1],pBufCh2,pCoef,
											  pFilter->gain,pFilter->FilterLen);					
					p16BitData[i*2]	  = Out16Bit[0];
					p16BitData[i*2+1] = Out16Bit[1];				
				}
			}else{
				pFilter->isProc = false;
				return FALSE;
			}		
			break;
	}

	pFilter->isProc = false;
	
	return TRUE;
}

