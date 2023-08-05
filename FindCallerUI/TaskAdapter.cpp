#include "stdafx.h"
#include "TaskAdapter.h"

namespace SOUI {
	CTaskAdapter::CTaskAdapter()
	{
	}


	CTaskAdapter::~CTaskAdapter()
	{
	}

	bool CTaskAdapter::AddTask(TASKINFO ti)
	{
		for(int i=0;i<m_arrTasks.GetCount();i++)
		{
			if(m_arrTasks[i].pid == ti.pid)
			{
				m_arrTasks[i] = ti;
				return false;
			}
		}
		m_arrTasks.Add(ti);
		return true;
	}

	TASKINFO * CTaskAdapter::GetTask(int iTask)
	{
		if (iTask >= m_arrTasks.GetCount())
			return NULL;
		return &m_arrTasks[iTask];
	}

	BOOL CTaskAdapter::DelTask(int iTask)
	{
		if (iTask >= m_arrTasks.GetCount())
			return FALSE;
		m_arrTasks.RemoveAt(iTask);
		notifyDataSetChanged();
		return TRUE;
	}

	int CTaskAdapter::getCount()
	{
		return m_arrTasks.GetCount();
	}

	void CTaskAdapter::getView(int position, IWindow * pItem, IXmlNode * xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}

		TASKINFO &ti = m_arrTasks[position];
		pItem->FindIChildByID(R.id.txt_pid)->SetWindowText(SStringT().Format(_T("%u"),ti.pid));
		pItem->FindIChildByID(R.id.txt_name)->SetWindowText(ti.szName);
		pItem->FindIChildByID(R.id.txt_path)->SetWindowText(ti.szPath);
	}

	//ɾ��һ�У��ṩ�ⲿ���á�

	void CTaskAdapter::DeleteItem(int iPosition)
	{
		if (iPosition >= 0 && iPosition < getCount())
		{
			m_arrTasks.RemoveAt(iPosition);
			notifyDataSetChanged();
		}
	}

	SStringW CTaskAdapter::GetColumnName(int iCol) const {
		LPCWSTR KColNames[] = {
			L"col_pid",
			L"col_name",
			L"col_path",
		};
		return KColNames[iCol];
	}

	 BOOL CTaskAdapter::OnSort(int iCol, UINT * stFlags, int nCols)
	{
		return TRUE;
	}

	 int CTaskAdapter::SortCmp(void * context, const void * p1, const void * p2)
	{

		return 0;
	}

	 void CTaskAdapter::RemoveAll()
	 {
		 m_arrTasks.RemoveAll();
		 notifyDataSetChanged();
	 }

}

