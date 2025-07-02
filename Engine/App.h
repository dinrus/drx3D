#pragma once

#include <drx3D/Devices/rocket.h>

#include <drx3D/Export.h>

#ifdef major
#undef major
#endif
#ifdef minor
#undef minor
#endif

namespace drx3d {
class DRX3D_EXPORT Version {
public:
	Version(uint8_t major, uint8_t minor, uint8_t patch) :
		major(major),
		minor(minor),
		patch(patch) {
	}

	uint8_t major, minor, patch;
};

/**
 * @brief Class that represents a application.
 */
class DRX3D_EXPORT App : public virtual rocket::trackable {
	friend class Engine;
public:
	explicit App(STxt name, const Version &version = {1, 0, 0}) :
		name(std::move(name)),
		version(version) {
	}

	virtual ~App() = default;

	/**
	 * Run when switching to this app from another.
	 */
	virtual void Start() = 0;

	/**
	 * Run before the module update pass.
	 */
	virtual void Update() = 0;

	/**
	 * Gets the application's name.
	 * @return The application's name.
	 */
	const STxt &GetName() const { return name; }

	/**
	 * Sets the application's name, for driver support.
	 * @param name The application's name.
	 */
	void SetName(const STxt &name) { this->name = name; }

	/**
	 * Gets the application's version.
	 * @return The application's version.
	 */
	const Version &GetVersion() const { return version; }

	/**
	 * Sets the application's version, for driver support.
	 * @param version The application version.
	 */
	void SetVersion(const Version &version) { this->version = version; }

private:
	bool started = false;
	STxt name;
	Version version;
};
}
