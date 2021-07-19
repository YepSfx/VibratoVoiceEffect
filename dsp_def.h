#define			DEF_8BIT_PLUS		127
#define			DEF_8BIT_MINUS		128
#define			DEF_16BIT_MAX		32767
#define			DEF_16BIT_MIN		-32768
#define			DEF_8BIT_MAX		127
#define			DEF_8BIT_MIN		-127
#define			DEF_WIN_HAN			0
#define			DEF_WIN_HAM			1
#define			DEF_WIN_BLACK		2

#define			PI					3.14159265358979323846

#define			TRUE				1
#define			FALSE				0

#define			DEF_TUNE_A4			440.0

typedef char			STRING[256];
typedef char			INT8;
typedef short			INT16;

typedef struct{
			int				nCh;
			int				nBit;
			int				nSamBuff;		//Internal Purpose(Number of Samples in Input Buffer)
			int				nSamplePerSec;  //e.g : 1s Buffer + nSamBuff + offset
			int				nDelay;
			int				in_idx;			//Internal Purpose
			int				out_idx;		//Internal Purpose
			float			*pBuffer;
			float			WetGain;
			float			DryGain;
			float			MixDryGain;
			float			*TabGain;
			int				nTabs;
			int				*TabDelaySams;
			bool			isProc;			//Internal Purpose
}DELAYDATA;

typedef struct{
			int				nCh;
			int				nBit;
			int				nSamplePerSec; 
			DELAYDATA		Buffer[2];		//Internal Purpose
			float			Fader[2];		//Internal Purpose
			float			 Delay[2];		//Internal Purpose
			float			Shift[2];		//Internal Purpose
			float			Vol[2];			//Internal Purpose
			bool			isProc;
}PITCHSHIFT;

typedef struct{
			int				nCh;
			int				nBit;
			int				nSamplePerSec;
			DELAYDATA		Buffer;
			unsigned long	lfotick;		//LFO Ticks
			float			depth;			//Vibrato Depth in Samples
			float			rate;			//Vibrato Rate in Hz
			float			lfoloc;			//DO NOT ACCESS rate, depth DIRECTLY!!!!!
			bool			lfosw;
			bool			isProc; 
}VIBRATO;

typedef struct{
			int				nCh;
			int				nBit;
			int				nSamplePerSec;
			DELAYDATA		Buffer;
			int				BufferLen;
			int				Count;
			int				Seg;
			float			CurrentPitch;
			float			power;
			float			NoiseTHR;
			bool			isProc;
}ACPITCH;									

typedef struct{
			int				nCh;
			int				nBit;
			int				nSamplePerSec;
			DELAYDATA		Buffer;
			float			*pWindow;
			int				BufferLen;
			int				Count;
			int				Seg;
			float			CurrentPitch;
			float			power;
			bool			isProc;
}HPSPITCH;									

typedef struct{
			int				nCh;
			int				nBit;
			int				nSamplePerSec; 
			int				FilterLen[2];		//0 : Pole, 1 : Zero			
			float			*ZeroCoeff;
			float			*PoleCoeff;
			float			*ZeroBuffer[2];		//0 : Ch1, 1 : Ch2
			float			*PoleBuffer[2];		//0 : Ch1, 1 : Ch2
			float			gain;			
			bool			isProc;
}FILTER;									

typedef struct{
			int				nCh;
			int				nBit;
			int				nSamplePerSec;
			DELAYDATA		AllPass[2];
			DELAYDATA		Comb[2];
			float			ComGain[2];
			float			DryGain;			//0.0 - 1.0
			float			WetGain;			//0.0 - 1.0
			float			DecaySec;			
			float			Diffuse;			//0.0 - 0.5
			float			PreGain;			//0.0 - 0.5
			bool			isProc;
}PRCREVERB;

typedef struct{
			int				nCh;
			int				nBit;
			int				nSamplePerSec;
			DELAYDATA		AllPass[3];
			DELAYDATA		Comb[4];
			DELAYDATA		Out[2];
			float			ComGain[4];
			float			DryGain;			//0.0 - 1.0
			float			WetGain;			//0.0 - 1.0
			float			DecaySec;			
			float			Diffuse;			//0.0 - 0.5
			float			PreGain;			//0.0 - 0.5
			bool			isProc;
}JCREVERB;