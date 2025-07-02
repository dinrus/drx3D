#pragma once

#include <drx3D/Geometry/Model.h>

namespace drx3d {
/**
 * @brief Resource that represents a rectangle model.
 */
class DRX3D_EXPORT RectangleModel : public Model::Registry<RectangleModel> {
	inline static const bool Registered = Register("rectangle");
public:
	/**
	 * Creates a new rectangle model, or finds one with the same values.
	 * @param node The node to decode values from.
	 * @return The rectangle model with the requested values.
	 */
	static std::shared_ptr<RectangleModel> Create(const Node &node);

	/**
	 * Creates a new rectangle model, or finds one with the same values.
	 * @param min The minimum extents.
	 * @param max The maximum extents.
	 * @return The rectangle model with the requested values.
	 */
	static std::shared_ptr<RectangleModel> Create(float min = 0.0f, float max = 1.0f);

	/**
	 * Creates a new rectangle model.
	 * @param min The minimum extents.
	 * @param max The maximum extents.
	 * @param load If this resource will be loaded immediately, otherwise {@link RectangleModel#Load} can be called later.
	 */
	explicit RectangleModel(float min = 0.0f, float max = 1.0f, bool load = true);

	friend const Node &operator>>(const Node &node, RectangleModel &model);
	friend Node &operator<<(Node &node, const RectangleModel &model);

private:
	void Load();

	float min, max;
};
}
