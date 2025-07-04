// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/GUID.h>
#include <drx3D/Schema/TypeName.h>

namespace sxema
{
struct SValidatorLink
{
	inline SValidatorLink(const DrxGUID& _elementGUID = DrxGUID(), const DrxGUID& _detailGUID = DrxGUID())
		: elementGUID(_elementGUID)
		, detailGUID(_detailGUID)
	{}

	DrxGUID elementGUID;
	DrxGUID detailGUID;
};

enum class ESerializationPass // #SchematycTODO : Rename load passes to read and save to write? Then use flag to indicate copy+paste?
{
	Undefined,
	LoadDependencies,   // Load data required to identify dependencies.
	Load,               // Main load pass. At this point all dependencies will be initialized and available in the script registry.
	PostLoad,           // Load auxiliary data which nothing else is dependent on e.g. graph nodes. At this point we can guarantee that all elements will be initialized and available in the script registry.
	Save,               // Save data.
	Edit,               // Edit data.
	Validate            // Collect warnings and errors.
};

struct SSerializationContextParams
{
	inline SSerializationContextParams(Serialization::IArchive& _archive, ESerializationPass _pass)
		: archive(_archive)
		, pass(_pass)
	{}

	Serialization::IArchive& archive;
	ESerializationPass       pass;
};

struct ISerializationContext
{
	virtual ~ISerializationContext() {}

	virtual ESerializationPass    GetPass() const = 0;
	virtual void                  SetValidatorLink(const SValidatorLink& validatorLink) = 0;
	virtual const SValidatorLink& GetValidatorLink() const = 0;
};

DECLARE_SHARED_POINTERS(ISerializationContext)

namespace SerializationContext
{
// #SchematycTODO : Reduce lookups by passing ISerializationContext pointer to utility functions?

inline ISerializationContext* Get(Serialization::IArchive& archive)
{
	return archive.context<ISerializationContext>();
}

inline ESerializationPass GetPass(Serialization::IArchive& archive)
{
	ISerializationContext* pSerializationContext = archive.context<ISerializationContext>();
	SXEMA_CORE_ASSERT(pSerializationContext);
	return pSerializationContext ? pSerializationContext->GetPass() : ESerializationPass::Undefined;
}

inline void SetValidatorLink(Serialization::IArchive& archive, const SValidatorLink& validatorLink)
{
	ISerializationContext* pSerializationContext = archive.context<ISerializationContext>();
	if (pSerializationContext)
	{
		pSerializationContext->SetValidatorLink(validatorLink);
	}
}

inline bool GetValidatorLink(Serialization::IArchive& archive, SValidatorLink& validatorLink)
{
	ISerializationContext* pSerializationContext = archive.context<ISerializationContext>();
	if (pSerializationContext)
	{
		validatorLink = pSerializationContext->GetValidatorLink();
		return true;
	}
	return false;
}
} // SerializationContext
} // sxema
