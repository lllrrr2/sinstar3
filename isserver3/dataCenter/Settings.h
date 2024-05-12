#pragma once


enum HotKeyIndex{
	HKI_CharMode = 0,	// ���ģʽ�л�
	HKI_Query,			// ��ѯ�ȼ�
	HKI_Mode,			// ���ƴ��ģʽ�л�
	HKI_EnSwitch,		// Ӣ�����뿪��
	HKI_MakePhrase,		// ����ȼ�
	HKI_ShowRoot,		// �ָ�����ʾ�ȼ�
	HKI_HideStatus,		// ����״̬��
	HKI_FilterGbk,		// filter gbk
	HKI_Record,			// record input
	HKI_TTS,			// tts
	HKI_UDMode,			// user define mode
	HKI_TempSpell,		// temp spell
	HKI_Repeat,			// repeat input
	HKI_LineMode,		// line mode
	HKI_COUNT,

	HKI_AdjustRate,		// ���ٵ�Ƶ: ctrl+����� 
	HKI_DelCandidate,	// ����ɾ��: Ctrl+Shift+����� 
};

enum KeyFunction
{//shift�����ܶ���
	Fun_None=0,	//��
	Fun_Ime_Switch,//���뷨����
	Fun_Tmpsp_Switch,//
	Fun_Sel_2nd_Cand,//ѡ��2��
	Fun_Sel_3rd_Cand,//ѡ��3��
};

enum KeyScanCode{
	Left_Shift = 0x2a,
	Right_Shift = 0x36,
	Left_Ctrl = 0xc0,
	Right_Ctrl = 0xc1,
};

class CModifiedMark
{
public:
	CModifiedMark():m_bModified(false){}
	bool IsModified() const{
		return m_bModified;
	}
	void SetModified(bool bModified)
	{
		m_bModified = bModified;
	}
private:
	bool m_bModified;
};
class CSettingsGlobal: public CModifiedMark
{
public:
	/*GBK������ʾ����ģʽ*/
	enum GbkMode
	{
		GBK_HIDE = 0,	//��GB����ʱ�Զ�����
		GBK_SHOW_MANUAL,//��ʾ�ֶ�����
		GBK_SHOW_NORMAL,//��ʾ��������
	};

	enum InputMode
	{
	    manul_input=0,
		auto_input =1,
		next_input = 2,
	};
	void Load(const SStringT & strDataPath);
	void Save(const SStringT & strDataPath);
	int		compMode;		//��ǰ��������
	BOOL	b23CandKey;			// ;+'ѡ23����
	BYTE	by2CandVK;			// 2 Cand key
	BYTE	by3CandVK;			// 3 Cand Key
	BYTE	byForecast;			// forecast mask
	WORD	byLineKey[6];		// �ʻ����뷨ת����ֵ
	BOOL	bShowOpTip;			// ��ʾ������ʾ����
	GbkMode	nGbkMode;			// GBK Show Mode

	BYTE	byRateAdjust;		// ��Ƶ�������� 0-��ʹ�ã��������ܵ�Ƶ���������ٵ�Ƶ
	BYTE	byAstMode;			// �������� (ASTMODE)
	BOOL	bAutoMatch;			// ����ѡ��

	BOOL	bBlendUD;			// ��ϼ����Զ�������
	BOOL	bBlendSpell;		// �������ƴ��
	InputMode	inputMode;		// ��������ģʽ

	BYTE	byTurnPageUpVK;		// �����Ϸ���
	BYTE	byTurnPageDownVK;	// �����·���
	DWORD   dwHotkeys[HKI_COUNT];

	BOOL	bPYPhraseFirst;		// ƴ����������
	BOOL	bEnterClear;		// �س���ձ���
	int		nSoundAlert;		// ������ʾ
	BOOL	bAutoDot;			// ����С����
	BOOL	bAutoPrompt;		// ����������ʾ
	BOOL	bDisableDelWordCand;// ��ֹɾ����������

	BOOL	bCandSelNoNum;		// ��������ѡ������
	BOOL	bOnlySimpleCode;	// ���򲻳�ȫ
	BOOL	bDisableFirstWild;	// ��ֹ�������ܼ�
	BOOL	bFullSpace;			// full space.
	BOOL	bAutoSpace;			// ����ʶ��ո���
	BOOL	bInitEnglish;		// init for English input.
	BYTE    bySentMode;
	int		nMaxCands;			// max candidate number.
	int		nDelayTime;			//delay timer for closing composition window in seconds.
	BOOL	bBackQuitUMode;		// ��Uģʽ����Ϊ��ʱ�����˳�Uģʽ
	BOOL	bQuitEnCancelCAP;	// �˳�Ӣ��ģʽʱ�Զ��˳���д״̬
	BOOL    bEnableLog;         // EnableLogFile
	KeyFunction m_funLeftShift;
	KeyFunction m_funRightShift;

	KeyFunction m_funLeftCtrl;
	KeyFunction m_funRightCtrl;

	SStringT strFontDesc;		//font description.

	BOOL	bEnableDebugSkin;	//enable debug skin. default is false
	SStringT   strDebugSkinPath;//debug skin path.

	SStringT   strSkin;   //skin
	POINT	ptInput;			//input window pos for not caret follow.
	POINT   ptStatus;			//status window pos

	SStringT urlForum;
	SStringT urlStatistics;

	BOOL	bShowTray;
	BOOL	bAutoQuit;
	int		nUpdateInterval;
	TCHAR   szUpdateDate[100];
	TCHAR	szUpdateUrl[MAX_PATH];
	TCHAR	szBackupDir[MAX_PATH];
	int		nTtsSpeed;
	int		iTtsChVoice, iTtsEnVoice;

	BOOL	bUsingVertLayout;
};

class CSettingsUI: public CModifiedMark
{
public:
	void Load(const SStringT & strDataPath);
	void Save(const SStringT & strDataPath);

	BOOL	bHideStatus;		// ��ǰ״̬������״̬
	BOOL	bMouseFollow;		// �����濪��
	BOOL	bEnglish;			// Ӣ�ĵ������뿪��
	BOOL	bFullStatus;		// ״̬��չ����־
	BOOL	bCharMode;			// ���ģʽ
	BOOL	bSound;				// �����϶�
	BOOL	bRecord;			// ��¼�������
	BOOL	bSentAssocite;		// ������뿪��
	BOOL	bInputBig5;			// ���������־
	BOOL    bFilterGbk;			// filter gbk
	BOOL	bUILessHideStatus;	// ��UILessģʽ���Զ�����״̬��
	BOOL	bFullScreenHideStatus;
	enum EInlineMode
	{
		INLINE_NO = 0,	//����ʾInline
		INLINE_Coms,	//��ʾ�����ַ���
		INLINE_NUMONE,	//��ѡ��һ����ѡ��
		INLINE_ONLYONE,	//��ѡΨһʱ��ʾ��ѡ�ʷ���Ϊ�����ַ���
	}enumInlineMode;
};

extern CSettingsGlobal	*g_SettingsG;
extern CSettingsUI   *g_SettingsUI;