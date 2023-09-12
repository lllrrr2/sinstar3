#include "stdafx.h"
#include "TextEditorDlg.h"
#include "../include/FileHelper.h"

CTextEditorDlg::CTextEditorDlg(int nMode, const SStringT & strFileName)
	:SHostDialog(UIRES.LAYOUT.dlg_texteditor)
	,m_mode(nMode- FU_USERDEF)
	,m_strFileName(strFileName)
	, m_pSciter(NULL)
	, m_pFindDlg(NULL)
{
}


CTextEditorDlg::~CTextEditorDlg()
{
}

BOOL CTextEditorDlg::OnInitDialog(HWND hWnd, LPARAM lp)
{
	FindChildByID(R.id.txt_edit_userdef + m_mode)->SetVisible(TRUE, TRUE);
	FindChildByID(R.id.tip_edit_userdef + m_mode)->SetVisible(TRUE, TRUE);
	SRealWnd * pRealWnd = FindChildByID2<SRealWnd>(R.id.real_scilexer);
	SASSERT(pRealWnd);
	m_pSciter = (CScintillaWnd *)pRealWnd->GetData();
	m_pSciter->SendMessage(SCI_USEPOPUP, 0, 0);

	FILE *f = _tfopen(m_strFileName, _T("rb"));
	if (f)
	{
		fseek(f, 0, SEEK_END);
		int nLen = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *buf = (char*)malloc(nLen+1);
		fread(buf, 1, nLen, f);
		buf[nLen]=0;
		fclose(f);
		if(memcmp(buf,"\xff\xfe",2)==0)
		{//utf16
			SStringA utf8 = S_CW2A(SStringW((WCHAR*)(buf+2),(nLen-2)/2), CP_UTF8);
			m_pSciter->SendMessage(SCI_SETTEXT, 0, (LPARAM)utf8.c_str());
		}else if(buf[0]==0xef && buf[1]==0xbb && buf[2]==0xbf)
		{//utf8
			m_pSciter->SendMessage(SCI_SETTEXT, 0, (LPARAM)(buf+3));
		}else
		{//ansi
			SStringA utf8 = S_CA2A(SStringA(buf,nLen),CP_ACP, CP_UTF8);
			m_pSciter->SendMessage(SCI_SETTEXT, 0, (LPARAM)utf8.c_str());
		}
		free(buf);
	}
	m_pSciter->UpdateLineNumberWidth();
	SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	return 0;
}

void CTextEditorDlg::OnBtnClose()
{
	EndDialog(IDCANCEL);
}

void CTextEditorDlg::OnBtnSave()
{
	m_pSciter->SaveFile(m_strFileName);
	EndDialog(IDOK);
}

void CTextEditorDlg::OnBtnFind()
{
	if(m_pFindDlg==NULL)
	{
		m_pFindDlg = new CFindDlg(this);
		m_pFindDlg->Create(m_hWnd);
		m_pFindDlg->CenterWindow(m_pSciter->m_hWnd);
	}		
	m_pFindDlg->ShowWindow(SW_SHOW);
}

bool CTextEditorDlg::OnFind(const SStringT & strText, bool bFindNext, bool bMatchCase, bool bMatchWholeWord)
{
	int flags = (bMatchCase?SCFIND_MATCHCASE:0) | (bMatchWholeWord?SCFIND_WHOLEWORD:0);
	TextToFind ttf;
	ttf.chrg.cpMin = m_pSciter->SendMessage(SCI_GETCURRENTPOS);
	if(bFindNext)
		ttf.chrg.cpMax = m_pSciter->SendMessage(SCI_GETLENGTH, 0, 0);
	else
		ttf.chrg.cpMax = 0;

	SStringA strUtf8 = S_CT2A(strText,CP_UTF8);
	ttf.lpstrText = (char *)(LPCSTR) strUtf8;
	int nPos = m_pSciter->SendMessage(SCI_FINDTEXT,flags,(LPARAM)&ttf);
	if(nPos==-1) return false;

	if(bFindNext)
		m_pSciter->SendMessage(SCI_SETSEL,nPos,nPos + strUtf8.GetLength());
	else
		m_pSciter->SendMessage(SCI_SETSEL,nPos+ strUtf8.GetLength(),nPos);

	m_pSciter->SetFocus();
	return true;
}

void CTextEditorDlg::OnReplace(const SStringT &strText)
{
	SStringA strUtf8 = S_CT2A(strText,CP_UTF8);
	m_pSciter->SendMessage(SCI_REPLACESEL,0,(LPARAM)strUtf8.c_str());
}

void CTextEditorDlg::OnBtnExport()
{
	CFileDialogEx saveDlg(FALSE, _T("txt"), NULL, 6, _T("�ı��ļ�(*.txt)\0*.txt\0All files (*.*)\0*.*\0\0"));
	if(saveDlg.DoModal()==IDOK)
	{
		m_pSciter->SaveFile(saveDlg.m_szFileName);
	}
}
