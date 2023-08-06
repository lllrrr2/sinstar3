#include "StdAfx.h"
#include "SSpellView.h"

namespace SOUI
{
	static LPCTSTR g_szLine[]={_T("-"),_T("|"),_T("/"),_T("\\"),_T("<"),_T("*")};

	SSpellView::SSpellView(void)
		:m_ctx(NULL)
		,m_crSpell(RGBA(200,220,255,255))
		,m_crResult(RGBA(200,200,200,255))
		,m_crEdit(RGBA(255,255,0,255))
		,m_crCaret(RGBA(51,102,204,255))
	{
	}

	SSpellView::~SSpellView(void)
	{
	}

	void SSpellView::OnPaint(IRenderTarget *pRT)
	{
		if(!m_ctx) 
			return;
		if(m_ctx->compMode!=IM_SPELL)
			return;
		if(m_ctx->bySyllables==0) 
			return;

		SPainter painter;
		BeforePaint(pRT,painter);


		CRect rcText;
		GetTextRect(rcText);
		CSize szText ;
		pRT->MeasureText(_T("A"),1,&szText);
		szText.cy +=1;//for underline.
		if(GetTextAlign()&DT_VCENTER)
		{
			rcText.DeflateRect(0,(rcText.Height()-szText.cy)/2);
		}else if(GetTextAlign()&DT_BOTTOM)
		{
			rcText.top = rcText.bottom - szText.cy;
		}

		CPoint pt = rcText.TopLeft();

		POINT pts[2];

		//�֣�����ʾԤ����
		pRT->SetTextColor(m_crResult);
		SStringT strLeft = SStringW(m_ctx->szWord,m_ctx->byCaret);
		SpTextOut(pRT,pt,strLeft);
		
		pRT->SetTextColor(m_crEdit);
		SStringT strEdit;
		if(m_ctx->spellData[m_ctx->byCaret].bySpellLen>0)
			strEdit=SStringW(m_ctx->szWord[m_ctx->byCaret]);
		else
			strEdit=L"��";
		pts[0]=pt;
		CSize sz= SpTextOut(pRT,pt,strEdit);
		pts[1]=pt;
		pts[0].y+=sz.cy +1;
		pts[1].y+=sz.cy +1;

		pRT->SetTextColor(m_crResult);
		SStringT strRight = SStringW(m_ctx->szWord+m_ctx->byCaret+1,
			m_ctx->bySyllables-m_ctx->byCaret-1);
		SpTextOut(pRT,pt,strRight);

		//��ʾ��ǰ���ڵ�ƴ��
		if(m_ctx->bySyCaret != 0xFF)
		{
			const SPELLINFO *lpSpi=m_ctx->spellData+m_ctx->byCaret;
			SStringT strLeft = SStringW(lpSpi->szSpell,m_ctx->bySyCaret);
			CSize sz ;
			pRT->MeasureText(strLeft,strLeft.GetLength(),&sz);
			pts[0].x = pt.x + sz.cx;
			pts[0].y = pt.y;
			pts[1].x = pt.x + sz.cx;
			pts[1].y = pt.y + sz.cy;
		}
		pRT->SetTextColor(m_crSpell);
		SStringT strSpell = SStringW(m_ctx->spellData[m_ctx->byCaret].szSpell,m_ctx->spellData[m_ctx->byCaret].bySpellLen);
		SpTextOut(pRT,pt,strSpell);
		//draw caret
		SAutoRefPtr<IPen> pen,oldPen;
		pRT->CreatePen(PS_SOLID,m_crCaret,1,&pen);
		pRT->SelectObject(pen,(IRenderObj**)&oldPen);
		BOOL bAntiAlias = pRT->SetAntiAlias(FALSE);
		pRT->DrawLines(pts,2);
		pRT->SetAntiAlias(bAntiAlias);
		pRT->SelectObject(oldPen);

		AfterPaint(pRT,painter);
	}

	CSize SSpellView::SpTextOut(IRenderTarget *pRT,CPoint &pt,const SStringT &str)
	{
		CSize sz;
		pRT->MeasureText(str,str.GetLength(),&sz);
		pRT->TextOut(pt.x,pt.y,str,str.GetLength());
		pt.x += sz.cx;
		return sz;
	}

	void SSpellView::GetDesiredSize(SIZE *szRet,int nParentWid, int nParentHei)
	{
		if(!m_ctx || m_ctx->compMode!=IM_SPELL) return;

		CAutoRefPtr<IRenderTarget> pRT;
		GETRENDERFACTORY->CreateRenderTarget(&pRT,0,0);
		BeforePaintEx(pRT);
		CSize sz;
		SStringT strResult = SStringW(m_ctx->szWord,m_ctx->bySyllables);
		pRT->MeasureText(strResult,strResult.GetLength(),&sz);
		*szRet = sz;
		SStringT strSpell = SStringW(m_ctx->spellData[m_ctx->byCaret].szSpell,m_ctx->spellData[m_ctx->byCaret].bySpellLen);
		pRT->MeasureText(strSpell,strSpell.GetLength(),&sz);
		szRet->cx += sz.cx;
		
		szRet->cy +=1; //for underline
	}

	void SSpellView::UpdateByContext(const InputContext *pCtx)
	{
		m_ctx = pCtx;
		Invalidate();
	}

}
