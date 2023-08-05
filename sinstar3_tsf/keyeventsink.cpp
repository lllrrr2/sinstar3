#include "stdafx.h"
#include "sinstar3_tsf.h"
#include "editsession.h"

STDAPI CSinstar3Tsf::OnSetFocus(BOOL fForeground)
{
	SLOGI()<<"ITfKeyEventSink::OnSetFocus, fForeground:"<<fForeground;
    return S_OK;
}

STDAPI CSinstar3Tsf::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if (!m_pSinstar3 || _IsKeyboardDisabled())
	{
		*pfEaten = FALSE;
		return S_OK;
	}

	_bKeyDownTested = TRUE;
	BYTE byKeyState[256];
	GetKeyboardState(byKeyState);

	m_pSinstar3->ProcessKeyStoke((UINT64)pContext, (UINT)wParam, lParam, TRUE, byKeyState, pfEaten);
	if (!(*pfEaten))
	{
		_bKeyDownTested = FALSE;
	}
    return S_OK;
}

STDAPI CSinstar3Tsf::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	if (!m_pSinstar3 || _IsKeyboardDisabled())
	{
		*pfEaten = FALSE;
		return S_OK;
	}
	_bKeyUpTested = TRUE;
	BYTE byKeyState[256];
	GetKeyboardState(byKeyState);

	m_pSinstar3->ProcessKeyStoke((UINT64)pContext, (UINT)wParam, lParam, FALSE, byKeyState, pfEaten);
	if (!(*pfEaten))
	{
		_bKeyUpTested = FALSE;
	}
	return S_OK;
}


STDAPI CSinstar3Tsf::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	HRESULT hr = E_FAIL;
	m_contextSet.insert(pContext);
	SLOGFMTI("OnKeyDown: %08x %08x", (DWORD)wParam, (DWORD)lParam);
	if (!_bKeyDownTested)
	{
		OnTestKeyDown(pContext, wParam, lParam, pfEaten);
		if (!(*pfEaten)) return S_OK;
	}
	CEsKeyHandler *pEs = new CEsKeyHandler(this, pContext, wParam, lParam);
	hr =  pContext->RequestEditSession(_tfClientId, pEs, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
	pEs->Release();
	if (SUCCEEDED(hr))
	{
		*pfEaten = TRUE;
	}
	_bKeyDownTested = FALSE;
	return hr;
}

STDAPI CSinstar3Tsf::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	SLOGFMTI("");
	if (!_bKeyUpTested)
	{
		OnTestKeyUp(pContext, wParam, lParam, pfEaten);
		_bKeyUpTested = FALSE;
	}
	else
	{
		*pfEaten = FALSE;
	}
	m_contextSet.erase(pContext);
    return S_OK;
}

STDAPI CSinstar3Tsf::OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten)
{
	SLOGFMTI("");
	*pfEaten = FALSE;
	return S_OK;
}

BOOL CSinstar3Tsf::_InitKeyEventSink()
{
    ITfKeystrokeMgr *pKeystrokeMgr;
    HRESULT hr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return FALSE;

    hr = pKeystrokeMgr->AdviseKeyEventSink(_tfClientId, (ITfKeyEventSink *)this, TRUE);

    pKeystrokeMgr->Release();

    return (hr == S_OK);
}

void CSinstar3Tsf::_UninitKeyEventSink()
{
    ITfKeystrokeMgr *pKeystrokeMgr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return;

    pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);

    pKeystrokeMgr->Release();
}
