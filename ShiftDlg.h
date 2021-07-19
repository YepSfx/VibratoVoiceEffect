// ShiftDlg.h : header file
//

#if !defined(AFX_SHIFTDLG_H__2C5E5F8B_5531_4CB9_86F8_B56E8BDA4934__INCLUDED_)
#define AFX_SHIFTDLG_H__2C5E5F8B_5531_4CB9_86F8_B56E8BDA4934__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CShiftDlg dialog

class CShiftDlg : public CDialog
{
// Construction
public:
	int m_note_int;
	UINT m_TimerID;
	void MakeRange();
	CShiftDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CShiftDlg)
	enum { IDD = IDD_SHIFT_DIALOG };
	CEdit	m_drygain;
	CButton	m_setrmsize;
	CEdit	m_rmsize;
	CEdit	m_wetgain;
	CButton	m_pitchset;
	CEdit	m_snr;
	CEdit	m_thr;
	CEdit	m_hold;
	CEdit	m_rate;
	CEdit	m_dep;
	CEdit	m_pitch;
	CButton	m_setdev;
	CEdit	m_nBuffers;
	CEdit	m_nSams;
	CStatic m_pit;
	CStatic m_pow;
	CStatic m_note;
	CComboBox	m_wave_out;
	CComboBox	m_wave_in;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShiftDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CShiftDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeCombo1();
	afx_msg void OnSelchangeCombo2();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnClose();
	afx_msg void OnButton3();
	afx_msg void OnButton4();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnButton13();
	afx_msg void OnButton10();
	afx_msg void OnButton11();
	afx_msg void OnButton12();
	afx_msg void OnButton7();
	afx_msg void OnButton8();
	afx_msg void OnButton9();
	afx_msg void OnButton14();
	afx_msg void OnButton15();
	afx_msg void OnRadio2();
	afx_msg void OnRadio1();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButton5();
	afx_msg void OnButton6();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHIFTDLG_H__2C5E5F8B_5531_4CB9_86F8_B56E8BDA4934__INCLUDED_)
