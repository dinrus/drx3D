// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Audio/IAudioInterfacesCommonData.h>

/**
 * @namespace DrxAudio
 * @brief Most parent audio namespace used throughout the entire engine.
 */
namespace DrxAudio
{
enum class EEventState : EnumFlagsType
{
	None,
	Playing,
	PlayingDelayed,
	Loading,
	Unloading,
	Virtual,
};

/**
 * @namespace DrxAudio::Impl
 * @brief Sub-namespace of the DrxAudio namespace used by audio middleware implementations.
 */
namespace Impl
{
/**
 * @struct SObject3DAttributes
 * @brief A struct holding velocity and transformation of audio objects.
 */
struct SObject3DAttributes
{
	SObject3DAttributes()
		: velocity(ZERO)
	{}

	static SObject3DAttributes const& GetEmptyObject() { static SObject3DAttributes const emptyInstance; return emptyInstance; }

	CObjectTransformation transformation;
	Vec3                  velocity;
};
} //endns Impl
} //endns DrxAudio
