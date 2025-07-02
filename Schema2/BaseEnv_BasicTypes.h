// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema2/BaseEnv_Prerequisites.h>

namespace SchematycBaseEnv
{
	struct STransform
	{
		enum class ESerialize : u32
		{
			Position = BIT(0),
			Rotation = BIT(1),
			Scale    = BIT(2),

			All = Position | Rotation | Scale
		};

		STransform();

		void Serialize(Serialization::IArchive& archive);

		bool operator == (const STransform& rhs) const;

		Vec3 pos;
		Vec3 scale;
		Ang3 rot;
	};

	struct ETransformComponent
	{
		enum : u32
		{
			None     = 0,
			Position = BIT(0),
			Rotation = BIT(1),
			Scale    = BIT(2),

			PositionAndRotation = Position | Rotation,
			PositionAndScale    = Position | Scale,
			RotationAndScale    = Rotation | Scale,
			All                 = Position | Rotation| Scale
		};
	};

	template <u32 EDITABLE_COMPONENT> class CTransformDecorator
	{
	public:

		CTransformDecorator(SchematycBaseEnv::STransform& _transform)
			: m_transform(_transform)
		{}

		void Serialize(Serialization::IArchive& archive)
		{
			archive(m_transform.pos, "pos", (EDITABLE_COMPONENT & ETransformComponent::Position) ? "Position" : "!Position");
			archive(GameSerialization::RadianAngleDecorator<0, 360>(m_transform.rot), "rot", (EDITABLE_COMPONENT & ETransformComponent::Rotation) ? "Rotation" : "!Rotation");
			archive(m_transform.scale, "scale", (EDITABLE_COMPONENT & ETransformComponent::Scale) ? "Scale" : "!Scale");
		}

	private:

		SchematycBaseEnv::STransform& m_transform;
	};

	template <u32 EDITABLE_COMPONENT> inline CTransformDecorator<EDITABLE_COMPONENT> TransformDecorator(SchematycBaseEnv::STransform& value)
	{
		return CTransformDecorator<EDITABLE_COMPONENT>(value);
	}
}