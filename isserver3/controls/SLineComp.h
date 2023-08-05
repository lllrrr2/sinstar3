#pragma once

namespace SOUI {
	class SLineComp : public SWindow
	{
		DEF_SOBJECT(SWindow,L"lineComp")
	public:
		SLineComp();
		~SLineComp();

	protected:
		void OnPaint(IRenderTarget *pRT);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
		SOUI_MSG_MAP_END()

		SOUI_ATTRS_BEGIN()
			ATTR_SKIN(L"lineSkin", m_lines, TRUE)
		SOUI_ATTRS_END()

	protected:
		virtual void WINAPI GetDesiredSize(SIZE *ret,int nParentWid, int nParentHei);

		SStringT GetMappedComp();
	private:
		SAutoRefPtr<ISkinObj> m_lines;
	};

}

