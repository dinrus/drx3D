// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "SmartPtr.h"
#include <drx3D/CoreX/Serialization/Serializer.h>
#include <drx3D/CoreX/Serialization/ClassFactory.h>

//! Exposes _smart_ptr<> as serializable type for Serialization::IArchive.
template<class T>
class SmartPtrSerializer : public Serialization::IPointer
{
public:
	SmartPtrSerializer(_smart_ptr<T>& ptr)
		: m_ptr(ptr)
	{}

	tukk registeredTypeName() const override
	{
		if (m_ptr)
			return Serialization::ClassFactory<T>::the().getRegisteredTypeName(m_ptr.get());
		else
			return "";
	}

	void create(tukk registeredTypeName) const override
	{
		if (registeredTypeName && registeredTypeName[0] != '\0')
			m_ptr.reset(Serialization::ClassFactory<T>::the().create(registeredTypeName));
		else
			m_ptr.reset((T*)0);
	}
	Serialization::TypeID          baseType() const override    { return Serialization::TypeID::get<T>(); }
	virtual Serialization::SStruct serializer() const override  { return Serialization::SStruct(*m_ptr); }
	uk                          get() const override         { return reinterpret_cast<uk>(m_ptr.get()); }
	ukk                    handle() const override      { return &m_ptr; }
	Serialization::TypeID          pointerType() const override { return Serialization::TypeID::get<_smart_ptr<T>>(); }
	Serialization::IClassFactory*  factory() const override     { return &Serialization::ClassFactory<T>::the(); }
protected:
	_smart_ptr<T>& m_ptr;
};

template<class T>
bool Serialize(Serialization::IArchive& ar, _smart_ptr<T>& ptr, tukk name, tukk label)
{
	SmartPtrSerializer<T> serializer(ptr);
	return ar(static_cast<Serialization::IPointer&>(serializer), name, label);
}
