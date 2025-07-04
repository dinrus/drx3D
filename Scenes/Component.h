#pragma once

#include <drx3D/Common/StreamFactory.h>

namespace drx3d {
class Entity;

/**
 * @brief Class that represents a functional component attached to entity.
 */
class DRX3D_EXPORT Component : public StreamFactory<Component> {
	friend class Entity;
public:
	virtual ~Component() = default;

	/**
	 * Run when starting the component if {@link Component#started} is false.
	 */
	virtual void Start() {}

	/**
	 * Run when updating the entity this is attached to.
	 */
	virtual void Update() {}

	bool IsEnabled() const { return enabled; }
	void SetEnabled(bool enable) { this->enabled = enable; }

	bool IsRemoved() const { return removed; }
	void SetRemoved(bool removed) { this->removed = removed; }

	/**
	 * Gets the entity this component is attached to.
	 * @return The entity this component is attached to.
	 */
	Entity *GetEntity() const { return entity; }

	/**
	 * Sets the entity that this component is attached to.
	 * @param entity The new entity this is attached to.
	 */
	void SetEntity(Entity *entity) { this->entity = entity; }

private:
	bool started = false;
	bool enabled = true;
	bool removed = false;
	Entity *entity = nullptr;
};
}
