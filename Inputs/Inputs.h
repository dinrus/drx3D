#pragma once

#include <drx3D/Engine/Engine.h>
#include <drx3D/Inputs/InputScheme.h>

namespace drx3d {
/**
 * @brief Module used for managing abstract inputs organized in schemes.
 */
class DRX3D_EXPORT Inputs : public Module::Registry<Inputs> {
	inline static const bool Registered = Register(Stage::Pre, Requires<Windows, Joysticks>());
public:
	Inputs();

	void Update() override;

	InputScheme *GetScheme() const { return currentScheme; }
	InputScheme *GetScheme(const STxt &name) const;
	InputScheme *AddScheme(const STxt &name, std::unique_ptr<InputScheme> &&scheme, bool setCurrent = false);
	void RemoveScheme(const STxt &name);
	void SetScheme(InputScheme *scheme);
	void SetScheme(const STxt &name);

	InputAxis *GetAxis(const STxt &name) const;
	InputButton *GetButton(const STxt &name) const;

private:
	std::map<STxt, std::unique_ptr<InputScheme>> schemes;
	std::unique_ptr<InputScheme> nullScheme;
	InputScheme *currentScheme = nullptr;
};
}
