#pragma once

#include <drx3D/Maths/Time.h>
#include <drx3D/Animation/JointTransform.h>

namespace drx3d {
/**
 * @brief Class that represents one keyframe of an animation. This contains the timestamp of the keyframe,
 * which is the time (in seconds) from the start of the animation when this keyframe occurs.
 *
 * It also contains the desired local-space transforms of all of the joints in the animated entity at this keyframe in the animation.
 * The joint transforms are stored in a map, indexed by the name of the joint that they should be applied to.
 */
class DRX3D_EXPORT Keyframe {
public:
	/**
	 * Creates a new keyframe.
	 */
	Keyframe() = default;

	/**
	 * Creates a new keyframe at a timestamp.
	 * @param timeStamp The time that this keyframe occurs during the animation.
	 * @param pose The local-space transforms for all the joints at this keyframe, indexed by the name of the joint that they should be applied to.
	 */
	Keyframe(const Time &timeStamp, std::map<STxt, JointTransform> pose);

	void AddJointTransform(const STxt &jointNameId, const Matrix4 &jointLocalTransform);

	/**
	 * Gets the time in seconds of the keyframe in the animation.
	 * @return The time in seconds.
	 */
	const Time &GetTimeStamp() const { return timeStamp; }

	/**
	 * Gets the desired local-space transforms of all the joints at this keyframe, of the animation,
	 * indexed by the name of the joint that they correspond to.
	 * @return The desired local-space transforms.
	 */
	const std::map<STxt, JointTransform> &GetPose() const { return pose; }

	friend const Node &operator>>(const Node &node, Keyframe &keyframe);
	friend Node &operator<<(Node &node, const Keyframe &keyframe);

private:
	Time timeStamp;
	std::map<STxt, JointTransform> pose;
};
}
