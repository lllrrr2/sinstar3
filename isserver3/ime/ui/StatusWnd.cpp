#include "StdAfx.h"
#include "StatusWnd.h"
#include <ShellAPI.h>

#include "../Utils.h"
#include "../InputState.h"
#include "../../../include/FileHelper.h"
#include "../../ui/TextEditorDlg.h"
#include "../../worker.h"
#include "../../IsSvrProxy.h"
namespace SOUI
{
	class CDonateDlg: public SHostWnd, public SDpiHandler<CDonateDlg>
	{
	public:
		CDonateDlg():SHostWnd(UIRES.LAYOUT.dlg_donate){}

		virtual void OnFinalMessage(HWND hWnd){
			__super::OnFinalMessage(hWnd);
			delete this;
		}

		void OnClose()
		{
			DestroyWindow();
		}
		EVENT_MAP_BEGIN()
			EVENT_ID_COMMAND(IDCANCEL,OnClose)
		EVENT_MAP_END()

	protected:
		BEGIN_MSG_MAP_EX(CDonateDlg)
			CHAIN_MSG_MAP(SDpiHandler<CDonateDlg>)
			CHAIN_MSG_MAP(SHostWnd)
		END_MSG_MAP()
	};

	static int PopupMenuEndID(int nStart)
	{
		if (nStart % 100 == 0) return nStart + 100;
		else return (nStart + 99) / 100 * 100;
	}

	CStatusWnd::CStatusWnd(SEventSet *pEvtSets, IInputListener *pListener)
		:CImeWnd(pEvtSets,UIRES.LAYOUT.wnd_status_bar)
		, m_pInputListener(pListener)
		,m_anchorMode(AMH_NULL|AMV_NULL)
	{
	}

	CStatusWnd::~CStatusWnd(void)
	{
	}


	HWND CStatusWnd::Create()
	{
		HWND hWnd = __super::Create();
		if(hWnd)
		{
			CPoint pt = g_SettingsG->ptStatus;
			if(pt.x==-1 || pt.y==-1)
			{
				m_anchorMode = AMH_RIGHT|AMV_BOTTOM;
			}else
			{
				SetWindowPos(HWND_TOPMOST,pt.x,pt.y,0,0,SWP_NOACTIVATE|SWP_NOSIZE);
			}
			UpdateUI();
		}
		return hWnd;
	}

	int CStatusWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		return OnRecreateUI(lpCreateStruct);
	}

	void CStatusWnd::OnReposition(CPoint pt)
	{
		g_SettingsG->ptStatus = pt;
		g_SettingsG->SetModified(true);
		UpdateAnchorMode();
	}

	int CStatusWnd::OnRecreateUI(LPCREATESTRUCT lpCreateStruct)
	{
		int nRet = __super::OnRecreateUI(lpCreateStruct);
		if (nRet != 0) return nRet;

		UpdateUI();
		return 0;
	}

	void CStatusWnd::UpdateUI()
	{
		UpdateToggleStatus(BTN_ALL, TRUE);
		UpdateCompInfo();
	}

	void CStatusWnd::UpdateAnchorMode()
	{
		CRect rcWnd;
		GetNative()->GetWindowRect(&rcWnd);
		CRect rcWorkArea;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);

		m_anchorMode = AMH_NULL|AMV_NULL;
		if(rcWnd.left == rcWorkArea.left)
			m_anchorMode |= AMH_LEFT;
		else if(rcWnd.right == rcWorkArea.right)
			m_anchorMode |= AMH_RIGHT;
		if(rcWnd.top == rcWorkArea.top)
			m_anchorMode |= AMV_TOP;
		else if(rcWnd.bottom == rcWorkArea.bottom)
			m_anchorMode |= AMV_BOTTOM;
	}

	BOOL CStatusWnd::onRootResize(EventArgs *e)
	{
		EventSwndSize *e2 = sobj_cast<EventSwndSize>(e);
		if(m_bResizing) return true;
		SHostWnd::onRootResize(e);

		CPoint pt = g_SettingsG->ptStatus;
		CRect rcTo=CRect(pt,e2->szWnd);

		CRect rcWorkArea;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
		if(m_anchorMode & AMH_LEFT)
			rcTo.MoveToX(rcWorkArea.left);
		else if(m_anchorMode & AMH_RIGHT)
			rcTo.MoveToX(rcWorkArea.right-rcTo.Width());
		if(m_anchorMode & AMV_TOP)
			rcTo.MoveToY(rcWorkArea.top);
		else if(m_anchorMode & AMV_BOTTOM)
			rcTo.MoveToY(rcWorkArea.bottom-rcTo.Height());

		if(rcTo.left <rcWorkArea.left)
			rcWorkArea.MoveToX(rcWorkArea.left);
		if(rcTo.top < rcWorkArea.top)
			rcWorkArea.MoveToY(rcWorkArea.top);
		if(rcTo.right>rcWorkArea.right)
			rcTo.MoveToX(rcWorkArea.right-e2->szWnd.cx);
		if(rcTo.bottom>rcWorkArea.bottom)
			rcTo.MoveToY(rcWorkArea.bottom-e2->szWnd.cy);

		SetWindowPos(NULL,rcTo.left,rcTo.top,0,0,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE);
		UpdateAnchorMode();
		g_SettingsG->ptStatus = rcTo.TopLeft();
		g_SettingsG->SetModified(true);
		return true;
	}

	void CStatusWnd::OnRButtonUp(UINT nFlags,CPoint pt)
	{
		const MSG * pMsg = GetCurrentMessage();
		SHostWnd::OnMouseEvent(pMsg->message,pMsg->wParam,pMsg->lParam);

		OnMenuClick();
	}

	void CStatusWnd::OnInitMenuPopup(HMENU menuPopup, UINT nIndex, BOOL bSysMenu)
	{
		SMenu smenuPopup(menuPopup);
		DWORD dwCtxID = ::GetMenuContextHelpId(menuPopup);
		switch (dwCtxID)
		{
		case 1:
			{//main menu
				smenuPopup.CheckMenuItem(R.id.switch_follow_caret, MF_BYCOMMAND | g_SettingsUI->bMouseFollow ? MF_CHECKED : 0);
				smenuPopup.CheckMenuItem(R.id.switch_hide_statusbar, MF_BYCOMMAND | g_SettingsUI->bHideStatus ? MF_CHECKED : 0);
				smenuPopup.CheckMenuItem(R.id.switch_input_big5, MF_BYCOMMAND | g_SettingsUI->bInputBig5 ? MF_CHECKED : 0);
				smenuPopup.CheckMenuItem(R.id.switch_read_input, MF_BYCOMMAND | g_SettingsUI->bSound ? MF_CHECKED : 0);
				smenuPopup.CheckMenuItem(R.id.switch_word_input, MF_BYCOMMAND | g_SettingsUI->bEnglish ? MF_CHECKED : 0);
				smenuPopup.CheckMenuItem(R.id.switch_filter_gbk, MF_BYCOMMAND | g_SettingsUI->bFilterGbk ? MF_CHECKED : 0);
				smenuPopup.CheckMenuItem(R.id.switch_char_mode,MF_BYCOMMAND | g_SettingsUI->bCharMode ? MF_CHECKED : 0);
				smenuPopup.CheckMenuItem(R.id.svr_showicon, MF_BYCOMMAND | g_SettingsG->bShowTray? MF_CHECKED : 0);
				break;
			}
		case 10://sent menu
			{
				smenuPopup.CheckMenuItem(R.id.sent_record, MF_BYCOMMAND | g_SettingsUI->bRecord ? MF_CHECKED : 0);
				smenuPopup.CheckMenuItem(R.id.sent_associate, MF_BYCOMMAND | g_SettingsUI->bSentAssocite ? MF_CHECKED : 0);
				SStringT strItem;
				smenuPopup.GetMenuString(R.id.sent_record_clear_all,MF_BYCOMMAND,&strItem);
				strItem+=SStringT().Format(_T("[%d]"),CIsSvrProxy::GetSvrCore()->GetTotalSentCount());
				smenuPopup.ModifyMenuString(R.id.sent_record_clear_all,MF_BYCOMMAND,strItem);
				smenuPopup.GetMenuString(R.id.sent_record_clear_today,MF_BYCOMMAND,&strItem);
				strItem+=SStringT().Format(_T("[%d]"),CIsSvrProxy::GetSvrCore()->GetTodaySentCount());
				smenuPopup.ModifyMenuString(R.id.sent_record_clear_today,MF_BYCOMMAND,strItem);
				break;
			}
		case 2:
			{//skin
				SStringT strCurSkin = g_SettingsG->strSkin;
				if (strCurSkin.IsEmpty())
				{
					smenuPopup.CheckMenuItem(R.id.skin_def, MF_BYCOMMAND|MF_CHECKED);
				}
				smenuPopup.CheckMenuItem(R.id.skin_using_vert,MF_BYCOMMAND|(g_SettingsG->bUsingVertLayout?MF_CHECKED:0));
				m_skinManager.InitSkinMenu(menuPopup, CDataCenter::getSingletonPtr()->GetDataPath() + _T("\\skins"), R.id.skin_def, strCurSkin);
				break;
			}
		case 4://comp select
			{
				const SArray<CNameTypePair> &comps = CDataCenter::getSingleton().GetData().UpdateCompList();
				int idStart = R.id.comp_install;
				TCHAR szCurComp[MAX_PATH]={0};
				CIsSvrProxy::GetSvrCore()->GetCurrentComp(szCurComp);
				SStringT strComp=szCurComp;
				strComp+=_T(".cit");
				strComp.MakeLower();
				for (int i = 0; i < (int)comps.GetCount(); i++)
				{
					SStringT strText = SStringT().Format(_T("%s[%s]"),comps[i].strName.c_str(),comps[i].strTitle.c_str());
					UINT flag = MF_STRING;
					if (strComp == comps[i].strTitle) flag |= MF_CHECKED;
					smenuPopup.AppendMenu( flag, idStart + i+1, strText);
				}
				break;
			}
		case 5://blend input
			{
				smenuPopup.CheckMenuItem(R.id.menu_blend_spell,MF_BYCOMMAND | (g_SettingsG->bBlendSpell ? MF_CHECKED : 0));
				smenuPopup.CheckMenuItem(R.id.menu_blend_userdef, MF_BYCOMMAND | (g_SettingsG->bBlendUD ? MF_CHECKED : 0));

				break;
			}
		case 7://tools
			{
				m_toolManager.InitToolMenu(menuPopup, CDataCenter::getSingletonPtr()->GetDataPath() + _T("\\tools"), R.id.menu_tool_base);
				break;
			}

		case 8://flm menu
			{
				const SArray<CNameTypePair> &flmDicts = CDataCenter::getSingleton().GetData().UpdateFlmList();
				int idStart = R.id.menu_flm_close;
				TCHAR szCurComp[MAX_PATH]={0};
				CIsSvrProxy::GetSvrCore()->GetCurrentFlmDict(szCurComp);
				SStringT strComp=szCurComp;
				strComp+=_T(".flm");
				strComp.MakeLower();
				for (int i = 0; i < (int)flmDicts.GetCount(); i++)
				{
					SStringT strText = SStringT().Format(_T("%s[%s]"),flmDicts[i].strName.c_str(),flmDicts[i].strTitle.c_str());
					UINT flag = MF_STRING;
					if (strComp == flmDicts[i].strTitle) flag |= MF_CHECKED;
					smenuPopup.AppendMenu( flag, idStart + i+1, strText);
				}
				break;
			}
		default:
			{
				SStringT strSkinDir = m_skinManager.SkinPathFromCtxID((int)dwCtxID);
				
				if(!strSkinDir.IsEmpty())
				{//sub skin menu
					SStringT strCurSkin = g_SettingsG->strSkin;
					m_skinManager.InitSkinMenu(menuPopup, strSkinDir, dwCtxID, strCurSkin);
				}
			}
			break;
		}
		::SetMenuContextHelpId(menuPopup,dwCtxID+1000000);
	}


	void CStatusWnd::OnBtnExtend()
	{
		g_SettingsUI->bFullStatus = TRUE;
		g_SettingsUI->SetModified(true);
		SWindow *pStatus=FindChildByID(R.id.status_extend);
		if(pStatus) 
		{
			pStatus->SetVisible(TRUE, TRUE);
			GetRoot()->RequestRelayout();
		}
		m_pInputListener->OnCommand(CMD_SYNCUI, BTN_STATUSMODE);
	}

	void CStatusWnd::OnBtnShrink()
	{
		g_SettingsUI->bFullStatus = FALSE;
		g_SettingsUI->SetModified(true);
		SWindow *pStatus=FindChildByID(R.id.status_shrink);
		if(pStatus) 
		{
			pStatus->SetVisible(TRUE,TRUE);
			GetRoot()->UpdateLayout();
		}
		m_pInputListener->OnCommand(CMD_SYNCUI, BTN_STATUSMODE);
	}

	void CStatusWnd::UpdateCompInfo2(SWindow *pParent)
	{
		SWindow *pText = pParent->FindChildByID(R.id.txt_comp);
		SFlagView * pFlagView = pParent->FindChildByID2<SFlagView>(R.id.img_logo);
		if(!m_pInputListener->IsInputEnable())
		{
			if (pText) pText->SetWindowText(_T("Ӣ��"));
		}
		else if (m_pInputListener->GetInputContext()->compMode == IM_SHAPECODE)
		{
			if (pText) pText->SetWindowText(CDataCenter::getSingletonPtr()->GetData().m_compInfo.strCompName);
			if (pFlagView)
			{
				pFlagView->ShowSpellFlag(FALSE);
				BYTE *pBuf=NULL;
				DWORD dwIconLen = CIsSvrProxy::GetSvrCore()->GetCompIconData(&pBuf);
				pFlagView->SetImeFlagData(pBuf,dwIconLen);
			}
		}
		else
		{
			if (pFlagView) pFlagView->ShowSpellFlag(TRUE);
			if (pText)
			{
				if (g_SettingsG->compMode == IM_SHAPECODE)
					pText->SetWindowText(_T("��ʱƴ��"));
				else
					pText->SetWindowText(_T("����ƴ��"));
			}
		}
	}

	void CStatusWnd::ShowServerExit()
	{
		{
			SWindow *pStatus = FindChildByID(R.id.status_shrink);
			if(pStatus)
			{
				SWindow *pText = pStatus->FindChildByID(R.id.txt_comp);
				if (pText) pText->SetWindowText(_T("�����˳�"));
			}
		}
		{
			SWindow *pStatus = FindChildByID(R.id.status_extend);
			if(pStatus)
			{
				SWindow *pText = pStatus->FindChildByID(R.id.txt_comp);
				if(pText) pText->SetWindowText(_T("�����˳�"));
			}
		}
	}
	void CStatusWnd::UpdateCompInfo()
	{
		{
			SWindow *pStatus = FindChildByID(R.id.status_shrink);
			SASSERT(pStatus);
			UpdateCompInfo2(pStatus);
		}
		{
			SWindow *pStatus = FindChildByID(R.id.status_extend);
			if(pStatus)
				UpdateCompInfo2(pStatus);
		}
	}


	void CStatusWnd::UpdateMode()
	{
		if (g_SettingsUI->bFullStatus)
		{
			SWindow *pStatus = FindChildByID(R.id.status_extend);
			if(pStatus)
				pStatus->SetVisible(TRUE, TRUE);
			else
				FindChildByID(R.id.status_shrink)->SetVisible(TRUE, TRUE);
		}
		else
		{
			SWindow *pStatus = FindChildByID(R.id.status_shrink);
			if(pStatus)
				pStatus->SetVisible(TRUE, TRUE);
			else
				FindChildByID(R.id.status_extend)->SetVisible(TRUE, TRUE);
		}
	}


	bool CStatusWnd::SwitchToggle(int nID, BOOL  bToggle)
	{
		bool bRet=false;
		SWindow *pShrink = FindChildByID(R.id.status_shrink);
		SWindow *pExtend = FindChildByID(R.id.status_extend);
		if(pShrink)
		{
			SToggle *pToggle = pShrink->FindChildByID2<SToggle>(nID);
			if(pToggle) 
			{
				pToggle->SetToggle(bToggle);
				if(pToggle->IsVisible(TRUE))
					bRet = true;
			}
		}
		if(pExtend)
		{
			SToggle *pToggle = pExtend->FindChildByID2<SToggle>(nID);
			if(pToggle) 
			{
				pToggle->SetToggle(bToggle);
				if(pToggle->IsVisible(TRUE))
					bRet = true;
			}
		}
		return bRet;
	}

	void CStatusWnd::UpdateToggleStatus(DWORD flags, BOOL bInit)
	{
		if (flags & BTN_STATUSMODE)
		{
			UpdateMode();
		}
		if(flags & BTN_CAPITAL)
		{
			UpdateCaptialMode();
		}

		if(flags & BTN_CHARMODE){
			bool bUpdated = SwitchToggle(R.id.btn_charmode,g_SettingsUI->bCharMode);
			if (m_pInputListener && !bInit && !bUpdated)
			{
				TIPINFO ti(_T("���ģʽ�ı�"));
				ti.strTip.Format(_T("��ǰ���:%s"), g_SettingsUI->bCharMode ? _T("���ı��") : _T("Ӣ�ı��"));
				m_pInputListener->OnCommand(CMD_SHOWTIP,(LPARAM)&ti);
			}
		}
		if(flags & BTN_SOUND){
			bool bUpdated = SwitchToggle(R.id.btn_sound,!g_SettingsUI->bSound);
			if (m_pInputListener && !bInit && !bUpdated)
			{
				TIPINFO ti(_T("����У�Ըı�"));
				ti.strTip.Format(_T("��ǰ����У��:%s"), g_SettingsUI->bSound ? _T("��") : _T("�ر�"));
				m_pInputListener->OnCommand(CMD_SHOWTIP, (LPARAM)&ti);
			}
		}
		if (flags & BTN_RECORD) {
			bool bUpdated = SwitchToggle(R.id.btn_record,!g_SettingsUI->bRecord);
			if (m_pInputListener && !bInit && !bUpdated)
			{
				TIPINFO ti(_T("��������ı�"));
				ti.strTip.Format(_T("��ǰ����״̬:%s"), g_SettingsUI->bRecord ? _T("����") : _T("�ر�"));
				m_pInputListener->OnCommand(CMD_SHOWTIP, (LPARAM)&ti);
			}

		}
		if (flags & BTN_ENGLISHMODE)
		{
			bool bUpdated = SwitchToggle(R.id.btn_english,!g_SettingsUI->bEnglish);
			if (m_pInputListener && !bInit && !bUpdated)
			{
				TIPINFO ti(_T("���ʲ�ȫ�ı�"));
				ti.strTip.Format(_T("��ǰ���ʲ�ȫ״̬:%s"), g_SettingsUI->bEnglish ? _T("����") : _T("�ر�"));
				m_pInputListener->OnCommand(CMD_SHOWTIP, (LPARAM)&ti);
			}
		}
		if (flags & BTN_FILTERGBK)
		{
			bool bUpdated = SwitchToggle(R.id.btn_filter_gbk,!g_SettingsUI->bFilterGbk);
			if (m_pInputListener && !bInit && !bUpdated)
			{
				TIPINFO ti(_T("GBK���ز��Ըı�"));
				ti.strTip.Format(_T("����GBK����:%s"), g_SettingsUI->bFilterGbk ? _T("����") : _T("�ر�"));
				m_pInputListener->OnCommand(CMD_SHOWTIP, (LPARAM)&ti);
			}
		}
	}


	void CStatusWnd::OnSvrNotify(EventArgs *e)
	{
		if(!IsWindow())
			return;

		EventSvrNotify *e2 = sobj_cast<EventSvrNotify>(e);
		if(e2->wp == NT_COMPINFO)
		{
			UpdateCompInfo();
		}
	}

	void CStatusWnd::OnSwitchCharMode(EventArgs *e)
	{
		SToggle * toggle = sobj_cast<SToggle>(e->Sender());
		if(toggle)
		{
			g_SettingsUI->bCharMode = toggle->GetToggle();
			g_SettingsUI->SetModified(true);
			m_pInputListener->OnCommand(CMD_SYNCUI, BTN_CHARMODE);
		}
	}

	void CStatusWnd::OnSwitchRecord(EventArgs *e)
	{
		SToggle * toggle = sobj_cast<SToggle>(e->Sender());
		if(toggle)
		{
			g_SettingsUI->bRecord = !toggle->GetToggle();
			g_SettingsUI->SetModified(true);
			m_pInputListener->OnCommand(CMD_SYNCUI, BTN_RECORD);
		}
	}

	void CStatusWnd::OnSwitchSound(EventArgs *e)
	{
		SToggle * toggle = sobj_cast<SToggle>(e->Sender());
		if(toggle)
		{
			g_SettingsUI->bSound = !toggle->GetToggle();
			g_SettingsUI->SetModified(true);
			m_pInputListener->OnCommand(CMD_SYNCUI, BTN_SOUND);
		}

	}

	void CStatusWnd::OnSwitchEnglish(EventArgs * e)
	{
		SToggle * toggle = sobj_cast<SToggle>(e->Sender());
		if (toggle)
		{
			g_SettingsUI->bEnglish = !toggle->GetToggle();
			g_SettingsUI->SetModified(true);
			m_pInputListener->OnCommand(CMD_SYNCUI, BTN_ENGLISHMODE);
		}
	}

	void CStatusWnd::OnSwitchFilterGbk(EventArgs * e)
	{
		SToggle * toggle = sobj_cast<SToggle>(e->Sender());
		if (toggle)
		{
			g_SettingsUI->bFilterGbk = !toggle->GetToggle();
			g_SettingsUI->SetModified(true);
			m_pInputListener->OnCommand(CMD_SYNCUI, BTN_FILTERGBK);
		}
	}

	
	void CStatusWnd::OnUpdateBtnTooltip(EventArgs *e)
	{
		EventSwndUpdateTooltip *e2 = sobj_cast<EventSwndUpdateTooltip>(e);
		SASSERT(e2);
		SStringT strAccel;
		SStringT strToolTip;
		strToolTip.Copy(e2->strToolTip);
		switch (e2->idFrom)
		{
		case R.id.img_logo:
			{
				SStringT strComp = CDataCenter::getSingletonPtr()->GetData().m_compInfo.strCompName;
				e2->bUpdated = TRUE;
				strToolTip = SStringT().Format(_T("�л�[ƴ��<=>%s]"), strComp.c_str());
			}
			break;
		case R.id.btn_charmode:
			e2->bUpdated = TRUE;
			strAccel = SAccelerator::FormatAccelKey(g_SettingsG->dwHotkeys[HKI_CharMode]);
			strToolTip = SStringT().Format(_T("���ģʽ:%s"), g_SettingsUI->bCharMode? _T("����"):_T("Ӣ��"));
			break;
		case R.id.btn_make_phrase:
			e2->bUpdated = TRUE;
			strAccel = SAccelerator::FormatAccelKey(g_SettingsG->dwHotkeys[HKI_MakePhrase]);
			strToolTip = _T("���������");
			break;
		case R.id.btn_record:
			e2->bUpdated = TRUE;
			strAccel = SAccelerator::FormatAccelKey(g_SettingsG->dwHotkeys[HKI_Record]);
			strToolTip = SStringT().Format(_T("��¼������ʷ:%s"), g_SettingsUI->bRecord ? _T("����") : _T("����"));
			break;
		case R.id.btn_sound:
			e2->bUpdated = TRUE;
			strAccel = SAccelerator::FormatAccelKey(g_SettingsG->dwHotkeys[HKI_TTS]);
			strToolTip = SStringT().Format(_T("����У��:%s"), g_SettingsUI->bSound ? _T("����") : _T("����"));
			break;
		case R.id.btn_english:
			e2->bUpdated = TRUE;
			strAccel = SAccelerator::FormatAccelKey(g_SettingsG->dwHotkeys[HKI_EnSwitch]);
			strToolTip = SStringT().Format(_T("���ʲ�ȫ:%s"), g_SettingsUI->bEnglish ? _T("����") : _T("����"));
			break;
		case R.id.btn_query:
			e2->bUpdated = TRUE;
			strAccel = SAccelerator::FormatAccelKey(g_SettingsG->dwHotkeys[HKI_Query]);
			strToolTip = _T("���뷴��");
			break;
		case R.id.btn_filter_gbk:
			e2->bUpdated = TRUE;
			strAccel = SAccelerator::FormatAccelKey(g_SettingsG->dwHotkeys[HKI_FilterGbk]);
			strToolTip = SStringT().Format(_T("����GBK����:%s"), g_SettingsUI->bFilterGbk ? _T("����") : _T("����"));
			break;
		case R.id.btn_menu:
			{
				e2->bUpdated = TRUE;
				strToolTip = _T("���뷨�˵�");
			}
			break;
		case R.id.btn_help:
			{
				e2->bUpdated = TRUE;
				strToolTip = _T("�򿪰���");
			}
			break;
		case R.id.btn_status_extend:
			if (strToolTip.IsEmpty())
			{
				e2->bUpdated = TRUE;
				strToolTip = _T("չ��״̬��");
			}
			break;
		case R.id.btn_status_shrink:
			{
				e2->bUpdated = TRUE;
				strToolTip = _T("����״̬��");
			}
			break;
		}
		if (e2->bUpdated && !strAccel.IsEmpty())
		{
			strToolTip += _T(",");
			strToolTip += strAccel;
		}
		e2->strToolTip->Copy(&strToolTip);
	}
	void CStatusWnd::OnBtnMakePhrase()
	{
		m_pInputListener->OnCommand(CMD_HOTKEY_MAKEPHRASE,0);
	}

	void CStatusWnd::OnLogoClick()
	{
		m_pInputListener->OnCommand(CMD_HOTKEY_INPUTMODE, 0);
	}

	void CStatusWnd::OnMenuClick()
	{
		if (!m_hOwner)
			return;
		CPoint pt;
		GetCursorPos(&pt);
		SMenu menu;
		BOOL bLoad = menu.LoadMenu(UIRES.smenu.context_status);
		m_skinManager.ClearMap();
		SLOGI()<<"before trackpopupmenu";

		DWORD dwThreadID = GetWindowThreadProcessId(m_hOwner,NULL);
		DWORD dwCurID = GetCurrentThreadId();
		AttachThreadInput(dwCurID,dwThreadID,TRUE);
		int nScale = SDpiHelper::getScale(m_hWnd);
		m_skinPreview.CreateEx(m_hWnd,0,WS_EX_TOPMOST|WS_EX_TOOLWINDOW,0,0,0,0);
		int nRet = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD, pt.x, pt.y, m_hWnd,NULL,nScale);
		m_skinPreview.DestroyWindow();
		AttachThreadInput(dwCurID,dwThreadID,FALSE);
		SLOGI()<<"after trackpopupmenu" << " nRet:" << nRet;
		if (nRet == R.id.config)
		{//system config
			m_pInputListener->OnCommand(CMD_OPENCONFIG, 0);
		}else if(nRet == R.id.skin_mgr)
		{
			m_pInputListener->OnCommand(CMD_OPENSKINDIR, 0);
		}
		else if (nRet == R.id.comp_install)
		{//install comp
			CFileDialogEx openDlg(TRUE, _T("cit"), 0, 6, _T("�������(*.cit)\0*.cit\0All files (*.*)\0*.*\0\0"));
			if (openDlg.DoModal() == IDOK)
			{
				CIsSvrProxy::GetInstance()->InstallCit(openDlg.m_szFileName);
			}
		}
		else if (nRet > R.id.comp_install && nRet < PopupMenuEndID(R.id.comp_install))
		{//comps
			int iComp = nRet - (R.id.comp_install +1);
			const SArray<CNameTypePair> & compList = CDataCenter::getSingleton().GetData().m_compList;
			if (iComp < (int)compList.GetCount())
			{
				SStringT strName = compList[iComp].strTitle;
				strName = strName.Left(strName.GetLength()-4);//remove ".cit"
				CIsSvrProxy::GetSvrCore()->OpenComp(strName);
			}
		}
		else if (nRet == R.id.svr_showicon)
		{//show icon
			g_SettingsG->bShowTray= !g_SettingsG->bShowTray;
			CIsSvrProxy::GetInstance()->ShowTray(!!g_SettingsG->bShowTray);
			g_SettingsG->SetModified(true);
		}
		else if (nRet == R.id.menu_blend_spell)
		{
			g_SettingsG->bBlendSpell = !g_SettingsG->bBlendSpell;
			g_SettingsG->SetModified(true);
		}
		else if (nRet == R.id.menu_blend_userdef)
		{
			g_SettingsG->bBlendUD = !g_SettingsG->bBlendUD;
			g_SettingsG->SetModified(true);
		}
		else if (nRet == R.id.key_map)
		{
			m_pInputListener->OnCommand(CMD_HOTKEY_KEYMAP, 0);
		}
		else if (nRet == R.id.switch_follow_caret)
		{
			m_pInputListener->OnCommand(CMD_FOLLOWCARET, 0);
		}
		else if (nRet == R.id.switch_hide_statusbar)
		{
			m_pInputListener->OnCommand(CMD_HOTKEY_HIDESTATUSBAR, 0);
		}
		else if (nRet == R.id.switch_input_big5)
		{
			g_SettingsUI->bInputBig5 = !g_SettingsUI->bInputBig5;
			g_SettingsUI->SetModified(true);
		}
		else if (nRet == R.id.key_speed)
		{
			m_pInputListener->OnCommand(CMD_KEYSPEED, 0);
		}
		else if (nRet == R.id.help)
		{
			OnHelpClick();
		}
		else if (nRet == R.id.menu_donate)
		{
			CDonateDlg *dlgDonate = new CDonateDlg;
			dlgDonate->CreateEx(m_hWnd,WS_POPUP,WS_EX_TOPMOST,0,0,0,0);
			dlgDonate->SendMessage(WM_INITDIALOG);
			dlgDonate->CenterWindow(GetDesktopWindow());
			dlgDonate->SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		}
		else if (nRet >= R.id.memu_edit_userdef && nRet <=R.id.menu_edit_userdict)
		{
			int types[] = {
				FU_USERDEF,
				FU_SYMBOL,
				FU_USERCMD,
				FU_USERJM,
				FU_USERDICT,
			};
			PostMessage(UM_EDITUSERDATA,types[nRet - R.id.memu_edit_userdef]);
		}
		else if (nRet > R.id.menu_tool_base && nRet < (R.id.menu_tool_base + 99) / 100 * 100)
		{//open tools.
			SStringT strToolPath = m_toolManager.ToolPathFromID(nRet);
			m_pInputListener->OnCommand(CMD_EXECUTETOOL, (LPARAM)&strToolPath);
		}
		else if (nRet == R.id.switch_filter_gbk)
		{
			g_SettingsUI->bFilterGbk = !g_SettingsUI->bFilterGbk;
			g_SettingsUI->SetModified(true);
			m_pInputListener->OnCommand(CMD_SYNCUI, BTN_FILTERGBK);
		}
		else if (nRet == R.id.switch_read_input)
		{
			g_SettingsUI->bSound = !g_SettingsUI->bSound;
			g_SettingsUI->SetModified(true);
			m_pInputListener->OnCommand(CMD_SYNCUI, BTN_SOUND);
		}
		else if (nRet == R.id.sent_record)
		{
			g_SettingsUI->bRecord = !g_SettingsUI->bRecord;
			g_SettingsUI->SetModified(true);
			m_pInputListener->OnCommand(CMD_SYNCUI, BTN_RECORD);
		}else if(nRet == R.id.sent_associate)
		{
			g_SettingsUI->bSentAssocite = !g_SettingsUI->bSentAssocite;
			g_SettingsUI->SetModified(true);
		}
		else if(nRet == R.id.sent_record_clear_all)
		{
			CIsSvrProxy::GetSvrCore()->ClearRecord(false);
		}else if(nRet == R.id.sent_record_clear_today)
		{
			CIsSvrProxy::GetSvrCore()->ClearRecord(true);
		}
		else if (nRet == R.id.switch_word_input)
		{
			g_SettingsUI->bEnglish = !g_SettingsUI->bEnglish;
			g_SettingsUI->SetModified(true);
			m_pInputListener->OnCommand(CMD_SYNCUI, BTN_ENGLISHMODE);
		}
		else if(nRet == R.id.switch_char_mode)
		{
			g_SettingsUI->bCharMode = !g_SettingsUI->bCharMode;
			g_SettingsUI->SetModified(true);
			m_pInputListener->OnCommand(CMD_SYNCUI, BTN_CHARMODE);
		}
		else if(nRet == R.id.skin_def)
		{
			m_pInputListener->OnCommand(CMD_CHANGESKIN, (LPARAM)&SStringT());
		}else if(nRet == R.id.skin_using_vert)
		{
			g_SettingsG->bUsingVertLayout = !g_SettingsG->bUsingVertLayout;
			//reload composition layout
			m_pInputListener->OnCommand(CMD_UPDATECOMPLAYOUT, 0);
		}
		else if(nRet == R.id.menu_forum)
		{
			ShellExecute(NULL, _T("open"), g_SettingsG->urlForum, NULL, NULL, SW_SHOWNORMAL);
		}else if(nRet == R.id.menu_flm_close)
		{
			CIsSvrProxy::GetSvrCore()->CloseFlm();
		}
		else if (nRet > R.id.menu_flm_close && nRet < (R.id.menu_flm_close + 99) / 100 * 100)
		{
			int iComp = nRet - (R.id.menu_flm_close +1);
			const SArray<CNameTypePair> & flmList = CDataCenter::getSingleton().GetData().m_flmList;
			if (iComp < (int)flmList.GetCount())
			{
				SStringT strName = flmList[iComp].strTitle;
				strName=strName.Left(strName.GetLength()-4);//remove .flm
				CIsSvrProxy::GetSvrCore()->OpenFlm(strName);
			}
		}
		else
		{
			SStringT strSkinPath = m_skinManager.SkinPathFromID(nRet);
			if(!strSkinPath.IsEmpty())
			{//select skin menu
				m_pInputListener->OnCommand(CMD_CHANGESKIN, (LPARAM)&strSkinPath);
			}
		}


		m_skinManager.ClearMap();
	}

	void CStatusWnd::OnHelpClick()
	{
		m_pInputListener->OnCommand(CMD_OPENHELP, 0);
	}

	void CStatusWnd::OnQueryClick()
	{
		m_pInputListener->OnCommand(CMD_HOTKEY_QUERYINFO, 0);
	}

	void CStatusWnd::OnConfigClick()
	{
		m_pInputListener->OnCommand(CMD_OPENCONFIG, 0);
	}

	LPARAM CStatusWnd::OnEditUserDefData(UINT uMsg,WPARAM wp,LPARAM lp)
	{
		int nType = (int)wp;
		TCHAR szPath[MAX_PATH];
		if (CIsSvrProxy::GetSvrCore()->ExportDataFile(nType,szPath))
		{
			CTextEditorDlg dlg(nType, szPath);
			if (dlg.DoModal() == IDOK)
			{
				CIsSvrProxy::GetSvrCore()->UpdateDataFile(szPath,nType);
			}
			DeleteFile(szPath);
		}
		else
		{
			CUtils::SoundPlay(_T("error"));
		}
		return 0;
	}

	void CStatusWnd::OnWndClick(EventArgs *e)
	{
		e->SetBubbleUp(true);
		SStringW strSound ;
		e->Sender()->GetAttribute(L"cmd_sound",&strSound);
		if(!strSound.IsEmpty())
		{
			CWorker::getSingletonPtr()->PlaySoundFromResource(strSound);
		}
	}

	void CStatusWnd::UpdateCaptialMode()
	{
		BOOL bCap = GetKeyState(VK_CAPITAL)&0x01;
		{
			SWindow *pStatus = FindChildByID(R.id.status_shrink);
			SASSERT(pStatus);
			SFlagView * pFlagView = pStatus->FindChildByID2<SFlagView>(R.id.img_logo);
			if(pFlagView)
			{
				pFlagView->UpdateCapitalMode(bCap);
			}
		}
		{
			SWindow *pStatus = FindChildByID(R.id.status_extend);
			SASSERT(pStatus);
			SFlagView * pFlagView = pStatus->FindChildByID2<SFlagView>(R.id.img_logo);
			if(pFlagView)
			{
				pFlagView->UpdateCapitalMode(bCap);
			}

		}

	}

	static int GetMenuItemIndex(HMENU hMenu, UINT uID)
	{
		int nItemCount = GetMenuItemCount(hMenu);
		if (nItemCount != -1)
		{
			for (int nIndex = 0; nIndex < nItemCount; ++nIndex)
			{
				UINT uItemID = GetMenuItemID(hMenu, nIndex);
				if (uItemID == uID)
				{
					return nIndex; // ���ز˵����������
				}
			}
		}

		return -1; // δ�ҵ�ƥ��Ĳ˵���
	}

	void CStatusWnd::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU menu)
	{
		if((nItemID>=R.id.skin_def)&&(nItemID<=m_skinManager.GetSkinMaxID()) || nFlags&MF_POPUP)
		{
			if(nFlags&MF_POPUP){
				SkinPrev_Hide();
			}else{
				int idx = GetMenuItemIndex(menu,nItemID);
				if(idx!=-1){
					RECT rcItem;
					GetMenuItemRect(NULL, menu, idx, &rcItem);
					SkinPrev_Show(nItemID,&rcItem, nFlags & MF_CHECKED);
				}
			}
		}
	}

	void CStatusWnd::SkinPrev_Show(int nID,LPCRECT pRc, BOOL bCheck)
	{
		SStringT strSkinPath = m_skinManager.SkinPathFromID(nID);
		SAutoRefPtr<IBitmapS> img;
		if(m_skinManager.ExtractPreview(strSkinPath,&img)){
			m_skinPreview.SetPreview(img);
			m_skinPreview.UpdateWindow();
			CRect rcWnd = m_skinPreview.GetClientRect();
			HMONITOR hMonitor = MonitorFromWindow(m_hOwner, MONITOR_DEFAULTTONEAREST);
			MONITORINFO info = { sizeof(info),0 };
			GetMonitorInfo(hMonitor, &info);
			CPoint pt(pRc->left-rcWnd.Width(),pRc->top);
			if(pt.x<info.rcWork.left)
				pt.x = pRc->right;
			if(pt.y+rcWnd.Height()>info.rcWork.bottom)
				pt.y = info.rcWork.bottom-rcWnd.Height();
			if(pt.y<info.rcWork.top)
				pt.y=info.rcWork.top;
			m_skinPreview.SetWindowPos(HWND_TOPMOST,pt.x,pt.y,0,0,SWP_NOSIZE|SWP_SHOWWINDOW|SWP_NOACTIVATE);
			SLOGI()<<"skin path="<<strSkinPath.c_str()
				<<" menuPos="<<pRc->left<<","<<pRc->top
				<<" imgSize="<<img->Width()<<","<<img->Height()
				<<" wndSize="<<rcWnd.Width()<<","<<rcWnd.Height();
		}else{
			SLOGI()<<"no preview image found in "<<strSkinPath.c_str();
			m_skinPreview.ShowWindow(SW_HIDE);
		}
	}

	void CStatusWnd::SkinPrev_Hide()
	{
		m_skinPreview.ShowWindow(SW_HIDE);
	}


}

