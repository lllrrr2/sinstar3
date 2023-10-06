#include "stdafx.h"
#include "sinstar3_tsf.h"
#include "EditSession.h"
#include "UILess.h"


BOOL CSinstar3Tsf::_IsCompositing() const
{
    return _pComposition != NULL;
}


void CSinstar3Tsf::_StartComposition(ITfContext *pContext)
{
	if(IsCompositing()) return;
	CEsStartComposition *pStartCompositionEditSession = new CEsStartComposition(this, pContext);
	SLOGI()<<"CSinstar3Tsf::_StartComposition,pContext:"<<pContext;

	_bCompositing = TRUE;
	_bChangedDocMgr = true;
	_AdviseTextLayoutSink(pContext);
	HRESULT hr;
	pContext->RequestEditSession(_tfClientId, pStartCompositionEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
	pStartCompositionEditSession->Release();
}

BOOL CSinstar3Tsf::_GetSegRange(TfEditCookie ec,ITfRange **pRange,int nLeft,int nRight)
{
	LONG cch=0;
	assert(_IsCompositing());
	if(S_OK!=_pComposition->GetRange(pRange)) return FALSE;
	(*pRange)->ShiftStart(ec,nLeft,&cch,NULL);
	assert(cch==nLeft);
	(*pRange)->Collapse(ec,TF_ANCHOR_START);
	if(nRight>nLeft) 
	{
		(*pRange)->ShiftEnd(ec,nRight-nLeft,&cch,NULL);
		assert(cch==nRight-nLeft);
	}
	return TRUE;
}


STDAPI CSinstar3Tsf::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition)
{
	if(pComposition == _pComposition)
	{
		SLOGI()<<"CSinstar3Tsf::OnCompositionTerminated,TfEditCookie:"<<ecWrite<< " pComposition:"<<pComposition;
		ITfContext *pCtx = GetImeContext();
		//_TerminateComposition(ecWrite,pCtx,true);
		_EndComposition(pCtx,true);
		ReleaseImeContext(pCtx);
	}else
	{
		SLOGW()<<"CSinstar3Tsf::OnCompositionTerminated,pCompsition:"<<pComposition<<" _pComposition:"<<_pComposition;
	}
	return S_OK;
}


void CSinstar3Tsf::_EndComposition(ITfContext *pContext,bool bClearCtx)
{
	SLOGI()<<"CSinstar3Tsf::_EndComposition IsCompositing()="<<_IsCompositing()<<" inKeyProc:"<<_bInKeyProc;
	if(!_IsCompositing()) return;
    CEsEndComposition *pEditSession = new CEsEndComposition(this, pContext,bClearCtx);
    HRESULT hr;
	pContext->RequestEditSession(_tfClientId, pEditSession, (_bInKeyProc?TF_ES_SYNC:TF_ES_ASYNCDONTCARE) | TF_ES_READWRITE, &hr);
	pEditSession->Release();
	_bCompositing = FALSE;	
	if (_bUILess) {
		_pcand->EndUI();
	}
}

void CSinstar3Tsf::_ChangeComposition(ITfContext *pContext,int nLeft,int nRight,const WCHAR* wszComp,int nLen)
{
	HRESULT hr;

	CEsChangeComposition *pEditSession;
	if (pEditSession = new CEsChangeComposition(this, pContext,nLeft,nRight,wszComp,nLen))
	{
		pContext->RequestEditSession(_tfClientId, pEditSession, (_bInKeyProc?TF_ES_SYNC:TF_ES_ASYNCDONTCARE) | TF_ES_READWRITE, &hr);
		pEditSession->Release();
	}
}

void CSinstar3Tsf::_UpdateResultAndCompositionStringW(ITfContext * pContext,const WCHAR *wszResultStr,int nResStrLen,const WCHAR *wszCompStr,int nCompStrLen)
{
	SLOGI()<<"CSinstar3Tsf::_UpdateResultAndCompositionStringW,wszResultStr:"<<wszResultStr<< " wszCompStr:"<<wszCompStr<<" isComposition:"<<IsCompositing();
	CEsUpdateResultAndComp *pEditSession;
	if (pEditSession = new CEsUpdateResultAndComp(this, pContext,wszResultStr,nResStrLen,wszCompStr,nCompStrLen))
	{
		HRESULT hr;
		pContext->RequestEditSession(_tfClientId, pEditSession, (_bInKeyProc?TF_ES_SYNC:TF_ES_ASYNCDONTCARE) | TF_ES_READWRITE, &hr);
		pEditSession->Release();
	}
}

void CSinstar3Tsf::_TerminateComposition(TfEditCookie ecWrite,ITfContext *pCtx, bool bClearCtx)
{
	SLOGI()<<"CSinstar3Tsf::_TerminateComposition, pComposition:"<<_pComposition<<" bCompositing:"<<_bCompositing;
	_pComposition = NULL;
	_bCompositing = FALSE;
	if ( pCtx != NULL)
	{
		_UnadviseTextLayoutSink(pCtx);
	}
	if(m_pSinstar3) 
	{
		m_pSinstar3->OnCompositionTerminated(bClearCtx);
	}
	if (_bUILess) {
		_pcand->EndUI();
	}
}
