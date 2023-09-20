#include "StdAfx.h"
#include "UiWnd.h"
#include "CompStrEx.h"
#include "../helper/helper.h"
#include "../include/global.h"

#define HEI_LINEMARGIN	3
#define TIMERID_CHKDEFIME	200
#define UM_ASYNCFOCUS	(WM_USER+10000)

#include "Minidump.h"
static bool g_bInstallDump = false;

LRESULT CALLBACK UIWndProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	CUiWnd *pUiWnd=(CUiWnd *)GetWindowLongPtr(hWnd,GWLP_USERDATA);
	switch(uMsg)
	{
	case WM_NCCREATE:
		_ASSERT(pUiWnd==NULL);
		pUiWnd=new CUiWnd;
		pUiWnd->Attach(hWnd);
		if(!g_bInstallDump)
		{
			g_bInstallDump = true;

			TCHAR szModule[MAX_PATH];
			GetModuleFileName(NULL,szModule,MAX_PATH);
			LPCTSTR pName=_tcsrchr(szModule,'\\');
			if(pName)
				pName++;
			else
				pName=szModule;

			TCHAR szDumpPath[MAX_PATH];
			_stprintf_s(szDumpPath,MAX_PATH,_T("%s\\log\\%s.dmp"),theModule->GetInstallPath(),pName);
			CMinidump::SetDumpFile(szDumpPath);
			CMinidump::Enable();
		}
		break;
	case WM_NCDESTROY:
		pUiWnd->WindowProc(uMsg,wParam,lParam);
		pUiWnd->Detatch();
		delete pUiWnd;
		return 0;
	}
	if(pUiWnd)	return pUiWnd->WindowProc(uMsg,wParam,lParam);
	else return DefWindowProc(hWnd,uMsg,wParam,lParam);
}


BOOL CUiWnd::RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wc;

	// register class of UI window.
	wc.cbSize         = sizeof(WNDCLASSEX);
	wc.style          = CS_IME;
	wc.lpfnWndProc    = UIWndProc;
	wc.cbClsExtra     = 0;
	wc.cbWndExtra     = 2 * sizeof(LONG_PTR);
	wc.hInstance      = (HINSTANCE)hInstance;
	wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
	wc.hIcon          = NULL;
	wc.lpszMenuName   = (LPTSTR)NULL;
	wc.lpszClassName  = CLS_SINSTAR3_IME_WND;
	wc.hbrBackground  = NULL;
	wc.hIconSm        = NULL;

	return RegisterClassEx( (LPWNDCLASSEX)&wc );
}

void CUiWnd::UnregisterClass(HINSTANCE hInstance)
{
	::UnregisterClass(CLS_SINSTAR3_IME_WND,hInstance);
}

CUiWnd::CUiWnd(void)
	: m_bCompositing(FALSE)
	, m_pCurContext(NULL)
	, m_pSinstar3(NULL)
	, m_nFontHei(20)
	, m_fntComp(0)
	, m_bActivate(TRUE)
	, m_bHasFocus(FALSE)
{
}

CUiWnd::~CUiWnd(void)
{
	if(m_fntComp) DeleteObject(m_fntComp);
}

BOOL CUiWnd::IsIMEMessage(UINT message)
{
	switch(message)
	{
	case WM_IME_STARTCOMPOSITION:
	case WM_IME_ENDCOMPOSITION:
	case WM_IME_COMPOSITION:
	case WM_IME_NOTIFY:
	case WM_IME_SETCONTEXT:
	case WM_IME_CONTROL:
	case WM_IME_COMPOSITIONFULL:
	case WM_IME_SELECT:
	case WM_IME_CHAR:
		return TRUE;
	default:
		return FALSE;
	}
}

LRESULT CUiWnd::OnPaint()
{
	PAINTSTRUCT ps;
	MSG sMsg;
	// for safety
	BeginPaint(m_hWnd, &ps);
	EndPaint(m_hWnd, &ps);

	// some application will not remove the WM_PAINT messages
	PeekMessage(&sMsg, m_hWnd, WM_PAINT, WM_PAINT, PM_REMOVE|PM_NOYIELD);
	return (0L);
}

LRESULT CUiWnd::WindowProc(UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HIMC hIMC=(HIMC)GetWindowLongPtr(m_hWnd,IMMGWLP_IMC);
	if(!hIMC && IsIMEMessage(uMsg)) return 0;
	switch(uMsg)
	{
	case UM_ASYNCFOCUS:
		return OnAsyncFocus();
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	case WM_CREATE:
		return OnCreate();
	case WM_DESTROY:
		return OnDestroy();
	case WM_NCDESTROY:
		AttachToIMC(FALSE);
		return 0;
	case WM_PAINT:
		return OnPaint();
	case WM_TIMER:
		return OnTimer(wParam);
	case WM_IME_NOTIFY:
		return OnImeNotify(wParam,lParam);
	case WM_IME_CONTROL:
		return OnImeControl(wParam,lParam);
	case WM_IME_SELECT:
		return OnImeSelect((BOOL)wParam,lParam);
	case WM_IME_SETCONTEXT:
		return OnSetContext((BOOL)wParam,lParam);
	case WM_IME_STARTCOMPOSITION:
		return 0;
	case WM_IME_COMPOSITION:
		return 0;
	case WM_IME_ENDCOMPOSITION:
		return 0;
	default:
		if(m_pSinstar3){
			LRESULT result;
			BOOL bHandled = m_pSinstar3->ProcessWindowMessage(m_hWnd, uMsg, wParam, lParam, result);
			if (bHandled) return result;
		}
		break;
	}

	return __super::WindowProc(uMsg,wParam,lParam);
}

LRESULT CUiWnd::OnTimer(WPARAM nEventID)
{
	if(nEventID == TID_INIT)
	{
		if(_InitSinstar3())
		{
			KillTimer(m_hWnd,nEventID);
		}
	}
	return 0;
}

LRESULT CUiWnd::OnImeSelect(BOOL bSelect,LPARAM lParam)
{
	SLOGFMTI("OnImeSelect,bSelect:%d,lParam:%d",bSelect,(int)lParam);
	AttachToIMC(bSelect);
	OnImeNotify(IMN_SETOPENSTATUS,0);
	OnImeNotify(IMN_SETCONVERSIONMODE,0);
	if(m_pSinstar3) m_pSinstar3->OnIMESelect(bSelect);
	return 0;
}

POINT CUiWnd::GetAbsPos(HWND hWnd,DWORD dwStyle,POINT ptCur,RECT rc)
{
	POINT pt;
	if(dwStyle==CFS_DEFAULT)
		GetCaretPos(&pt);
	else if(dwStyle==CFS_EXCLUDE)
		pt.x=ptCur.x,
		pt.y=rc.bottom-m_nFontHei+HEI_LINEMARGIN;
	else
		pt=ptCur;
	ClientToScreen(hWnd,&pt);
	return pt;
}

LRESULT CUiWnd::OnImeControl(WPARAM wParam,LPARAM lParam)
{
	SLOGFMTI("OnImeControl, wParam:%d,lParam:%d",(int)wParam,(int)lParam);
	LPINPUTCONTEXT lpIMC;
	HIMC hIMC=(HIMC)GetWindowLongPtr(m_hWnd,IMMGWLP_IMC);
	_ASSERT(hIMC);
	if (!(lpIMC = ImmLockIMC(hIMC)))
		return 1L;
	LRESULT lRet=1;
	switch (wParam)
	{
	case IMC_GETCANDIDATEPOS:
		*(LPCANDIDATEFORM)lParam  = lpIMC->cfCandForm[0]; 
		lRet = 0;
		break;

	case IMC_GETCOMPOSITIONWINDOW:
		*(LPCOMPOSITIONFORM)lParam  = lpIMC->cfCompForm; 
		lRet = 0;
		break;

	default:
		break;
	}
	ImmUnlockIMC(hIMC);
	return lRet;
}

LRESULT CUiWnd::OnImeNotify(WPARAM wParam,LPARAM lParam)
{
	HIMC hIMC=(HIMC)GetWindowLongPtr(m_hWnd,IMMGWLP_IMC);
	switch(wParam)
	{
	case IMN_OPENSTATUSWINDOW:
		SetFocus(TRUE);
		SLOGFMTI("IMN_OPENSTATUSWINDOW");
		break;
	case IMN_CLOSESTATUSWINDOW:
		SetFocus(FALSE);
		SLOGFMTI("IMN_CLOSESTATUSWINDOW");
		break;
	case IMN_OPENCANDIDATE:
		OnImeNotify(IMN_SETCOMPOSITIONFONT,0);
		break;
	case IMN_CHANGECANDIDATE:
		break;
	case IMN_CLOSECANDIDATE:
		break;
	case IMN_SETOPENSTATUS:
		if(m_pSinstar3)
		{
			BOOL bOpen = ImmGetOpenStatus(hIMC);
			if(!bOpen)
			{
				CImeContext *ctx = GetImeContext();
				if(ctx)
				{
					EndComposition(ctx);
				}
				ReleaseImeContext(ctx);
			}
			m_pSinstar3->OnOpenStatusChanged(bOpen);
		}
		break;
	case IMN_SETCONVERSIONMODE:
		break;
	case IMN_SETCOMPOSITIONWINDOW:
		if(m_pSinstar3 && hIMC && m_bActivate)
		{
			LPINPUTCONTEXT lpIMC=(LPINPUTCONTEXT)ImmLockIMC(hIMC);
			if(lpIMC)
			{
				CCompStrEx *pCompStr=(CCompStrEx*)ImmLockIMCC(lpIMC->hCompStr);
				if(pCompStr)
				{
					POINT pt=GetAbsPos(lpIMC->hWnd,lpIMC->cfCompForm.dwStyle,lpIMC->cfCompForm.ptCurrentPos,lpIMC->cfCompForm.rcArea);
					{		
						IMECHARPOSITION charPos={sizeof(charPos),pCompStr->GetTargetPos(),0};
						COMPOSITIONFORM compForm={0};
						if(charPos.dwCharPos==-1) charPos.dwCharPos=0;
						if(ImmRequestMessage(hIMC,IMR_COMPOSITIONWINDOW,(LPARAM)&compForm))
						{
							pt=GetAbsPos(lpIMC->hWnd,compForm.dwStyle,compForm.ptCurrentPos,compForm.rcArea);
						}else if(ImmRequestMessage(hIMC,IMR_QUERYCHARPOSITION,(LPARAM)&charPos))
						{
							pt=charPos.pt;
						} 
					}
					pt.y-=HEI_LINEMARGIN;
					m_pSinstar3->OnSetCaretPosition(pt,m_nFontHei);
					SLOGFMTI("IMN_SETCOMPOSITIONWINDOW,pt=(%d,%d)",pt.x,pt.y);
					ImmUnlockIMCC(lpIMC->hCompStr);
				}
				ImmUnlockIMC(hIMC);
			}
		}
		break;
	case IMN_SETCANDIDATEPOS:
		if(m_pSinstar3 && hIMC)
		{
			LPINPUTCONTEXT lpIMC=(LPINPUTCONTEXT)ImmLockIMC(hIMC);
			if(lpIMC)
			{
				CCompStrEx *pCompStr=(CCompStrEx*)ImmLockIMCC(lpIMC->hCompStr);
				if(pCompStr)
				{
					if(pCompStr->GetTargetPos()!=-1)
					{
						POINT pt;
						{

							IMECHARPOSITION charPos={sizeof(charPos),0};
							charPos.dwCharPos=pCompStr->GetTargetPos();
							if(charPos.dwCharPos!=DWORD(-1) && ImmRequestMessage(hIMC,IMR_QUERYCHARPOSITION,(LPARAM)&charPos))
							{
								pt=charPos.pt;
							}else
							{
								pt=GetAbsPos(lpIMC->hWnd,lpIMC->cfCandForm[0].dwStyle,lpIMC->cfCandForm[0].ptCurrentPos,lpIMC->cfCandForm[0].rcArea);
							}
						}
						pt.y-=HEI_LINEMARGIN;
						SLOGFMTI("IMN_SETCANDIDATEPOS,pt=(%d,%d)",pt.x,pt.y);
						m_pSinstar3->OnSetFocusSegmentPosition(pt,m_nFontHei);
					}
					ImmUnlockIMCC(lpIMC->hCompStr);
				}
				ImmUnlockIMC(hIMC);
			}
		}
		break;
	case IMN_SETCOMPOSITIONFONT:
		if(hIMC){
			LPINPUTCONTEXT lpIMC=(LPINPUTCONTEXT)ImmLockIMC(hIMC);
			if(lpIMC)
			{
				LOGFONT lf;
				
				if(!ImmRequestMessage(hIMC,IMR_COMPOSITIONFONT,(LPARAM)&lf))
				{
					memcpy(&lf,&lpIMC->lfFont.W,sizeof(LOGFONT));
				}
				m_fntComp=CreateFontIndirectW(&lf);
				
				IMECHARPOSITION charPos={sizeof(charPos),0,0};
				if(ImmRequestMessage(hIMC,IMR_QUERYCHARPOSITION,(LPARAM)&charPos) 
					&& charPos.cLineHeight<100 //��word2007�п��ܻ��һ���ǳ����ֵ
					)
				{
					m_nFontHei= charPos.cLineHeight+HEI_LINEMARGIN*2;
				}
				else
				{
					SIZE sz;
					HDC hdc=GetDC(lpIMC->hWnd);
					HGDIOBJ hOldFont=SelectObject(hdc,m_fntComp);
					GetTextExtentPoint(hdc,_T("A"),1,&sz);
					SelectObject(hdc,hOldFont);
					ReleaseDC(lpIMC->hWnd,hdc);
					m_nFontHei=sz.cy+HEI_LINEMARGIN*2;
				}

				OnImeNotify(IMN_SETCOMPOSITIONWINDOW,0);
			}
			ImmUnlockIMC(hIMC);
		}
		break;
	}
	return 0;
}

LRESULT CUiWnd::OnSetContext(BOOL bActivate,LPARAM lParam)
{
	SLOGFMTI("OnSetContext,bActivate:%d",bActivate);
	m_bActivate=bActivate;
	if(bActivate)
	{
		AttachToIMC(TRUE);
		OnImeNotify(IMN_OPENSTATUSWINDOW,0);
	}else
	{
		OnImeNotify(IMN_CLOSESTATUSWINDOW,0);
		AttachToIMC(FALSE);
	}
	return 0;
}

BOOL CUiWnd::_InitSinstar3()
{
	SASSERT(!m_pSinstar3);
	if(GetFileAttributes(theModule->GetSvrPath()) == INVALID_FILE_ATTRIBUTES)
		return FALSE;
	m_pSinstar3 = new CSinstarProxy(this);
	int nInit = m_pSinstar3->Init(m_hWnd, theModule->GetSvrPath());
	SLOGW()<<"m_pSinstar3->Init ret="<<nInit<<" hWnd="<<m_hWnd<<" svrPath="<<theModule->GetSvrPath();
	if (nInit!=0)
	{
		delete m_pSinstar3;
		m_pSinstar3 = NULL;
		return FALSE;
	}
	m_pSinstar3->NotifyScaleInfo(GetActiveWindow());
	m_pSinstar3->OnIMESelect(m_bActivate);
	PostMessage(WM_IME_NOTIFY,IMN_SETCONVERSIONMODE,0);
	return TRUE;
}

BOOL CUiWnd::_UninitSinstar3()
{
	if(m_pSinstar3)
	{
		m_pSinstar3->OnIMESelect(FALSE);
		delete m_pSinstar3;
		m_pSinstar3=NULL;
	}
	return TRUE;
}

LRESULT CUiWnd::OnCreate()
{
	if(theModule->GetSysInfoFlags() & IME_SYSINFO_WINLOGON) return -1;

#ifdef ENABLE_LOG
	TCHAR szPathInstall[MAX_PATH] = { 0 };
	CRegKey reg;
	LONG ret = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SetoutSoft\\sinstar3"), KEY_READ | KEY_WOW64_64KEY);
	if (ret == ERROR_SUCCESS)
	{
		ULONG len = MAX_PATH;
		reg.QueryStringValue(_T("path_client"), szPathInstall, &len);
		reg.Close();
	}
	new SOUI::SLogMgr(szPathInstall);
#endif
	SLOGFMTI("CUiWnd::OnCreate,hWnd:%p",m_hWnd);
	Helper_ChangeWindowMessageFilter(SOUI::UM_CALL_FUN, MSGFLT_ADD);
	Helper_ChangeWindowMessageFilter(UM_GETPROCPATH, MSGFLT_ADD);
	Helper_ChangeWindowMessageFilter(UM_RECONN, MSGFLT_ADD);

	if(!_InitSinstar3())
	{
		SetTimer(m_hWnd,TID_INIT,200,NULL);
	}
	return 0;
}

LRESULT CUiWnd::OnDestroy()
{
	SLOGFMTI("CUiWnd::OnDestroy");
	_UninitSinstar3();
	AttachToIMC(FALSE);
#ifdef ENABLE_LOG
	delete SOUI::SLogMgr::getSingletonPtr();
#endif
	return 0;
}

BOOL CUiWnd::AttachToIMC(BOOL bAttach)
{
	SLOGI()<<"bAttach="<<bAttach;
	BOOL bRet=FALSE;
	HIMC hIMC=(HIMC)GetWindowLongPtr(m_hWnd,IMMGWLP_IMC);
	LPINPUTCONTEXT lpIMC=(LPINPUTCONTEXT)ImmLockIMC(hIMC);
	if(lpIMC)
	{
		//��UIWnd������Private������ڴ棬������IMM�Ľӿ��з��ʵ�ǰ��UIWND��
		CUiWnd **pData=(CUiWnd**)ImmLockIMCC(lpIMC->hPrivate);
		if(pData)
		{
			if(bAttach)
				*pData=this;
			else
			{
				m_pCurContext = NULL;
				*pData=0;
			}
			ImmUnlockIMCC(lpIMC->hPrivate);
			bRet=TRUE;
		}
		ImmUnlockIMC(hIMC);
	}
	return bRet;
}

BOOL CUiWnd::IsDefaultIme(void)
{
	HKL hKlCur=GetKeyboardLayout(0);
	HKL hDefIME=0;
	SystemParametersInfo(SPI_GETDEFAULTINPUTLANG, 0, &hDefIME, 0);
	return hKlCur==hDefIME;
}

void CUiWnd::StartComposition(UINT64 imeContext)
{
	StartComposition((CImeContext*)imeContext);
}

void CUiWnd::ReplaceSelCompositionW(UINT64 imeContext,int nLeft,int nRight,const WCHAR *wszComp,int nLen)
{
	ReplaceSelCompositionW((CImeContext*)imeContext,nLeft,nRight,wszComp,nLen);
}

void CUiWnd::UpdateResultAndCompositionStringW(UINT64 imeContext,const WCHAR *wszResultStr,int nResStrLen,const WCHAR *wszCompStr,int nCompStrLen)
{
	UpdateResultAndCompositionStringW((CImeContext*)imeContext,wszResultStr,nResStrLen,wszCompStr,nCompStrLen);
}

void CUiWnd::EndComposition(UINT64 imeContext)
{
	EndComposition((CImeContext*)imeContext);
}

void CUiWnd::SetOpenStatus(BOOL bOpen)
{
	CImeContext *pCtx = GetImeContext();
	if(pCtx)
	{
		SetOpenStatus(pCtx,bOpen);
		ReleaseImeContext(pCtx);
	}
}

BOOL CUiWnd::GetOpenStatus() const
{
	BOOL bOpen = FALSE;
	CImeContext *pCtx = GetImeContext();
	if(pCtx)
	{
		bOpen=GetOpenStatus(pCtx);
		ReleaseImeContext(pCtx);
	}
	return bOpen;
}

void CUiWnd::_SyncFocus()
{
	PostMessage(UM_ASYNCFOCUS,0,0);
}

LRESULT CUiWnd::OnAsyncFocus()
{
	if(m_pSinstar3)
	{
		DWORD dwActiveWnd = m_bHasFocus?GetActiveWnd():0;
		m_pSinstar3->OnSetFocus(m_bHasFocus,dwActiveWnd);
	}
	return 0;
}

void CUiWnd::OnReconnReady()
{
	if(m_pSinstar3)
	{
		_UninitSinstar3();
		_InitSinstar3();
		SetOpenStatus(TRUE);
		OnImeNotify(IMN_SETOPENSTATUS,0);
		_SyncFocus();
	}
}

void CUiWnd::SetFocus(BOOL bFocus)
{
	m_bHasFocus = bFocus;
	_SyncFocus();
}

