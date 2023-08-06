#pragma once

#include <helper/obj-ref-impl.hpp>

namespace SOUI
{
    class SRealWndHandler_Scintilla :public TObjRefImpl2<IRealWndHandler,SRealWndHandler_Scintilla>
    {
    public:
        SRealWndHandler_Scintilla(void);
        ~SRealWndHandler_Scintilla(void);

        /**
         * SRealWnd::OnRealWndCreate
         * @brief    �����洰��
         * @param    SRealWnd * pRealWnd --  ����ָ��
         * @return   HWND -- �����������洰�ھ��
         * Describe  
         */    
        virtual HWND WINAPI OnRealWndCreate(IWindow *pRealWnd);

        /**
        * SRealWnd::OnRealWndDestroy
        * @brief    ���ٴ���
        * @param    SRealWnd *pRealWnd -- ����ָ��
        *
        * Describe  ���ٴ���
        */
        virtual void WINAPI OnRealWndDestroy(IWindow *pRealWnd);

        /**
        * SRealWnd::OnRealWndInit
        * @brief    ��ʼ������
        * @param    SRealWnd *pRealWnd -- ����ָ��
        *
        * Describe  ��ʼ������
        */
        virtual BOOL WINAPI OnRealWndInit(IWindow *pRealWnd);

        /**
        * SRealWnd::OnRealWndSize
        * @brief    �������ڴ�С
        * @param    SRealWnd *pRealWnd -- ����ָ��
        * @return   BOOL -- TRUE:�û������ڵ��ƶ���FALSE������SOUI�Լ�����
        * Describe  �������ڴ�С, ��pRealWnd�л�ô���λ�á�
        */
        virtual BOOL WINAPI OnRealWndSize(IWindow *pRealWnd);

		virtual BOOL WINAPI OnRealWndPosition(IWindow *pRealWnd, const RECT *rcWnd);

	};

}
