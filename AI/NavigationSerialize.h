// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/INavigationSystem.h>
#include <drx3D/AI/INavigationQuery.h>

#include <drx3D/CoreX/Serialization/Forward.h>

// Serialization Decorators

namespace NavigationSerialization
{
	struct NavigationQueryFilter
	{
		SNavMeshQueryFilterDefault& variable;
		NavigationQueryFilter(SNavMeshQueryFilterDefault& filter) 
			: variable(filter) 
		{}
		void Serialize(Serialization::IArchive& ar);
	};

	struct NavigationAreaCost
	{
		u32& index;
		float&  cost;
		NavigationAreaCost(u32& index, float& cost)
			: index(index), cost(cost)
		{}
		void Serialize(Serialization::IArchive& ar);
	};

	struct NavigationAreaFlagsMask
	{
		MNM::AreaAnnotation::value_type& value;
		NavigationAreaFlagsMask(MNM::AreaAnnotation::value_type& value) 
			: value(value) 
		{}
		void Serialize(Serialization::IArchive& ar);
	};
}

bool Serialize(Serialization::IArchive& archive, NavigationVolumeID& value, tukk szName, tukk szLabel);
bool Serialize(Serialization::IArchive& archive, NavigationAgentTypeID& value, tukk szName, tukk szLabel);
bool Serialize(Serialization::IArchive& archive, NavigationAreaTypeID& value, tukk szName, tukk szLabel);
bool Serialize(Serialization::IArchive& archive, NavigationAreaFlagID& value, tukk szName, tukk szLabel);
bool Serialize(Serialization::IArchive& archive, SNavMeshQueryFilterDefault& value, tukk szName, tukk szLabel);

#include <drx3D/AI/NavigationSerialize.inl>
