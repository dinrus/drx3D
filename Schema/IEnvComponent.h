// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/IEnvElement.h>
#include <drx3D/Entity/IEntityComponent.h>

namespace sxema
{

struct IEnvComponent : public IEnvElementBase<EEnvElementType::Component>
{
	virtual ~IEnvComponent() {}

	virtual const CEntityComponentClassDesc&  GetDesc() const = 0;
	virtual std::shared_ptr<IEntityComponent> CreateFromPool() const = 0;
	virtual size_t                            GetSize() const = 0;
	virtual std::shared_ptr<IEntityComponent> CreateFromBuffer(uk pBuffer) const = 0;
};

} // sxema
