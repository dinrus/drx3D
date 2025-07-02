// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Serialization/Math.h>
#include <drx3D/CoreX/Serialization/Forward.h>

namespace Serialization
{

struct LocalPosition
{
	Vec3*       value;
	i32         space;
	tukk parentName;
	ukk handle;

	LocalPosition(Vec3& vec, i32 space, tukk parentName, ukk handle)
		: value(&vec)
		, space(space)
		, parentName(parentName)
		, handle(handle)
	{
	}

	void Serialize(IArchive& ar);
};

struct LocalOrientation
{
	Quat*       value;
	i32         space;
	tukk parentName;
	ukk handle;

	LocalOrientation(Quat& vec, i32 space, tukk parentName, ukk handle)
		: value(&vec)
		, space(space)
		, parentName(parentName)
		, handle(handle)
	{
	}

	void Serialize(IArchive& ar);
};

struct LocalFrame
{
	Quat*       rotation;
	Vec3*       position;
	tukk parentName;
	i32         rotationSpace;
	i32         positionSpace;
	ukk handle;

	LocalFrame(Quat* rotation, i32 rotationSpace, Vec3* position, i32 positionSpace, tukk parentName, ukk handle)
		: rotation(rotation)
		, position(position)
		, parentName(parentName)
		, rotationSpace(rotationSpace)
		, positionSpace(positionSpace)
		, handle(handle)
	{
	}

	void Serialize(IArchive& ar);
};

enum
{
	SPACE_JOINT,
	SPACE_ENTITY,
	SPACE_JOINT_WITH_PARENT_ROTATION,
	SPACE_JOINT_WITH_CHARACTER_ROTATION,
	SPACE_SOCKET_RELATIVE_TO_JOINT,
	SPACE_SOCKET_RELATIVE_TO_BINDPOSE
};

inline LocalPosition LocalToEntity(Vec3& position, ukk handle = 0)
{
	return LocalPosition(position, SPACE_ENTITY, "", handle ? handle : &position);
}
inline LocalPosition LocalToJoint(Vec3& position, const string& jointName, ukk handle = 0)
{
	return LocalPosition(position, SPACE_JOINT, jointName.c_str(), handle ? handle : &position);
}

inline LocalPosition LocalToJointCharacterRotation(Vec3& position, const string& jointName, ukk handle = 0)
{
	return LocalPosition(position, SPACE_JOINT_WITH_CHARACTER_ROTATION, jointName.c_str(), handle ? handle : &position);
}

bool Serialize(Serialization::IArchive& ar, Serialization::LocalPosition& value, tukk name, tukk label);
bool Serialize(Serialization::IArchive& ar, Serialization::LocalOrientation& value, tukk name, tukk label);
bool Serialize(Serialization::IArchive& ar, Serialization::LocalFrame& value, tukk name, tukk label);

}

#include "LocalFrameImpl.h"
