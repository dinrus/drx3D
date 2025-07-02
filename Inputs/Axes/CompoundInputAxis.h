#pragma once

#include <drx3D/Common/NonCopyable.h>
#include <drx3D/Inputs/InputAxis.h>

namespace drx3d {
/**
 * @brief Combines multiple axes inputs into a single axis.
 */
class DRX3D_EXPORT CompoundInputAxis : public InputAxis::Registry<CompoundInputAxis>, NonCopyable {
	inline static const bool Registered = Register("compound");
public:
	/**
	 * Creates a new compound axis.
	 * @param axes The axes that will be combined into a compound axis.
	 */
	explicit CompoundInputAxis(std::vector<std::unique_ptr<InputAxis>> &&axes = {});
	
	/**
	 * Creates a new compound axis.
	 * @tparam Args The axis argument types.
	 * @param args The axes on the being added.
	 */
	template<typename... Args>
	explicit CompoundInputAxis(Args &&... args) {
		axes.reserve(sizeof...(Args));
		(axes.emplace_back(std::forward<Args>(args)), ...);
		ConnectAxes();
	}

	float GetAmount() const override;

	ArgumentDescription GetArgumentDescription() const override;

	const std::vector<std::unique_ptr<InputAxis>> &GetAxes() const { return axes; }
	InputAxis *AddAxis(std::unique_ptr<InputAxis> &&axis);
	void RemoveAxis(InputAxis *axis);

	friend const Node &operator>>(const Node &node, CompoundInputAxis &inputAxis);
	friend Node &operator<<(Node &node, const CompoundInputAxis &inputAxis);

private:
	void ConnectAxis(std::unique_ptr<InputAxis> &axis);
	void ConnectAxes();

	std::vector<std::unique_ptr<InputAxis>> axes;
};
}
