// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ClassWeaver.h
//  Version:     v1.00
//  Created:     02/25/2009 by CarstenW
//  Описание: Part of DinrusX's extension framework.
// -------------------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////

#ifndef _CLASSWEAVER_H_
#define _CLASSWEAVER_H_

#pragma once

#include "TypeList.h"
#include "Conversion.h"
#include "RegFactoryNode.h"
#include <drx3D/CoreX/Extension/IDrxUnknown.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>

namespace CW
{

namespace Internal
{
template<class Dst> struct InterfaceCast;

template<class Dst>
struct InterfaceCast
{
	template<class T>
	static uk Op(T* p)
	{
		return (Dst*) p;
	}
};

template<>
struct InterfaceCast<IDrxUnknown>
{
	template<class T>
	static uk Op(T* p)
	{
		return const_cast<IDrxUnknown*>(static_cast<const IDrxUnknown*>(static_cast<ukk>(p)));
	}
};
}

template<class TList> struct InterfaceCast;

template<>
struct InterfaceCast<TL::NullType>
{
	template<class T>
	static uk Op(T*, const DrxInterfaceID&)
	{
		return 0;
	}
};

template<class Head, class Tail>
struct InterfaceCast<TL::Typelist<Head, Tail>>
{
	template<class T>
	static uk Op(T* p, const DrxInterfaceID& iid)
	{
		if (drxiidof<Head>() == iid)
			return Internal::InterfaceCast<Head>::Op(p);
		return InterfaceCast<Tail>::Op(p, iid);
	}
};

template<class TList> struct FillIIDs;

template<>
struct FillIIDs<TL::NullType>
{
	static void Op(DrxInterfaceID*)
	{
	}
};

template<class Head, class Tail>
struct FillIIDs<TL::Typelist<Head, Tail>>
{
	static void Op(DrxInterfaceID* p)
	{
		*p++ = drxiidof<Head>();
		FillIIDs<Tail>::Op(p);
	}
};

namespace Internal
{
template<bool, typename S> struct PickList;

template<bool, typename S>
struct PickList
{
	typedef TL::BuildTypelist<>::Result Result;
};

template<typename S>
struct PickList<true, S>
{
	typedef typename S::FullCompositeList Result;
};
}

template<typename T>
struct ProbeFullCompositeList
{
private:
	typedef char y[1];
	typedef char n[2];

	template<typename S>
	static y& test(typename S::FullCompositeList*);

	template<typename>
	static n& test(...);

public:
	enum
	{
		listFound = sizeof(test<T>(0)) == sizeof(y)
	};

	typedef typename Internal::PickList<listFound, T>::Result ListType;
};

namespace Internal
{
template<class TList> struct CompositeQuery;

template<>
struct CompositeQuery<TL::NullType>
{
	template<typename T>
	static uk Op(const T&, tukk )
	{
		return 0;
	}
};

template<class Head, class Tail>
struct CompositeQuery<TL::Typelist<Head, Tail>>
{
	template<typename T>
	static uk Op(const T& ref, tukk name)
	{
		uk p = ref.Head::CompositeQueryImpl(name);
		return p ? p : CompositeQuery<Tail>::Op(ref, name);
	}
};
}

struct CompositeQuery
{
	template<typename T>
	static uk Op(const T& ref, tukk name)
	{
		return Internal::CompositeQuery<typename ProbeFullCompositeList<T>::ListType>::Op(ref, name);
	}
};

inline bool NameMatch(tukk name, tukk compositeName)
{
	if (!name || !compositeName)
		return false;
	size_t i = 0;
	for (; name[i] && name[i] == compositeName[i]; ++i)
	{
	}
	return name[i] == compositeName[i];
}

template<typename T>
uk CheckCompositeMatch(tukk name, const std::shared_ptr<T>& composite, tukk compositeName)
{
	typedef TC::SuperSubClass<IDrxUnknown, T> Rel;
	static_assert(Rel::exists, "Неожиданное значение перечня!");
	return NameMatch(name, compositeName) ? const_cast<uk>(static_cast<ukk>(&composite)) : 0;
}

}

#define DRXINTERFACE_BEGIN() \
  private:                   \
    typedef TL::BuildTypelist < IDrxUnknown

#define DRXINTERFACE_ADD(iname)                        , iname

#define DRXINTERFACE_END()                             > ::Result _UserDefinedPartialInterfaceList; \
  protected:                                                                                        \
    typedef TL::NoDuplicates<_UserDefinedPartialInterfaceList>::Result FullInterfaceList;

#define _DRX_TPL_APPEND0(base)                         TL::Append<base::FullInterfaceList, _UserDefinedPartialInterfaceList>::Result
#define _DRX_TPL_APPEND(base, intermediate)            TL::Append<base::FullInterfaceList, intermediate>::Result

#define DRXINTERFACE_ENDWITHBASE(base)                 > ::Result _UserDefinedPartialInterfaceList; \
  protected:                                                                                        \
    typedef TL::NoDuplicates<_DRX_TPL_APPEND0(base)>::Result FullInterfaceList;

#define DRXINTERFACE_ENDWITHBASE2(base0, base1)        > ::Result _UserDefinedPartialInterfaceList; \
  protected:                                                                                        \
    typedef TL::NoDuplicates<_DRX_TPL_APPEND(base0, _DRX_TPL_APPEND0(base1))>::Result FullInterfaceList;

#define DRXINTERFACE_ENDWITHBASE3(base0, base1, base2) > ::Result _UserDefinedPartialInterfaceList; \
  protected:                                                                                        \
    typedef TL::NoDuplicates<_DRX_TPL_APPEND(base0, _DRX_TPL_APPEND(base1, _DRX_TPL_APPEND0(base2)))>::Result FullInterfaceList;

#define DRXINTERFACE_SIMPLE(iname) \
  DRXINTERFACE_BEGIN()             \
  DRXINTERFACE_ADD(iname)          \
  DRXINTERFACE_END()

#define DRXCOMPOSITE_BEGIN()                         \
  private:                                           \
    uk CompositeQueryImpl(tukk name) const \
    {                                                \
      (void)(name);                                  \
      uk res = 0; (void)(res);                    \

#define DRXCOMPOSITE_ADD(member, membername)                                   \
  static_assert(DRX_ARRAY_COUNT(membername) > 1, "'membername' слишком короткое"); \
  if ((res = CW::CheckCompositeMatch(name, member, membername)) != 0)          \
    return res;

#define _DRXCOMPOSITE_END(implclassname)                                  \
  return 0;                                                               \
  };                                                                      \
protected:                                                                \
  typedef TL::BuildTypelist<implclassname>::Result _PartialCompositeList; \
                                                                          \
  template<bool, typename S> friend struct CW::Internal::PickList;

#define DRXCOMPOSITE_END(implclassname) \
  _DRXCOMPOSITE_END(implclassname)      \
protected:                              \
  typedef _PartialCompositeList FullCompositeList;

#define _DRXCOMPOSITE_APPEND0(base)              TL::Append<_PartialCompositeList, CW::ProbeFullCompositeList<base>::ListType>::Result
#define _DRXCOMPOSITE_APPEND(base, intermediate) TL::Append<intermediate, CW::ProbeFullCompositeList<base>::ListType>::Result

#define DRXCOMPOSITE_ENDWITHBASE(implclassname, base) \
  _DRXCOMPOSITE_END(implclassname)                    \
protected:                                            \
  typedef _DRXCOMPOSITE_APPEND0 (base) FullCompositeList;

#define DRXCOMPOSITE_ENDWITHBASE2(implclassname, base0, base1) \
  _DRXCOMPOSITE_END(implclassname)                             \
protected:                                                     \
  typedef TL::NoDuplicates<_DRXCOMPOSITE_APPEND(base1, _DRXCOMPOSITE_APPEND0(base0))>::Result FullCompositeList;

#define DRXCOMPOSITE_ENDWITHBASE3(implclassname, base0, base1, base2) \
  _DRXCOMPOSITE_END(implclassname)                                    \
protected:                                                            \
  typedef TL::NoDuplicates<_DRXCOMPOSITE_APPEND(base2, _DRXCOMPOSITE_APPEND(base1, _DRXCOMPOSITE_APPEND0(base0)))>::Result FullCompositeList;

template<typename T>
class CFactory : public IDrxFactory
{
public:
	virtual tukk GetName() const
	{
		return T::GetCName();
	}

	virtual const DrxClassID& GetClassID() const
	{
		static constexpr DrxClassID cid = T::GetCID();
		return cid;
	}

	virtual bool ClassSupports(const DrxInterfaceID& iid) const
	{
		for (size_t i = 0; i < m_numIIDs; ++i)
		{
			if (iid == m_pIIDs[i])
				return true;
		}
		return false;
	}

	virtual void ClassSupports(const DrxInterfaceID*& pIIDs, size_t& numIIDs) const
	{
		pIIDs = m_pIIDs;
		numIIDs = m_numIIDs;
	}

public:
	struct CustomDeleter
	{
		void operator()(T* p)
		{
			// Explicit call to the destructor
			p->~T();
			// Memory aligned free
			DrxModuleMemalignFree(p);
		}
	};

	virtual IDrxUnknownPtr CreateClassInstance() const
	{
		uk pAlignedMemory = DrxModuleMemalign(sizeof(T), std::alignment_of<T>::value);

		return drxinterface_cast<IDrxUnknown>(std::shared_ptr<T>(new(pAlignedMemory) T(), CustomDeleter()));
	}

	CFactory()
		: m_numIIDs(0)
		, m_pIIDs(0)
		, m_regFactory()
	{
		static DrxInterfaceID supportedIIDs[TL::Length < typename T::FullInterfaceList > ::value];
		CW::FillIIDs<typename T::FullInterfaceList>::Op(supportedIIDs);
		m_pIIDs = &supportedIIDs[0];
		m_numIIDs = TL::Length<typename T::FullInterfaceList>::value;
		new(&m_regFactory)SRegFactoryNode(this);
	}

protected:
	CFactory(const CFactory&);
	CFactory& operator=(const CFactory&);

	size_t          m_numIIDs;
	DrxInterfaceID* m_pIIDs;
	SRegFactoryNode m_regFactory;
};

template<typename T>
class CSingletonFactory : public CFactory<T>
{
public:
	CSingletonFactory()
		: CFactory<T>()
		, m_csCreateClassInstance()
	{
	}

	virtual IDrxUnknownPtr CreateClassInstance() const
	{
		DrxAutoLock<DrxCriticalSection> lock(m_csCreateClassInstance);
		static IDrxUnknownPtr p = CFactory<T>::CreateClassInstance();
		return p;
	}

	mutable DrxCriticalSection m_csCreateClassInstance;
};

#define _DRXFACTORY_DECLARE(implclassname) \
  private:                                 \
    friend class CFactory<implclassname>;  \
    static CFactory<implclassname> s_factory;

#define _DRXFACTORY_DECLARE_SINGLETON(implclassname) \
  private:                                           \
    friend class CFactory<implclassname>;            \
    static CSingletonFactory<implclassname> s_factory;

#define _IMPLEMENT_IDRXUNKNOWN()                                           \
  public:                                                                  \
    virtual IDrxFactory* GetFactory() const override                       \
    {                                                                      \
      return &s_factory;                                                   \
    }                                                                      \
                                                                           \
  protected:                                                               \
    virtual uk QueryInterface(const DrxInterfaceID &iid) const override \
    {                                                                      \
      return CW::InterfaceCast<FullInterfaceList>::Op(this, iid);          \
    }                                                                      \
                                                                           \
    template<class TList> friend struct CW::Internal::CompositeQuery;      \
                                                                           \
    virtual uk QueryComposite(tukk name) const override          \
    {                                                                      \
      return CW::CompositeQuery::Op(*this, name);                          \
    }

#define _ENFORCE_DRXFACTORY_USAGE(implclassname, cname, cidHigh, cidLow) DRX_PP_ERROR("Deprecated macro: Use _ENFORCE_DRXFACTORY_USAGE_GUID instead.")

#define _ENFORCE_DRXFACTORY_USAGE_GUID(implclassname, cname, guid)                                                  \
  public:                                                                                                           \
    static tukk GetCName() { return cname; }                                                                 \
    static constexpr DrxClassID GetCID() { return guid; }                                                           \
    static std::shared_ptr<implclassname> CreateClassInstance()                                                     \
    {                                                                                                               \
      IDrxUnknownPtr p = s_factory.CreateClassInstance();                                                           \
      return std::shared_ptr<implclassname>(*static_cast<std::shared_ptr<implclassname>*>(static_cast<uk>(&p))); \
    }

#define _BEFRIEND_OPS()           \
  _BEFRIEND_DRXINTERFACE_CAST()   \
  _BEFRIEND_DRXCOMPOSITE_QUERY()

#define DRXGENERATE_CLASS(implclassname, cname, cidHigh, cidLow) DRX_PP_ERROR("Deprecated macro: Use DRXGENERATE_CLASS_GUID instead.")

#define DRXGENERATE_CLASS_GUID(implclassname, cname, classGuid)  \
  friend struct CFactory<implclassname>::CustomDeleter;          \
  _DRXFACTORY_DECLARE(implclassname)                             \
  _BEFRIEND_OPS()                                                \
  _IMPLEMENT_IDRXUNKNOWN()                                       \
  _ENFORCE_DRXFACTORY_USAGE_GUID(implclassname, cname, classGuid)


#define DRXGENERATE_CLASS_FROM_INTERFACE(implclassname, interfaceName, cname, cidHigh, cidLow) DRX_PP_ERROR("Deprecated macro: Use DRXGENERATE_CLASS_FROM_INTERFACE_GUID instead.")

#define DRXGENERATE_CLASS_FROM_INTERFACE_GUID(implclassname, interfaceName, cname, classGuid) \
  DRXINTERFACE_SIMPLE(interfaceName)                                                           \
  DRXGENERATE_CLASS_GUID(implclassname, cname, classGuid)

#define DRXGENERATE_SINGLETONCLASS(implclassname, cname, cidHigh, cidLow) DRX_PP_ERROR("Deprecated macro: Use DRXGENERATE_SINGLETONCLASS_GUID instead.")

#define DRXGENERATE_SINGLETONCLASS_GUID(implclassname, cname, classGuid) \
  friend struct CFactory<implclassname>::CustomDeleter;                  \
  _DRXFACTORY_DECLARE_SINGLETON(implclassname)                           \
  _BEFRIEND_OPS()                                                        \
  _IMPLEMENT_IDRXUNKNOWN()                                               \
  _ENFORCE_DRXFACTORY_USAGE_GUID(implclassname, cname, classGuid)

#define DRXREGISTER_CLASS(implclassname) \
  CFactory<implclassname> implclassname::s_factory;

#define DRXREGISTER_SINGLETON_CLASS(implclassname) \
  CSingletonFactory<implclassname> implclassname::s_factory;

#endif // #ifndef _CLASSWEAVER_H_