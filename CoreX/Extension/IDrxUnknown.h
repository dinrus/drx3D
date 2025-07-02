// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "DrxTypeID.h"

struct IDrxFactory;
struct IDrxUnknown;

namespace InterfaceCastSemantics
{
#if !defined(SWIG)
template<class T>
struct has_drxiidof
{
	typedef char(&yes)[1];
	typedef char(&no)[2];

	template <typename C> static yes check(decltype(&C::IID));
	template <typename> static no check(...);

	static constexpr bool value = sizeof(check<T>(0)) == sizeof(yes);
};
#endif

template<class T>
const DrxInterfaceID& drxiidof()
{
	return T::IID();
}

#if !defined(SWIG)
#define _BEFRIEND_DRXIIDOF() \
  template<class T> friend const DrxInterfaceID &InterfaceCastSemantics::drxiidof(); \
  template<class T> friend struct InterfaceCastSemantics::has_drxiidof;
#else
#define _BEFRIEND_DRXIIDOF() \
  template<class T> friend const DrxInterfaceID &InterfaceCastSemantics::drxiidof();
#endif

template<class Dst, class Src>
Dst* drxinterface_cast(Src* p)
{
	return static_cast<Dst*>(p ? p->QueryInterface(drxiidof<Dst>()) : nullptr);
}

template<class Dst, class Src>
Dst* drxinterface_cast(const Src* p)
{
	return static_cast<const Dst*>(p ? p->QueryInterface(drxiidof<Dst>()) : nullptr);
}

namespace Internal
{
template<class Dst, class Src>
struct drxinterface_cast_helper
{
	static std::shared_ptr<Dst> Op(const std::shared_ptr<Src>& p)
	{
		Dst* dp = drxinterface_cast<Dst>(p.get());
		return dp ? std::shared_ptr<Dst>(p, dp) : std::shared_ptr<Dst>();
	}
};

template<class Src>
struct drxinterface_cast_helper<IDrxUnknown, Src>
{
	static std::shared_ptr<IDrxUnknown> Op(const std::shared_ptr<Src>& p)
	{
		IDrxUnknown* dp = drxinterface_cast<IDrxUnknown>(p.get());
		return dp ? std::shared_ptr<IDrxUnknown>(*((const std::shared_ptr<IDrxUnknown>*)& p), dp) : std::shared_ptr<IDrxUnknown>();
	}
};

template<class Src>
struct drxinterface_cast_helper<const IDrxUnknown, Src>
{
	static std::shared_ptr<const IDrxUnknown> Op(const std::shared_ptr<Src>& p)
	{
		const IDrxUnknown* dp = drxinterface_cast<const IDrxUnknown>(p.get());
		return dp ? std::shared_ptr<const IDrxUnknown>(*((const std::shared_ptr<const IDrxUnknown>*)& p), dp) : std::shared_ptr<const IDrxUnknown>();
	}
};
}

template<class Dst, class Src>
std::shared_ptr<Dst> drxinterface_cast(const std::shared_ptr<Src>& p)
{
	return Internal::drxinterface_cast_helper<Dst, Src>::Op(p);
}

#define _BEFRIEND_DRXINTERFACE_CAST()                                                                \
  template<class Dst, class Src> friend Dst * InterfaceCastSemantics::drxinterface_cast(Src*);       \
  template<class Dst, class Src> friend Dst * InterfaceCastSemantics::drxinterface_cast(const Src*); \
  template<class Dst, class Src> friend std::shared_ptr<Dst> InterfaceCastSemantics::drxinterface_cast(const std::shared_ptr<Src> &);

}

using InterfaceCastSemantics::drxiidof;
using InterfaceCastSemantics::drxinterface_cast;

template<class S, class T>
bool DrxIsSameClassInstance(S* p0, T* p1)
{
	return static_cast<ukk>(p0) == static_cast<ukk>(p1) || drxinterface_cast<const IDrxUnknown>(p0) == drxinterface_cast<const IDrxUnknown>(p1);
}

template<class S, class T>
bool DrxIsSameClassInstance(const std::shared_ptr<S>& p0, T* p1)
{
	return DrxIsSameClassInstance(p0.get(), p1);
}

template<class S, class T>
bool DrxIsSameClassInstance(S* p0, const std::shared_ptr<T>& p1)
{
	return DrxIsSameClassInstance(p0, p1.get());
}

template<class S, class T>
bool DrxIsSameClassInstance(const std::shared_ptr<S>& p0, const std::shared_ptr<T>& p1)
{
	return DrxIsSameClassInstance(p0.get(), p1.get());
}

namespace CompositeQuerySemantics
{

template<class Src>
std::shared_ptr<IDrxUnknown> drxcomposite_query(Src* p, tukk name, bool* pExposed = nullptr)
{
	uk pComposite = p ? p->QueryComposite(name) : nullptr;
	if(pExposed) *pExposed = pComposite != nullptr;
	return pComposite ? *static_cast<std::shared_ptr<IDrxUnknown>*>(pComposite) : std::shared_ptr<IDrxUnknown>();
}

template<class Src>
std::shared_ptr<const IDrxUnknown> drxcomposite_query(const Src* p, tukk name, bool* pExposed = nullptr)
{
	uk pComposite = p ? p->QueryComposite(name) : nullptr;
	if (pExposed) *pExposed = pComposite != nullptr;
	return pComposite ? *static_cast<std::shared_ptr<const IDrxUnknown>*>(pComposite) : std::shared_ptr<const IDrxUnknown>();
}

template<class Src>
std::shared_ptr<IDrxUnknown> drxcomposite_query(const std::shared_ptr<Src>& p, tukk name, bool* pExposed = nullptr)
{
	return drxcomposite_query(p.get(), name, pExposed);
}

template<class Src>
std::shared_ptr<const IDrxUnknown> drxcomposite_query(const std::shared_ptr<const Src>& p, tukk name, bool* pExposed = nullptr)
{
	return drxcomposite_query(p.get(), name, pExposed);
}
}

#define _BEFRIEND_DRXCOMPOSITE_QUERY()                                                                                                                   \
  template<class Src> friend std::shared_ptr<IDrxUnknown> CompositeQuerySemantics::drxcomposite_query(Src*, tukk , bool*);                         \
  template<class Src> friend std::shared_ptr<const IDrxUnknown> CompositeQuerySemantics::drxcomposite_query(const Src*, tukk , bool*);             \
  template<class Src> friend std::shared_ptr<IDrxUnknown> CompositeQuerySemantics::drxcomposite_query(const std::shared_ptr<Src> &, tukk , bool*); \
  template<class Src> friend std::shared_ptr<const IDrxUnknown> CompositeQuerySemantics::drxcomposite_query(const std::shared_ptr<const Src> &, tukk , bool*);

using CompositeQuerySemantics::drxcomposite_query;

#define DRXINTERFACE_DECLARE(iname, iidHigh, iidLow) DRX_PP_ERROR("Deprecated macro: Use DRXINTERFACE_DECLARE_GUID instead. Please refer to the Migration Guide from DRXENGINE 5.3 to DRXENGINE 5.4 for more details.")

#define DRXINTERFACE_DECLARE_GUID(iname, guid)                                                \
  _BEFRIEND_DRXIIDOF()                                                                        \
  friend struct std::default_delete<iname>;                                                   \
private:                                                                                      \
  static const DrxInterfaceID& IID() { static constexpr DrxGUID sguid = guid; return sguid; } \
public:

struct IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(IDrxUnknown, "10000000-1000-1000-1000-100000000000"_drx_guid);

	_BEFRIEND_DRXINTERFACE_CAST()
	_BEFRIEND_DRXCOMPOSITE_QUERY()

	virtual IDrxFactory* GetFactory() const = 0;

protected:
	// Prevent explicit destruction from client side (exception is std::checked_delete which gets befriended).
	virtual ~IDrxUnknown() = default;

	virtual uk QueryInterface(const DrxInterfaceID& iid) const = 0;
	virtual uk QueryComposite(tukk name) const = 0;
};

typedef std::shared_ptr<IDrxUnknown> IDrxUnknownPtr;
typedef std::shared_ptr<const IDrxUnknown> IDrxUnknownConstPtr;
