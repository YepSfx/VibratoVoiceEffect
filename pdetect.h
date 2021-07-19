//Autocorelation Pitch Detect
int	InitACPitchDetect(ACPITCH *pPitch);
int MakeACPitchDetect(ACPITCH *pPitch);
int ACDetectPitch(ACPITCH *pPitch, char *pData, int nSam);
int SetACPitchSNR(ACPITCH *pPitch, float snr);

//Harmonic Product Segment Pitch Detect
int	InitHPSPitchDetect(HPSPITCH *pPitch);
int MakeHPSPitchDetect(HPSPITCH *pPitch);
int HPSDetectPitch(HPSPITCH *pPitch, char *pData, int nSam);

