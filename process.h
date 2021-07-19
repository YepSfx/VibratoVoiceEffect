#include "Shift.h"
#include "ShiftDlg.h"
#include "WaveCtrl.h"
#include "WaveFile.h"
#include "dsp_def.h"

void	InitAudioDev(CWnd *pParent);
void	ClearAudioDev();
int		GetDevInNum();
int		GetDevOutNum();
void	GetDevInName(int ID, char *pBuff);
void	GetDevOutName(int ID, char *pBuff);
int		GetInPortStatus();
int     GetOutPortStatus();
bool	SetInPort(int ID, int nSams, int nBuffs);
bool	SetOutPort(int ID);
int		GetSamRate();
void    SetReal(bool parm);
void	ProcMsg();
int		GetnSamps();
int		GetnBuffs();
void	SetnSamps(int num);
void	SetnBuffs(int num);
bool    ProcNonRealTime(char *inName, char*outName);
bool	CloseInPort();
bool	CloseOutPort();
void	ClearEffect();
void	SetFullFmt();
void	SetPitch(float amt);
void	SetVCamount(float amt);
void	SetVib(int dep, float rate,int moment);
void	SetVibSw(bool vibsw);
float	GetHPSPitch(char *str);
float	GetHPSPower();
int		GetVocalInfo(float *power, float *pitch);
void	SetProcHWND(HWND hWnd);
void	SetPitchParam(float thr,float snr);
void	SetReverbGains(float d, float w);
void	SetRoomSize(float rm);