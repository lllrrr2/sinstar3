#include "stdafx.h"
#include "editsession.h"

//////////////////////////////////////////////////////////////////////////
CEditSessionBase::CEditSessionBase(CSinstar3Tsf *pTextService, ITfContext *pContext)
:_pTextService(pTextService)
,_pContext(pContext)
{
}


//////////////////////////////////////////////////////////////////////////

CEsKeyHandler::CEsKeyHandler(CSinstar3Tsf *pTextService, ITfContext *pContext,WPARAM wParam, LPARAM lParam) :CEditSessionBase(pTextService,pContext)
,_wParam(wParam)
,_lParam(lParam)
{
	GetKeyboardState(_byKeyState);
}

STDMETHODIMP CEsKeyHandler::DoEditSession(TfEditCookie ec)
{
	BOOL fEaten = FALSE;
	GetTextService()->_bInKeyProc = TRUE;

	GetSinstar3()->TranslateKey((UINT64)GetContext(),(UINT)_wParam, MapVirtualKey((UINT)_wParam,0), TRUE, _byKeyState, &fEaten);
	GetTextService()->_bInKeyProc = FALSE;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CEsStartComposition::CEsStartComposition(CSinstar3Tsf *pTextService, ITfContext *pContext) 
: CEditSessionBase(pTextService, pContext)
{

}

STDMETHODIMP CEsStartComposition::DoEditSession(TfEditCookie ec)
{
	SLOGI()<<"TfEditCookie:"<<ec;
	SOUI::SComPtr<ITfInsertAtSelection> pInsertAtSelection;
	SOUI::SComPtr<ITfRange> pRangeInsert;
	SOUI::SComPtr<ITfContextComposition> pContextComposition;
	SOUI::SComPtr<ITfComposition> pComposition;
	HRESULT hr = E_FAIL;

	// A special interface is required to insert text at the selection
	hr = _pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection);
	SASSERT_HR(hr);

	// insert the text
	hr = pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert);
	SASSERT_HR(hr);

	// get an interface on the context to deal with compositions
	hr = _pContext->QueryInterface(IID_ITfContextComposition, (void **)&pContextComposition);
	SASSERT_HR(hr);

	// start the new composition
	hr = pContextComposition->StartComposition(ec, pRangeInsert, _pTextService, &pComposition);
	SASSERT_HR(hr);
	SASSERT_RET(pComposition, return E_FAIL);

	// insert a dummy blank
	pRangeInsert->SetText(ec, TF_ST_CORRECTION, L" ", 1);
	_pTextService->_SetCompositionDisplayAttributes(ec,_pContext,pRangeInsert);

	pRangeInsert->Collapse(ec, TF_ANCHOR_START);

	// 
	//  set selection to the adjusted range
	// 
	TF_SELECTION tfSelection;
	tfSelection.range = pRangeInsert;
	tfSelection.style.ase = TF_AE_NONE;
	tfSelection.style.fInterimChar = FALSE;
	_pContext->SetSelection(ec, 1, &tfSelection);

	// Store the pointer of this new composition object in the instance 
	// of the CTextService class. So this instance of the CTextService 
	// class can know now it is in the composition stage.
	//_pTextService->OnStartComposition(ec,pComposition);
	_pTextService->OnStartComposition(ec, pComposition, _pContext);
	//trigger layout changed
	SOUI::SComPtr<ITfContextView> pCtxView;
	hr = _pContext->GetActiveView(&pCtxView);
	if(SUCCEEDED(hr))
	{
		_pTextService->OnLayoutChange(_pContext,TF_LC_CHANGE,pCtxView);
	}
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CEsEndComposition::CEsEndComposition(CSinstar3Tsf *pTextService, ITfContext *pContext,bool bClearCtx) 
: CEditSessionBase(pTextService, pContext)
, _bClearCtx(bClearCtx)
{

}

STDMETHODIMP CEsEndComposition::DoEditSession(TfEditCookie ec)
{
	SLOGI()<<"TfEditCookie:"<<ec;
	SOUI::SComPtr<ITfComposition>  pComposition = _pTextService->GetITfComposition();
	if(!pComposition)
	{
		SLOGW()<<"CEditSessionEndComposition::DoEditSession not in compositing";
		return E_FAIL;
	}

	SOUI::SComPtr<ITfRange> pRange;
	if ( pComposition->GetRange( &pRange) == S_OK && pRange != NULL)
	{
		//clear dummy blank
		pRange->SetText(ec, 0, L"", 0);
		TF_SELECTION sel={0};
		sel.style.ase = TF_AE_NONE;
		sel.style.fInterimChar = FALSE;
		sel.range=pRange;
		pRange->Collapse(ec,TF_ANCHOR_END);
		_pContext->SetSelection(ec,1,&sel);
	}

	if(!_bClearCtx)
	{
		//trigger layout changed
		SOUI::SComPtr<ITfContextView> pCtxView;
		HRESULT hr = _pContext->GetActiveView(&pCtxView);
		if (SUCCEEDED(hr))
		{
			_pTextService->OnLayoutChange(_pContext, TF_LC_CHANGE, pCtxView);
		}
	}
	pComposition->EndComposition(ec);
	_pTextService->_TerminateComposition(ec,_pContext,_bClearCtx);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CEsGetTextExtent::CEsGetTextExtent(CSinstar3Tsf *pTextService, ITfContext *pContext, ITfContextView *pContextView) 
: CEditSessionBase(pTextService, pContext)
, _pContextView(pContextView)
{
}



STDMETHODIMP CEsGetTextExtent::DoEditSession(TfEditCookie ec)
{
	SLOGI()<<"TfEditCookie:"<<ec;

	SOUI::SComPtr<ITfRange> pRange;
	ISinstar * pSinstar3 = GetSinstar3();
	SOUI::SComPtr<ITfComposition> pComposition = _pTextService->GetITfComposition();
	if(!pSinstar3 || !pComposition) return S_FALSE;

	SOUI::SComPtr<ITfRange> range;
	if ( pComposition->GetRange( &range) == S_OK && range != NULL)
	{
		BOOL fClip = FALSE;
		RECT rc={0};
		_pContextView->GetTextExt(ec, range, &rc, &fClip);
		POINT pt = { rc.left,rc.top };
		int nHei = rc.bottom - rc.top;
		pSinstar3->OnSetCaretPosition( pt, nHei);
		SLOGFMTI("SetCaret pos:%d,%d, height: %d",pt.x,pt.y, nHei);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CEsChangeComposition::CEsChangeComposition(CSinstar3Tsf *pTextService, ITfContext *pContext,int nLeft,int nRight,LPCWSTR pszBuf,int nLen) : CEditSessionBase(pTextService, pContext)
{
	if(nLen) 
	{
		m_pszBuf=new WCHAR[nLen];
		wcsncpy(m_pszBuf,pszBuf,nLen);
	}else
	{
		m_pszBuf=NULL;
	}
	m_nLeft=nLeft,m_nRight=nRight;
	m_nLen=nLen;
}

CEsChangeComposition::~CEsChangeComposition()
{
	if(m_pszBuf) delete []m_pszBuf;
}


STDMETHODIMP CEsChangeComposition::DoEditSession(TfEditCookie ec)
{
	SLOGI()<<"TfEditCookie:"<<ec;

	SOUI::SComPtr<ITfRange> pRangeComposition;
	SOUI::SComPtr<ITfRange> pRangeSel;
	SOUI::SComPtr<ITfComposition> pComposition = _pTextService->GetITfComposition();

	SASSERT_RET(pComposition, return E_FAIL);

	TF_SELECTION tfSelection;
	ULONG cFetched;
	BOOL fCovered;
	// first, test where a keystroke would go in the document if an insert is done
	if (_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
		return S_FALSE;

	pRangeSel.Attach(tfSelection.range);

	// is the insertion point covered by a composition?
	if (pComposition->GetRange(&pRangeComposition) == S_OK)
	{
		fCovered = IsRangeCovered(ec, pRangeSel, pRangeComposition);
		if(fCovered)
		{
			if(m_nLeft==0 && m_nRight==-1)
			{//ȫ���滻
				pRangeSel->ShiftStartToRange(ec,pRangeComposition,TF_ANCHOR_START);
				pRangeSel->ShiftEndToRange(ec,pRangeComposition,TF_ANCHOR_END);
			}else	if(m_nLeft!=-1 && m_nRight!=-1)
			{//�滻ָ����Χ
				LONG cch=0;
				pRangeSel->ShiftStartToRange(ec,pRangeComposition,TF_ANCHOR_START);
				pRangeSel->ShiftStart(ec,m_nLeft,&cch,NULL);
				pRangeSel->Collapse(ec,TF_ANCHOR_START);
				pRangeSel->ShiftEnd(ec,m_nRight-m_nLeft,&cch,NULL);
			}
		}

		if (!fCovered)
		{
			return S_OK;
		}
	}

	// insert the text
	// Use SetText here instead of InsertTextAtSelection because a composition is already started
	// Don't allow the app to adjust the insertion point inside our composition
	if (pRangeSel->SetText(ec, 0, m_pszBuf, m_nLen) != S_OK)
		return S_OK;

	// update the selection, and make it an insertion point just past
	// the inserted text.
	pRangeSel->Collapse(ec, TF_ANCHOR_END);
	_pContext->SetSelection(ec, 1, &tfSelection);

	if(_pTextService->m_pSinstar3) _pTextService->m_pSinstar3->OnCompositionChanged();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CEsUpdateResultAndComp::CEsUpdateResultAndComp(CSinstar3Tsf *pTextService, 
																 ITfContext *pContext,
																 LPCWSTR pszResultStr,
																 int nResStrLen,
																 LPCWSTR pszCompStr,
																 int nCompStrLen) 
																 : CEditSessionBase(pTextService, pContext)
{
	if(pszResultStr)
	{
		if(nResStrLen==-1) nResStrLen= (int)wcslen(pszResultStr);
		m_nResStrLen=nResStrLen;
		m_pszResultStr=new WCHAR[m_nResStrLen];
		wcsncpy(m_pszResultStr,pszResultStr,m_nResStrLen);
	}else
	{
		m_pszResultStr=NULL;
		m_nResStrLen=0;
	}
	if(pszCompStr)
	{
		if(nCompStrLen==-1) nCompStrLen=(int)wcslen(pszCompStr);
		m_nCompStrLen=nCompStrLen;
		m_pszCompStr=new WCHAR[m_nCompStrLen];
		wcsncpy(m_pszCompStr,pszCompStr,m_nCompStrLen);
	}else
	{
		m_pszCompStr=NULL;
		m_nCompStrLen=0;
	}
	m_pComposition = pTextService->GetITfComposition();
}

CEsUpdateResultAndComp::~CEsUpdateResultAndComp()
{
	if(m_pszCompStr) delete []m_pszCompStr;
	if(m_pszResultStr) delete []m_pszResultStr;
}

STDMETHODIMP CEsUpdateResultAndComp::DoEditSession(TfEditCookie ec)
{
	SLOGI()<<"TfEditCookie:"<<ec;

	SOUI::SComPtr<ITfRange> pRangeComposition;
	SASSERT_RET(m_pComposition, return E_FAIL);
	//����ǰ��������
	m_pComposition->GetRange(&pRangeComposition);
	if(!pRangeComposition)
	{
		SLOGW()<<"CEsUpdateResultAndComp::DoEditSession getRange return null";
		return E_FAIL;
	}
	if(m_pszResultStr && m_nResStrLen)
	{
		pRangeComposition->SetText(ec,0,m_pszResultStr,m_nResStrLen);
		BOOL fEmpty=TRUE;
		if(pRangeComposition->IsEmpty(ec,&fEmpty)==S_OK && !fEmpty)
		{
			pRangeComposition->Collapse(ec,TF_ANCHOR_END);
			m_pComposition->ShiftStart(ec,pRangeComposition);
		}
	}

	TF_SELECTION tfSelection;
	tfSelection.style.ase = TF_AE_NONE;
	tfSelection.style.fInterimChar = FALSE;
	tfSelection.range=pRangeComposition;
	//�����µı���
	if(m_pszCompStr && m_nCompStrLen)
	{
		POINT pt={-1,-1};
		if(_pTextService->m_pSinstar3) _pTextService->m_pSinstar3->OnSetFocusSegmentPosition(pt,0);
		pRangeComposition->SetText(ec,0,m_pszCompStr,m_nCompStrLen);

		pRangeComposition->Clone(&tfSelection.range);
		tfSelection.range->Collapse(ec,TF_ANCHOR_END);
		_pContext->SetSelection(ec,1,&tfSelection);
		tfSelection.range->Release();

		TfGuidAtom  gaDisplayAttribute = _pTextService->GetDisplayAttribInfo();	
		if (TF_INVALID_GUIDATOM != gaDisplayAttribute)
		{
			SOUI::SComPtr<ITfProperty> pDisplayAttributeProperty;
			if (SUCCEEDED(_pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty)))
			{
				VARIANT var;
				VariantInit(&var);
				//All display attributes are TfGuidAtoms and TfGuidAtoms are VT_I4. 
				var.vt = VT_I4;
				var.lVal = gaDisplayAttribute;
				//Set the display attribute value over the range. 
				pDisplayAttributeProperty->SetValue(ec, pRangeComposition, &var);
			}
		}
		if(_pTextService->m_pSinstar3) _pTextService->m_pSinstar3->OnCompositionChanged();
	}else
	{
		_pContext->SetSelection(ec,1,&tfSelection);
	}
	return S_OK;
}
