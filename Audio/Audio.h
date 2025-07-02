#pragma once

#include <drx3D/Devices/rocket.h>
#include <drx3D/Engine/Engine.h>

namespace drx3d {
/**
 * @brief Module used for loading, managing and playing a variety of different sound types.
 */
class DRX3D_EXPORT Audio : public Module::Registry<Audio> {
	inline static const bool Registered = Register(Stage::Pre);
public:
	enum class Type {
		Master, General, Effect, Music
	};

	Audio();
	~Audio();

	void Update() override;

	DRX3D_NO_EXPORT static STxt StringifyResultAl(int32_t result);
	DRX3D_NO_EXPORT static void CheckAl(int32_t result);

	float GetGain(Type type) const;
	void SetGain(Type type, float volume);

	/**
	 * Called when a gain value has been modified.
	 * @return The delegate.
	 */
	rocket::signal<void(Type, float)> &OnGain() { return onGain; }

private:
	// TODO: Only using p-impl because of signature differences from OpenAL and OpenALSoft.
	struct _intern;
	std::unique_ptr<_intern> impl;

	std::map<Type, float> gains;

	rocket::signal<void(Type, float)> onGain;
};
}
