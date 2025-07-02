// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __IENTITYATTRIBUTESPROXY_H__
#define __IENTITYATTRIBUTESPROXY_H__

#pragma once

#include <drx3D/Entity/IEntityComponent.h>
#include <drx3D/Entity/IEntity.h>
#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/Serializer.h>

#include <drx3D/Schema2/ILib.h>

struct IEntityAttribute;
DECLARE_SHARED_POINTERS(IEntityAttribute)

//! Derive from this interface to expose custom entity properties in the editor using the serialization framework.
struct IEntityAttribute
{
	virtual ~IEntityAttribute() {}

	virtual tukk         GetName() const = 0;
	virtual tukk         GetLabel() const = 0;
	virtual void                Serialize(Serialization::IArchive& archive) = 0;
	virtual IEntityAttributePtr Clone() const = 0;
	virtual void                OnAttributeChange(IEntity* pEntity) const {}
	virtual sxema2::ILibClassPropertiesConstPtr GetProperties() const { return nullptr; }
};

typedef DynArray<IEntityAttributePtr> TEntityAttributeArray;

//////////////////////////////////////////////////////////////////////////
struct SEntityAttributesSerializer
{
	SEntityAttributesSerializer(TEntityAttributeArray& _attributes)
		: attributes(_attributes)
		, entityId(INVALID_ENTITYID)
	{}

	SEntityAttributesSerializer(TEntityAttributeArray& _attributes, const EntityId _entityId)
		: attributes(_attributes)
		, entityId(_entityId)
	{}

	void Serialize(Serialization::IArchive& archive)
	{
		Serialization::SContext context(archive, this);

		for (size_t iAttribute = 0, attributeCount = attributes.size(); iAttribute < attributeCount; ++iAttribute)
		{
			IEntityAttribute* pAttribute = attributes[iAttribute].get();
			if (pAttribute != NULL)
			{
				archive(*pAttribute, pAttribute->GetName(), pAttribute->GetLabel());
			}
		}
	}

	TEntityAttributeArray& attributes;
	EntityId               entityId;
};

//////////////////////////////////////////////////////////////////////////
// Описание:
//    Proxy for storage of entity attributes.
//////////////////////////////////////////////////////////////////////////
struct IEntityAttributesComponent : public IEntityComponent
{
	DRX_ENTITY_COMPONENT_INTERFACE_GUID(IEntityAttributesComponent, "807b8678-0dbf-405e-892f-24737d11cb20"_drx_guid)

	virtual void                         SetAttributes(const TEntityAttributeArray& attributes) = 0;
	virtual TEntityAttributeArray&       GetAttributes() = 0;
	virtual const TEntityAttributeArray& GetAttributes() const = 0;
};

namespace EntityAttributeUtils
{
//////////////////////////////////////////////////////////////////////////
// Описание:
//    Clone array of entity attributes.
//////////////////////////////////////////////////////////////////////////
inline void CloneAttributes(const TEntityAttributeArray& src, TEntityAttributeArray& dst)
{
	dst.clear();
	dst.reserve(src.size());
	for (TEntityAttributeArray::const_iterator iAttribute = src.begin(), iEndAttribute = src.end(); iAttribute != iEndAttribute; ++iAttribute)
	{
		dst.push_back((*iAttribute)->Clone());
	}
}

//////////////////////////////////////////////////////////////////////////
// Описание:
//    Find entity attribute by name.
//////////////////////////////////////////////////////////////////////////
inline IEntityAttributePtr FindAttribute(TEntityAttributeArray& attributes, tukk name)
{
	DRX_ASSERT(name != NULL);
	if (name != NULL)
	{
		for (TEntityAttributeArray::iterator iAttribute = attributes.begin(), iEndAttribute = attributes.end(); iAttribute != iEndAttribute; ++iAttribute)
		{
			if (strcmp((*iAttribute)->GetName(), name) == 0)
			{
				return *iAttribute;
			}
		}
	}
	return IEntityAttributePtr();
}

//////////////////////////////////////////////////////////////////////////
// Описание:
//    Find entity attribute by name.
//////////////////////////////////////////////////////////////////////////
inline IEntityAttributeConstPtr FindAttribute(const TEntityAttributeArray& attributes, tukk name)
{
	DRX_ASSERT(name != NULL);
	if (name != NULL)
	{
		for (TEntityAttributeArray::const_iterator iAttribute = attributes.begin(), iEndAttribute = attributes.end(); iAttribute != iEndAttribute; ++iAttribute)
		{
			if (strcmp((*iAttribute)->GetName(), name) == 0)
			{
				return *iAttribute;
			}
		}
	}
	return IEntityAttributePtr();
}
}

#endif //__IENTITYATTRIBUTESPROXY_H__
