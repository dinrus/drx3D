// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IEnvElement.h>
#include <drx3D/Schema/GUID.h>

namespace sxema
{

// Forward declare interfaces.
struct IObject;
struct IObjectPreviewer;
// Forward declare classes.
class CAnyConstPtr;

struct IEnvClass : public IEnvElementBase<EEnvElementType::Class>
{
	virtual ~IEnvClass() {}

	virtual u32            GetInterfaceCount() const = 0;
	virtual DrxGUID             GetInterfaceGUID(u32 interfaceIdx) const = 0;
	virtual u32            GetComponentCount() const = 0;
	virtual DrxGUID             GetComponentTypeGUID(u32 componentIdx) const = 0;
	virtual CAnyConstPtr      GetProperties() const = 0;
	virtual IObjectPreviewer* GetPreviewer() const = 0;
};

} // sxema
