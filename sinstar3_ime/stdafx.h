// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "..\targetver.h"

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ���ų�����ʹ�õ�����
#define ENABLE_LANGUAGEBAR
#define _CRT_NON_CONFORMING_SWPRINTFS
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif//_CRT_SECURE_NO_WARNINGS
#define LIB_SOUI_COM		//using lib to import soui com objects.

// Windows ͷ�ļ�:
#define NOIME
#include <windows.h>
#include <tchar.h>
#include <crtdbg.h>

#include "imm.h"
#pragma comment(lib,"imm32.lib")

#include "imeContext.h"

#ifndef SASSERT
#include <assert.h>
#define SASSERT(x) assert(x)
#endif

#include "../include/version.h"
#include "../include/sinstar-i.h"
#include "../include/reg.h"

#include "../slog/slog.h"
#define kLogTag "sinstar3_ime"

#include "ImeModule.h"

