#include "stdafx.h"
#include "Utils.h"
#include <ShellAPI.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib")


CUtils::CUtils()
{
}


CUtils::~CUtils()
{
}


int CUtils::GetClipboardText(HWND hWnd, WCHAR *pszBuf, int nBufSize)
{
	int nRet = 0;
	if (OpenClipboard(hWnd))
	{//��������������
		HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
		if (hglb)
		{
			LPWSTR lpstr = (LPWSTR)GlobalLock(hglb);
			SStringW str = lpstr;
			str.TrimBlank();
			if (str.GetLength() < nBufSize)
			{
				wcscpy(pszBuf, str);
				nRet = str.GetLength();
			}
			GlobalUnlock(hglb);
		}
		CloseClipboard();
	}
	return nRet;
}


void CUtils::SoundPlay(LPCTSTR pszSound)
{
	if (g_SettingsG->nSoundAlert == 1)
	{
		SStringT strPath = SStringT().Format(_T("%s\\sound\\%s.wav"), CDataCenter::getSingletonPtr()->GetDataPath().c_str(), pszSound);
		PlaySound(strPath, NULL, SND_ASYNC | SND_FILENAME);
	}
	else if (g_SettingsG->nSoundAlert == 2)
	{
		MessageBeep(1000);
	}
}

//----------------------------------------------------------------------------
// GB2312��תGBK��
// �л����񹲺͹� --> ���A���񹲺͇�
int  CUtils::GB2GIB5(LPCWSTR szBuf, int nBufLen, WCHAR *szBig5, int nOutLen)
{
	DWORD wLCID = MAKELCID(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), SORT_CHINESE_PRC);
	return LCMapStringW(wLCID, LCMAP_TRADITIONAL_CHINESE, szBuf, nBufLen*sizeof(WCHAR), szBig5, nOutLen * sizeof(WCHAR));
}

BOOL CUtils::CmdExecute(BYTE * pszBuf)
{
	UINT_PTR uRet = FALSE;
	LPBYTE pCmd = (pszBuf + pszBuf[2]*2 + 3);//rate+flag+length
	SStringW strCmd((WCHAR*)(pCmd+1),pCmd[0]);
	SStringW strParam;
	if (strCmd[0] == '\"')
	{
		int nPos =strCmd.Find(L"\" ");
		if(nPos!=-1)
		{
			strParam = strCmd.Right(strCmd.GetLength()-nPos-2);
			strCmd = strCmd.Left(nPos);
		}
	}
	else
	{
		int nPos =strCmd.Find(L" ");
		if(nPos!=-1)
		{
			strParam = strCmd.Right(strCmd.GetLength()-nPos-1);
			strCmd = strCmd.Left(nPos);
		}
	}
	uRet = (UINT_PTR)ShellExecuteW(NULL, L"open", strCmd, strParam, NULL, SW_SHOWDEFAULT);
	if (uRet <= 32) uRet = (UINT_PTR)ShellExecuteW(NULL, L"explorer", strCmd, NULL, NULL, SW_SHOWDEFAULT);
	return uRet>32;
}

