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


#define _sinstar3_ime_int_version(a,b,c,d)		a,b,c,d
#define sinstar3_ime_int_version(a,b,c,d)		_sinstar3_ime_int_version(a,b,c,d)

#define _sinstar3_ime_str_version(a,b,c,d)		#a "." #b "." #c "." #d
#define sinstar3_ime_str_version(a,b,c,d)		_sinstar3_ime_str_version(a,b,c,d)

#define _sinstar3_ime_str_version2(a,b,c,d)	_T(#a) _T(".") _T(#b) _T(".") _T(#c) _T(".") _T(#d)
#define sinstar3_ime_str_version2(a,b,c,d)		_sinstar3_ime_str_version2(a,b,c,d)

#define VERSION_INT		sinstar3_ime_int_version( _sinstar3_ime_version_a, _sinstar3_ime_version_b, _sinstar3_ime_version_c, _sinstar3_ime_version_d)
#define VERSION_STR		sinstar3_ime_str_version( _sinstar3_ime_version_a, _sinstar3_ime_version_b, _sinstar3_ime_version_c, _sinstar3_ime_version_d)


