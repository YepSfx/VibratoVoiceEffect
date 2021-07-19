//PRCRev
int InitPRCRev(PRCREVERB *pRev);
int MakePRCRev(PRCREVERB *pRev, float sec);
int SetPRCRevDryWetGain(PRCREVERB *pRev, float dry, float wet);
int SetPRCRevPreGain(PRCREVERB *pRev, float pre);
int ProcPRCRev(PRCREVERB *pRev, char *pBuff, int nSam);

//JCRev
int InitJCRev(JCREVERB *pRev);
int MakeJCRev(JCREVERB *pRev, float sec);
int SetJCRevDryWetGain(JCREVERB *pRev, float dry, float wet);
int SetJCRevPreGain(JCREVERB *pRev, float pre);
int ProcJCRev(JCREVERB *pRev, char *pBuff, int nSam);

