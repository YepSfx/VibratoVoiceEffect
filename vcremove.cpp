#include "stdafx.h"
#include "dsp_def.h"
#include "dsp_util.h"
#include <math.h>

int VocalRemove(char *pData, int nSam, int nBit, float Th)
{
	int i;
	char	*p8BitData , Tmp8L,  Tmp8R;
	INT16	*p16BitData, Tmp16L, Tmp16R;
	
	switch(nBit){
		case 8:
			p8BitData	= pData;
			for (i = 0 ; i < nSam ; i++)
			{
				Tmp8L = (INT8)(Set8Bit(p8BitData[i*2]) - Set8Bit(p8BitData[i*2+1]*Th));
				Tmp8R = (INT8)(Set8Bit(p8BitData[i*2+1])   - Set8Bit(p8BitData[i*2]*Th));

				p8BitData[i*2]		= Return8Bit(Tmp8L);
				p8BitData[i*2+1]	= Return8Bit(Tmp8R);
			}
			break;
		case 16:
			p16BitData	= (INT16*)pData;
			for (i = 0 ; i < nSam ; i++)
			{
				Tmp16L = (INT16)(p16BitData[i*2]   - p16BitData[i*2+1]*Th);
				Tmp16R = (INT16)(p16BitData[i*2+1] - p16BitData[i*2]*Th);

				p16BitData[i*2]		= Return16Bit(Tmp16L);
				p16BitData[i*2+1]	= Return16Bit(Tmp16R);
			}
			break;
	}
	return TRUE;
}
