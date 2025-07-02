// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

namespace sxema2
{
	struct IGameResourceList
	{
		enum class EType : u32
		{
			Invalid = 0,
			Character,
			StatObject,
			Material,
			Texture,
			ParticleFX,
			MannequinADB,
			MannequinControllerDefinition,
			EntityClass,
			EntityArchetype,
			BehaviorTree,
			LensFlare,
		};

		struct SItem
		{
			SItem(tukk _szResourceName, EType _type)
				: szResourceName(_szResourceName)
				, type(_type)
			{

			}

			tukk szResourceName;
			EType       type;
		};

		virtual ~IGameResourceList() {}

		virtual void   AddResource(tukk szResource, EType type) = 0;
		virtual size_t GetResourceCount() const = 0;
		virtual SItem  GetResourceItemAt(size_t idx) const = 0;
		virtual void   Sort() = 0;
	};

	DECLARE_SHARED_POINTERS(IGameResourceList)
}
