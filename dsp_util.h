void		DoMsgProc();
INT8		Set8Bit(INT8 data);
INT8		Return8Bit(float data);
INT16		Return16Bit(float data);
int			GetnSam(int nByte, int nBit, int nCh);
int			GetnByte(int nSam, int nBit, int nCh);
int			MakeWindow(int nSam, int win, float *pWin);
//Pitch Detect Uitlity Function
float		PitchToMIDINote(float Pitch);
int			VocalMIDINoteToString(int note,char *str);

float		log2(float val);