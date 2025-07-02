// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfxBaseEnv.h>
#include <drx3D/Schema2/BaseEnv_BasicTypes.h>

#include <drx3D/Schema2/BaseEnv_AutoRegistrar.h>

namespace SchematycBaseEnv
{
	STransform::STransform()
		: pos(ZERO)
		, scale(1.0f)
		, rot(ZERO)
	{}

	void STransform::Serialize(Serialization::IArchive& archive)
	{
		static i32k minAngleValue = 0;
		static i32k maxAngleValue = 360;

		archive(pos, "pos", "Position");
		archive(GameSerialization::RadianAngleDecorator<minAngleValue, maxAngleValue>(rot), "rot", "Rotation");
		archive(scale, "scale", "Scale");
	}

	bool STransform::operator == (const STransform& rhs) const
	{
		return (pos == rhs.pos) && (scale == rhs.scale) && (rot == rhs.rot);
	}

	static const sxema2::SGUID s_transformTypeGUID = "a19ff444-7d11-49ea-80ea-5b629c44f588";

	void RegisterBasicTypes()
	{
		sxema2::IEnvRegistry& envRegistry = gEnv->pSchematyc2->GetEnvRegistry();

		envRegistry.RegisterTypeDesc(sxema2::MakeEnvTypeDescShared(s_transformTypeGUID, "Transform", "Transform", STransform(), sxema2::EEnvTypeFlags::None));
	}
}

SXEMA2_GAME_ENV_AUTO_REGISTER(&SchematycBaseEnv::RegisterBasicTypes)
