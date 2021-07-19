int		InitFilter(FILTER *pFilter);
int		MakeFilter(FILTER *pFilter, float *pCoef[], int Len[]);
int		Make1stAP(FILTER *pFilter, float cutoff);
int		Make2ndAP(FILTER *pFilter, float bw, float cutoff);
int		MakeBiQuadFilter(FILTER *pFilter, float freq[2], float damp[2]);//0 : Pole, 1 : Zero
int		ProcFilter(FILTER *pFilter, char *pBuff, int nSam);
