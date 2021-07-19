#include "stdafx.h"
#include "dsp_def.h"
#include "dsp_util.h"
#include <math.h>
#include <stdio.h>
#define		DEF_VOCAL_LO	39
#define		DEF_VOCAL_HI	80
#define		DEF_LOG10_2		log10(2)	

const char *NOTENAME[] = {"C","C#","D","D#",
                          "E","F","F#","G",
						  "G#","A","A#","B"};

inline float log2(float val)
{
   int * const  exp_ptr = reinterpret_cast <int *> (&val);
   int          x = *exp_ptr;
   const int    log_2 = ((x >> 23) & 255) - 128;
   x &= ~(255 << 23);
   x += 127 << 23;
   *exp_ptr = x;

   return (val + log_2);
}

void DoMsgProc()
{
	MSG		msg;
	
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

INT8 Set8Bit(INT8 data)
{
//Turn (0-255) INT8 to (-128 - 127) INT8 
	INT8 ans;

	if (data&0x80)
		ans = -(DEF_8BIT_MINUS + data);
	else 
		ans = DEF_8BIT_PLUS - data;

	return ans;
}

INT8 Return8Bit(float data)
{
//Turn float to (0-255) INT8	
	INT8 ans;

	if (data > DEF_8BIT_MAX) 
			data = DEF_8BIT_MAX;

	if (data < DEF_8BIT_MIN)
			data = DEF_8BIT_MIN;

	if ((INT8)data&0x80)
		ans = -(DEF_8BIT_MINUS + (INT8)data);
	else 
		ans = DEF_8BIT_PLUS - (INT8)data;

	return ans;
}

INT16 Return16Bit(float data)
{
//Check Upper/Lower Bound for INT16	
	INT16 ans;

	if (data > DEF_16BIT_MAX) 
			data = DEF_16BIT_MAX;

	if (data < DEF_16BIT_MIN)
			data = DEF_16BIT_MIN;

	ans = (INT16)data;
	
	return ans;
}

int GetnSam(int nByte,int nBit,int nCh)
{
	//nByte -> Buffer Length in Bytes
	return (8*nByte)/(nBit*nCh);
}

int GetnByte(int nSam, int nBit, int nCh)
{
	return (nSam*nBit*nCh)/8;
}

int MakeWindow(int nSam, int win, float *pWin)
{
	int		i;
	float	phase = 0, delta;

	delta = 2 * PI / (float)nSam;

	switch(win){
		case DEF_WIN_HAN:
			for (i = 0 ; i < nSam ; i++)	
			{
				pWin[i] = (0.5  - 0.5*cos(phase));
				phase += delta;
			}
			break;
		case DEF_WIN_HAM:
			for (i = 0 ; i < nSam ; i++)	
			{
				pWin[i] = (0.54  - 0.46*cos(phase));
				phase += delta;
			}
			break;
		case DEF_WIN_BLACK:
			for (i = 0 ; i < nSam ; i++)	
			{
				pWin[i] = (0.42 - 0.5*cos(phase) + 0.08*cos(2*phase));
				phase += delta;
			}
			break;
	}
	return TRUE;
}

float PitchToMIDINote(float Pitch)
{
	
	float out = 0;

	if (Pitch < 0)
			Pitch = 0;

	out = fabs( 12 * (log10(Pitch/DEF_TUNE_A4)/DEF_LOG10_2) + 69 );

	if (out > 127) 
			out = -1;

	return out;
}

int VocalMIDINoteToString(int note,char *str)
{
	if ((note > DEF_VOCAL_LO) && (note < DEF_VOCAL_HI))  
	{
		sprintf(str,"%s%d",NOTENAME[note%12],(note/12 - 1));	
	}else{
		sprintf(str,"---");
	}
	return 1;

}
