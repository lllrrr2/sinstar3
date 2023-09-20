#include "StdAfx.h"
#include "ConfigDlg.h"
#include <helper/STime.h>
#include <helper/SAdapterBase.h>
#include "../include/cf_helper.hpp"
#include "../include/filehelper.h"
#include "../iscomm/isProtocol.h"
#include "../helper/helper.h"
#include <string>
#include <shellapi.h>
#pragma comment(lib,"version.lib")
#include "AddBlurDlg.h"
#include "souidlgs.h"
#include "../IsSvrProxy.h"
#include "../dataCenter/SearchCfg.h"

#pragma warning(disable:4244)
namespace SOUI
{
	SStringT Ver2String(WORD wVer[4])
	{
		return SStringT().Format(_T("%u.%u.%u.%u"), wVer[0],wVer[1],wVer[2],wVer[3]);
	}

	class CGroupAdapter : public SMcAdapterBase
	{
	public:
		struct GroupInfo
		{
			int bEnable;
			SStringT strName;
			DWORD dwCount;
			SStringT strEditor;
			SStringT strRemark;
		};

		void AddGroup(const GroupInfo & gi)
		{
			m_arrGroupInfo.Add(gi);
		}

		void RemoveAll()
		{
			m_arrGroupInfo.RemoveAll();
		}
	protected:

		virtual int WINAPI getCount()
		{
			return m_arrGroupInfo.GetCount();
		}

		virtual void WINAPI getView(int position, IWindow * _pItem, IXmlNode * xmlTemplate)
		{
			SItemPanel *pItem = sobj_cast<SItemPanel>(_pItem);
			if (pItem->GetChildrenCount() == 0)
			{
				pItem->InitFromXml(xmlTemplate);
			}

			SCheckBox *pCheck = pItem->FindChildByID2<SCheckBox>(R.id.chk_group_name);
			pCheck->SetUserData(position);
			pCheck->SetWindowText(m_arrGroupInfo[position].strName);
			pCheck->GetEventSet()->setMutedState(true);
			pCheck->SetAttribute(L"checked", m_arrGroupInfo[position].bEnable?L"1":L"0");
			subscribeCheckEvent(pCheck);
			pCheck->GetEventSet()->setMutedState(false);

			pItem->FindChildByID(R.id.txt_group_size)->SetWindowText(SStringT().Format(_T("%d"), m_arrGroupInfo[position].dwCount));
			pItem->FindChildByID(R.id.txt_editor)->SetWindowText(m_arrGroupInfo[position].strEditor);
			pItem->FindChildByID(R.id.txt_remark)->SetWindowText(m_arrGroupInfo[position].strRemark);
		}

		virtual SStringW WINAPI GetColumnName(int iCol) const override{
			const wchar_t * pszColNames[] = {
				L"col_group",L"col_size",L"col_editor",L"col_remark"
			};
			return pszColNames[iCol];
		}

		virtual void subscribeCheckEvent(SCheckBox *pCheck) = 0;
	protected:
		SArray<GroupInfo> m_arrGroupInfo;
	};

	class CPhraseGroupAdapter: public CGroupAdapter
	{

	protected:
		virtual void subscribeCheckEvent(SCheckBox *pCheck)
		{
			pCheck->GetEventSet()->subscribeEvent(EventSwndStateChanged::EventID, Subscriber(&CPhraseGroupAdapter::OnGroupEnableChanged, this));

		}

		BOOL OnGroupEnableChanged(EventArgs *e)
		{
			EventSwndStateChanged *e2 = sobj_cast<EventSwndStateChanged>(e);
			if (EventSwndStateChanged_CheckState(e2,WndState_Check))
			{
				SWindow *pSender = sobj_cast<SWindow>(e->Sender());
				int idx = pSender->GetUserData();
				BOOL bEnable = (e2->dwNewState & WndState_Check)?1:0;
				if(ISACK_SUCCESS==CIsSvrProxy::GetSvrCore()->ReqEnablePhraseGroup(m_arrGroupInfo[idx].strName,bEnable))
				{
					m_arrGroupInfo[idx].bEnable=bEnable;
				}
			}
			return false;
		}
	};

	class CCelibGroupAdapter: public CGroupAdapter
	{

	protected:
		virtual void subscribeCheckEvent(SCheckBox *pCheck)
		{
			pCheck->GetEventSet()->subscribeEvent(EventSwndStateChanged::EventID, Subscriber(&CCelibGroupAdapter::OnGroupEnableChanged, this));

		}

		BOOL OnGroupEnableChanged(EventArgs *e)
		{
			EventSwndStateChanged *e2 = sobj_cast<EventSwndStateChanged>(e);
			if (EventSwndStateChanged_CheckState(e2,WndState_Check))
			{
				SWindow *pSender = sobj_cast<SWindow>(e->Sender());
				int idx = pSender->GetUserData();
				BOOL bEnable = (e2->dwNewState & WndState_Check)?1:0;
				if(ISACK_SUCCESS == CIsSvrProxy::GetSvrCore()->ReqFlmEnableGroup(m_arrGroupInfo[idx].strName,bEnable))
					m_arrGroupInfo[idx].bEnable = bEnable;
			}
			return false;
		}
	};

	class CBlurListAdapter : public SAdapterBase
	{
		struct INDEXINFO
		{
			int iGroup;
			int iIndex;
		};

		struct BLUREQUILTEX : BLUREQUILT
		{
			SStringT strBlur;
		};
	public:
		enum BLURTYPE {
			BLUR_TUNE = 0,
			BLUR_RHYME,
			BLUR_FULL,
		};

		void RemoveAll()
		{
			for (int i = 0; i < 3; i++)
			{
				m_lstBlur[i].RemoveAll();
			}
		}

		void AddBlur(BLURTYPE bt, LPCSTR pszFrom,LPCSTR pszTo)
		{
			BLUREQUILTEX br;
			strcpy_s(br.szFrom, 7, pszFrom);
			strcpy_s(br.szTo, 7, pszTo);
			br.strBlur = S_CA2T(SStringA().Format("%s=%s", pszFrom, pszTo));
			m_lstBlur[bt].Add(br);
		}
		
		void update()
		{
			m_lstIndex.RemoveAll();
			for (int i = 0; i < 3; i++)
			{
				INDEXINFO ii = { i,-1 };
				m_lstIndex.Add(ii);
				for (UINT j = 0; j < m_lstBlur[i].GetCount(); j++)
				{
					ii.iIndex = j;
					m_lstIndex.Add(ii);
				}
			}
			notifyDataSetChanged();
		}

		bool getBlur(int iItem, char szFrom[7], char szTo[7])
		{
			INDEXINFO ii = position2IndexInfo(iItem);
			if (ii.iIndex == -1) return false;
			strcpy(szFrom, m_lstBlur[ii.iGroup][ii.iIndex].szFrom);
			strcpy(szTo, m_lstBlur[ii.iGroup][ii.iIndex].szTo);
			return true;
		}
	protected:
		INDEXINFO position2IndexInfo(int position)
		{
			return m_lstIndex[position];
		}

		void getGroupView(int position, IWindow * _pItem, IXmlNode * xmlTemplate)
		{
			SItemPanel *pItem = sobj_cast<SItemPanel>(_pItem);
			if (pItem->GetChildrenCount() == 0)
			{
				pItem->InitFromXml(xmlTemplate->Child(L"group",FALSE));
			}
			const TCHAR * KGroupName[3] = {
				_T("��ĸģ��"),
				_T("��ĸģ��"),
				_T("ȫƴģ��"),
			};
			INDEXINFO ii = position2IndexInfo(position);
			pItem->FindChildByID(R.id.txt_blur_group)->SetWindowText(KGroupName[ii.iGroup]);
		}

		void getItemView(int position, IWindow * _pItem, IXmlNode * xmlTemplate)
		{
			SItemPanel *pItem =sobj_cast<SItemPanel>(_pItem);
			if (pItem->GetChildrenCount() == 0)
			{
				pItem->InitFromXml(xmlTemplate->Child(L"item",FALSE));
			}
			INDEXINFO ii = position2IndexInfo(position);
			pItem->FindChildByID(R.id.txt_blur_info)->SetWindowText(m_lstBlur[ii.iGroup][ii.iIndex].strBlur);
		}

		virtual void WINAPI getView(int position, IWindow * pItem, IXmlNode * xmlTemplate) override
		{
			int viewType = getItemViewType(position, 0);
			if (viewType == 0)
			{
				getGroupView(position, pItem, xmlTemplate);
			}
			else
			{
				getItemView(position, pItem, xmlTemplate);
			}
		}

		virtual int WINAPI getCount() override
		{
			return (int)m_lstIndex.GetCount();
		}

		virtual int WINAPI getViewTypeCount()
		{
			return 2;
		}
		
		virtual int WINAPI getItemViewType(int position, DWORD dwState)
		{
			INDEXINFO ii = m_lstIndex[position];
			return ii.iIndex == -1 ? 0 : 1;
		}
		
	private:
		SArray<BLUREQUILTEX> m_lstBlur[3];
		SArray<INDEXINFO> m_lstIndex;
	};

	
	CConfigDlg * CConfigDlg::_instance = NULL;
	CConfigDlg::CConfigDlg(IUpdateIntervalObserver *pObserver)
		:SHostWnd(UIRES.LAYOUT.dlg_config)
		,m_pObserver(pObserver)
	{
		SASSERT(_instance==NULL);
		_instance=this;
	}

	CConfigDlg::~CConfigDlg(void)
	{
		_instance=NULL;
	}

	void CConfigDlg::FindAndSetCheck(int id,BOOL bcheck)
	{
		SWindow *pCtrl = FindChildByID(id);
			SASSERT(pCtrl);
			if (pCtrl)
				pCtrl->SetCheck(bcheck);
	}

	void CConfigDlg::FindAndSetText(int id,LPCTSTR text)
	{
		SWindow *pCtrl = FindChildByID(id);
			SASSERT(pCtrl); 
			if (pCtrl)
				pCtrl->SetWindowText(text);
	}

	void CConfigDlg::FindAndSetSpin(int id, int nValue)
	{
		SSpinButtonCtrl *pCtrl = FindChildByID2<SSpinButtonCtrl>(id);
		SASSERT(pCtrl);
		if (pCtrl)
		{
			SWindow *pBuddy = pCtrl->GetBuddy();
			if(pBuddy) pBuddy->GetEventSet()->setMutedState(true);
			pCtrl->GetEventSet()->setMutedState(true);
			pCtrl->SetValue(nValue);
			pCtrl->GetEventSet()->setMutedState(false);
			if (pBuddy) pBuddy->GetEventSet()->setMutedState(false);
		}
	}

	void CConfigDlg::FindAndSetHotKey(int id,DWORD accel)
	{
		SHotKeyCtrl *pCtrl = FindChildByID2<SHotKeyCtrl>(id);
			SASSERT(pCtrl);
			if (pCtrl)
				pCtrl->SetHotKey(LOWORD(accel), HIWORD(accel));
	}

	void CConfigDlg::FindAndSetCombobox(int id,int nSel)
	{
		SComboBox *pCtrl = FindChildByID2<SComboBox>(id);
		SASSERT(pCtrl);
		if (pCtrl)
			pCtrl->SetCurSel(nSel);
	}

	WORD Char2VKey(TCHAR wChar)
	{
		TCHAR szBuf[2] = { wChar,0 };
		return SAccelerator::VkFromString(szBuf);
	}

	WORD Vkey2Char(WORD wVK)
	{
		SStringT strName = SAccelerator::GetKeyName(wVK);
		if (strName.GetLength() > 1 || strName.IsEmpty()) return 0;
		return strName[0];
	}

	void CConfigDlg::InitPageHabit()
	{
		FindAndSetCombobox(R.id.cbx_left_shift_func,g_SettingsG->m_funLeftShift);
		FindAndSetCombobox(R.id.cbx_right_shift_func,g_SettingsG->m_funRightShift);
		FindAndSetCombobox(R.id.cbx_left_ctrl_func,g_SettingsG->m_funLeftCtrl);
		FindAndSetCombobox(R.id.cbx_right_ctrl_func,g_SettingsG->m_funRightCtrl);

		FindAndSetCheck(R.id.chk_blend_for_spell,g_SettingsG->bBlendSpell);
		FindAndSetCheck(R.id.chk_blend_for_userdef,g_SettingsG->bBlendUD);

		//����״̬�س�
		int CtrlId = g_SettingsG->bEnterClear ? R.id.enter_for_clear : R.id.enter_for_input;
		FindAndSetCheck(CtrlId, TRUE);

		//�����Զ�����
		FindAndSetCheck(R.id.cand_manul_input+g_SettingsG->inputMode,TRUE);
		//ƴ�������������
		FindAndSetCheck(R.id.cand_py_phrase_first, g_SettingsG->bPYPhraseFirst);
		//op tip
		FindAndSetCheck(R.id.chk_show_op_tip, g_SettingsG->bShowOpTip);

		//UILessʱ�Զ�����״̬��
		FindAndSetCheck(R.id.chk_autoHideStatusForUILess,g_SettingsUI->bUILessHideStatus);

		FindAndSetCheck(R.id.chk_autoHideStatusForFullScreen, g_SettingsUI->bFullScreenHideStatus);

		FindAndSetCheck(R.id.chk_disable_first_wild,g_SettingsG->bDisableFirstWild);

		FindAndSetCheck(R.id.chk_full_space,g_SettingsG->bFullSpace);
		
		FindAndSetHotKey(R.id.hk_to_sentmode, Char2VKey(g_SettingsG->bySentMode));

		FindAndSetSpin(R.id.spin_delay_time, g_SettingsG->nDelayTime);
		
		FindAndSetSpin(R.id.spin_cand_num,g_SettingsG->nMaxCands);

		SStringT strFontDesc = g_SettingsG->strFontDesc;
		if(strFontDesc.IsEmpty())
			strFontDesc=_T("<Ƥ��Ĭ��>");
		FindAndSetText(R.id.edit_font,strFontDesc);

		FindAndSetCheck(R.id.radio_init_ch+g_SettingsG->bInitEnglish,TRUE);

		FindAndSetCheck(220 + g_SettingsUI->enumInlineMode, TRUE);

		FindAndSetCheck(R.id.chk_autoQuitCAP, g_SettingsG->bQuitEnCancelCAP);

		FindAndSetCheck(R.id.chk_autoQuitUMode, g_SettingsG->bBackQuitUMode);
	}

	void CConfigDlg::InitPageHotKey()
	{
		FindAndSetHotKey(R.id.hk_repeat, g_SettingsG->dwHotkeys[HKI_Repeat]);
		FindAndSetHotKey(R.id.hk_switch_py, g_SettingsG->dwHotkeys[HKI_Mode]);
		FindAndSetHotKey(R.id.hk_make_phrase, g_SettingsG->dwHotkeys[HKI_MakePhrase]);
		FindAndSetHotKey(R.id.hk_show_table, g_SettingsG->dwHotkeys[HKI_ShowRoot]);
		FindAndSetHotKey(R.id.hk_show_comp, g_SettingsG->dwHotkeys[HKI_Query]);
		FindAndSetHotKey(R.id.hk_show_statusbar, g_SettingsG->dwHotkeys[HKI_HideStatus]);
		FindAndSetHotKey(R.id.hk_input_en, g_SettingsG->dwHotkeys[HKI_EnSwitch]);
		FindAndSetHotKey(R.id.hk_filter_gbk, g_SettingsG->dwHotkeys[HKI_FilterGbk]);
		FindAndSetHotKey(R.id.hk_tts, g_SettingsG->dwHotkeys[HKI_TTS]);
		FindAndSetHotKey(R.id.hk_record, g_SettingsG->dwHotkeys[HKI_Record]);
		FindAndSetHotKey(R.id.hk_to_umode, g_SettingsG->dwHotkeys[HKI_UDMode]);
		FindAndSetHotKey(R.id.hk_to_linemode,g_SettingsG->dwHotkeys[HKI_LineMode]);
		FindAndSetHotKey(R.id.hk_switch_tempspell,g_SettingsG->dwHotkeys[HKI_TempSpell]);
	}

	void CConfigDlg::InitPageAssociate()
	{
		int CtrlId;
		//����
		CtrlId = R.id.ass_mode_none +g_SettingsG->byAstMode;
		FindAndSetCheck(CtrlId, TRUE);
		CtrlId= R.id.rate_adjust_disable +g_SettingsG->byRateAdjust;
		FindAndSetCheck(CtrlId, TRUE);
		CtrlId = 600;
		if (g_SettingsG->byForecast == MQC_FORECAST)
			CtrlId = 601;
		else if (g_SettingsG->byForecast == MQC_FCNOCAND)
			CtrlId = 602;
		FindAndSetCheck(CtrlId, TRUE);

		FindAndSetCheck(R.id.chk_auto_comp_promp, g_SettingsG->bAutoPrompt);
		FindAndSetCheck(R.id.chk_auto_dot, g_SettingsG->bAutoDot);
		FindAndSetCheck(R.id.chk_auto_select_cand, g_SettingsG->bAutoMatch);
		
		FindAndSetCheck(R.id.chk_sent_associate,g_SettingsUI->bSentAssocite);
		int nSentMax = CIsSvrProxy::GetSvrCore()->GetSentRecordMax();
		FindAndSetText(R.id.edit_sent_record_max, SStringT().Format(_T("%d"), nSentMax));
		int nPredictLength = CIsSvrProxy::GetSvrCore()->GetMaxPhrasePreictLength();
		FindAndSetSpin(R.id.spin_predict_phrase_maxlength, nPredictLength);

		int nDeepness = CIsSvrProxy::GetSvrCore()->GetMaxPhraseAstDeepness();
		FindAndSetSpin(R.id.spin_phrase_ast_deepness_max, nDeepness);
	}


	void CConfigDlg::InitPageCandidate()
	{
		FindAndSetCheck(R.id.chk_enable_23cand_hotkey, g_SettingsG->b23CandKey);

		FindAndSetHotKey(R.id.hk_2_cand, g_SettingsG->by2CandVK);
		FindAndSetHotKey(R.id.hk_3_cand, g_SettingsG->by3CandVK);
		
		FindAndSetHotKey(R.id.hk_turn_prev, g_SettingsG->byTurnPageUpVK);
		FindAndSetHotKey(R.id.hk_turn_next, g_SettingsG->byTurnPageDownVK);
		
		FindAndSetCheck(R.id.chk_disable_number_to_select_cand, g_SettingsG->bCandSelNoNum);
		FindAndSetCheck(R.id.chk_full_skip_simple, g_SettingsG->bOnlySimpleCode);

		FindAndSetCheck(R.id.gbk_show_only + g_SettingsG->nGbkMode, TRUE);
	}
	
	void CConfigDlg::InitPageMisc()
	{
		SComboBox *cbxSearchEngine = FindChildByID2<SComboBox>(R.id.cbx_search_engine);
		int iSearch=0;
		const SArray<CSearchCfg::SearchInfo> & urls = CSearchCfg::getSingleton().GetUrls();
		for(int i=0;i<urls.GetCount();i++)
		{
			SStringT value = SStringT().Format(_T("%s|%s"),urls[i].name.c_str(),urls[i].url.c_str());
			cbxSearchEngine->InsertItem(i,value,0,0);
		}
		cbxSearchEngine->GetEventSet()->setMutedState(true);
		cbxSearchEngine->SetCurSel(CSearchCfg::getSingleton().GetSel());
		cbxSearchEngine->GetEventSet()->setMutedState(false);

		FindAndSetCheck(R.id.sound_disable + g_SettingsG->nSoundAlert, TRUE);

		for (int i = 0; i < 6; i++)
		{
			FindAndSetHotKey(R.id.hk_bihua_heng+i, g_SettingsG->byLineKey[i]);
		}

		FindAndSetText(R.id.edit_backup,g_SettingsG->szBackupDir);
	}

	void CConfigDlg::InitPageAbout()
	{
		TCHAR szPath[1000];
		GetModuleFileName(NULL, szPath, 1000);

		WIN32_FIND_DATA wfd;
		HANDLE h=FindFirstFile(szPath, &wfd);
		FindClose(h);
		FILETIME lft;
		FileTimeToLocalFileTime(&wfd.ftLastWriteTime,&lft);
		SYSTEMTIME tm;
		FileTimeToSystemTime(&lft, &tm);
		STime time(tm.wYear,tm.wMonth,tm.wDay,tm.wHour,tm.wMinute,tm.wSecond);
		SStringT strTm = time.Format(_T("%Y-%m-%d %H:%M:%S %A"));
		FindChildByID(R.id.txt_build_time)->SetWindowText(strTm);
		
		TCHAR szExe[MAX_PATH];
		GetModuleFileName(NULL,szExe,MAX_PATH);
		WORD ver[4];
		SDpiHelper::PEVersion(szExe,ver[0],ver[1],ver[2],ver[3]);
		SStringT strVer = Ver2String(ver);
		FindChildByID(R.id.txt_svr_ver)->SetWindowText(strVer);

		SWindow *pCheck = FindChildByID(R.id.chk_auto_update);
		int nCheckUpdateInterval = m_pObserver->GetUpdateInterval();
		if(nCheckUpdateInterval != 0)
		{
			pCheck->GetEventSet()->setMutedState(true);
			pCheck->SetCheck(TRUE);
			pCheck->GetEventSet()->setMutedState(false);
			int intervals[] = {0,7,30,90 };
			if(nCheckUpdateInterval>90)
				nCheckUpdateInterval = 90;
			SComboBox *pCbxInterval = FindChildByID2<SComboBox>(R.id.cbx_update_interval);
			int idx = -1;
			for(int i=0;i<ARRAYSIZE(intervals)-1;i++)
			{
				if(nCheckUpdateInterval>intervals[i] && nCheckUpdateInterval<=intervals[i+1])
					idx = i;
			}
			pCbxInterval->SetCurSel(idx);
		}
	}

	void CConfigDlg::InitTtsTokenInfo(bool bChVoice, SComboBox *pCbx)
	{
		int nTokens= CIsSvrProxy::GetInstance()->TtsGetTokensInfo(bChVoice,NULL,0);
		if (nTokens)
		{
			WCHAR (*szToken)[MAX_TOKEN_NAME_LENGHT] = new WCHAR[nTokens][MAX_TOKEN_NAME_LENGHT];
			CIsSvrProxy::GetInstance()->TtsGetTokensInfo(bChVoice,szToken,nTokens);

			for (int i = 0; i < nTokens; i++)
			{
				pCbx->InsertItem(-1, S_CW2T(szToken[i]), 0, 0);
			}
			int iSel=CIsSvrProxy::GetInstance()->TtsGetVoice(bChVoice);
		
			if (iSel>=0 && iSel<nTokens)
			{
				pCbx->GetEventSet()->setMutedState(true);
				pCbx->SetCurSel(iSel);
				pCbx->GetEventSet()->setMutedState(false);
			}
			delete []szToken;
		}
	}

	void CConfigDlg::InitPageTTS()
	{
		InitTtsTokenInfo(false, FindChildByID2<SComboBox>(R.id.cbx_tts_en_token));
		InitTtsTokenInfo(true, FindChildByID2<SComboBox>(R.id.cbx_tts_ch_token));
		int nTtsSpeed = CIsSvrProxy::GetInstance()->TtsGetSpeed();
		FindChildByID(R.id.txt_tts_speed)->SetWindowText(SStringT().Format(_T("%d"), nTtsSpeed));
		FindChildByID2<SSliderBar>(R.id.slider_tts_speed)->SetValue(nTtsSpeed);
	}

	void CConfigDlg::InitPinyinBlur(COMFILE & cf, CBlurListAdapter * pBlurAdapter, int iGroup)
	{
		int nCount;
		CF_Read(&cf, &nCount, sizeof(int));
		for (int i = 0; i < nCount; i++)
		{
			char szPY1[7], szPY2[7];
			CF_ReadString(&cf, szPY1, 7);
			CF_ReadString(&cf, szPY2, 7);
			pBlurAdapter->AddBlur((CBlurListAdapter::BLURTYPE)iGroup,szPY1,szPY2);
		}
	}

	void CConfigDlg::InitPinyinBlurListView(SListView *pLvBLur)
	{
		CBlurListAdapter *pAdapter = (CBlurListAdapter*)pLvBLur->GetAdapter();
		pAdapter->RemoveAll();
		
		if (CIsSvrProxy::GetSvrCore()->ReqBlurQuery() == ISACK_SUCCESS)
		{
			PMSGDATA pMsgData = CIsSvrProxy::GetSvrCore()->GetAck();
			COMFILE cf = CF_Init(pMsgData->byData, MAX_BUF_ACK, pMsgData->sSize, 0);
			int bEnableBlur = 0, bZcsBlur = 0;
			CF_Read(&cf, &bEnableBlur, sizeof(int));
			FindAndSetCheck(R.id.chk_py_blur, bEnableBlur);
			CF_Read(&cf, &bZcsBlur, sizeof(int));
			FindAndSetCheck(R.id.chk_jp_zcs, bZcsBlur);

			InitPinyinBlur(cf, pAdapter, CBlurListAdapter::BLUR_TUNE);
			InitPinyinBlur(cf, pAdapter, CBlurListAdapter::BLUR_RHYME);
			InitPinyinBlur(cf, pAdapter, CBlurListAdapter::BLUR_FULL);
		}
		pAdapter->update();
	}

	void CConfigDlg::InitPagePinYin()
	{
		CBlurListAdapter * pAdapter = new CBlurListAdapter;
		SListView *pLvBlur = FindChildByID2<SListView>(R.id.lv_blur);
		SASSERT(pLvBlur);
		pLvBlur->SetAdapter(pAdapter);
		InitPinyinBlurListView(pLvBlur);
		pAdapter->Release();

	}

	void CConfigDlg::InitPhraseLib()
	{
		SMCListView * pLvPhraseLib = FindChildByID2<SMCListView>(R.id.mc_phraselib);
		CPhraseGroupAdapter *pAdapter = new CPhraseGroupAdapter();
		pLvPhraseLib->SetAdapter(pAdapter);
		pAdapter->Release();
		InitPhraseLibListview();
	}

	void CConfigDlg::InitPhraseLibListview()
	{
		int nPhraseGroup = CIsSvrProxy::GetSvrCore()->GetPhraseGroupCount();
		PGROUPINFO pGroupInfo = new GROUPINFO[nPhraseGroup];
		if (CIsSvrProxy::GetSvrCore()->QueryPhraseGroup(pGroupInfo,nPhraseGroup))
		{
			SMCListView * pLvPhraseLib = FindChildByID2<SMCListView>(R.id.mc_phraselib);
			CPhraseGroupAdapter *pAdapter = (CPhraseGroupAdapter*)pLvPhraseLib->GetAdapter();
			pAdapter->RemoveAll();
			for (int i = 0; i < nPhraseGroup; i++)
			{
				CGroupAdapter::GroupInfo gi;
				gi.dwCount = pGroupInfo[i].dwCount;
				gi.bEnable = pGroupInfo[i].bValid;
				gi.strName = pGroupInfo[i].szName;
				gi.strEditor = pGroupInfo[i].szEditor;
				gi.strRemark = pGroupInfo[i].szRemark;
				pAdapter->AddGroup(gi);
			}
			pAdapter->notifyDataSetChanged();
		}
	}

	void CConfigDlg::InitCeLib()
	{
		SMCListView * pLvCeLib = FindChildByID2<SMCListView>(R.id.mc_celib);
		CCelibGroupAdapter *pAdapter = new CCelibGroupAdapter();
		pLvCeLib->SetAdapter(pAdapter);

		CIsSvrProxy::GetSvrCore()->ReqFlmGetInfo();
		PMSGDATA pMsgData = CIsSvrProxy::GetSvrCore()->GetAck();
		COMFILE cf = CF_Init(pMsgData->byData, MAX_BUF_ACK, pMsgData->sSize, 0);
		BOOL bOpen=FALSE;
		CF_ReadT(cf,&bOpen);
		if(bOpen)
		{
			FLMINFO flmInfo;
			CF_ReadT(cf,&flmInfo);
			FindChildByID(R.id.edit_flm_name)->SetWindowText(flmInfo.szName);
			FindChildByID(R.id.edit_flm_addtion)->SetWindowText(flmInfo.szAddition);

			BYTE byGroup = 0;
			CF_ReadT(cf, &byGroup);
			for (BYTE i = 0; i < byGroup; i++)
			{
				CGroupAdapter::GroupInfo gi;
				CF_ReadT(cf, &gi.bEnable);
				std::wstring buf;
				CF_ReadWString(cf, buf);
				gi.strName = buf.c_str();
				CF_ReadT(cf, &gi.dwCount);
				CF_ReadWString(cf, buf);
				gi.strEditor = buf.c_str();
				CF_ReadWString(cf, buf);
				gi.strRemark = buf.c_str();
				pAdapter->AddGroup(gi);
			}
			pAdapter->notifyDataSetChanged();
		}else
		{
			FindChildByID(R.id.edit_flm_name)->SetWindowText(_T("none"));
		}

		pAdapter->Release();

	}


	void CConfigDlg::InitPages()
	{		
		InitPageHabit();
		InitPageHotKey();
		InitPageAssociate();	
		InitPageCandidate();
		InitPageMisc();
		InitPageTTS();
		InitPagePinYin();
		InitPhraseLib();
		InitCeLib();
		InitPageAbout();
	}


#define GetGroupCheck(id) int CheckId=0;\
SWindow *pCtrl = FindChildByID(id);\
	SASSERT(pCtrl);\
	pCtrl=pCtrl->GetSelectedSiblingInGroup();\
	SASSERT(pCtrl);\
	CheckId=pCtrl->GetID()

	void CConfigDlg::OnLeftShiftFun(EventArgs *e)
	{
		EventCBSelChange *e2 = sobj_cast<EventCBSelChange>(e);
		g_SettingsG->m_funLeftShift = (KeyFunction)(Fun_None + e2->nCurSel);
	}

	void CConfigDlg::OnRightShiftFun(EventArgs *e)
	{
		EventCBSelChange *e2 = sobj_cast<EventCBSelChange>(e);
		g_SettingsG->m_funRightShift = (KeyFunction)(Fun_None + e2->nCurSel);
	}

	void CConfigDlg::OnLeftCtrlFun(EventArgs *e)
	{
		EventCBSelChange *e2 = sobj_cast<EventCBSelChange>(e);
		g_SettingsG->m_funLeftCtrl = (KeyFunction)(Fun_None + e2->nCurSel);
	}

	void CConfigDlg::OnRightCtrlFun(EventArgs *e)
	{
		EventCBSelChange *e2 = sobj_cast<EventCBSelChange>(e);
		g_SettingsG->m_funRightCtrl = (KeyFunction)(Fun_None + e2->nCurSel);
	}

	void CConfigDlg::OnClickEnter(int id)
	{
		GetGroupCheck(id);
		switch (CheckId)
		{
		case R.id.enter_for_clear:
			g_SettingsG->bEnterClear=TRUE;
			break;
		case R.id.enter_for_input:
		default:
			g_SettingsG->bEnterClear = FALSE;
			break;
		}
	}


	void CConfigDlg::OnClickAlertMode(int id)
	{
		GetGroupCheck(id);
		switch (CheckId)
		{
		case R.id.sound_disable:
			g_SettingsG->nSoundAlert = 0;
			break;
		case R.id.sound_wave:
			g_SettingsG->nSoundAlert = 1;
			break;
		case R.id.sound_beep:
		default:
			g_SettingsG->nSoundAlert = 2;
			break;
		}
	}

	void CConfigDlg::OnAutoInput()
	{
		g_SettingsG->inputMode=CSettingsGlobal::auto_input;
	}

	void CConfigDlg::OnNextInput()
	{
		g_SettingsG->inputMode=CSettingsGlobal::next_input;
	}

	void CConfigDlg::OnManulInput()
	{
		g_SettingsG->inputMode=CSettingsGlobal::manul_input;
	}

	void CConfigDlg::OnPyPhraseFirst()
	{
		g_SettingsG->bPYPhraseFirst= FindChildByID(R.id.cand_py_phrase_first)->IsChecked();
	}

	void CConfigDlg::OnDislabeFirstWild()
	{
		g_SettingsG->bDisableFirstWild = FindChildByID(R.id.chk_disable_first_wild)->IsChecked();
	}

	void CConfigDlg::OnFullSpace()
	{
		g_SettingsG->bFullSpace = FindChildByID(R.id.chk_full_space)->IsChecked();
	}

	void CConfigDlg::OnChkOpTip(EventArgs *e)
	{
		SCheckBox *pCheck = sobj_cast<SCheckBox>(e->Sender());
		SASSERT(pCheck);
		g_SettingsG->bShowOpTip = pCheck->IsChecked();
	}

	void CConfigDlg::OnChkAutoHideStausForUILess(EventArgs* e)
	{
		SCheckBox* pCheck = sobj_cast<SCheckBox>(e->Sender());
		SASSERT(pCheck);
		g_SettingsUI->bUILessHideStatus = pCheck->IsChecked();
	}

	void CConfigDlg::OnChkAutoHideStausForFullScreen(EventArgs* e)
	{
		SCheckBox* pCheck = sobj_cast<SCheckBox>(e->Sender());
		SASSERT(pCheck);
		g_SettingsUI->bFullScreenHideStatus = pCheck->IsChecked();
	}
	
	void CConfigDlg::OnClickAssMode(int id)
	{
		GetGroupCheck(id);
		g_SettingsG->byAstMode = CheckId - 400;		
	}

	void CConfigDlg::OnClickForcast(int id)
	{
		GetGroupCheck(id);

		switch (CheckId)
		{
		case 600:
			g_SettingsG->byForecast = 0;
			break;
		case 601:
			g_SettingsG->byForecast = MQC_FORECAST;
			break;
		case 602:
			g_SettingsG->byForecast = MQC_FCNOCAND;
		}
	}

	void CConfigDlg::OnClickGBK(int id)
	{
		GetGroupCheck(id);

		switch (CheckId)
		{
		case 700:
			g_SettingsG->nGbkMode = CSettingsGlobal::GBK_HIDE;
			break;
		case 701:
			g_SettingsG->nGbkMode = CSettingsGlobal::GBK_SHOW_MANUAL;
			break;
		case 702:
			g_SettingsG->nGbkMode = CSettingsGlobal::GBK_SHOW_NORMAL;
		}
	}

	void CConfigDlg::OnChkFullSkipSimple()
	{
		g_SettingsG->bOnlySimpleCode = FindChildByID(R.id.chk_full_skip_simple)->IsChecked();
	}

	void CConfigDlg::OnClickRateAdjust(int id)
	{
		GetGroupCheck(id);
		g_SettingsG->byRateAdjust = CheckId - 500;
	}
	void CConfigDlg::OnClickAutoCompPromp()
	{
		g_SettingsG->bAutoPrompt= FindChildByID(R.id.chk_auto_comp_promp)->IsChecked();	
	}
	void CConfigDlg::OnClickAutoDot()
	{
		g_SettingsG->bAutoDot=FindChildByID(R.id.chk_auto_dot)->IsChecked();
	}
	void CConfigDlg::OnClickAutoSelectCand()
	{
		g_SettingsG->bAutoMatch = FindChildByID(R.id.chk_auto_select_cand)->IsChecked();
	}
	void CConfigDlg::OnClickSentAssociate()
	{
		g_SettingsUI->bSentAssocite = FindChildByID(R.id.chk_sent_associate)->IsChecked();
	}

	void CConfigDlg::OnDisableNumSelCand()
	{
		g_SettingsG->bCandSelNoNum= FindChildByID(R.id.chk_disable_number_to_select_cand)->IsChecked();
	}
	void CConfigDlg::OnEnable23Cand()
	{
		g_SettingsG->b23CandKey = FindChildByID(R.id.chk_enable_23cand_hotkey)->IsChecked();
	}
	void CConfigDlg::OnHotKeyEvent(EventArgs * pEvt)
	{
		EventSetHotKey *pHotKeyEvt = sobj_cast<EventSetHotKey>(pEvt);
		SHotKeyCtrl * pHotKeyCtrl = sobj_cast<SHotKeyCtrl>(pEvt->Sender());
		SASSERT(pHotKeyCtrl);
		DWORD dwAccel = MAKELONG(pHotKeyEvt->vKey, pHotKeyEvt->wModifiers);
		SLOGI()<<"id:" << pHotKeyCtrl->GetID() << " accel:" << SAccelerator::FormatAccelKey(dwAccel);
		switch (pHotKeyCtrl->GetID())
		{
			//hotkey page
		case R.id.hk_repeat:
			g_SettingsG->dwHotkeys[HKI_Repeat] = dwAccel; break;
		case R.id.hk_switch_py:
			g_SettingsG->dwHotkeys[HKI_Mode] = dwAccel; break;
		case R.id.hk_switch_tempspell:
			g_SettingsG->dwHotkeys[HKI_TempSpell] = dwAccel; break;
		case R.id.hk_make_phrase:
			g_SettingsG->dwHotkeys[HKI_MakePhrase] = dwAccel; break;
		case R.id.hk_show_table:
			g_SettingsG->dwHotkeys[HKI_ShowRoot] = dwAccel; break;
		case R.id.hk_show_comp:
			g_SettingsG->dwHotkeys[HKI_Query] = dwAccel; break;
		case R.id.hk_show_statusbar:
			g_SettingsG->dwHotkeys[HKI_HideStatus] = dwAccel; break;
		case R.id.hk_input_en:
			g_SettingsG->dwHotkeys[HKI_EnSwitch] = dwAccel; break;	
		case R.id.hk_filter_gbk:
			g_SettingsG->dwHotkeys[HKI_FilterGbk] = dwAccel; break;
		case R.id.hk_tts:
			g_SettingsG->dwHotkeys[HKI_TTS] = dwAccel; break;
		case R.id.hk_record:
			g_SettingsG->dwHotkeys[HKI_Record] = dwAccel; break;
		case R.id.hk_to_umode:
			g_SettingsG->dwHotkeys[HKI_UDMode] = dwAccel;break;
		case R.id.hk_to_linemode:
			g_SettingsG->dwHotkeys[HKI_LineMode] = dwAccel;break;
		case R.id.hk_2_cand:
			g_SettingsG->by2CandVK = pHotKeyEvt->vKey; break;
		case R.id.hk_3_cand:
			g_SettingsG->by3CandVK = pHotKeyEvt->vKey; break;
		case R.id.hk_turn_prev:
			g_SettingsG->byTurnPageUpVK = pHotKeyEvt->vKey; break;
		case R.id.hk_turn_next:
			g_SettingsG->byTurnPageDownVK = pHotKeyEvt->vKey; break;
		case R.id.hk_bihua_heng:
		case R.id.hk_bihua_shu:
		case R.id.hk_bihua_pie:
		case R.id.hk_bihua_na:
		case R.id.hk_bihua_zhe:
		case R.id.hk_bihua_wild:
			g_SettingsG->byLineKey[pHotKeyCtrl->GetID() - R.id.hk_bihua_heng] = pHotKeyEvt->vKey;
			break;
			break;
		case R.id.hk_to_sentmode:
			g_SettingsG->bySentMode = Vkey2Char(pHotKeyEvt->vKey);
			break;
		}
	}

	void CConfigDlg::OnClose()
	{
		DestroyWindow();
	}

	void CConfigDlg::OnTtsSpeedChanged(EventArgs * e)
	{
		EventSliderPos *e2 = sobj_cast<EventSliderPos>(e);
		SASSERT(e2);
		FindChildByID(R.id.txt_tts_speed)->SetWindowText(SStringT().Format(_T("%d"), e2->nPos));
		CIsSvrProxy::GetInstance()->TtsSetSpeed(e2->nPos);
	}

	const WCHAR KTTS_SAMPLE_CH[] = L"�����ʶ��ٶȲ��ԡ�";
	const WCHAR KTTS_SAMPLE_EN[] = L"speed test for English speaking.";

	void CConfigDlg::OnTtsChPreview()
	{
		CIsSvrProxy::GetInstance()->TtsSpeakText(KTTS_SAMPLE_CH,-1,true);
	}

	void CConfigDlg::OnTtsEnPreview()
	{
		CIsSvrProxy::GetInstance()->TtsSpeakText(KTTS_SAMPLE_EN,-1,false);
	}

	void CConfigDlg::OnPyBlurClick(EventArgs * e)
	{
		SCheckBox *pCheck = sobj_cast<SCheckBox>(e->Sender());
		BOOL bCheck = pCheck->IsChecked();
		CIsSvrProxy::GetSvrCore()->BlurEnable(bCheck);
	}

	void CConfigDlg::OnJPBlurClick(EventArgs * e)
	{
		SCheckBox *pCheck = sobj_cast<SCheckBox>(e->Sender());
		BOOL bCheck = pCheck->IsChecked();
		CIsSvrProxy::GetSvrCore()->BlurZCS(bCheck);
	}

	void CConfigDlg::OnTtsChTokenChange(EventArgs * e)
	{
		EventCBSelChange *e2 = sobj_cast<EventCBSelChange>(e);
		CIsSvrProxy::GetInstance()->TtsSetVoice(true,e2->nCurSel);
	}

	void CConfigDlg::OnTtsEnTokenChange(EventArgs * e)
	{
		EventCBSelChange *e2 = sobj_cast<EventCBSelChange>(e);
		CIsSvrProxy::GetInstance()->TtsSetVoice(false,e2->nCurSel);
	}

	void CConfigDlg::OnReNotify(EventArgs * e)
	{
		EventRENotify *e2 = sobj_cast<EventRENotify>(e);
		if (e2->iNotify == EN_CHANGE)
		{
			SRichEdit *pEdit = sobj_cast<SRichEdit>(e->Sender());
			SStringT str = pEdit->GetWindowText();
			switch (e2->idFrom)
			{
			case R.id.edit_sent_record_max:
				{
					int nSentMax = _ttoi(str);
					CIsSvrProxy::GetSvrCore()->SetSentRecordMax(nSentMax);
				}
				break;
			}
		}
	}

	void CConfigDlg::OnSpinValue2String(EventArgs * e)
	{
		EventSpinValue2String *e2 = sobj_cast<EventSpinValue2String>(e);
		if(e2->bInit) return;
		switch (e2->idFrom)
		{
		case R.id.spin_predict_phrase_maxlength:
			{
				int nPredictLength = e2->nValue;
				CIsSvrProxy::GetSvrCore()->SetMaxPhrasePreictLength(nPredictLength);
			}
			break;
		case R.id.spin_phrase_ast_deepness_max:
			{
				int nDeepness =  e2->nValue;
				CIsSvrProxy::GetSvrCore()->SetMaxPhraseAstDeepness(nDeepness);
			}
			break;
		case R.id.spin_delay_time:
			g_SettingsG->nDelayTime =  e2->nValue;
			break;
		case R.id.spin_cand_num:
			g_SettingsG->nMaxCands = e2->nValue;
			break;
		}
	}

	void CConfigDlg::OnInstallSysPhraseLib()
	{
		CFileDialogEx dlg(TRUE, _T("spl"), 0, 6, _T("���̴ʿ��ļ�(*.spl)\0*.spl\0All files (*.*)\0*.*\0\0"));
		if (dlg.DoModal() == IDOK)
		{
			::SetCursor(SApplication::getSingleton().LoadCursor(_T("wait")));
			BOOL bRet = CIsSvrProxy::GetSvrCore()->InstallPlt(dlg.m_szFileName);
			::SetCursor(SApplication::getSingleton().LoadCursor(_T("arrow")));
			if (bRet)
			{
				InitPhraseLibListview();
				SMessageBox(m_hWnd, _T("��װ�ɹ�"), _T("��ʾ"), MB_OK);
			}
			else
			{
				SMessageBox(m_hWnd,_T("��װʧ��"),_T("��ʾ"),MB_OK|MB_ICONSTOP);
			}
		}
	}

	void CConfigDlg::OnAddBlur()
	{
		CAddBlurDlg addBlurDlg;
		if (addBlurDlg.DoModal() == IDOK)
		{
			
			if (-1 != CIsSvrProxy::GetSvrCore()->BlurAdd(addBlurDlg.m_strFrom, addBlurDlg.m_strTo))
			{
				SListView *pLvBlur = FindChildByID2<SListView>(R.id.lv_blur);
				InitPinyinBlurListView(pLvBlur);
			}
		}
	}

	void CConfigDlg::OnDelBlur()
	{
		SListView *pLvBlur = FindChildByID2<SListView>(R.id.lv_blur);
		int iSel = pLvBlur->GetSel();
		if (iSel != -1)
		{
			CBlurListAdapter *pAdapter = (CBlurListAdapter*)pLvBlur->GetAdapter();
			char szFrom[7], szTo[7];
			if (pAdapter->getBlur(iSel, szFrom, szTo))
			{
				if (-1 != CIsSvrProxy::GetSvrCore()->BlurDel(szFrom, szTo))
				{
					InitPinyinBlurListView(pLvBlur);
				}
			}
		}
	}


	int CConfigDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		int nRet = __super::OnCreate(lpCreateStruct);
		if (nRet != 0) return nRet;	
		InitPages();
		return 0;
	}


	void CConfigDlg::OnUpdateNow()
	{
		m_pObserver->OnUpdateNow();
		DestroyWindow();
	}

	void CConfigDlg::OnAutoUpdateClick()
	{
		SWindow *pWnd = FindChildByID(R.id.chk_auto_update);
		if (!pWnd->IsChecked())
		{
			m_pObserver->OnUpdateIntervalChanged(0);
		}
		else
		{
			SComboBox * pCbxInterval = FindChildByID2<SComboBox>(R.id.cbx_update_interval);
			int iSel = pCbxInterval->GetCurSel();
			if (iSel == -1)
			{
				pCbxInterval->SetCurSel(0);
			}
			else
			{
				int interval[3] = { 7,30,90 };
				m_pObserver->OnUpdateIntervalChanged(interval[iSel % 3]);
			}
		}
	}

	void CConfigDlg::OnCbUpdateIntervalSelChange(EventArgs * e)
	{
		EventCBSelChange *e2 = sobj_cast<EventCBSelChange>(e);
		SASSERT(e2);
		int iSel = e2->nCurSel;
		if (iSel >= 0 && iSel < 3)
		{
			int intervals[3] = { 7,30,90 };
			m_pObserver->OnUpdateIntervalChanged(intervals[iSel]);
			FindChildByID(R.id.chk_auto_update)->SetCheck(TRUE);
		}
	}

	void CConfigDlg::OnFinalMessage(HWND hWnd)
	{
		__super::OnFinalMessage(hWnd);
		g_SettingsG->SetModified(true);
		g_SettingsUI->SetModified(true);
		delete this;
	}

	void CConfigDlg::OnChangeFont()
	{
		LOGFONT lf={0};
		if(!g_SettingsG->strFontDesc.IsEmpty())
		{
			FontInfo fi = SFontPool::FontInfoFromString(g_SettingsG->strFontDesc,GETUIDEF->GetDefFontInfo());
			_tcscpy(lf.lfFaceName,S_CW2T(fi.strFaceName));
			lf.lfWeight = fi.style.attr.byWeight*4;
			if(lf.lfWeight == 0)
				lf.lfWeight = fi.style.attr.fBold?FW_BOLD:FW_NORMAL;
			lf.lfCharSet = fi.style.attr.byCharset;
			lf.lfHeight = -(short)fi.style.attr.nSize;
			lf.lfItalic = fi.style.attr.fItalic;
			lf.lfUnderline = fi.style.attr.fUnderline;
			lf.lfStrikeOut = fi.style.attr.fStrike;
		}else
		{
			IFontPtr font = GETUIDEF->GetFont(L"",100);
			memcpy(&lf,font->LogFont(),sizeof(lf));
		}
		CFontDialog fontDlg(&lf, CF_SCREENFONTS|CF_NOVERTFONTS);
		if(fontDlg.DoModal()== IDOK)
		{
			lf = fontDlg.m_lf;
			FontInfo fi;
			fi.strFaceName = lf.lfFaceName;
			fi.style.attr.nSize = abs(lf.lfHeight);
			fi.style.attr.byWeight = lf.lfWeight/4;
			fi.style.attr.byCharset = lf.lfCharSet;
			fi.style.attr.fItalic = lf.lfItalic;
			fi.style.attr.fUnderline = lf.lfUnderline;
			fi.style.attr.fStrike = lf.lfStrikeOut;
			if(lf.lfWeight == FW_BOLD)
			{
				fi.style.attr.fBold = 1;
				fi.style.attr.byWeight = 0;
			}
			g_SettingsG->strFontDesc = SFontPool::FontInfoToString(fi);
			GETUIDEF->SetDefFontInfo(g_SettingsG->strFontDesc);
			FindAndSetText(R.id.edit_font,g_SettingsG->strFontDesc);
		}
	}

	void CConfigDlg::OnSkinFont()
	{
		SStringW fi = SUiDef::getSingletonPtr()->GetUiDef()->GetDefFontInfo();
		FindAndSetText(R.id.edit_font,_T("<Ƥ��Ĭ��>"));
		GETUIDEF->SetDefFontInfo(fi);
		g_SettingsG->strFontDesc.Empty();
	}

	CConfigDlg* CConfigDlg::GetInstance()
	{
		return _instance;
	}

	void CConfigDlg::OnHelp()
	{
		ShellExecute(NULL, _T("open"), _T("https://soime.cn/help"), NULL, NULL, SW_SHOWNORMAL);
	}

	void CConfigDlg::OnPickBackupDir()
	{
		BROWSEINFO bi; 
		ZeroMemory(&bi,sizeof(BROWSEINFO)); 
		bi.hwndOwner = m_hWnd; 
		TCHAR szBuf[MAX_PATH];
		_tcscpy(szBuf,g_SettingsG->szBackupDir);
		bi.pszDisplayName = szBuf; 
		bi.lpszTitle = _T("ѡ�����ݱ����ļ���:"); 
		bi.ulFlags = BIF_RETURNFSANCESTORS|BIF_DONTGOBELOWDOMAIN|BIF_BROWSEFORCOMPUTER|BIF_USENEWUI; 
		LPITEMIDLIST idl = SHBrowseForFolder(&bi); 
		if (NULL == idl) 
		{ 
			return; 
		}
		SHGetPathFromIDList(idl,szBuf);
		if(GetFileAttributes(szBuf)!=INVALID_FILE_ATTRIBUTES)
		{
			if(_tcsicmp(szBuf,CDataCenter::getSingleton().GetDataPath().c_str())==0)
			{
				SMessageBox(m_hWnd,_T("���ܱ��ݵ��������뷨��װĿ¼"),_T("����"),MB_OK|MB_ICONSTOP);
			}else
			{
				_tcscpy(g_SettingsG->szBackupDir,szBuf);
				FindAndSetText(R.id.edit_backup,g_SettingsG->szBackupDir);
				g_SettingsG->SetModified(true);
				g_SettingsG->Save(CDataCenter::getSingleton().GetDataPath());
			}
		}else
		{
			SMessageBox(m_hWnd,_T("��Ч·��"),_T("����"),MB_OK|MB_ICONSTOP);
		}
	}

	void CConfigDlg::OnBackup()
	{
		SStringT strFrom = CDataCenter::getSingleton().GetDataPath();
		SStringT strTo = g_SettingsG->szBackupDir;
		int nRet = CIsSvrProxy::BackupDir(strFrom,strTo);
		if(nRet==0)
		{
			SMessageBox(m_hWnd,_T("���ݱ��ݳɹ�"),_T("��ʾ"),MB_OK|MB_ICONINFORMATION);
		}else
		{
			SStringT strMsg = SStringT().Format(_T("���ݱ���ʧ��,������:%d"),nRet);
			SMessageBox(m_hWnd,_T("���ݱ���ʧ��"),_T("��ʾ"),MB_OK|MB_ICONSTOP);
		}
	}

	void CConfigDlg::OnRestore()
	{
		if(!CIsSvrProxy::IsBackupDirValid(g_SettingsG->szBackupDir))
		{
			SMessageBox(m_hWnd,_T("�ޱ��ݻ��߱�����!"),_T("��ʾ"),MB_OK|MB_ICONSTOP);
			return;
		}
		if(SMessageBox(m_hWnd,_T("ȷ��Ҫ�ӱ���Ŀ¼�ָ������𣿷��������Զ�����!"),_T("Σ�ղ���!"),MB_OKCANCEL|MB_ICONQUESTION)==IDOK)
		{
			CIsSvrProxy::GetInstance()->PostMessage(WM_QUIT,CODE_RESTORE);
		}
	}

	void CConfigDlg::OnCloseBackup()
	{
		CIsSvrProxy::GetInstance()->CloseBackup();
		FindAndSetText(R.id.edit_backup,g_SettingsG->szBackupDir);
	}

	void CConfigDlg::OnSearchEngineChange(EventArgs *e)
	{
		EventCBSelChange *e2=sobj_cast<EventCBSelChange>(e);
		CSearchCfg::getSingleton().SetSel(e2->nCurSel);
	}

	void CConfigDlg::OnBlendForSpell()
	{
		g_SettingsG->bBlendSpell = FindChildByID(R.id.chk_blend_for_spell)->IsChecked();
	}

	void CConfigDlg::OnBlendForUD()
	{
		g_SettingsG->bBlendUD = FindChildByID(R.id.chk_blend_for_userdef)->IsChecked();
	}

	void CConfigDlg::OnQuitUMode()
	{
		g_SettingsG->bBackQuitUMode=  FindChildByID(R.id.chk_autoQuitUMode)->IsChecked();
	}

	void CConfigDlg::OnQuitCAP()
	{
		g_SettingsG->bQuitEnCancelCAP= FindChildByID(R.id.chk_autoQuitCAP)->IsChecked();
	}

	void CConfigDlg::OnInitMode(int id)
	{
		g_SettingsG->bInitEnglish = id==R.id.radio_init_en;
	}

	void CConfigDlg::OnClickInlineMode(int id)
	{
		g_SettingsUI->enumInlineMode =(CSettingsUI::EInlineMode)(id - 220);
	}
}
