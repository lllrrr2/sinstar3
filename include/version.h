#pragma once

//
// ���ǰѰ汾��Ϣ�޸ļ��е���һ���ط���
// ������ʹ�õ��ں˶������ƣ��汾��Ϣ�����Ժ궨�壬������4���汾��Ϣ�ֶ�չ�����ɣ�
// Ҫ�ı�汾�ţ�ֻ��Ҫ����4���ֶν����޸ġ�
//

#define COPYRIGHT_STR "Copy right (C) 2003-2030"
#define _sinstar3_ime_version_a		4
#define _sinstar3_ime_version_b		0
#define _sinstar3_ime_version_c		0
#define _sinstar3_ime_version_d		0

#define _sinstar3_ime_name			�������뷨
#define _sinstar3_ime		 		�������뷨IME
#define _sinstar3_tsf				�������뷨TSF


#define _sinstar3_ime_string(a)		#a
#define sinstar3_ime_string(a)			_sinstar3_ime_string(a)
#define _sinstar3_ime_tstring(a)		_T(#a)
#define sinstar3_ime_tstring(a)		_sinstar3_ime_tstring(a)
#define _sinstar3_ime_wstring(a)		L#a
#define sinstar3_ime_wstring(a)		_sinstar3_ime_wstring(a)

#define _sinstar3_ime_int_version(a,b,c,d)		a,b,c,d
#define sinstar3_ime_int_version(a,b,c,d)		_sinstar3_ime_int_version(a,b,c,d)

#define _sinstar3_ime_str_version(a,b,c,d)		#a "." #b "." #c "." #d
#define sinstar3_ime_str_version(a,b,c,d)		_sinstar3_ime_str_version(a,b,c,d)

#define _sinstar3_ime_str_version2(a,b,c,d)	_T(#a) _T(".") _T(#b) _T(".") _T(#c) _T(".") _T(#d)
#define sinstar3_ime_str_version2(a,b,c,d)		_sinstar3_ime_str_version2(a,b,c,d)

#define VERSION_INT		sinstar3_ime_int_version( _sinstar3_ime_version_a, _sinstar3_ime_version_b, _sinstar3_ime_version_c, _sinstar3_ime_version_d)
#define VERSION_STR		sinstar3_ime_str_version( _sinstar3_ime_version_a, _sinstar3_ime_version_b, _sinstar3_ime_version_c, _sinstar3_ime_version_d)
#define VERSION_TSTR	sinstar3_ime_str_version2( _sinstar3_ime_version_a, _sinstar3_ime_version_b, _sinstar3_ime_version_c, _sinstar3_ime_version_d)
#define PRODUCT_NAME	sinstar3_ime_tstring( _sinstar3_ime_name)
#define PRODUCT_WNAME	sinstar3_ime_wstring( _sinstar3_ime_name)

#define PRODUCT_IME	sinstar3_ime_tstring( _sinstar3_ime)
#define PRODUCT_TSF	sinstar3_ime_wstring( _sinstar3_tsf)


#define _sinstar3_str_name_version(n,a,b)		#n #a "." #b
#define sinstar3_str_name_version(n,a,b)		_sinstar3_str_name_version(n,a,b)
#define PRODUCT_NAMEVER		sinstar3_str_name_version( _sinstar3_ime_name,_sinstar3_ime_version_a, _sinstar3_ime_version_b)

#define _sinstar3_tstr_name_version(n,a,b)		_T(#n) _T(#a) _T(".") _T(#b)
#define sinstar3_tstr_name_version(n,a,b)		_sinstar3_tstr_name_version(n,a,b)
#define PRODUCT_TNAMEVER		sinstar3_tstr_name_version( _sinstar3_ime_name,_sinstar3_ime_version_a, _sinstar3_ime_version_b)

#define _sinstar3_wstr_name_version(n,a,b)		L#n L#a L"." L#b
#define sinstar3_wstr_name_version(n,a,b)		_sinstar3_wstr_name_version(n,a,b)
#define PRODUCT_WNAMEVER		sinstar3_wstr_name_version( _sinstar3_ime_name,_sinstar3_ime_version_a, _sinstar3_ime_version_b)

