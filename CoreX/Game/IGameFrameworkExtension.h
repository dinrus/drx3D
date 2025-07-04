// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Extension/IDrxUnknown.h>

struct IGameFramework;

struct IGameFrameworkExtensionCreator : public IDrxUnknown
{
	DRXINTERFACE_DECLARE_GUID(IGameFrameworkExtensionCreator, "86197e35-ad02-4da8-a8d4-83a98f424ffd"_drx_guid);

#if (eDrxModule != eDrxM_LegacyGameDLL && eDrxModule != eDrxM_EditorPlugin && eDrxModule != eDrxM_Legacy)
	DRX_DEPRECATED("(v5.5) Game framework extensions are deprecated, please use IDrxPlugin instead") IGameFrameworkExtensionCreator() = default;
#endif

	//! Creates an extension and returns the interface to it (extension interface must derivate for IDrxUnknown).
	//! \param pIGameFramework Pointer to game framework interface, so the new extension can be registered against it.
	//! \param pData Pointer to data needed for creation. Each system interface is responsible for explaining what data is expected to be passed here, if any, 'nullptr' is valid for default extensions.
	//! \return IDrxUnknown pointer to just created extension (it can be safely casted with drxinterface_cast< > to the corresponding interface).
	virtual IDrxUnknown* Create(IGameFramework* pIGameFramework, uk pData) = 0;
};
DECLARE_SHARED_POINTERS(IGameFrameworkExtensionCreator);
