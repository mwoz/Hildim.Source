// MWCore.cpp: implementation of the CMWMap class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "MWCore.h"



WNDPROC wP;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMWMap map;

CMWMap::CMWMap()
{
	pUp=NULL;
	m_pNewCl=NULL;
}

CMWMap::~CMWMap()
{
	MAP* pN;
	while (pUp)
	{
		pN=pUp;
		pUp=pN->pNext;
		delete pN;
	}
}
bool CMWMap::AddAdr(HWND hWnd, CMWWnd *pWW)
{	
	if (GetPClass(hWnd)) return FALSE;//-уже есть
	MAP* pNew=new MAP;
	pNew->hW=hWnd;
	pNew->pWW=pWW;
	pNew->pNext=pUp;
	pUp=pNew;
	return TRUE;
}

bool CMWMap::DelAdr(HWND hWnd)
{
/**/MAP* pCur=NULL;
	MAP* pN=pUp;
	while (pN)
	{
		if (pN->hW==hWnd)//найдена запись
		{
			if (!pCur)//-верхняя запись
			{
				pUp=pN->pNext;
			}else
			{
				pCur->pNext=pN->pNext;
			}
			delete pN;
			return TRUE;
		}
		pCur=pN;
		pN=pN->pNext;
	}
	//не найдена запись
	return TRUE;//FALSE;
}

CMWWnd* CMWMap::GetPClass(HWND hWnd)
{
	MAP* pN=pUp;
	while (pN)
	{
		if (pN->hW==hWnd) return pN->pWW;
		pN=pN->pNext;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////
// CMWWnd Class
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMWWnd::CMWWnd()
{
	m_hwnd=NULL;
	m_oldWP=NULL;
}

CMWWnd::~CMWWnd()
{
	if (m_hwnd)map.DelAdr(m_hwnd);
}
LPARAM CMWWnd::ConvertLparam(LPARAM lParam)
{
	POINTS pts=MAKEPOINTS(lParam);
	POINT p;
	p.x=pts.x;
	p.y=pts.y;
	ClientToScreen(m_hwnd,&p);
	return MAKELPARAM(p.x,p.y);
}


void CMWWnd::SetNewClass(CMWWnd *pc)
{
	map.m_pNewCl=pc;
}

LRESULT CMWWnd::WWndProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{	
	LRESULT lRes=::DefWindowProc(m_hwnd,nMsg,  wParam,  lParam);
	return lRes;
}
void CMWWnd::Attach(HWND hWnd)
{
	m_hwnd=hWnd;		
	m_oldWP=(WNDPROC)::SetWindowLong(hWnd,GWL_WNDPROC,(LONG)MwxWndProc);
	map.AddAdr(hWnd,this);
}
void CMWWnd::DelWnd()
{
	map.DelAdr(m_hwnd);
}
void CMWWnd::DeAttach()
{
	SetWindowLong(m_hwnd,GWL_WNDPROC,(LONG)m_oldWP);
	map.DelAdr(m_hwnd);
}

BOOL CMWWnd::Show(int nCmdShow)
{	
#ifdef _DEBUG
	if(nCmdShow==SW_HIDE)
	{
		int i=0;
	}
#endif
	return ::ShowWindow(m_hwnd,nCmdShow);
}
LRESULT CMWWnd::CallOldWP(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return ::CallWindowProc(m_oldWP,m_hwnd,Msg,wParam,lParam);
}


CMWWnd* CMWWnd::GetCMW(HWND hWnd)
{
	return map.GetPClass(hWnd);
}
void CMWWnd::CorrectRect(LONG* left,LONG* top,LONG* right,LONG* bottom)
{
	RECT dw;
	GetWindowRect(GetDesktopWindow(),&dw);
	if((0>*left||dw.right<*left||0>*top||dw.bottom<*top)&&
	   (0>*right||dw.right<*right||0>*bottom||dw.bottom<*bottom))
	{
		*left=20;
		*top=20;
		*right=400;
		*bottom=300;
	}
}
////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK MwxWndProc(HWND hWnd,UINT Msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes;
	CMWWnd* pc;
	pc=map.GetPClass(hWnd);
	if(!pc) 
	{
		pc=map.m_pNewCl;
		map.AddAdr(hWnd,pc);
		pc->m_hwnd=hWnd;
		map.m_pNewCl=NULL;
	}
	lRes=pc->WWndProc(Msg,wParam,lParam);
	return lRes;
}




LRESULT CMWWnd::DefWindProc(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(m_hwnd,nMsg,wParam,lParam);
}

void CMWWnd::ResetTiledPlasement(HWND hPrev,WINDOWPLACEMENT* wp)
{
    WINDOWPLACEMENT wpPrev;
    RECT dRect;
    GetWindowPlacement(hPrev,&wpPrev);
    GetWindowRect(GetDesktopWindow(),&dRect);

    int sdvig=GetSystemMetrics(SM_CYCAPTION);
    int dx=wp->rcNormalPosition.right-wp->rcNormalPosition.left;
    int dy=wp->rcNormalPosition.bottom-wp->rcNormalPosition.top;

    if(dx>dRect.right-sdvig)  dx=dRect.right-sdvig;
    if(dy>dRect.bottom-sdvig) dy=dRect.bottom-sdvig;

    wp->rcNormalPosition.left=0;
    wp->rcNormalPosition.right=dx;
    if(wpPrev.rcNormalPosition.left+sdvig+dx<=dRect.right)
    {
        wp->rcNormalPosition.left+=wpPrev.rcNormalPosition.left+sdvig;
        wp->rcNormalPosition.right+=wpPrev.rcNormalPosition.left+sdvig;
    }

    wp->rcNormalPosition.top=0;
    wp->rcNormalPosition.bottom=dy;
    if(wpPrev.rcNormalPosition.top+sdvig+dy<=dRect.bottom)
    {
        wp->rcNormalPosition.top+=wpPrev.rcNormalPosition.top+sdvig;
        wp->rcNormalPosition.bottom+=wpPrev.rcNormalPosition.top+sdvig;
    }
    wp->showCmd=SW_SHOW;
}
HWND CMWWnd::PingCaretWnd(HWND hWnd)
{
	if(!hWnd)hWnd=GetForegroundWindow();
	GUITHREADINFO gui;
	gui.cbSize=sizeof(GUITHREADINFO);
	if(GetGUIThreadInfo(GetWindowThreadProcessId(hWnd,NULL),&gui))
	{
		SendMessage(gui.hwndCaret,0,0,0);
		return gui.hwndCaret;
	}
	return 0;
}
#ifdef _REALDBG
void REALTRACE(WPARAM wParam,LPARAM lParam)
{
	static HWND hw;
	if(!hw)hw=FindWindow(NULL,"TBDEBBUGER");
	else SendMessage(hw,0x8999,wParam,lParam);
}
#endif




