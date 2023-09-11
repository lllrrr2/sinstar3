#pragma once

typedef enum tagPROGTYPE { PT_MAX = 0, PT_PROG, PT_END } PROGTYPE;

#pragma pack(push,1)
typedef struct tagBLURINFO2
{//�ʺ������������еĽṹ
	short sCount;	//ƴ������
	short sIndex;	//��������ȫƥ���ƴ�����,-1����ȫ����ģ����
	BYTE byBlurSpell[1][2];
}BLURINFO2,*PBLURINFO2;
#pragma pack(pop)

typedef struct tagGROUPINFO
{//����Ϣ
	WCHAR szName[50];	//����
	WCHAR szEditor[50];	//�༭
	WCHAR szRemark[200];	//��ע
	DWORD dwCount;		//����
	BOOL  bValid;		//��Ч��־
}GROUPINFO,*PGROUPINFO;


//����֮��ͨѶʹ�õ��ڴ��ļ�ӳ�������
#define NAME_REQ_MAPFILE	_T("_sinstar3 req map file")	//�ͻ�����������������������ͨ��
#define MAX_BUF_REQ_MAP		1<<12						//4K						
#define MAX_BUF_REQ			(MAX_BUF_REQ_MAP)									
#define NAME_ACK_MAPFILE	_T("_sinstar3 ack map file")	//��������ͻ��˷���ָʾ������ͨ��
#define MAX_BUF_ACK_MAP		1<<15						//32K
#define MAX_BUF_ACK			((MAX_BUF_ACK_MAP)-sizeof(HWND))

#define MSG_NAME_SINSTAR3		_T("_sinstar3 communicate msg")	//ͨѶʱʹ�õ���Ϣ���ƣ�ȡ���ϰ汾�̶�����ϢID�������������������

#define  CLS_SINSTAR3_IME_WND _T("sinstar3_uiwnd")	//the class name must like this. otherwise the ime module maybe crash!!!
#define  UM_GETPROCPATH  (WM_USER+5000)
#define  CDT_RET_PROCPATH (2134)
#define  UM_RECONN		 (WM_USER+5001)

#pragma pack(push,1)

typedef struct tagMSGDATA{
	short	sSize;		//�������ݳ���
	BYTE	byData[1];	//����������ʼ��ַ
}MSGDATA,*PMSGDATA;


typedef struct tagFLMINFO
{
	WCHAR szName[50];
	WCHAR szAddition[50];
	WCHAR szAddFont[LF_FACESIZE+1];
}FLMINFO,*PFLMINFO;
#pragma pack(pop)


typedef struct tagCOMPHEAD
{
	WCHAR szName[50];	//����
	WCHAR szCode[50];	//��Ԫ
	WCHAR cWildChar;		//���ܼ�
	char cCodeMax;		//����볤
	WCHAR szUrl[50];		//�����ַ
	DWORD dwWords;		//�Ѿ������ַ���
	BOOL bSymbolFirst;	//����ױ���
	DWORD dwEncrypt : 1;	//���ܱ�־
	DWORD dwYinXingMa : 1;	//�������Ż���־
	DWORD dwPhraseCompPart : 1;	//������벻һ����ȫ��
	DWORD dwAutoSelect : 1;	//��һ�����Զ�����
	DWORD dwLicense : 1;		//������Ҫ��Ȩ����ʹ��
	DWORD dwReserved : 27;	//����,��ʼ��Ϊ0
}COMPHEAD;

#define CISIZE_BASE		164		//����������Ϣ������

#define MAX_SENTLEN		50		//�����󳤶�
#define MAX_WORDLEN		30		//������󳤶�
#define MAX_COMPLEN		20		//ϵͳ֧�ֵ�����볤

#define RATE_USERDEF	0xFF	//�Զ������ʹ�õĴ�Ƶ
#define RATE_FORECAST	0xFE	//����Ԥ��õ��Ĵ�ʹ�õĴ�Ƶ
#define RATE_USERCMD	0xFC	//Command ID
#define RATE_MIXSP		0xFB	//��ƴ����
#define RATE_WILD		0xFA	//���ܼ���ѯ���
#define RATE_ASSOCIATE	0xF9	//��������
#define RATE_USERJM		0xF8	//user defined jm.
#define RATE_USERDICT	0xF7	//user lib

//ϵͳ��Ӧ
#define ISACK_SUCCESS	0
#define ISACK_DELAY		1	
#define ISACK_UNKNOWN	100		//����ʶ�����Ϣ
#define ISACK_ERRCLIENT	101		//����Ŀͻ����ھ��
#define ISACK_SVRCANCEL	102		//�������쳣�˳�
#define ISACK_UNINIT	103		//ͨѶû��������ʼ��
#define ISACK_ERRBUF	104		//���ݳ��ȴ���
#define ISACK_ERROR		105		//һ���Դ���

//��ȡ��������ⷵ��ֵ
#define MCR_NORMAL		0		//������ȡ����
#define MCR_ASSOCIATE	1		//������ʾ
#define MCR_LONGERCOMP	(1<<1)	//�����Ե�ǰ����Ϊǰ׺�ĳ��Զ������
#define MCR_MIXSP		(1<<2)	//���ƴ��
#define MCR_USERDEF		(1<<3)	//����Զ�������
#define MCR_WILD		(1<<4)	//���ܼ��Ĳ�ѯ
#define MCR_AUTOSELECT	(1<<5)	//Ψһ������
#define MCR_FORECAST	(1<<6)  //Ԥ����

//����mask
#define MKI_ASTCAND		1		//��������
#define MKI_ASTSENT		(1<<1)	//��������
#define MKI_ASTENGLISH	(1<<2)	//Ӣ������
#define MKI_RECORD		(1<<3)	//��¼����
#define MKI_AUTOPICK	(1<<4)	//����ѡ��ʱ����������ӵĿ���
#define MKI_TTSINPUT	(1<<5)	//�ʶ���������
#define MKI_PHRASEREMIND (1<<6)	//��������

//��ѯ����mask
#define MQC_NORMAL		0		//��ѯ��ͨ����
#define MQC_FORECAST	1		//��ѯ�����Ԥ����
#define MQC_FCNOCAND	(1<<1)	//�޺�ѡ��ʱԤ��
#define MQC_MATCH		(1<<2)	//�Զ�����ƥ���������Ϊ��ѡ��
#define MQC_ONLYSC		(1<<3)	//���򲻳�ȫ
#define MQC_USERDEF		(1<<4)	//ͬʱ��ѯ�Զ������
#define MQC_AUTOPROMPT	(1<<5)	//���ܱ�����ʾ
#define MQC_SPCAND		(1<<6)	//����Զ�������

//TTSѡ��
#define MTTS_EN			1		//ʹ��Ӣ������	1
#define MTTS_CH			2		//ʹ����������

//rateAdjust mode
#define RAM_AUTO		0		//���ܵ�Ƶ
#define RAM_FAST		1		//���ٵ�Ƶ

#define FU_USERDEF	1		//�����û������ļ�
#define FU_SYMBOL   2  //user define symbol
#define FU_USERCMD	3	//��������ļ�����
#define FU_USERJM   4  //user define jm map
#define FU_USERDICT 5  //user dict
#define FU_SENTENCE 6  //sentence

/////////////////////////////////////////////////////////////////////////////
//�������㲥����Ϣ
/////////////////////////////////////////////////////////////////////////////
#define UM_SVR_NOTIFY	(WM_USER+9000)
//----------------------------------------------
//	�������˳�
//----------------------------------------------
#define NT_SERVEREXIT	100

//----------------------------------------------
//	��ǰ�������
//----------------------------------------------
#define NT_COMPINFO		101

//----------------------------------------------
//	��ǰӢ�Ŀ����
//----------------------------------------------
#define NT_FLMINFO  	102

//----------------------------------------------
//	�������
//----------------------------------------------
#define NT_COMPERROR	103

//----------------------------------------------
//	�����������굱ǰ����
//----------------------------------------------
#define NT_KEYIN	110