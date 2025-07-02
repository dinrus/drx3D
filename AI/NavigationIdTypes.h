// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

enum ENavigationIDTag
{
	MeshIDTag = 0,
	AgentTypeIDTag,
	VolumeIDTag,
	TileGeneratorExtensionIDTag,
	AreaTypeIDTag,
	AreaFlagIDTag,
};

template<ENavigationIDTag T, u32 valueInvalid = 0>
struct TNavigationID
{
	explicit TNavigationID(u32 id = valueInvalid) : id(id) {}

	TNavigationID& operator=(const TNavigationID& other)        { id = other.id; return *this; }
	ILINE          operator u32() const                      { return id; } //TODO: remove this operator and add function for returning u32 value
	ILINE bool     operator==(const TNavigationID& other) const { return id == other.id; }
	ILINE bool     operator!=(const TNavigationID& other) const { return id != other.id; }
	ILINE bool     operator<(const TNavigationID& other) const  { return id < other.id; }
	ILINE bool     IsValid() const { return id != valueInvalid; }
private:
	u32 id;
};

//TODO: Don't change 'valueInvalid' for other types than NavigationAreaTypeID and NavigationAreaFlagID,
// before all locations are checked in the code where the value is validated using operator u32()
typedef TNavigationID<MeshIDTag>                   NavigationMeshID;
typedef TNavigationID<AgentTypeIDTag>              NavigationAgentTypeID;
typedef TNavigationID<VolumeIDTag>                 NavigationVolumeID;
typedef TNavigationID<TileGeneratorExtensionIDTag> TileGeneratorExtensionID;
typedef TNavigationID<AreaTypeIDTag, (u32) -1>           NavigationAreaTypeID;
typedef TNavigationID<AreaFlagIDTag, (u32) -1>           NavigationAreaFlagID;

namespace std
{
template <ENavigationIDTag T, u32 valueInvalid>
struct hash<TNavigationID<T, valueInvalid>>
{
	size_t operator () (const TNavigationID<T, valueInvalid>& value) const
	{
		std::hash<u32> hasher;
		return hasher(value);
	}
};
}
