// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IFunctorBase.h
//  Version:     v1.00
//  Created:     05/30/2013 by Paulo Zaffari.
//  Описание: Base header for multi DLL functors.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IFUNCTORBASE_H_
#define __IFUNCTORBASE_H_

#pragma once

//! Base class for functor storage. Not intended for direct usage.
class IFunctorBase
{
public:
	IFunctorBase() : m_nReferences(0){}
	virtual ~IFunctorBase(){};
	virtual void Call() = 0;

	void         AddRef()
	{
		DrxInterlockedIncrement(&m_nReferences);
	}

	void Release()
	{
		if (DrxInterlockedDecrement(&m_nReferences) <= 0)
		{
			delete this;
		}
	}

protected:
	 i32 m_nReferences;
};

//! Base Template for specialization. Not intended for direct usage.
template<typename tType> class TFunctor : public IFunctorBase
{
};

#endif //__IFUNCTORBASE_H_
