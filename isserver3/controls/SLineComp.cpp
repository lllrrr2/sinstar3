#include "stdafx.h"
#include "SLineComp.h"

namespace SOUI
{
	const TCHAR *KBiHuaMap[] = {
		_T("һ"),
		_T("ح"),
		_T("د"),
		_T("�T"),
		_T("�^"),
		_T("*"),
	};

	SLineComp::SLineComp()
	{
	}


	SLineComp::~SLineComp()
	{
	}

	const int KWnd_MaxSize = 0x7fffff;

	void SLineComp::GetDesiredSize(SIZE *szRet,int nParentWid, int nParentHei)
	{
		szRet->cx = szRet->cy = KWnd_MaxSize;
		if (GetLayoutParam()->IsSpecifiedSize(Horz))
		{//������ô�С
			szRet->cx = GetLayoutParam()->GetSpecifiedSize(Horz).toPixelSize(GetScale());
		}
		else if (GetLayoutParam()->IsMatchParent(Horz))
		{
			szRet->cx = nParentWid;
		}

		if (GetLayoutParam()->IsSpecifiedSize(Vert))
		{//������ô�С
			szRet->cy = GetLayoutParam()->GetSpecifiedSize(Vert).toPixelSize(GetScale());
		}
		else if (GetLayoutParam()->IsMatchParent(Vert))
		{
			szRet->cy = nParentHei;
		}

		if (szRet->cx != KWnd_MaxSize && szRet->cy != KWnd_MaxSize)
			return;

		CSize szExp;
		if (m_lines)
		{
			CSize szWord = m_lines->GetSkinSize();
			szWord.cx *= GetWindowText().GetLength();
			szExp = szWord;
		}
		else
		{
			CAutoRefPtr<IRenderTarget> pRT;
			GETRENDERFACTORY->CreateRenderTarget(&pRT, 0, 0);
			BeforePaintEx(pRT);

			CRect rcTest4Text(0, 0, KWnd_MaxSize, KWnd_MaxSize);
			SStringT strMappedComp = GetMappedComp();
			DrawText(pRT, strMappedComp, strMappedComp.GetLength(), rcTest4Text, DT_CALCRECT);

			szExp = rcTest4Text.Size();
		}

		CRect rcTest(CPoint(), szExp);
		rcTest.InflateRect(GetStyle().GetMargin());
		rcTest.InflateRect(GetStyle().GetPadding());

		if (GetLayoutParam()->IsWrapContent(Horz))
			szRet->cx = rcTest.Width();
		if (GetLayoutParam()->IsWrapContent(Vert))
			szRet->cy = rcTest.Height();
	}

	void SLineComp::OnPaint(IRenderTarget *pRT)
	{
		CRect rcText;
		GetTextRect(rcText);

		UINT uAlign = GetTextAlign();
		if (m_lines)
		{
			SStringT strTxt = GetWindowText();
			CSize szSkin = m_lines->GetSkinSize();
			CPoint pt = rcText.TopLeft();
			if(uAlign & DT_VCENTER)
				pt.y += (rcText.Height() - szSkin.cy) / 2;
			else if(uAlign & DT_BOTTOM)
				pt.y = rcText.bottom - szSkin.cy;

			CRect rcWord(pt, szSkin);
			for (int i = 0; i < strTxt.GetLength(); i++)
			{
				m_lines->DrawByIndex(pRT, rcWord, strTxt[i] - '1');
				rcWord.OffsetRect(rcWord.Width(), 0);
			}
		}
		else
		{
			SPainter painter;
			BeforePaint(pRT, painter);
			SStringT strComp = GetMappedComp();
			DrawText(pRT, strComp, strComp.GetLength(), rcText, uAlign);
			AfterPaint(pRT, painter);
		}
	}
	SStringT SLineComp::GetMappedComp()
	{
		SStringT strComp = GetWindowText();
		SStringT strMapedComp;
		for (int i = 0; i < strComp.GetLength(); i++)
		{
			int idx = strComp[i] - '1';
			SASSERT(idx >= 0 && idx < 6);//'1'->'6'
			strMapedComp += KBiHuaMap[idx];
		}
		return strMapedComp;
	}
}
