// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------

   Описание: Список статических экземпляров. Вспомогательный класс
             для автоматической регистрации типов/данных при статической
             инициализации.

   -------------------------------------------------------------------------
   История:
   - 08:06:2012: Created by Paul Slinger

*************************************************************************/

#pragma once

template<typename TYPE> class CStaticInstanceList
{
private:

	struct SInstanceList
	{
		CStaticInstanceList* pFirstInstance = nullptr;
	};

public:

	inline CStaticInstanceList()
	{
		SInstanceList& instanceList = GetInstanceList();
		m_pNextInstance = instanceList.pFirstInstance;
		instanceList.pFirstInstance = this;
	}

	virtual ~CStaticInstanceList() {}

	static inline TYPE* GetFirstInstance()
	{
		return static_cast<TYPE*>(GetInstanceList().pFirstInstance);
	}

	inline TYPE* GetNextInstance() const
	{
		return static_cast<TYPE*>(m_pNextInstance);
	}

private:

	static inline SInstanceList& GetInstanceList()
	{
		static SInstanceList instanceList;
		return instanceList;
	}

private:

	CStaticInstanceList* m_pNextInstance;
};

#define DRX_PP_JOIN_XY_(x, y)                         x ## y
#define DRX_PP_JOIN_XY(x, y)                          DRX_PP_JOIN_XY_(x, y)

#define DRX_STATIC_AUTO_REGISTER_FUNCTION(CallbackFunctionPtr) \
	static Detail::CStaticAutoRegistrar<Detail::FunctionFirstArgType<decltype(CallbackFunctionPtr)>::type> \
		DRX_PP_JOIN_XY(g_drx_static_AutoRegistrar, __COUNTER__)(CallbackFunctionPtr);

namespace Detail
{
template <typename F>             struct FunctionFirstArgType;
template <typename R, typename T> struct FunctionFirstArgType<R(*)(T)> { typedef T type; };
template <typename R>             struct FunctionFirstArgType<R(*)(void)> { typedef void type; };

template <typename ... PARAM_TYPE>
class CStaticAutoRegistrar : public CStaticInstanceList< CStaticAutoRegistrar<PARAM_TYPE...> >
{
	typedef void (*CallbackPtr)(PARAM_TYPE...);
	CallbackPtr m_pCallback;

public:
	CStaticAutoRegistrar(CallbackPtr pCallback) : m_pCallback(pCallback) {}

	static void InvokeStaticCallbacks(PARAM_TYPE ... params)
	{
		for (const CStaticAutoRegistrar* pInstance = CStaticInstanceList<CStaticAutoRegistrar<PARAM_TYPE...>>::GetFirstInstance(); pInstance; pInstance = pInstance->GetNextInstance())
		{
			(*pInstance->m_pCallback)(std::forward<PARAM_TYPE>(params)...);
		}
	}
};

template <>
class CStaticAutoRegistrar<void> : public CStaticInstanceList< CStaticAutoRegistrar<void> >
{
	typedef void(*CallbackPtr)();
	CallbackPtr m_pCallback;
public:
	CStaticAutoRegistrar(CallbackPtr pCallback) : m_pCallback(pCallback) {}

	static void InvokeStaticCallbacks()
	{
		for (const CStaticAutoRegistrar* pInstance = GetFirstInstance(); pInstance; pInstance = pInstance->GetNextInstance())
		{
			(*pInstance->m_pCallback)();
		}
	}
};

} // Detail

template <typename T>
static void DrxInvokeStaticCallbacks(T param)
{
	Detail::CStaticAutoRegistrar<T>::InvokeStaticCallbacks(param);
}