// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Gameside struct for locking when accessing data that is access from a thread

-------------------------------------------------------------------------
История:
- 15:03:2010 : Created By Ben Parbury

*************************************************************************/
#pragma once

template <class T, class D>
struct SAutoLockData
{
	SAutoLockData(T* pClass, bool tryLock = false)
	{
		if(tryLock)
		{
			if(pClass->___m_mutex.TryLock())
			{
				m_pData = &pClass->___m_private;
				m_pClass = pClass;
			}
			else
			{
				m_pData = NULL;
			}
		}
		else
		{
			pClass->___m_mutex.Lock();
			m_pData = &pClass->___m_private;
			m_pClass = pClass;
		}
	}

	~SAutoLockData()
	{
		if(m_pData)
		{
			m_pClass->___m_mutex.Unlock();
		}
	}

	ILINE bool IsValid()
	{
		return m_pData != NULL;
	}

	ILINE D* Get()
	{
		DRX_ASSERT(m_pData);
		return m_pData;
	}

	ILINE const D* Get() const
	{
		DRX_ASSERT(m_pData);
		return m_pData;
	}

	D* m_pData;
	T* m_pClass;
};

