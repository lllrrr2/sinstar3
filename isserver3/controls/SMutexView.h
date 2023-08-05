#pragma once

namespace SOUI
{
	class SMutexView : public SWindow
	{
		DEF_SOBJECT(SWindow,L"mutexView")
	public:
		SMutexView(void);
		~SMutexView(void);

	protected:
		void OnShowWindow(BOOL bShow, UINT nStatus);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_SHOWWINDOW(OnShowWindow)
		SOUI_MSG_MAP_END()
	};

}
