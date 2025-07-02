// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/STL.h>

namespace Serialization
{
struct ICustomResourceParams
{
	virtual ~ICustomResourceParams() {}
};

DECLARE_SHARED_POINTERS(ICustomResourceParams)

struct IResourceSelector
{
	tukk              resourceType;
	ICustomResourceParamsPtr pCustomParams;

	virtual ~IResourceSelector() {}
	virtual tukk           GetValue() const = 0;
	virtual void                  SetValue(tukk s) = 0;
	virtual i32                   GetId() const { return -1; }
	virtual ukk           GetHandle() const = 0;
	virtual Serialization::TypeID GetType() const = 0;
};

//! Provides a way to annotate resource reference so different UI can be used for them.
//! See IResourceSelector.h to see how selectors for specific types are registered.
//! TString could be SCRCRef or CDrxName as well.
//! Do not use this class directly, instead use function that wraps it for
//! specific type, see Resources.h for example.
template<class TString>
struct ResourceSelector : IResourceSelector
{
	TString& value;

	tukk           GetValue() const        { return value.c_str(); }
	void                  SetValue(tukk s) { value = s; }
	ukk           GetHandle() const       { return &value; }
	Serialization::TypeID GetType() const         { return Serialization::TypeID::get<TString>(); }

	ResourceSelector(TString& value, tukk resourceType, const ICustomResourceParamsPtr& pCustomParams = ICustomResourceParamsPtr())
		: value(value)
	{
		this->resourceType = resourceType;
		this->pCustomParams = pCustomParams;
	}
};

struct ResourceSelectorWithId : IResourceSelector
{
	string& value;
	i32     id;

	tukk           GetValue() const        { return value.c_str(); }
	void                  SetValue(tukk s) { value = s; }
	i32                   GetId() const           { return id; }
	ukk           GetHandle() const       { return &value; }
	Serialization::TypeID GetType() const         { return Serialization::TypeID::get<string>(); }

	ResourceSelectorWithId(string& value, tukk resourceType, i32 id)
		: value(value)
		, id(id)
	{
		this->resourceType = resourceType;
	}
};

template<class T>
bool Serialize(IArchive& ar, ResourceSelector<T>& value, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serialization::SStruct::forEdit(static_cast<IResourceSelector&>(value)), name, label);
	else
		return ar(value.value, name, label);
}

inline bool Serialize(IArchive& ar, ResourceSelectorWithId& value, tukk name, tukk label)
{
	if (ar.isEdit())
		return ar(Serialization::SStruct::forEdit(static_cast<IResourceSelector&>(value)), name, label);
	else
		return ar(value.value, name, label);
}

}

//! \endcond