#pragma once

#define CODE_RESTORE	100

#define CISIZE_BASE		164		//����������Ϣ������

#define MAX_SENTLEN		50		//�����󳤶�
#define MAX_WORDLEN		30		//������󳤶�
#define MAX_COMPLEN		20		//ϵͳ֧�ֵ�����볤

#define RATE_USERDEF	0xFF	//�Զ������ʹ�õĴ�Ƶ
#define RATE_FORECAST	0xFE	//����Ԥ��õ��Ĵ�ʹ�õĴ�Ƶ
#define RATE_GBK		0xFD	//GBK flag
#define RATE_USERCMD	0xFC	//Command ID
#define RATE_MIXSP		0xFB	//��ƴ����
#define RATE_WILD		0xFA	//���ܼ���ѯ���
#define RATE_ASSOCIATE	0xF9	//��������
#define RATE_USERJM		0xF8	//user defined jm.
#define RATE_USERDICT	0xF7	//user dict

enum{
	SENDER_SINSTAR3	= 100,
	SENDER_SKINMANAGER,
};


enum{
	EVENT_BASE = EVT_EXTERNAL_BEGIN,
	EVENT_DRAGMOVE,
	EVENT_SETSKIN,
	EVENT_SVRNOTIFY,
	EVENT_CHECK_UPDATE_RESULT,
	EVENT_CAND_CLICK,
	EVENT_QUERY_TIP,
};

namespace SOUI
{
	DEF_EVT_EXT(EventSvrNotify,EVENT_SVRNOTIFY,{
		WPARAM wp;
		LPARAM lp;
	})

	DEF_EVT_EXT(EventSetSkin,EVENT_SETSKIN,{int fake;})

	DEF_EVT_EXT(EventCheckUpdateResult,EVENT_CHECK_UPDATE_RESULT,{
		bool    bServerOK;
		bool	bManual;
		SStringW strUrl;
		SStringW strInfo;
		SStringW strNewUpdateUrl;
		DWORD	 dwNewVer;
		DWORD    dwCurVer;
	})

	DEF_EVT_EXT(EventCandClick,EVENT_CAND_CLICK,{
		SStringW strText;
		BYTE	 byRate;
	})
	

	DEF_EVT_EXT(EventQueryTip,EVENT_QUERY_TIP,{
		SStringW strText;
		SStringW strTip;
	})

}

