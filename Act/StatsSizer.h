// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   Имя файла:		StatsSizer.h
   Version:			v1.00
   Created:			23/10/2009 by Sergey Mikhtonyuk
   Описание:  Sizer implementation that used by game statistics
   -------------------------------------------------------------------------
   История:
*************************************************************************/
#ifndef   _STATSSIZER_H__
#define   _STATSSIZER_H__

#if _MSC_VER > 1000
	#pragma once
#endif

class CStatsSizer : public IDrxSizer
{
public:
	CStatsSizer();
	virtual void                Release();
	virtual size_t              GetTotalSize();
	virtual size_t              GetObjectCount();
	virtual bool                AddObject(ukk pIdentifier, size_t nSizeBytes, i32 nCount);
	virtual IResourceCollector* GetResourceCollector();
	virtual void                SetResourceCollector(IResourceCollector* pColl);
	virtual void                Push(tukk szComponentName);
	virtual void                PushSubcomponent(tukk szSubcomponentName);
	virtual void                Pop();
	virtual void                Reset();
	virtual void                End();

private:
	size_t m_count, m_size;
};

#endif // __STATSSIZER_H__
