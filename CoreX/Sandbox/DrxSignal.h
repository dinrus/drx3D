// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <vector>

#include "DrxFunction.h"

namespace DrxSignalPrivate
{
template<typename Signature>
struct Observer
{
	std::function<Signature> m_function;
	uintptr_t                m_id;

	template<typename Object, typename MemberFunction>
	void SetMemberFunction(Object* pObject, MemberFunction function)
	{
		m_function = function_cast<Signature>(WrapMemberFunction(pObject, function));
	}

	template<typename Function>
	void SetFunction(const Function& function)
	{
		m_function = function_cast<Signature>(function);
	}
};

template<typename Signature> class DrxSignalBase
{
};

template<typename... Args>
class DrxSignalBase<void(Args...)>
{
public:
	void operator()(Args... args) const
	{
		//Iterating is done as such on purpose to avoid problems if the signal is resized during iteration
		i32k count = m_observers.size();
		for (i32 i = count - 1; i >= 0; i--)
		{
			m_observers[i].m_function(args...);
		}
	}

protected:
	typedef Observer<void(Args...)> TObserver;
	std::vector<TObserver> m_observers;
};

}

//////////////////////////////////////////////////////////////////////////

template<typename Signature>
class CDrxSignal : public DrxSignalPrivate::DrxSignalBase<Signature>
{
public:

	CDrxSignal();
	~CDrxSignal();

	template<typename Object, typename MemberFunction, typename std::enable_if<DrxMemFunTraits<MemberFunction>::isMemberFunction, i32>::type* = 0>
	void Connect(Object* pObject, MemberFunction function, uintptr_t forceId = 0);

	template<typename Function, typename std::enable_if<IsCallable<Function>::value, i32>::type* = 0>
	void Connect(const Function& function, uintptr_t id = 0);

	void DisconnectObject(uk pObject);
	void DisconnectById(uintptr_t id);
	void DisconnectAll();
};

//////////////////////////////////////////////////////////////////////////

template<typename Signature>
CDrxSignal<Signature>::CDrxSignal()
{

}

template<typename Signature>
CDrxSignal<Signature>::~CDrxSignal()
{
	DisconnectAll();
}

template<typename Signature>
template<typename Object, typename MemberFunction, typename std::enable_if<DrxMemFunTraits<MemberFunction>::isMemberFunction, i32>::type*>
void CDrxSignal<Signature >::Connect(Object* pObject, MemberFunction function, uintptr_t forceId /*= 0*/)
{
	TObserver observer;
	observer.m_id = forceId == 0 ? reinterpret_cast<uintptr_t>(pObject) : forceId;
	observer.SetMemberFunction(pObject, function);
	m_observers.push_back(observer);
}

template<typename Signature>
template<typename Function, typename std::enable_if<IsCallable<Function>::value, i32>::type*>
void CDrxSignal<Signature >::Connect(const Function& function, uintptr_t id /*= 0*/)
{
	TObserver observer;
	observer.m_id = id;
	observer.SetFunction(function);
	m_observers.push_back(observer);
}

template<typename Signature>
void CDrxSignal<Signature >::DisconnectObject(uk pObject)
{
	DisconnectById(reinterpret_cast<uintptr_t>(pObject));
}

template<typename Signature>
void CDrxSignal<Signature >::DisconnectById(uintptr_t id)
{
	for (auto it = m_observers.begin(); it != m_observers.end(); )
	{
		if (it->m_id == id)
			it = m_observers.erase(it);
		else
			++it;
	}
}

template<typename Signature>
void CDrxSignal<Signature >::DisconnectAll()
{
	m_observers.clear();
}
