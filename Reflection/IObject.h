// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Reflection/IReflection.h>
#include <drx3D/Reflection/TypeRegistrar.h>

namespace Drx {
namespace Reflection {

template<typename TYPE>
struct CObject : public IObject
{
	virtual ~CObject() {}

	// IObject
	virtual bool             IsReflected() const override final  { return s_registration.IsRegistered(); }
	virtual DrxTypeId        GetTypeId() const override final    { return s_registration.GetTypeId(); }
	virtual TypeIndex        GetTypeIndex() const override final { return s_registration.GetTypeIndex(); }
	virtual const ITypeDesc* GetTypeDesc() const override final  { return s_registration.GetTypeDesc(); }

	virtual bool             IsEqualType(const IObject* pObject) const override final
	{
		DRX_ASSERT_MESSAGE(IsReflected() && pObject->IsReflected(), "Object is not reflected.");
		return IsReflected() && pObject->IsReflected() && pObject->GetTypeId() == GetTypeId();
	}

	virtual bool IsEqualObject(const IObject* pObject) const override final
	{
		if (IsEqualType(pObject))
		{
			Type::CEqualOperator isEqual = GetTypeDesc()->GetEqualOperator();
			if (isEqual)
			{
				return isEqual(this, pObject);
			}
			DRX_ASSERT_MESSAGE(isEqual, "Reflected object can't be equal compared.");
		}
		return false;
	}
	// ~IObject

	static const ITypeDesc* Type() { return s_registration.GetTypeDesc(); }

private:
	static CTypeRegistrar s_registration;
};

#define DRX_REFLECTION_REGISTER_OBJECT(type, guid)                                                                                                       \
  namespace Drx {                                                                                                                                        \
  namespace Reflection {                                                                                                                                 \
  template<>                                                                                                                                             \
  CTypeRegistrar CObject<type>::s_registration = CTypeRegistrar(Drx::TypeDescOf<type>(), guid, &type::ReflectType, SSourceFileInfo(__FILE__, __LINE__)); \
  }                                                                                                                                                      \
  }

#define DRX_REFLECTION_REGISTER_TYPE(type, guid, reflectFunc)                                                                                     \
  namespace Drx {                                                                                                                                 \
  namespace Reflection {                                                                                                                          \
  template<>                                                                                                                                      \
  CTypeRegistrar CObject<type>::s_registration = CTypeRegistrar(Drx::TypeDescOf<type>(), guid, reflectFunc, SSourceFileInfo(__FILE__, __LINE__)); \
  }                                                                                                                                               \
  }

} // ~Reflection namespace
} // ~Drx namespace

inline bool operator==(const Drx::Reflection::IObject& lh, DrxTypeId rh)
{
	return lh.GetTypeId() == rh;
}

inline bool operator!=(const Drx::Reflection::IObject& lh, DrxTypeId rh)
{
	return lh.GetTypeId() != rh;
}

inline bool operator==(const Drx::Reflection::IObject& lh, const DrxTypeDesc& rh)
{
	return lh.GetTypeId() == rh.GetTypeId();
}

inline bool operator!=(const Drx::Reflection::IObject& lh, const DrxTypeDesc& rh)
{
	return lh.GetTypeId() != rh.GetTypeId();
}
