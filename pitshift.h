int		InitPitchShift(PITCHSHIFT *pShift);
int		MakePitchShift(PITCHSHIFT *pShift);
int		SetShiftAmount(PITCHSHIFT *pShift, float amt);
int     ShiftPitch(PITCHSHIFT *pShift, char *pData, int nSam);
