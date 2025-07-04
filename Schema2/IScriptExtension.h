// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/CoreX/Serialization/Forward.h>

#include <drx3D/Schema2/GUID.h>

namespace sxema2
{
	struct IGUIDRemapper;
	struct SScriptRefreshParams;

	enum EScriptExtensionId
	{
		Void,
		Graph
	};

	struct IScriptExtension
	{
		virtual ~IScriptExtension() {}

		virtual EScriptExtensionId GetId_New() const = 0; // #SchematycTODO : Rename GetId()!!!
		virtual void Refresh_New(const SScriptRefreshParams& params) = 0; // #SchematycTODO : Rename Refresh()!!!
		virtual void Serialize_New(Serialization::IArchive& archive) = 0; // #SchematycTODO : Rename Serialize()!!!
		virtual void RemapGUIDs_New(IGUIDRemapper& guidRemapper) = 0; // #SchematycTODO : Rename RemapGUIDs()!!!
	};

	struct IScriptExtensionMap // #SchematycTODO : Is map really the best name for this?
	{
		virtual ~IScriptExtensionMap() {}

		virtual IScriptExtension* QueryExtension(EScriptExtensionId id) = 0;
		virtual const IScriptExtension* QueryExtension(EScriptExtensionId id) const = 0;
		virtual void Refresh(const SScriptRefreshParams& params) = 0;
		virtual void Serialize(Serialization::IArchive& archive) = 0;
		virtual void RemapGUIDs(IGUIDRemapper& guidRemapper) = 0;
	};
}
