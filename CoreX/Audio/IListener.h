// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

/**
 * @namespace DrxAudio
 * @brief Most parent audio namespace used throughout the entire engine.
 */
namespace DrxAudio
{
/**
 * @struct DrxAudio::IListener
 * @brief A struct exposing public methods to interact with audio listeners.
 */
struct IListener
{
	// <interfuscator:shuffle>
	/** @cond */
	virtual ~IListener() = default;
	/** @endcond */

	/**
	 * Sets the listener's transformation.
	 * @param transformation - constant reference to an object holding the transformation to apply.
	 * @param userData - optional struct used to pass additional data to the internal request.
	 * @return void
	 */
	virtual void SetTransformation(CObjectTransformation const& transformation, SRequestUserData const& userData = SRequestUserData::GetEmptyObject()) = 0;

	/**
	 * Sets this audio listener's name.
	 * Is used only during production whenever the owning entity's name is changed to adjust corresponding audio listeners as well.
	 * @param szName - name to set.
	 * @param userData - optional struct used to pass additional data to the internal request.
	 * @return void
	 */
	virtual void SetName(char const* const szName, SRequestUserData const& userData = SRequestUserData::GetEmptyObject()) = 0;
	// </interfuscator:shuffle>
};
} //endns DrxAudio