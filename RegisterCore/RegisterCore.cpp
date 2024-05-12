// RegisterCore.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "RegisterCore.h"
#include <Sddl.h>
#include <AccCtrl.h>
#include <AclAPI.h>

#pragma comment(lib,"Version.lib")

TCHAR g_szPath[MAX_PATH] = { 0 };	//��������λ��

const TCHAR * KFiles[] =
{
	_T("program\\isserver3.exe"),
	_T("program\\sinstar3_ime.dll"),
	_T("program\\x64\\sinstar3_ime.dll"),
	_T("program\\sinstar3_tsf.dll"),
	_T("program\\x64\\sinstar3_tsf.dll"),
};


struct CopyInfo {
	const TCHAR *pszSrc;
	const TCHAR *pszDest;
	bool   bReg;
};

const CopyInfo KSrcX86Files[] =
{
	{ _T("program\\sinstar3_ime.dll"),_T("sinstar3_ime.ime"),false },
	{ _T("program\\sinstar3_tsf.dll"),_T("sinstar3_tsf.dll"),true },
};

const CopyInfo KSrcX64Files[] =
{
	{ _T("program\\x64\\sinstar3_ime.dll"),_T("sinstar3_ime.ime"),true },
	{ _T("program\\x64\\sinstar3_tsf.dll"),_T("sinstar3_tsf.dll"),true },
};

struct UpdateInfo
{
	LPCTSTR pszFile;
	bool	bClean;	//true- delete at first.
};

const UpdateInfo KUpdateInfo[] =
{
	{_T("program"),true},
	{_T("defskin"),true },
	{_T("sound"),true },
	{_T("skins"),false},
	{_T("tools"),false },	
	{_T("register.exe"),false },
	{_T("RegisterCore.dll"),false },
	{_T("license.rtf"),false },
	{_T("ʹ��˵��.txt"),false },
};

BOOL Is64OS()
{
	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);

	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
		si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static TCHAR s_LastMsg[500]=_T("ok");//error msg;

void Sinstar_SetErrMsg(LPCTSTR szMsg)
{
	_tcscpy_s(s_LastMsg,500*sizeof(TCHAR),szMsg);
}

LPCWSTR RC_API  Sinstar_GetErrMsgW(){
	return s_LastMsg;
}

LPCSTR RC_API  Sinstar_GetErrMsgA(){
	static char szErrMsg[1000];
	WideCharToMultiByte(CP_ACP,0,s_LastMsg,-1,szErrMsg,1000,NULL,NULL);
	return szErrMsg;
}

typedef BOOL(WINAPI *FunWow64DisableWow64FsRedirection)(
	PVOID *OldValue
	);

void MyDisableWow64FsRedirection()
{
	HMODULE hMod = LoadLibrary(_T("Kernel32.dll"));
	FunWow64DisableWow64FsRedirection fWow64DisableWow64FsRedirection = (FunWow64DisableWow64FsRedirection)GetProcAddress(hMod, "Wow64DisableWow64FsRedirection");
	if (fWow64DisableWow64FsRedirection)
	{
		PVOID pData = NULL;
		fWow64DisableWow64FsRedirection(&pData);
	}
	FreeLibrary(hMod);
}


void RC_API  Sinstar_InitW(LPCWSTR pszPath)
{
	wcscpy_s(g_szPath,MAX_PATH*sizeof(TCHAR),pszPath);
	MyDisableWow64FsRedirection();
}

void RC_API  Sinstar_InitA(LPCSTR pszPath)
{
	MultiByteToWideChar(CP_ACP,0,pszPath,strlen(pszPath),g_szPath,MAX_PATH);
	MyDisableWow64FsRedirection();
}

BOOL RC_API  Sinstar_GetInstallDir(LPWSTR  pszPath,int nSize)
{
	CRegKey reg;
	LONG ret = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SetoutSoft\\sinstar3"), KEY_READ | KEY_WOW64_64KEY);
	if (ret != ERROR_SUCCESS)
	{
		return FALSE;
	}

	ULONG nLen = nSize;
	reg.QueryStringValue(_T("path_client"),pszPath,&nLen);
	reg.Close();
	return TRUE;
}

BOOL RC_API  Sinstar_ShowCaller(BOOL bOrg)
{
	TCHAR szPath[MAX_PATH];
	STARTUPINFO si = { sizeof(STARTUPINFO),0 };
	PROCESS_INFORMATION pi = { 0 };
	if(bOrg)
	{
		TCHAR szOriPath[MAX_PATH];
		if(!Sinstar_GetInstallDir(szOriPath,MAX_PATH))
			return FALSE;
		_stprintf(szPath, _T("%s\\program\\findcallerUI.exe sinstar3_ime.ime|sinstar3_tsf.dll"), szOriPath);
	}else
	{
		_stprintf(szPath, _T("%s\\program\\findcallerUI.exe sinstar3_ime.ime|sinstar3_tsf.dll"), g_szPath);
	}
	if (CreateProcess(NULL, szPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return TRUE;
	}
	else
	{
		Sinstar_SetErrMsg(_T("����FindCallerUIʧ��"));
		return FALSE;
	}
}

DWORD CallRegsvr32(LPCTSTR pszFileName, BOOL bReg)
{
	STARTUPINFO         si = { 0 };
	PROCESS_INFORMATION pi;
	DWORD dwWaitRet = -1;
	si.cb = sizeof(si);
	si.dwFlags = 0;
	si.wShowWindow = 0;

	TCHAR szSysDir[MAX_PATH];
	GetSystemDirectory(szSysDir, MAX_PATH);

	TCHAR szCmd[1000];
	_stprintf(szCmd, _T("regsvr32 /s %s %s"), bReg ? _T("") : _T("/u"), pszFileName);
	//�Ժ�̨������ʽ��������������
	if (!CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP, NULL, szSysDir, &si, &pi))
	{
		return FALSE;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &dwWaitRet);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return dwWaitRet;
}

typedef UINT
(RC_API
 *FunGetSystemWow64Directory)(
 LPTSTR lpBuffer,
 UINT uSize
 );


BOOL GetSysWow64Dir(LPTSTR pszDir, int nLen)
{
	if (!Is64OS())
		return FALSE;
	HMODULE hMod = LoadLibrary(_T("Kernel32.dll"));
#ifdef _UNICODE
	FunGetSystemWow64Directory fGetSystemWow64Directory = (FunGetSystemWow64Directory)GetProcAddress(hMod, "GetSystemWow64DirectoryW");
#else
	FunGetSystemWow64Directory fGetSystemWow64Directory = (FunGetSystemWow64Directory)GetProcAddress(hMod, "GetSystemWow64DirectoryA");
#endif
	if (!fGetSystemWow64Directory) return FALSE;
	fGetSystemWow64Directory(pszDir, nLen);
	FreeLibrary(hMod);
	return TRUE;
}


BOOL RC_API Sinstar_IsUpdateIME()
{
	TCHAR szPath[MAX_PATH],szSysPath[MAX_PATH];
	::GetSystemDirectory(szSysPath, MAX_PATH);
	_stprintf(szPath,_T("%s\\%s"),szSysPath,KSrcX86Files[0].pszDest);
	int v1[4]={0};
	Sinstar_PEVersion2W(szPath,v1+0,v1+1,v1+2,v1+3);
	_stprintf(szPath,_T("%s\\%s"),g_szPath,KSrcX86Files[0].pszSrc);
	int v2[4]={0};
	Sinstar_PEVersion2W(szPath,v2+0,v2+1,v2+2,v2+3);
	return memcmp(v1,v2,sizeof(v1))!=0;
}

BOOL RC_API  Sinstar_IsRunning()
{
	HANDLE hMutex = CreateMutex(NULL, FALSE, SINSTAR3_MUTEX);
	BOOL bRet = GetLastError() == ERROR_ALREADY_EXISTS;
	CloseHandle(hMutex);
	return bRet;
}

BOOL Sinstar_Unreg(LPCTSTR sysWow64,LPCTSTR szSysPath,BOOL bDelFile)
{
	TCHAR szPath[1000];
	//ɾ�����뷨�ļ�
	if (Is64OS())
	{
		for (int i = 0; i<ARRAYSIZE(KSrcX64Files); i++)
		{
			_stprintf(szPath, _T("%s\\%s"), szSysPath, KSrcX64Files[i].pszDest);
			if (KSrcX64Files[i].bReg && 0 != CallRegsvr32(szPath, FALSE))
			{
				return FALSE;
			}
			if(bDelFile) DeleteFile(szPath);
		}

		for (int i = 0; i<ARRAYSIZE(KSrcX86Files); i++)
		{
			_stprintf(szPath, _T("%s\\%s"), sysWow64, KSrcX86Files[i].pszDest);
			if (KSrcX86Files[i].bReg && 0 != CallRegsvr32(szPath, FALSE))
			{
				return FALSE;
			}
			if(bDelFile) DeleteFile(szPath);
		}
	}
	else
	{
		for (int i = 0; i<ARRAYSIZE(KSrcX86Files); i++)
		{
			_stprintf(szPath, _T("%s\\%s"), szSysPath, KSrcX86Files[i].pszDest);
			if (0 != CallRegsvr32(szPath, FALSE))
			{
				return FALSE;
			}
			if(bDelFile) DeleteFile(szPath);
		}
	}
	return TRUE;
}

BOOL Sinstar_Reg(LPCTSTR szSysWow64,LPCTSTR szSysPath)
{

	TCHAR szPath[1000];
	//step6:��װ���뷨
	if (Is64OS())
	{
		for (int i = 0; i<ARRAYSIZE(KSrcX86Files); i++)
		{
			if (!KSrcX86Files[i].bReg) continue;
			_stprintf(szPath, _T("%s\\%s"), szSysWow64, KSrcX86Files[i].pszDest);
			if (0 != CallRegsvr32(szPath, TRUE))
			{
				return FALSE;
			}
		}

		for (int i = 0; i<ARRAYSIZE(KSrcX64Files); i++)
		{
			if (!KSrcX64Files[i].bReg) continue;
			_stprintf(szPath, _T("%s\\%s"), szSysPath, KSrcX64Files[i].pszDest);
			if (0 != CallRegsvr32(szPath, TRUE))
			{
				return FALSE;
			}
		}
	}
	else
	{
		for (int i = 0; i<ARRAYSIZE(KSrcX86Files); i++)
		{
			_stprintf(szPath, _T("%s\\%s"), szSysPath, KSrcX86Files[i].pszDest);
			if (0 != CallRegsvr32(szPath, TRUE))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL Sinstar_UpdateIME()
{
	TCHAR szPath1[MAX_PATH], szPath2[MAX_PATH], szSysPath[MAX_PATH], szSysWow64[MAX_PATH];
	::GetSystemDirectory(szSysPath, MAX_PATH);
	GetSysWow64Dir(szSysWow64, MAX_PATH);

	if(Sinstar_IsRunning())
	{
		Sinstar_SetErrMsg(_T("���뷨����ʹ�ã����ܸ���"));
		return FALSE;
	}

	BOOL bCopied = TRUE;
	if (Is64OS())
	{
		//copy x64 files to system dir
		for (int i = 0; i<ARRAYSIZE(KSrcX64Files) && bCopied; i++)
		{
			_stprintf(szPath1, _T("%s\\%s"), g_szPath, KSrcX64Files[i].pszSrc);
			_stprintf(szPath2, _T("%s\\%s"), szSysPath, KSrcX64Files[i].pszDest);
			bCopied = CopyFile(szPath1, szPath2, FALSE);
		}

		//copy x86 files to wow64 dir.
		for (int i = 0; i<ARRAYSIZE(KSrcX86Files) && bCopied; i++)
		{
			_stprintf(szPath1, _T("%s\\%s"), g_szPath, KSrcX86Files[i].pszSrc);
			_stprintf(szPath2, _T("%s\\%s"), szSysWow64, KSrcX86Files[i].pszDest);
			bCopied = CopyFile(szPath1, szPath2, FALSE);
		}
	}
	else
	{
		//copy x86 files to system dir
		for (int i = 0; i<ARRAYSIZE(KSrcX86Files) && bCopied; i++)
		{
			_stprintf(szPath1, _T("%s\\%s"), g_szPath, KSrcX86Files[i].pszSrc);
			_stprintf(szPath2, _T("%s\\%s"), szSysPath, KSrcX86Files[i].pszDest);
			bCopied = CopyFile(szPath1, szPath2, FALSE);
		}
	}
	if(!bCopied)
	{
		Sinstar_SetErrMsg(_T("�����ļ���ϵͳĿ¼ʧ�ܣ�"));
	}
	return bCopied;
}
BOOL RC_API  Sinstar_PEVersionW(LPCWSTR pszFileName,  int wVers[4])
{
	return Sinstar_PEVersion2W(pszFileName,wVers+0,wVers+1,wVers+2,wVers+3);
}

BOOL RC_API  Sinstar_PEVersion2W(LPCWSTR pszFileName,int *v1,int *v2,int *v3,int *v4)
{
	DWORD dwResHandle;
	void *pBuf;
	BOOL bRet = FALSE;
	DWORD dwVerInfoSize = GetFileVersionInfoSizeW(pszFileName, &dwResHandle);
	if (!dwVerInfoSize) return FALSE;
	pBuf = malloc(dwVerInfoSize);
	GetFileVersionInfo(pszFileName, dwResHandle, dwVerInfoSize, pBuf);
	UINT nVersionLen;
	VS_FIXEDFILEINFO *pstFileVersion;
	if (VerQueryValueW(pBuf, _T("\\"), (void**)&pstFileVersion, &nVersionLen) && nVersionLen >= sizeof(VS_FIXEDFILEINFO))
	{
		*v4 = LOWORD(pstFileVersion->dwFileVersionLS);
		*v3 = HIWORD(pstFileVersion->dwFileVersionLS);
		*v2 = LOWORD(pstFileVersion->dwFileVersionMS);
		*v1 = HIWORD(pstFileVersion->dwFileVersionMS);
	}
	free(pBuf);
	return TRUE;
}

BOOL RC_API  Sinstar_PEVersion2A(LPCSTR pszFileName,int *v1,int *v2,int *v3,int *v4)
{
	WCHAR szPath[MAX_PATH]={0};
	MultiByteToWideChar(CP_ACP,0,pszFileName,strlen(pszFileName),szPath,MAX_PATH);
	return Sinstar_PEVersion2W(szPath,v1,v2,v3,v4);
}

BOOL RC_API  Sinstar_PEVersionA(LPCSTR pszFileName,  int wVers[4])
{
	return Sinstar_PEVersion2A(pszFileName,wVers+0,wVers+1,wVers+2,wVers+3);
}

BOOL RC_API  Sinstar_GetCurrentVer2(int *v1,int *v2,int *v3,int *v4)
{
	WCHAR szPath[MAX_PATH];
	if(!Sinstar_GetInstallDir(szPath,MAX_PATH))
		return FALSE;

	TCHAR szSvrPath[MAX_PATH];
	_stprintf(szSvrPath,_T("%s\\program\\isserver3.exe"),szPath);
	return Sinstar_PEVersion2W(szSvrPath,v1,v2,v3,v4);
}

BOOL RC_API  Sinstar_GetCurrentVer(int wVers[4])
{
	return Sinstar_GetCurrentVer2(wVers+0,wVers+1,wVers+2,wVers+3);
}

BOOL RC_API  Sinstar_Update()
{	
	BOOL bUpgradeIme = Sinstar_IsUpdateIME();
	BOOL bRunning = Sinstar_IsRunning();
	if(bUpgradeIme && bRunning)
	{
		Sinstar_SetErrMsg(_T("���뷨����ʹ�ã����ܰ�װ"));
		return FALSE;
	}

	CRegKey reg;
	LONG ret = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SetoutSoft\\sinstar3"), KEY_READ | KEY_WOW64_64KEY);
	if (ret != ERROR_SUCCESS)
	{
		return Sinstar_Install();
	}

	//�˳�������
	Sinstar_QuitServer();


	TCHAR szPath[MAX_PATH];
	ULONG nLen = MAX_PATH;
	reg.QueryStringValue(_T("path_client"),szPath,&nLen);
	reg.Close();

	if(_tcsicmp(szPath,g_szPath)!=0)
	{
		for (int i = 0; i < ARRAYSIZE(KUpdateInfo); i++)
		{
			TCHAR szDest[MAX_PATH]={0};
			TCHAR szSour[MAX_PATH]={0};
			_stprintf(szSour, _T("%s\\%s"), g_szPath, KUpdateInfo[i].pszFile);
			if(KUpdateInfo[i].bClean)
			{
				memset(szDest,0,sizeof(szDest));
				_stprintf(szDest, _T("%s\\%s"), szPath,KUpdateInfo[i].pszFile);
				//delete old program dir
				SHFILEOPSTRUCT fileOp = { 0 };
				fileOp.wFunc = FO_DELETE;
				fileOp.pFrom = szDest;
				fileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT | FOF_ALLOWUNDO;
				int nRet = SHFileOperation(&fileOp);
			}
			{
				memset(szDest,0,sizeof(szDest));
				_stprintf(szDest, _T("%s"), szPath);
				//copy file to old path.
				SHFILEOPSTRUCT fileOp = { 0 };
				fileOp.wFunc = FO_COPY;
				fileOp.pFrom = szSour;
				fileOp.pTo = szDest;
				fileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT|FOF_NOCOPYSECURITYATTRIBS|FOF_NOCONFIRMMKDIR ;
				int nRet = SHFileOperation(&fileOp);
			}
		}
	}

	if(bUpgradeIme) Sinstar_UpdateIME();

	//step8:reg ime file type.
	TCHAR szSvrCmd[MAX_PATH];
	_stprintf(szSvrCmd,_T("%s\\program\\isserver3.exe"),szPath);
	ShellExecute(NULL, _T("open"), szSvrCmd, _T("-reg"), NULL, 0);
	
	if(!bUpgradeIme && bRunning)
	{
		ShellExecute(NULL, _T("open"), szSvrCmd, NULL, NULL, 0);
	}
	return TRUE;
}

BOOL RC_API  Sinstar_Uninstall()
{
	TCHAR szSysPath[MAX_PATH], sysWow64[MAX_PATH];
	::GetSystemDirectory(szSysPath, MAX_PATH);
	GetSysWow64Dir(sysWow64, MAX_PATH);
	if(Sinstar_IsRunning())
	{
		Sinstar_SetErrMsg(_T("���뷨����ʹ�ã�����ж��"));
		return FALSE;
	}

	if(!Sinstar_Unreg(sysWow64,szSysPath,TRUE))
	{
		Sinstar_SetErrMsg(_T("��ע�����뷨�ӿ�ʧ��"));
		return FALSE;
	}

	//delete reg
	CRegKey reg;
	LONG ret = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SetoutSoft"), KEY_WRITE | KEY_READ | KEY_WOW64_64KEY);
	if (ret != ERROR_SUCCESS)
	{
		Sinstar_SetErrMsg(_T("��ע���ʧ��"));
		return FALSE;
	}

	TCHAR szClient[MAX_PATH];
	ULONG uSize = MAX_PATH;
	reg.QueryStringValue(_T("path_client"), szClient, &uSize);

	reg.RecurseDeleteKey(_T("sinstar3"));
	reg.Close();


	//�˳�������
	Sinstar_QuitServer();

	//step8:reg ime file type.
	TCHAR szRegCmd[MAX_PATH];
	_stprintf(szRegCmd, _T("%s\\program\\isserver3.exe"), szClient);
	ShellExecute(NULL, _T("open"), szRegCmd, _T("-unreg"), NULL, 0);

	return TRUE;
}


void DeleteFileEx(LPCTSTR pszPath)
{
	TCHAR szTmpFile[MAX_PATH*2];
	for(int i=0;;i++)
	{
		_stprintf(szTmpFile,_T("%s.bak.%d"),pszPath,i);
		if(GetFileAttributes(szTmpFile)==INVALID_FILE_ATTRIBUTES)
			break;
	}
	MoveFile(pszPath,szTmpFile);
	MoveFileEx(szTmpFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
}

BOOL RC_API  Sinstar_ForceUninstall()
{
	TCHAR szSysPath[MAX_PATH], sysWow64[MAX_PATH];
	TCHAR szPath[MAX_PATH];
	::GetSystemDirectory(szSysPath, MAX_PATH);
	GetSysWow64Dir(sysWow64, MAX_PATH);
	if(!Sinstar_IsRunning())
	{
		return Sinstar_Uninstall();
	}

	//ɾ�����뷨�ļ�
	if (Is64OS())
	{
		for (int i = 0; i<ARRAYSIZE(KSrcX64Files); i++)
		{
			_stprintf(szPath, _T("%s\\%s"), szSysPath, KSrcX64Files[i].pszDest);
			if (KSrcX64Files[i].bReg && 0 != CallRegsvr32(szPath, FALSE))
			{
				Sinstar_SetErrMsg(_T("��ע�����뷨ģ��ʧ��!"));
				return FALSE;
			}

			DeleteFileEx(szPath);
		}

		for (int i = 0; i<ARRAYSIZE(KSrcX86Files); i++)
		{
			_stprintf(szPath, _T("%s\\%s"), sysWow64, KSrcX86Files[i].pszDest);
			if (KSrcX86Files[i].bReg && 0 != CallRegsvr32(szPath, FALSE))
			{
				Sinstar_SetErrMsg(_T("��ע�����뷨ģ��ʧ��!"));
				return FALSE;
			}
			DeleteFileEx(szPath);
		}
	}
	else
	{
		for (int i = 0; i<ARRAYSIZE(KSrcX86Files); i++)
		{
			_stprintf(szPath, _T("%s\\%s"), szSysPath, KSrcX86Files[i].pszDest);
			if (0 != CallRegsvr32(szPath, FALSE))
			{
				Sinstar_SetErrMsg(_T("��ע�����뷨ģ��ʧ��!"));
				return FALSE;
			}
			DeleteFileEx(szPath);
		}
	}

	//delete reg
	CRegKey reg;
	LONG ret = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SetoutSoft"), KEY_WRITE | KEY_READ | KEY_WOW64_64KEY);
	if (ret != ERROR_SUCCESS)
	{
		Sinstar_SetErrMsg(_T("��ע���ʧ��!"));
		return FALSE;
	}

	TCHAR szClient[MAX_PATH];
	ULONG uSize = MAX_PATH;
	reg.QueryStringValue(_T("path_client"), szClient, &uSize);

	reg.RecurseDeleteKey(_T("sinstar3"));
	reg.Close();


	//step8:reg ime file type.
	TCHAR szRegCmd[MAX_PATH];
	_stprintf(szRegCmd, _T("%s\\program\\isserver3.exe"), g_szPath);
	ShellExecute(NULL, _T("open"), szRegCmd, _T("-unreg"), NULL, 0);

	return TRUE;
}

void RC_API Sinstar_QuitServer()
{
	HWND hWndSvr = FindWindow(NULL, SINSTAR3_SERVER_HWND);
	if (IsWindow(hWndSvr))
	{
		DWORD dwProcID = 0;
		GetWindowThreadProcessId(hWndSvr, &dwProcID);
		PostMessage(hWndSvr, WM_QUIT, 0, 0);
		if (dwProcID != 0)
		{
			HANDLE processHandle = OpenProcess(SYNCHRONIZE , FALSE, dwProcID);
			if(processHandle!=0)
			{
				WaitForSingleObject(processHandle, INFINITE);
				CloseHandle(processHandle); 
			}else
			{
				Sleep(500);
			}
		}else
		{
			Sleep(500);
		}
	}
}

BOOL RC_API  Sinstar_Install()
{
	TCHAR szSysPath[MAX_PATH];
	TCHAR szSysWow64[MAX_PATH] = { 0 };

	GetSystemDirectory(szSysPath, MAX_PATH);
	GetSysWow64Dir(szSysWow64, MAX_PATH);

	BOOL bCopyIme = Sinstar_IsUpdateIME();

	if(bCopyIme && Sinstar_IsRunning())
	{
		Sinstar_SetErrMsg( _T("���뷨����ʹ�ã����ܰ�װ"));
		return FALSE;
	}

	//step2 copy files.
	if (bCopyIme && !Sinstar_UpdateIME())
	{
		return FALSE;
	}

	TCHAR szSvrExe[256] = { 0 }, szSvrData[256] = { 0 };
	_stprintf(szSvrExe, _T("%s\\program\\isserver3.exe"), g_szPath);
	_stprintf(szSvrData, _T("%s\\server"), g_szPath);

	//step3: write reg
	CRegKey reg;
	LONG ret = reg.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SetoutSoft\\sinstar3"), KEY_WRITE | KEY_WOW64_64KEY);
	if (ret != ERROR_SUCCESS)
	{
		ret = reg.Create(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\SetoutSoft\\sinstar3"), 0, 0, KEY_WRITE | KEY_WOW64_64KEY);
		if (ret != ERROR_SUCCESS)
		{
			Sinstar_SetErrMsg(_T("��ע���ʧ��"));
			return FALSE;
		}
	}
	{
		reg.SetStringValue(_T("path_client"), g_szPath);
		reg.SetStringValue(_T("path_svr"), szSvrExe);
		reg.SetStringValue(_T("path_svr_data"), szSvrData);
		reg.Close();
	}

	if(!Sinstar_Reg(szSysWow64,szSysPath))
	{
		Sinstar_SetErrMsg(_T("ע��ʧ��"));
		return FALSE;
	}
	//step8:reg ime file type.
	ShellExecute(NULL, _T("open"), szSvrExe, _T("-reg"), NULL, 0);

	return TRUE;
}


BOOL RC_API  Sinstar_CheckFiles()
{
	for (int i = 0; i<ARRAYSIZE(KFiles); i++)
	{
		TCHAR szBuf[1000];
		_stprintf(szBuf, _T("%s\\%s"),g_szPath, KFiles[i]);
		if (GetFileAttributes(szBuf) == INVALID_FILE_ATTRIBUTES)
		{
			_stprintf(szBuf, _T("��װĿ¼�ļ�[%s]û���ҵ����뱣֤��װ��������"), KFiles[i]);
			Sinstar_SetErrMsg(szBuf);
			return 0;
		}
	}
	return TRUE;
}

BOOL RC_API  Sinstar_SetFileACL(LPCTSTR pszPath)
{
	SECURITY_DESCRIPTOR* pSD = NULL;

	PSID pSid = NULL;
	SID_IDENTIFIER_AUTHORITY authNt = SECURITY_NT_AUTHORITY;
	AllocateAndInitializeSid(&authNt, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_USERS, 0, 0, 0, 0, 0, 0, &pSid);

	EXPLICIT_ACCESS ea = { 0 };
	ea.grfAccessMode = GRANT_ACCESS;
	ea.grfAccessPermissions = GENERIC_ALL;
	ea.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
	ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.ptstrName = (LPTSTR)pSid;

	ACL* pNewDACL = 0;
	DWORD err = SetEntriesInAcl(1, &ea, NULL, &pNewDACL);

	if (pNewDACL)
	{
		HANDLE hFile = CreateFile(pszPath, READ_CONTROL | WRITE_DAC, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		if (hFile != INVALID_HANDLE_VALUE) SetSecurityInfo(hFile, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pNewDACL, NULL);
		CloseHandle(hFile);
		//if (bSubFile)
		{
			WIN32_FIND_DATA wfd = { 0 };
			TCHAR szPath[MAX_PATH], szFilter[MAX_PATH];
			_stprintf(szFilter, _T("%s\\*.*"), pszPath);
			HANDLE hFind = FindFirstFile(szFilter, &wfd);
			if (hFind)
			{
				do
				{
					if (!(wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
					{
						_stprintf(szPath, _T("%s\\%s"), pszPath, wfd.cFileName);
						HANDLE hFile = CreateFile(szPath, READ_CONTROL | WRITE_DAC, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
						if (hFile != INVALID_HANDLE_VALUE) SetSecurityInfo(hFile, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pNewDACL, NULL);
						CloseHandle(hFile);
					}
				} while (FindNextFile(hFind, &wfd));
				FindClose(hFind);
			}
		}
	}
	FreeSid(pSid);
	LocalFree(pNewDACL);
	LocalFree(pSD);

	return TRUE;

}

