// ShiftDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Shift.h"
#include "ShiftDlg.h"
#include <stdio.h>
#include "process.h"
#include "dsp_util.h"
#include <math.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

float	tck = 0.0008333;

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShiftDlg dialog

CShiftDlg::CShiftDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CShiftDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShiftDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CShiftDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShiftDlg)
	DDX_Control(pDX, IDC_EDIT3, m_drygain);
	DDX_Control(pDX, IDC_BUTTON7, m_setrmsize);
	DDX_Control(pDX, IDC_EDIT10, m_rmsize);
	DDX_Control(pDX, IDC_EDIT4, m_wetgain);
	DDX_Control(pDX, IDC_BUTTON5, m_pitchset);
	DDX_Control(pDX, IDC_EDIT9, m_snr);
	DDX_Control(pDX, IDC_EDIT8, m_thr);
	DDX_Control(pDX, IDC_EDIT7, m_hold);
	DDX_Control(pDX, IDC_EDIT6, m_rate);
	DDX_Control(pDX, IDC_EDIT5, m_dep);
	DDX_Control(pDX, IDC_BUTTON1, m_setdev);
	DDX_Control(pDX, IDC_EDIT2, m_nBuffers);
	DDX_Control(pDX, IDC_EDIT1, m_nSams);
	DDX_Control(pDX, IDC_PITCH,  m_pit);
	DDX_Control(pDX, IDC_POWER,  m_pow);
	DDX_Control(pDX, IDC_NOTE,	 m_note);
	DDX_Control(pDX, IDC_COMBO2, m_wave_out);
	DDX_Control(pDX, IDC_COMBO1, m_wave_in);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CShiftDlg, CDialog)
	//{{AFX_MSG_MAP(CShiftDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnSelchangeCombo2)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON15, OnButton15)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnButton7)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CShiftDlg message handlers

BOOL CShiftDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	int i;
	STRING tmp;

	sprintf(tmp,"%d",GetnSamps());
	m_nSams.SetWindowText(tmp);
	
	sprintf(tmp,"%d",GetnBuffs());
	m_nBuffers.SetWindowText(tmp);

	InitAudioDev(this);
	for ( i = 0 ; i < GetDevOutNum() ; i++)
	{
		GetDevOutName(i,tmp);
		m_wave_out.AddString(tmp);	
	}
	m_wave_out.SetCurSel(0);
	SetOutPort(0);

	for ( i = 0 ; i < GetDevInNum() ; i++)
	{
		GetDevInName(i,tmp);
		m_wave_in.AddString(tmp);	
	}
	m_wave_in.SetCurSel(0);
	SetInPort(0,GetnSamps(),GetnBuffs());

	m_TimerID = SetTimer(1,100,NULL);

	SetProcHWND(this->m_hWnd);

	m_hold.SetWindowText("500");
	m_dep.SetWindowText("5");
	m_rate.SetWindowText("2");

	m_thr.SetWindowText("-40.0");
	m_snr.SetWindowText("0.4");
	
	m_drygain.SetWindowText("0.7");
	m_wetgain.SetWindowText("0.3");
	m_rmsize.SetWindowText("2.0");

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control	
}

void CShiftDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CShiftDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CShiftDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CShiftDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	int nKey;
    switch(pMsg->message){
		case WM_KEYDOWN:
             nKey = (int)pMsg->wParam;
             switch(nKey){
			        case VK_RETURN: //Enter
                    case VK_ESCAPE: // ESC?
                    return TRUE;
             }
            break;
    }
	
		
	return CDialog::PreTranslateMessage(pMsg);
}

void CShiftDlg::OnSelchangeCombo1() 
{
	// TODO: Add your control notification handler code here	
	int ID;
	ID = m_wave_in.GetCurSel();
	SetInPort(ID,GetnSamps(),GetnBuffs());	
}

void CShiftDlg::OnSelchangeCombo2() 
{
	// TODO: Add your control notification handler code here
	int ID;
	ID = m_wave_out.GetCurSel();
	SetOutPort(ID);	
}

void CShiftDlg::OnButton1() 
{
	// TODO: Add your control notification handler code here
	STRING	tmp;
	int		ID;	

	CloseInPort();
	DoMsgProc();
	CloseOutPort();
	DoMsgProc();

	m_nSams.GetWindowText(tmp,sizeof(tmp));
	SetnSamps(atoi(tmp));

	m_nBuffers.GetWindowText(tmp,sizeof(tmp));
	SetnBuffs(atoi(tmp));

	ID = m_wave_in.GetCurSel();
	SetInPort(ID,GetnSamps(),GetnBuffs());		

	ID = m_wave_out.GetCurSel();
	SetOutPort(ID);	
}

void CShiftDlg::OnButton2() 
{
	// TODO: Add your control notification handler code here
	WinExec("sndvol32.exe",SW_SHOWNOACTIVATE);

}

void CShiftDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	KillTimer(m_TimerID);
	ClearAudioDev();
	CDialog::OnClose();
}

void CShiftDlg::OnButton3() 
{
	// TODO: Add your control notification handler code here
	CAboutDlg aboutbox;

	aboutbox.DoModal();
	
}

void CShiftDlg::OnButton4() 
{
	// TODO: Add your control notification handler code here
	STRING FileName,SaveName;
	STRING msg;

	CFileDialog dlg(TRUE,_T("WAV"),_T("*.WAV"),OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
					_T("WAVE FILE (*.WAV)|*.WAV|ALL FILE (*.*)|*.*|"));

	if(dlg.DoModal()==IDOK)
	{
		KillTimer(m_TimerID);

		strcpy(FileName,dlg.GetPathName());
		strcpy(SaveName,dlg.GetFileTitle());
		strcat(SaveName,"SFX-ac.wav");
		sprintf(msg,"Finish! Check '%s'.",SaveName);
		m_TimerID = SetTimer(1,50,NULL);		
		if (ProcNonRealTime(FileName, SaveName))
			MessageBox(msg, "Vocal Vibrato (Autocorrelation)", MB_OK );
		else
			MessageBox("Unsupport File Format!", "Vocal Vibrato (Autocorrelation)", MB_OK );
		KillTimer(m_TimerID);
		m_TimerID = SetTimer(1,100,NULL);
	}		
}

void CShiftDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CShiftDlg::MakeRange()
{
/*
	int		idx = 0, tmp = 0;
	float	amt = 0.0, ovr = 0.0;
	STRING  wrk ;
	idx = m_sld.GetPos();
	
	if (idx == 1200)
	{
		amt = 0;
		SetPitch(0);
	}else if (idx > 1200) {
		tmp = idx - 1200;
		amt = (float)tmp*tck*-1;
		SetPitch(amt);
	}else if (idx < 1200){
		tmp = 1200 - idx;
		amt = (float)tmp*tck*0.5;
		SetPitch(amt);
	}
	sprintf(wrk,"%.4f",amt);
	m_pitch.SetWindowText(wrk);
*/
}

void CShiftDlg::OnButton5() 
{
	// TODO: Add your control notification handler code here
	STRING	tmp1, tmp2;
	float thr, snr;

	m_thr.GetWindowText(tmp1,sizeof(tmp1));
	m_snr.GetWindowText(tmp2,sizeof(tmp2));

	thr = atof(tmp1);
	snr = atof(tmp2);
	
	SetPitchParam(thr,snr);
}

void CShiftDlg::OnButton13() 
{
	// TODO: Add your control notification handler code here
	float amt;
	STRING tmp;

//	m_sld.SetPos(800);
	amt = tck*400*0.5;
	SetPitch(amt);
	sprintf(tmp,"%.4f",amt);
	m_pitch.SetWindowText(tmp);

}

void CShiftDlg::OnButton10() 
{
	// TODO: Add your control notification handler code here
	float amt;
	STRING tmp;

//	m_sld.SetPos(1100);
	amt = tck*100*0.5;
	SetPitch(amt);

	sprintf(tmp,"%.4f",amt);
	m_pitch.SetWindowText(tmp);

}

void CShiftDlg::OnButton11() 
{
	// TODO: Add your control notification handler code here
	float amt;
	STRING tmp;

//	m_sld.SetPos(1000);	
	amt = tck*200*0.5;
	SetPitch(amt);
	sprintf(tmp,"%.4f",amt);
	m_pitch.SetWindowText(tmp);

}

void CShiftDlg::OnButton12() 
{
	// TODO: Add your control notification handler code here
	float amt;
	STRING tmp;

//	m_sld.SetPos(900);	
	amt = tck*300*0.5;
	SetPitch(amt);
	sprintf(tmp,"%.4f",amt);
	m_pitch.SetWindowText(tmp);


}

void CShiftDlg::OnButton7() 
{
	// TODO: Add your control notification handler code here

	STRING	tmp;
	int		ID;	

	CloseInPort();
	DoMsgProc();
	CloseOutPort();
	DoMsgProc();

	m_nSams.GetWindowText(tmp,sizeof(tmp));
	SetnSamps(atoi(tmp));

	m_nBuffers.GetWindowText(tmp,sizeof(tmp));
	SetnBuffs(atoi(tmp));

	m_rmsize.GetWindowText(tmp,sizeof(tmp));
	SetRoomSize(atof(tmp));

	ID = m_wave_in.GetCurSel();
	SetInPort(ID,GetnSamps(),GetnBuffs());		

	ID = m_wave_out.GetCurSel();
	SetOutPort(ID);	

}

void CShiftDlg::OnButton8() 
{
	// TODO: Add your control notification handler code here
	float amt;
	STRING tmp;

//	m_sld.SetPos(1500);	
	amt = tck*300*-1;
	SetPitch(tck*300*-1);		
	sprintf(tmp,"%.4f",amt);
	m_pitch.SetWindowText(tmp);

}

void CShiftDlg::OnButton9() 
{
	// TODO: Add your control notification handler code here
	float amt;
	STRING tmp;

//	m_sld.SetPos(1600);	
	amt = tck*400*-1;
	SetPitch(tck*400*-1);			
	sprintf(tmp,"%.4f",amt);
	m_pitch.SetWindowText(tmp);

}

void CShiftDlg::OnButton14() 
{
	// TODO: Add your control notification handler code here
	STRING	tmp;

	//m_vc.GetWindowText(tmp,sizeof(tmp));
	SetVCamount((float)atof(tmp));	
}

void CShiftDlg::OnButton15() 
{
	// TODO: Add your control notification handler code here
	STRING	tmp1, tmp2, tmp3;
	int moment;

	m_dep.GetWindowText(tmp1,sizeof(tmp1));
	m_rate.GetWindowText(tmp2,sizeof(tmp2));
	m_hold.GetWindowText(tmp3,sizeof(tmp3));
	moment = atoi(tmp3);
	moment = moment/100;
	SetVib(atof(tmp1),atof(tmp2),moment);
	
}

void CShiftDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	SetVibSw(true);		
}

void CShiftDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	SetVibSw(false);	
}

void CShiftDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	STRING	tmp,str;
	float	pit,pw, tmpnote;
	int		nt;
	
	GetVocalInfo(&pw, &pit);
	
	sprintf(tmp,"%.2f Hz", pit);
	m_pit.SetWindowText(tmp);

	tmpnote = PitchToMIDINote(pit);	
	VocalMIDINoteToString(tmpnote, str);
	sprintf(tmp,"%s (%.2f)",str,tmpnote);
	m_note.SetWindowText(tmp);
	
	sprintf(tmp,"%.2f db",pw);
	m_pow.SetWindowText(tmp);
	
	CDialog::OnTimer(nIDEvent);
}

void CShiftDlg::OnButton6() 
{
	STRING	tmp1, tmp2;
	float   d, w;

	m_drygain.GetWindowText(tmp1,sizeof(tmp1));
	m_wetgain.GetWindowText(tmp2,sizeof(tmp2));

	d = atof(tmp1);
	w = atof(tmp2);
	
	SetReverbGains(d,w);
}
