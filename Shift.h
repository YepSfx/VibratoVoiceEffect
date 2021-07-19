// Shift.h : main header file for the SHIFT application
//

#if !defined(AFX_SHIFT_H__F8D8D69C_764C_4B5D_B957_0AAFE01E4F0C__INCLUDED_)
#define AFX_SHIFT_H__F8D8D69C_764C_4B5D_B957_0AAFE01E4F0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CShiftApp:
// See Shift.cpp for the implementation of this class
//

class CShiftApp : public CWinApp
{
public:
	CShiftApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShiftApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CShiftApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHIFT_H__F8D8D69C_764C_4B5D_B957_0AAFE01E4F0C__INCLUDED_)
