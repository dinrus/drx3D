// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/FactionSystem.h>

#include <drx3D/AI/VisionMapTypes.h>
#include <drx3D/CoreX/Serialization/Decorators/BitFlags.h>

namespace Perception
{
namespace ComponentHelpers
{

struct SLocation
{
	static void ReflectType(sxema::CTypeDesc<SLocation>& typeInfo)
	{
		typeInfo.SetGUID("80827c81-dd8f-4020-b033-163f6a130ede"_drx_guid);
	}
	void Serialize(Serialization::IArchive& archive)
	{
		archive(type, "type", "Location Type");
		archive.doc("How the location is defined.");

		switch (type)
		{
		case SLocation::EType::Bone:
			archive(boneName, "boneName", "Bone Name");
			archive.doc("The name of the bone/joint from which to take the location.");
			if (boneName.empty())
			{
				archive.error(boneName, "Bone name is empty!");
			}
			break;
		case SLocation::EType::Pivot:
			// Nothing special to serialize
			break;
		default:
			DRX_ASSERT(false);     // We should never get here!
			break;
		}
		static_assert(i32(SLocation::EType::Count) == 2, "Unexpected enum count");

		archive(Serialization::SPosition(offset), "offset", "Offset");
		archive.doc("A fixed offset from position.");
	}

	enum class EType
	{
		Pivot,
		Bone,
		Count,
	};

	bool operator==(const SLocation& other) const
	{
		return type == other.type && offset == other.offset && boneName == other.boneName;
	}

	EType  type = EType::Pivot;
	Vec3   offset = ZERO;
	string boneName;

	bool Validate(IEntity* pEntity, tukk szComponentName) const;
	void GetPositionAndOrientation(IEntity* pEntity, Vec3* pOutPosition, Vec3* pOutOrientation) const;

private:
	bool GetTransformFromPivot(IEntity* pEntity, Vec3* pOutPosition, Vec3* pOutOrientation) const;
	bool GetTransformFromBone(IEntity* pEntity, Vec3* pOutPosition, Vec3* pOutOrientation) const;
};

struct SLocationsArray
{
	static void ReflectType(sxema::CTypeDesc<SLocationsArray>& typeInfo)
	{
		typeInfo.SetGUID("3542865D-DE8F-4496-97A2-4FA59D9C9440"_drx_guid);
	}
	bool operator==(const SLocationsArray& other) const { return locations == other.locations; }

	std::vector<SLocation> locations;
};

inline bool Serialize(Serialization::IArchive& archive, SLocationsArray& value, tukk szName, tukk szLabel)
{
	return archive(value.locations, szName, szLabel);
}

//////////////////////////////////////////////////////////////////////////

struct SVisionMapType
{
	static void ReflectType(sxema::CTypeDesc<SVisionMapType>& typeInfo)
	{
		typeInfo.SetGUID("5dc7e1c7-fc02-42ae-988e-606e6f876785"_drx_guid);
	}
	bool operator==(const SVisionMapType& other) const { return mask == other.mask; }
	u32 mask = 0;
};

inline bool Serialize(Serialization::IArchive& archive, SVisionMapType& value, tukk szName, tukk szLabel)
{
	return archive(Serialization::BitFlags<VisionMapTypes>(value.mask), szName, szLabel);
}

}   //! ComponentHelpers

} //! Perception
