#pragma once
#include <core/swnd.h>

#include "SSkinAni.h"

namespace SOUI
{

    /**
    * @class     SGifPlayer
    * @brief     GIFͼƬ��ʾ�ؼ�
    * 
    * Describe
    */
    class SApngPlayer : public SWindow , public ITimelineHandler
    {
        DEF_SOBJECT(SWindow, L"apngPlayer")
    public:
        SApngPlayer();
        ~SApngPlayer();

		BOOL PlayFile(LPCTSTR pszFileName);

    protected://SWindow���麯��
        void WINAPI GetDesiredSize(SIZE *ret, int nWid,int nHei ) override;
		void OnScaleChanged(int scale) override;
    protected://ITimerLineHander
        virtual void WINAPI OnNextFrame();

    public://���Դ���
        SOUI_ATTRS_BEGIN()		
            ATTR_CUSTOM(L"skin", OnAttrSkin) //Ϊ�ؼ��ṩһ��skin���ԣ���������SSkinObj�����name
        SOUI_ATTRS_END()
    protected:
        HRESULT OnAttrSkin(const SStringW & strValue, BOOL bLoading);
        
    protected://��Ϣ����SOUI�ؼ�����Ϣ�����WTL��MFC�����ƣ��������Ƶ�ӳ�����ͬ�������Ƶ���Ϣӳ���
        
        /**
         * OnPaint
         * @brief    ���ڻ�����Ϣ��Ӧ����
         * @param    IRenderTarget * pRT --  ����Ŀ��
         * @return   void
         * Describe  ע������Ĳ�����IRenderTarget *,������WTL��ʹ�õ�HDC��ͬʱ��Ϣӳ���Ҳ��ΪMSG_WM_PAINT_EX
         */    
        void OnPaint(IRenderTarget *pRT);

        /**
         * OnShowWindow
         * @brief    ��������ʾ��Ϣ
         * @param    BOOL bShow --  true:��ʾ
         * @param    UINT nStatus --  ��ʾԭ��
         * @return   void 
         * Describe  �ο�MSDN��WM_SHOWWINDOW��Ϣ
         */    
        void OnShowWindow(BOOL bShow, UINT nStatus);
    
        void OnDestroy();

        //SOUI�ؼ���Ϣӳ���
        SOUI_MSG_MAP_BEGIN()	
            MSG_WM_PAINT_EX(OnPaint)    //���ڻ�����Ϣ
            MSG_WM_SHOWWINDOW(OnShowWindow)//������ʾ״̬��Ϣ
            MSG_WM_DESTROY(OnDestroy)
        SOUI_MSG_MAP_END()	

    protected:
        SAutoRefPtr<SSkinAni> m_aniSkin;
        int	m_iCurFrame;
        int     m_nNextInterval;
    };

}