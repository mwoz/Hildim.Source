// MWCore.h: interface for the CMWMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(MWX_MWCORE__INCLUDED_)
#define MWX_MWCORE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef COREEXP
#ifdef MAINPROG
#define COREEXP __declspec(dllexport)
#else
#define COREEXP 
#endif
#endif

typedef struct FINDDATA{
	DWORD findIn;
	int nWindow;
	HWND hwnd;				//out
	int nEntries;
	LPCSTR lFindString;
	LPSTR lReplaceString;
	DWORD clipAtr;			//out
	int clipN;				//out
	DWORD dAtrib;
}finddata;
#define LPFINDDATA FINDDATA*
#define FND_LIST		1
#define FND_EDITORS	2
#define FNDATR_RTF	1
#define FNDATR_TEMPL	2
#define FNDATR_INS	4
#define FNDATR_CODE	8

class COREEXP CMWWnd  
{
public:
	void DelWnd();
	void DeAttach();
	LRESULT DefWindProc(UINT nMsg,WPARAM wParam,LPARAM lParam);
	HINSTANCE m_hInst;
	WNDPROC		m_oldWP;
	HWND		m_hwnd;

	static CMWWnd*		GetCMW(HWND hWnd);
	static HWND			PingCaretWnd(HWND hWnd=NULL);

	BOOL		Show(int nCmdShow);
	LRESULT				CallOldWP(UINT Msg, WPARAM wParam, 
							LPARAM lParam);
	void				SetNewClass(CMWWnd* pc);
	void				Attach(HWND hWnd);
	virtual LRESULT		WWndProc(UINT nMsg, WPARAM wParam, 
							LPARAM lParam);
	LPARAM				ConvertLparam(LPARAM lParam);
	//void				SetIcon();


	CMWWnd();
	virtual ~CMWWnd();
	void				CorrectRect(LONG* left,LONG* top,LONG* right,LONG* bottom);
	static void			ResetTiledPlasement(HWND hPrev,WINDOWPLACEMENT* wp);

//protected:
};
#ifdef _REALDBG
	void			REALTRACE(WPARAM wParam,LPARAM lParam);
#else
#define REALTRACE//
#endif

class CMWMap  
{
public:
	CMWWnd*		m_pNewCl;
	CMWWnd*		GetPClass(HWND hWnd);
	bool		DelAdr(HWND hWnd);
	bool		AddAdr(HWND hWnd,CMWWnd* pWW);
				CMWMap();
	virtual		~CMWMap();
protected:
	typedef struct MAP{
		HWND	hW;
		CMWWnd* pWW;
		MAP*	pNext;
	}map;

	MAP* pUp;
};
COREEXP LRESULT CALLBACK MwxWndProc(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam);
#endif // !defined(AFX_MWCORE_H__580B7901_1308_11D4_A602_BBC833E3BFCD__INCLUDED_)
