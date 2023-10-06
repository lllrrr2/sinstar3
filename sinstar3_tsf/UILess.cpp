#include "stdafx.h"
#include "UILess.h"
#include "EditSession.h"

SOUI::SComPtr<ITfContext> CCandidateList::GetContextDocument()
{
	return SOUI::SComPtr<ITfContext>();
}

CCandidateList::CCandidateList(CSinstar3Tsf* pTextService) :
	_tsf(pTextService),
	_ui_id(TF_INVALID_UIELEMENTID),
	_pbShow(FALSE),
	_changed_flags(0),
#if __REQUIRED_RPCNDR_H_VERSION__ > 500
	_selectionStyle(STYLE_IMPLIED_SELECTION),
#endif
	_idx(0)
{
	_ctx.cinfo.highlighted = 0;
	_ctx.cinfo.currentPage = 0;	
}

CCandidateList::~CCandidateList()
{
}

std::wstring GuidToString(const GUID& guid)
{
	wchar_t buf[64] = { 0 };
	swprintf_s(
		buf,
		L"({%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X})",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
	return std::wstring(buf);
}

STDMETHODIMP CCandidateList::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == NULL)
	{
		return E_INVALIDARG;
	}

	*ppvObj = NULL;

	if (IsEqualIID(riid, IID_ITfUIElement) ||
		IsEqualIID(riid, IID_ITfCandidateListUIElement) ||
		IsEqualIID(riid, IID_ITfCandidateListUIElementBehavior)
		)
	{
		*ppvObj = (ITfCandidateListUIElementBehavior*)this;
	}
#if __REQUIRED_RPCNDR_H_VERSION__ > 500
	else if (IsEqualIID(riid, __uuidof(ITfFnSearchCandidateProvider)))
	{
		*ppvObj = (ITfFnSearchCandidateProvider*)this;
	}
	else if (IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, __uuidof(ITfIntegratableCandidateListUIElement)))
	{
		*ppvObj = (ITfIntegratableCandidateListUIElement*)this;
	}
#endif
	if (*ppvObj)
	{
		SLOGI()<<"UILess::QueryInterface INTERFACE GUID "<< GuidToString(riid).c_str();
		AddRef();
		return S_OK;
	}
	else//�����˲�֧�ֵ�IID����¼������
	{
		SLOGW()<<"UILess::QueryInterface NOINTERFACE GUID "<< GuidToString(riid).c_str();
	}

	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CCandidateList::AddRef(void)
{
	return ++_cRef;
}

STDMETHODIMP_(ULONG) CCandidateList::Release(void)
{
	LONG cr = --_cRef;

	assert(_cRef >= 0);

	if (_cRef == 0)
	{
		delete this;
	}

	return cr;
}

STDMETHODIMP CCandidateList::GetDescription(BSTR* pbstr)
{
	static BSTR str = SysAllocString(L"Sinstar Candidate List");
	if (pbstr)
	{
		*pbstr = str;
	}
	return S_OK;
}

STDMETHODIMP CCandidateList::GetGUID(GUID* pguid)
{
	// {0EEC72CF-711A-443A-A403-FF8CAFCD9AC0}
	/*typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
	} GUID;*/
	pguid->Data1=0xeec72cf;
	pguid->Data2=0x711a;
	pguid->Data3=0x443a;
	pguid->Data4[0]=0xa4;
	pguid->Data4[1]=0x3;
	pguid->Data4[2]=0xff;
	pguid->Data4[3]=0x8c;
	pguid->Data4[4]=0xaf;
	pguid->Data4[5]=0xcd;
	pguid->Data4[6]=0x9a;
	pguid->Data4[7]=0xc0;
	//*pguid = { 0xeec72cf, 0x711a, 0x443a, { 0xa4, 0x3, 0xff, 0x8c, 0xaf, 0xcd, 0x9a, 0xc0 } };
	return S_OK;
}

STDMETHODIMP CCandidateList::Show(BOOL showCandidateWindow)
{
	SLOGFMTI("UILess::Show :%s", showCandidateWindow ? "TRUE" : "FALSE");
	return E_NOTIMPL;
}

STDMETHODIMP CCandidateList::IsShown(BOOL* pIsShow)
{
	SLOGFMTI("UILess::IsShown");
	if (!pIsShow) return E_INVALIDARG;
	// If _ui_id is valid, means we are showing a candidate ui. show_ui_ means we
	// should show the ui via text service's own window.
	*pIsShow = ((_ui_id != TF_INVALID_UIELEMENTID) && _pbShow);
	return S_OK;
}

STDMETHODIMP CCandidateList::GetUpdatedFlags(DWORD* pdwFlags)
{
	if (!pdwFlags)
		return E_INVALIDARG;

	*pdwFlags = _changed_flags;

	// TSF call GetUpdatedFlags to ask what is changed after the previous call.
	// We clear the flags after TSF got the changed information, so that the next
	// call to this function will return the changes from now on.
	_changed_flags = 0;

	return S_OK;
}

STDMETHODIMP CCandidateList::GetDocumentMgr(ITfDocumentMgr** ppdim)
{
	*ppdim = NULL;
	ITfThreadMgr* pThreadMgr = _tsf->_GetThreadMgr();
	if (pThreadMgr == NULL)
	{
		return E_FAIL;
	}
	if (FAILED(pThreadMgr->GetFocus(ppdim)) || (*ppdim == NULL))
	{
		return E_FAIL;
	}
	return S_OK;
}

STDMETHODIMP CCandidateList::GetCount(UINT* pCandidateCount)
{
	SLOGFMTI("UILess::GetCount Current Count:%d", _ctx.cinfo.candies.size());
	if (!pCandidateCount)
		return E_INVALIDARG;
	*pCandidateCount = (UINT)_ctx.cinfo.candies.size();
	return S_OK;
}

STDMETHODIMP CCandidateList::GetSelection(UINT* pSelectedCandidateIndex)
{
	SLOGFMTI("UILess::GetSelection Current Selection:%d", _ctx.cinfo.highlighted);
	if (!pSelectedCandidateIndex)
		return E_INVALIDARG;
	*pSelectedCandidateIndex = _ctx.cinfo.highlighted;
	return S_OK;
}

STDMETHODIMP CCandidateList::GetString(UINT uIndex, BSTR* pbstr)
{
	SLOGFMTI("UILess::GetString uIndex:%d", uIndex);
	if (!pbstr)
		return E_INVALIDARG;

	*pbstr = NULL;
	CandidateInfo& cinfo = _ctx.cinfo;
	if (uIndex >= cinfo.candies.size())
		return E_INVALIDARG;
	std::wstring& str = cinfo.candies[uIndex].str;
	*pbstr = SysAllocString(str.c_str());
	SLOGFMTI(L"UILess::GetString uIndex:%d,bstr:%s", uIndex, *pbstr);
	return S_OK;
}

STDMETHODIMP CCandidateList::GetPageIndex(UINT* pIndex, UINT uSize, UINT* puPageCnt)
{
	//uSize =pIndex�Ĵ�С�� ���÷�����pIndex==NULL��������ȡpuPageCnt�е�ҳ��
	//ÿҳ��С
#define PAGESIZE 5

	SLOGFMTI("UILess::GetPageIndex uSize:%d", uSize);
	if (!puPageCnt)
		return E_INVALIDARG;
	CandidateInfo& cinfo = _ctx.cinfo;
	*puPageCnt = cinfo.candies.size() / PAGESIZE + (cinfo.candies.size() % PAGESIZE ? 1 : 0);
	if (pIndex) {
		if (uSize < *puPageCnt) {
			return E_INVALIDARG;
		}
		for (UINT i = 0; i < uSize; i++)
		{
			pIndex[i] = i * PAGESIZE;
		}
	}
	return S_OK;
}

STDMETHODIMP CCandidateList::SetPageIndex(UINT* pIndex, UINT uPageCnt)
{
	SLOGFMTI("UILess::SetPageIndex");
	if (!pIndex)
		return E_INVALIDARG;
	return S_OK;
}

STDMETHODIMP CCandidateList::GetCurrentPage(UINT* puPage)
{
	SLOGFMTI("UILess::GetCurrentPage");
	if (!puPage)
		return E_INVALIDARG;
	*puPage = _ctx.cinfo.currentPage;
	return S_OK;
}

STDMETHODIMP CCandidateList::SetSelection(UINT nIndex)
{
	SLOGFMTI("UILess::SetSelection");
	return S_OK;
}

STDMETHODIMP CCandidateList::Finalize(void)
{
	SLOGFMTI("UILess::Finalize");
	ITfContext* pCtx = _tsf->GetImeContext();
	if (pCtx)
	{
		_tsf->_EndComposition(pCtx, true);
	}
	return S_OK;
}

STDMETHODIMP CCandidateList::Abort(void)
{
	SLOGFMTI("UILess::Abort");
	ITfContext* pCtx = _tsf->GetImeContext();
	if (pCtx)
	{
		_tsf->_EndComposition(pCtx, false);
	}
	return S_OK;
}

#if __REQUIRED_RPCNDR_H_VERSION__ > 500
STDMETHODIMP CCandidateList::SetIntegrationStyle(GUID guidIntegrationStyle)
{
	SLOGFMTI("UILess::SetIntegrationStyle");
	return S_OK;
}

STDMETHODIMP CCandidateList::GetSelectionStyle(_Out_ TfIntegratableCandidateListSelectionStyle* ptfSelectionStyle)
{
	SLOGFMTI("UILess::GetSelectionStyle");
	*ptfSelectionStyle = _selectionStyle;
	return S_OK;
}

STDMETHODIMP CCandidateList::OnKeyDown(_In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pIsEaten)
{
	SLOGFMTI("UILess::OnKeyDown");
	*pIsEaten = TRUE;
	return S_OK;
}

STDMETHODIMP CCandidateList::ShowCandidateNumbers(_Out_ BOOL* pIsShow)
{
	SLOGFMTI("UILess::ShowCandidateNumbers");
	*pIsShow = TRUE;
	return S_OK;
}

STDMETHODIMP CCandidateList::FinalizeExactCompositionString()
{
	SLOGFMTI("UILess::FinalizeExactCompositionString");
	return E_NOTIMPL;
}

HRESULT __stdcall CCandidateList::GetDisplayName(BSTR* pbstrName)
{
	*pbstrName = SysAllocString(L"�������뷨");
	return S_OK;
}

HRESULT __stdcall CCandidateList::GetSearchCandidates(BSTR bstrQuery, BSTR bstrApplicationId, ITfCandidateList** pplist)
{
	*pplist = NULL;
	return S_FALSE;
}

HRESULT __stdcall CCandidateList::SetResult(BSTR bstrQuery, BSTR bstrApplicationID, BSTR bstrResult)
{
	return S_OK;
}
#endif

HRESULT CCandidateList::BeginUIElement()
{
	SOUI::SComPtr<ITfThreadMgr> pThreadMgr = _tsf->_GetThreadMgr();
	SOUI::SComPtr<ITfUIElementMgr> pUIElementMgr;
	HRESULT hr = pThreadMgr->QueryInterface(&pUIElementMgr);
	_pbShow = TRUE;

	if (FAILED(hr) || (pUIElementMgr == NULL))
		return S_OK;

	if (FAILED(pUIElementMgr->BeginUIElement(this, &_pbShow, &_ui_id))) {
		_pbShow = TRUE;
		_ui_id = TF_INVALID_UIELEMENTID;
		return S_OK;
	}
	if (_pbShow)
	{
		return pUIElementMgr->EndUIElement(_ui_id);
	}
	return S_OK;
}

HRESULT CCandidateList::UpdateUIElement()
{
	//������ʾUI���ٵ��� UpdateUIElement
	if (_pbShow)
		return S_OK;
	if (_ui_id == TF_INVALID_UIELEMENTID) return E_UNEXPECTED;

	HRESULT hr = S_OK;
	SOUI::SComPtr<ITfUIElementMgr> pUIElementMgr;
	SOUI::SComPtr<ITfThreadMgr> pThreadMgr = _tsf->_GetThreadMgr();
	if (NULL == pThreadMgr)
	{
		return S_OK;
	}
	hr = pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void**)&pUIElementMgr);

	if (hr == S_OK)
	{		
		return pUIElementMgr->UpdateUIElement(_ui_id);
	}
	return S_OK;
}

HRESULT CCandidateList::EndUI()
{
	_ctx.cinfo.candies.clear(); _idx = 0;
	if (_pbShow)
		return S_OK;

	if (_ui_id == TF_INVALID_UIELEMENTID) return E_UNEXPECTED;

	SOUI::SComPtr<ITfThreadMgr> pThreadMgr = _tsf->_GetThreadMgr();
	SOUI::SComPtr<ITfUIElementMgr> pUIElementMgr;
	HRESULT hr = pThreadMgr->QueryInterface(&pUIElementMgr);

	if (pUIElementMgr != NULL)
		return pUIElementMgr->EndUIElement(_ui_id);
	return S_OK;
}


