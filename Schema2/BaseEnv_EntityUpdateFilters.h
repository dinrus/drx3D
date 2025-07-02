// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

#include <drx3D/Eng3D/I3DEngine.h> // Must be included before CREOcclusionQuery.h.
#include <drx3D/CoreX/Renderer/RendElements/CREOcclusionQuery.h> // Must be included before IEntityRenderState.h.

namespace SchematycBaseEnv
{
	struct SEntityNotHidden
	{
		inline bool operator () (IEntity* pEntity) const
		{
			return pEntity && !pEntity->IsHidden();
		}
	};

	struct SEntityVisible
	{
		inline bool operator () (IEntity* pEntity) const
		{
			if(pEntity)
			{
				return pEntity->IsRendered();
			}
			return false;
		}
	};

	typedef sxema2::CConfigurableUpdateFilter<IEntity*, NTypelist::CConstruct<SEntityNotHidden>::TType>                 EntityNotHiddenUpdateFilter;
	typedef sxema2::CConfigurableUpdateFilter<IEntity*, NTypelist::CConstruct<SEntityNotHidden, SEntityVisible>::TType> EntityVisibleUpdateFilter;
}