#pragma once

#include "inputContext.h"

interface ICmdListener {
	virtual void OnCommand(WORD cmd, LPARAM lp) = 0;
	virtual InputContext * GetInputContext() = 0;
};

interface IInputListener: public ICmdListener {
	virtual BOOL IsCompositing() const = 0;
	virtual HWND GetHwnd() const = 0;
	virtual void OnInputStart() = 0;
	virtual void OnInputResult(const SStringW & strResult,const SStringW & strComp=SStringW())=0;
	virtual void OnInputEnd() = 0;
	virtual void UpdateInputWnd() = 0;
	virtual void OnCapital(BOOL bCap) = 0;
	virtual BOOL GoNextCandidatePage() = 0;
	virtual BOOL GoPrevCandidatePage() = 0;
	virtual short SelectCandidate(short iCand)=0;

	virtual void OpenInputWnd() = 0;
	virtual void CloseInputWnd(int nDelayMS) = 0;
	virtual void SetOpenStatus(BOOL bOpen)=0;
	virtual BOOL GetOpenStatus() const =0;

	virtual void EnableInput(BOOL bEnable) = 0;
	virtual BOOL IsInputEnable() const = 0;
	virtual void DelayCaretLeft() = 0;
};

class CInputState
{
	friend class CSinstar3Impl;
public:
	CInputState(void);
	~CInputState(void);

	InputContext * GetInputContext() {return &m_ctx;}
	void SetInputListener(IInputListener * pListener){m_pListener=pListener;}

	BOOL TestKeyDown(UINT uKey,LPARAM lKeyData,const BYTE * lpbKeyState);
	BOOL HandleKeyDown(UINT vKey,UINT uScanCode,const BYTE * lpbKeyState);

	void OnImeSelect(BOOL bSelect);
	void OnSetFocus(BOOL bFocus);
	BOOL OnSvrNotify(UINT wp, PMSGDATA pMsgData);
	
	BOOL IsTempSpell() const;

	void ClearContext(UINT ccMask);

protected:
	void SetOpenStatus(BOOL bOpen);
protected:
	void InputStart();
	BOOL InputResult(const SStringW &strResult,BYTE byAstMask);
	void InputEnd();
	void InputUpdate();
	void InputOpen();
	void InputHide(BOOL bDelay = FALSE);
	void StatusbarUpdate();

	int Tips_Next(BOOL bSpell, TCHAR *pszBuf, int iTip, bool bNext = true);
private:
	int TestHotKey(UINT uVk, const BYTE * lpbKeyState) const;
	void Tips_Init();
	int Tips_Rand(BOOL bSpell, TCHAR * pszBuf);
	
	void TurnToTempSpell();

	BYTE GetKeyinMask(BOOL bAssociate,BYTE byMask);

	BOOL KeyIn_RepeatInput(InputContext * lpCntxtPriv,const BYTE * lpbKeyState);
	BOOL KeyIn_Test_RepeatInput(InputContext * lpCntxtPriv,const BYTE * lpbKeyState);
	BOOL KeyIn_Test_FuncKey(UINT uKey,LPARAM lKeyData,const BYTE * lpbKeyState);
	void KeyIn_Spell_UpdateCandList(InputContext * lpCntxtPriv,BYTE byCaret);
	void KeyIn_Spell_Forecast(InputContext * lpCntxtPriv,BYTE byStartPos);
	BOOL KeyIn_Spell_MoveCaret(InputContext *lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	BOOL KeyIn_Spell_SyFix(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	BOOL KeyIn_Spell_Normal(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	BOOL KeyIn_Spell_ChangeComp(InputContext* lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	BOOL KeyIn_Spell_InputText(InputContext* lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	BOOL KeyIn_Spell_GetSpellInput(InputContext * lpCntxtPriv,BYTE bySpellID[MAX_SYLLABLES][2]);
	BOOL KeyIn_Spell_Locate(InputContext *lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	BOOL KeyIn_Spell_Symbol(InputContext* lpCntxtPriv,UINT byInput, CONST BYTE* lpbKeyState);

	BOOL KeyIn_All_TurnCandPage(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	BOOL KeyIn_All_SelectCand(InputContext * lpCntxtPriv,UINT byInput,char cCompLen, CONST BYTE * lpbKeyState,bool bKeepVisible=false);
	BOOL KeyIn_InputAndAssociate(InputContext * lpCntxtPriv,LPCWSTR pszInput,short sLen,BYTE byMask);
	void GetShapeComp(const WCHAR *pInput,char cLen);
	BOOL KeyIn_Code_Normal(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	BOOL KeyIn_Code_ChangeComp(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	BOOL KeyIn_Code_Symbol(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	BOOL KeyIn_All_Sentence(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	BOOL KeyIn_All_Associate(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);
	void KeyIn_Sent_Input(InputContext* lpCntxtPriv);
	BOOL KeyIn_Code_English(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE* lpbKeyState);
	BOOL KeyIn_Digital_ChangeComp(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE* lpbKeyState);
	BOOL KeyIn_UserDef_ChangeComp(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE* lpbKeyState);
	BOOL KeyIn_Line_ChangeComp(InputContext * lpCntxtPriv,UINT byInput, CONST BYTE * lpbKeyState);

	SStringW Symbol_Convert(InputContext * lpCntxtPriv,UINT byInput,const BYTE * lpbKeyState);
	InputContext m_ctx;
	IInputListener * m_pListener;
	BYTE * m_pbyMsgBuf;
	BOOL		 m_fOpen;

	enum TipType {
		TT_SHAPE=0,
		TT_SPELL,
		TT_BOTH,
		TT_COUNT,
	};

	SArray<SStringT>  m_tips[TT_COUNT];
	BOOL			 m_bUpdateTips;

	DWORD			 m_tmInputStart;
	DWORD			 m_tmInputEnd;

	BOOL m_bPressOther;		//flag for other key is down.
	BOOL m_bReleaseOther;   //flag for other key had been down.
	BOOL m_bPressShift;     //flag for shift key down
	BOOL m_bPressCtrl;      //flag for ctrl key down
};
