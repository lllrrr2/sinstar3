#pragma once

namespace SOUI
{
	class SCandView : public SWindow
	{
		DEF_SOBJECT(SWindow,L"candidate")
	public:
		SCandView(void);
		~SCandView(void);

		void SetCandData(TCHAR cWild,const SStringT& strInput,const BYTE* pbyCandData);
	protected:
		virtual void WINAPI GetDesiredSize(SIZE *ret,int nParentWid, int nParentHei) override;
		virtual BOOL UpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo) override;
		void OnPaint(IRenderTarget *pRT);
		void OnLButtonUp(UINT nFlags,CPoint pt);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
			MSG_WM_LBUTTONUP(OnLButtonUp)
		SOUI_MSG_MAP_END()
	protected:
		SOUI_ATTRS_BEGIN()
			ATTR_STRINGT(L"index", m_strIndex, TRUE)
			ATTR_STRINGT(L"cand", m_strCand, TRUE)
			ATTR_STRINGT(L"comp", m_strComp, TRUE)
			ATTR_COLOR(L"colorIndex", m_crIndex, TRUE)
			ATTR_COLOR(L"colorCand", m_crCand[CAND_NORMAL], TRUE)
			ATTR_COLOR(L"colorCandGbk", m_crCand[CAND_GBK], TRUE)
			ATTR_COLOR(L"colorCandForecast", m_crCand[CAND_FORECAST], TRUE)
			ATTR_COLOR(L"colorCandUserDef", m_crCand[CAND_USERDEF], TRUE)
			ATTR_COLOR(L"colorCandBlendPY", m_crCand[CAND_BLENDPY], TRUE)
			ATTR_COLOR(L"colorCandUserCmd",m_crCand[CAND_USERCMD],TRUE)
			ATTR_COLOR(L"colorComp", m_crComp, TRUE)
			ATTR_COLOR(L"colorWild", m_crWild, TRUE)
			ATTR_COLOR(L"shadowColor", m_crShadow, TRUE)
			ATTR_POINT(L"shadowOffset", m_ptShadowOffset, TRUE);
			ATTR_LAYOUTSIZE(L"maxCandWidth",m_maxCandWidth,TRUE)
		SOUI_ATTRS_END()

		TCHAR	 m_cWild;
		SStringT m_strIndex;
		SStringT m_strInput;
		SStringT m_strCand;
		SStringT m_strComp;
		SLayoutSize m_maxCandWidth;

		enum {
			CAND_NORMAL=0,//normal candidate
			CAND_GBK,	  //gbk candidate
			CAND_FORECAST, //forecast candidate
			CAND_USERDEF, //user defined candidate
			CAND_USERCMD, //user command candidate
			CAND_BLENDPY, //pinyin candidate

			CAND_COUNT,
		};
		COLORREF m_crIndex;
		COLORREF m_crCand[CAND_COUNT];
		COLORREF m_crComp;
		COLORREF m_crWild;

		BYTE	 m_byRate;
		bool	 m_bGbk;
		COLORREF m_crShadow;
		CPoint   m_ptShadowOffset;
	};

}
