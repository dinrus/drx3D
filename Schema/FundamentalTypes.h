// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/Entity/IEntityBasicTypes.h>

#include <drx3D/Schema/GUID.h>
#include <drx3D/Schema/PreprocessorUtils.h>

#include <drx3D/Schema/TypeDesc.h>

#define SXEMA_SOURCE_FILE_INFO sxema::SSourceFileInfo(SXEMA_FILE_NAME, SXEMA_LINE_NUMBER)

namespace sxema
{
enum : u32
{
	InvalidId = ~0u,
	InvalidIdx = ~0u
};

enum class EVisitStatus
{
	Continue,   // Continue visit.
	Recurse,    // Continue visit recursively (if applicable).
	Stop,       // Stop visit.
	Error       // Stop visit because error occurred.
};

enum class EVisitResult
{
	Complete,   // Visit was completed.
	Stopped,    // Visit was stopped before completion.
	Error       // Visit was stopped before completion error occurred.
};

// In-place storage parameters.
struct SInPlaceStorageParams
{
	explicit inline SInPlaceStorageParams(u32 _capacity, uk _pData)
		: capacity(_capacity)
		, pData(_pData)
	{}

	u32 capacity;
	uk  pData;
};

struct SSourceFileInfo
{
	explicit inline SSourceFileInfo(tukk _szFileName, u32 _lineNumber)
		: szFileName(_szFileName)
		, lineNumber(_lineNumber)
	{}

	tukk szFileName;
	u32      lineNumber;
};

enum class EDomain /* : u8*/ // Ideally this would have u8 as an underlying type but that screws with serialization.
{
	Unknown,
	Env,
	Script
};

struct SElementId
{
	inline SElementId()
		: domain(EDomain::Unknown)
		, guid(DrxGUID())
	{}

	inline SElementId(EDomain _domain, const DrxGUID& _guid)
		: domain(_domain)
		, guid(_guid)
	{}

	inline void Serialize(Serialization::IArchive& archive)
	{
		archive(domain, "domain");
		archive(guid, "guid");
	}

	inline bool operator==(const SElementId& rhs) const
	{
		return (domain == rhs.domain) && (guid == rhs.guid);
	}

	inline bool operator!=(const SElementId& rhs) const
	{
		return (domain != rhs.domain) || (guid != rhs.guid);
	}

	EDomain domain;
	DrxGUID   guid;
};

enum class EOverridePolicy // #SchematycTODO : Move to IScriptElement.h and rename EScriptElementOverridePolicy?
{
	Default,
	Override,
	Final
};

using ESimulationMode = EEntitySimulationMode;

enum class ObjectId : u32
{
	Invalid = 0xffffffff
};

// Reflect 'ObjectId' type.
inline void ObjectIdToString(IString& output, const ObjectId& input)
{
	output.Format("%d", static_cast<u32>(input));
}

inline void ReflectType(CTypeDesc<ObjectId>& desc)
{
	desc.SetGUID("95b8918e-9e65-4b6c-9c48-8899754f9d3c"_drx_guid);
	desc.SetLabel("ObjectId");
	desc.SetDescription("Object id");
	desc.SetDefaultValue(ObjectId::Invalid);
	desc.SetToStringOperator<&ObjectIdToString>();
}

inline bool Serialize(Serialization::IArchive& archive, ObjectId& value, tukk szName, tukk szLabel)
{
	if (!archive.isEdit())
	{
		return archive(static_cast<u32>(value), szName, szLabel);
	}
	return true;
}

constexpr tukk g_szDinrus = "DinrusPro";
constexpr tukk g_szNoType = "No Type";
} // sxema

enum { ESYSTEM_EVENT_REGISTER_SXEMA_ENV = ESYSTEM_EVENT_GAME_POST_INIT_DONE };
