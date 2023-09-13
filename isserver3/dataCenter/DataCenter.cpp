#include "StdAfx.h"
#include "DataCenter.h"
#include "../ime/ui/SkinMananger.h"
#include "../IsSvrProxy.h"
#include <SouiFactory.h>

//#define kLogTag "CDataCenter"
namespace SOUI
{
	template<>
	CDataCenter * SSingleton<CDataCenter>::ms_Singleton = NULL;

	CDataCenter::CDataCenter(const SStringT & strDataPath):m_data(strDataPath),m_path(strDataPath)
	{
		InitializeCriticalSection(&m_cs);
	}

	CDataCenter::~CDataCenter(void)
	{
		DeleteCriticalSection(&m_cs);
	}

	void CDataCenter::Lock()
	{
		EnterCriticalSection(&m_cs);
	}

	void CDataCenter::Unlock()
	{
		LeaveCriticalSection(&m_cs);
	}

	SStringT CDataCenter::GetDataPath() const
	{
		return m_path;
	}

	CMyData::CMyData(const SStringT & strDataPath)
		:m_tmInputSpan(0), m_cInputCount(0)
		,m_tmTotalSpan(0), m_cTotalInput(0)
		,m_nScale(100)
	{
		m_compInfo.cWild = 'z';
		m_compInfo.strCompName = _T("����...");

		{
			pugi::xml_document fontMap;
			if (fontMap.load_file(strDataPath + _T("\\data\\fontmap.xml")))
			{
				pugi::xml_node font = fontMap.child(L"fontmap").child(L"font");
				while (font)
				{
					m_fontMap[font.attribute(L"face").as_string()] = font.attribute(L"file").as_string();
					font = font.next_sibling(L"font");
				}
			}
		}

		CRegKey reg;
		LONG ret = reg.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\SetoutSoft\\sinstar3"), KEY_READ | KEY_WOW64_64KEY);
		if (ret == ERROR_SUCCESS)
		{
			reg.QueryDWORDValue(_T("input_count"), m_cTotalInput);
			reg.QueryDWORDValue(_T("input_span"), m_tmTotalSpan);
			reg.Close();
			if (m_tmTotalSpan == 0) m_cTotalInput = 0;
			if(m_cTotalInput>0x7fffffff || m_tmTotalSpan>0x7fffffff)
			{//clear data
				m_cTotalInput = m_tmTotalSpan = 0;
			}
		}
		m_strDataPath = strDataPath+_T("\\server");

	}

	const SArray<CNameTypePair>& CMyData::UpdateCompList()
	{
		m_compList.RemoveAll();

		TCHAR szFilter[MAX_PATH];
		TCHAR szFullPath[MAX_PATH];
		COMPHEAD header = { 0 };
		_stprintf(szFilter, _T("%s\\*.cit"), GetDataPath().c_str());
		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(szFilter, &fd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do {
				if (!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
				{
					_stprintf(szFullPath, _T("%s\\%s"), GetDataPath().c_str(), fd.cFileName);
					if (CIsSvrProxy::GetSvrCore()->ExtractCompInfo(szFullPath, &header, NULL))
					{
						CNameTypePair item;
						item.strTitle=fd.cFileName;
						item.strTitle.MakeLower();
						item.strName=header.szName;
						m_compList.Add(item);
					}
				}
			} while (FindNextFile(hFind, &fd));
			FindClose(hFind);
		}

		return m_compList;
	}

	const SArray<CNameTypePair>& CMyData::UpdateFlmList()
	{
		m_flmList.RemoveAll();

		TCHAR szFilter[MAX_PATH];
		TCHAR szFullPath[MAX_PATH];
		FLMINFO header = { 0 };
		_stprintf(szFilter, _T("%s\\*.flm"), GetDataPath().c_str());
		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(szFilter, &fd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do {
				if (!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
				{
					_stprintf(szFullPath, _T("%s\\%s"), GetDataPath().c_str(), fd.cFileName);
					if (CIsSvrProxy::GetSvrCore()->ExtractFlmInfo(szFullPath, &header))
					{
						CNameTypePair item;
						item.strTitle=fd.cFileName;
						item.strTitle.MakeLower();
						item.strName=header.szName;
						m_flmList.Add(item);
					}
				}
			} while (FindNextFile(hFind, &fd));
			FindClose(hFind);
		}

		return m_flmList;
	}




	CMyData::~CMyData()
	{
		saveSpeed();
	}

	bool CMyData::saveSpeed()
	{
		CRegKey reg;
		LONG ret = reg.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\SetoutSoft\\sinstar3"), KEY_READ|KEY_WRITE | KEY_WOW64_64KEY);
		if(ret != ERROR_SUCCESS)
			return false;
		reg.SetDWORDValue(_T("input_count"), getTotalInput());
		reg.SetDWORDValue(_T("input_span"), getTotalSpan());
		reg.Close();
		return true;
	}

	
	SStringW CMyData::getFontFile(const SStringW & strFace) const
	{
		const SMap<SStringW,SStringW>::CPair *p= m_fontMap.Lookup(strFace);
		if(!p)
			return SStringW();
		else
			return p->m_value;
	}

	DWORD CMyData::getTotalInput() const
	{
		return m_cInputCount + m_cTotalInput;
	}

	DWORD CMyData::getTotalSpan() const
	{
		return m_tmInputSpan+m_tmTotalSpan;
	}

	bool CMyData::changeSkin(const SStringT &strSkin)
	{
		SouiFactory souiFac;
		DWORD dwBegin = GetTickCount();
		if (!strSkin.IsEmpty())
		{//�����ⲿƤ��
			SAutoRefPtr<IResProvider> pResProvider;
			if(strSkin.EndsWith(_T("sskn"),true))
			{
				g_ComMgr2->CreateResProvider_ZIP((IObjRef**)&pResProvider);
				ZIPRES_PARAM param;
				ZipFile(&param,GETRENDERFACTORY, strSkin);
				if (!pResProvider->Init((WPARAM)&param, 0))
					return false;
			}else
			{//load folder
				pResProvider.Attach(souiFac.CreateResProvider(RES_FILE));
				if(!pResProvider->Init((WPARAM)(LPCTSTR)strSkin,0))
					return false;
			}

			SApplication::getSingleton().AddResProvider(pResProvider, NULL);
			IUiDefInfo *pUiDef = SUiDef::CreateUiDefInfo();
			pUiDef->Init(pResProvider, _T("uidef:xml_init"));
			SApplication::getSingleton().RemoveResProvider(pResProvider);
			if (pUiDef->GetObjDefAttr())
			{//������Ƥ���д���ȫ�ֵ�object default attribute
				pUiDef->Release();
				return false;
			}

			if (!g_SettingsG->strSkin.IsEmpty() && g_SettingsG->strSkin!= strSkin)
			{//�������ʹ�õ�����Ƥ����
				//remove skin pool and style pool created from this resource package.
				IResProvider *pLastRes = SApplication::getSingleton().GetTailResProvider();
				SApplication::getSingleton().RemoveResProvider(pLastRes);
				SUiDef::getSingletonPtr()->PopUiDefInfo(NULL);
			}

			SApplication::getSingleton().AddResProvider(pResProvider, NULL);
			CSkinMananger::ExtractSkinOffset(pResProvider,m_skinInfo);
			SUiDef::getSingletonPtr()->PushUiDefInfo(pUiDef);
			if(g_SettingsG->strFontDesc.IsEmpty())
				SUiDef::getSingletonPtr()->SetDefFontInfo(pUiDef->GetDefFontInfo());
			pUiDef->Release();
		}
		else if (!g_SettingsG->strSkin.IsEmpty())
		{//�������ʹ�õ�����Ƥ��,��ԭʹ��ϵͳ����Ƥ��
			IResProvider *pLastRes = SApplication::getSingleton().GetTailResProvider();
			SApplication::getSingleton().RemoveResProvider(pLastRes);
			SUiDef::getSingletonPtr()->PopUiDefInfo(NULL);
			if(g_SettingsG->strFontDesc.IsEmpty())
				SUiDef::getSingletonPtr()->SetDefFontInfo(m_defUiDefine->GetDefFontInfo());

			IResProvider *pCurRes = SApplication::getSingleton().GetHeadResProvider();
			CSkinMananger::ExtractSkinOffset(pCurRes,m_skinInfo);
		}
		SLOGI()<<"change skin "<<strSkin<< " cost " << (GetTickCount()-dwBegin);

		return true;
	}

	SStringT CMyData::GetDataPath() const
	{
		return m_strDataPath;
	}


	static int CharCmp(const void * p1, const void * p2)
	{
		const WCHAR *c1 = (const WCHAR*)p1;
		const WCHAR *c2 = (const WCHAR*)p2;
		return (*c1) - (*c2);
	}

	void CompInfo::SetSvrCompInfo(const COMPHEAD * compInfo)
	{
		cWild = compInfo->cWildChar;
		strCompName = S_CW2T(compInfo->szName);
		wcscpy(szCode,compInfo->szCode);
		nCodeNum = (int)wcslen(szCode);
		qsort(szCode,nCodeNum,sizeof(WCHAR),CharCmp);
	}

	BOOL CompInfo::IsCompChar(WCHAR cInput)
	{
		if(cWild == cInput) return TRUE;
		return NULL != bsearch(&cInput,szCode,nCodeNum,sizeof(WCHAR),CharCmp);
	}

}


