#pragma once

#include <drx3D/Devices/Joysticks.h>
#include <drx3D/Devices/Windows.h>
#include <drx3D/Files/File.h>
#include <drx3D/Inputs/InputAxis.h>
#include <drx3D/Inputs/InputButton.h>

namespace drx3d {
/**
 * Class is used to abstract and wrap input methods inside a serializable factory.
 */
class DRX3D_EXPORT InputScheme : NonCopyable {
	friend class Inputs;
public:
	using AxisMap = std::map<STxt, std::unique_ptr<InputAxis>>;
	using ButtonMap = std::map<STxt, std::unique_ptr<InputButton>>;
	using JoystickMap = std::map<STxt, JoystickPort>;

	InputScheme() = default;
	explicit InputScheme(const std::filesystem::path &filename);

	InputAxis *GetAxis(const STxt &name);
	InputAxis *AddAxis(const STxt &name, std::unique_ptr<InputAxis> &&axis);
	void RemoveAxis(const STxt &name);

	InputButton *GetButton(const STxt &name);
	InputButton *AddButton(const STxt &name, std::unique_ptr<InputButton> &&button);
	void RemoveButton(const STxt &name);

	const File &GetFile() const { return file; }
	
	friend const Node &operator>>(const Node &node, InputScheme &inputScheme);
	friend Node &operator<<(Node &node, const InputScheme &inputScheme);

private:
	void MoveSignals(InputScheme *other);

	AxisMap axes;
	ButtonMap buttons;

	File file;
};
}
