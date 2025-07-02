#pragma once

#include <drx3D/Maths/Time.h>
#include <drx3D/Animation/Animation.h>
#include <drx3D/Animation/Skeleton/Joint.h>

namespace drx3d {

class DRX3D_EXPORT Animator {
public:
	void Update(const Joint &rootJoint, std::vector<Matrix4> &jointMatrices);
	void IncreaseAnimationTime();
	std::map<STxt, Matrix4> CalculateCurrentAnimationPose() const;

	/**
	 * Находит в анимации предыдущий и следующий ключевой кадр, возвращая их массивом длиной в 2.
	 * Если предыдущий кадр отстствует (вероятно, текущее время анимации равно 0.5, а первый
	 * ключевой кадр по времени приходится на 1.5), тогда следующий ключевой кадр используется
	 * за оба сразу. Обратный случай бывает, когда отсутствует следующий кадр.
	 * @return The previous and next keyframes, in an array which therefore will always have a length of 2.
	 */
	std::pair<Keyframe, Keyframe> GetPreviousAndNextFrames() const;

	/**
	 * Calculates how far between the previous and next keyframe the current animation time is, and returns it as a value between 0 and 1.
	 * @param previousFrame The previous keyframe in the animation.
	 * @param nextFrame The next keyframe in the animation.
	 * </param>
	 * @return A number between 0 and 1 indicating how far between the two keyframes the current animation time is.
	 */
	float CalculateProgression(const Keyframe &previousFrame, const Keyframe &nextFrame) const;

	/**
	 * Calculates all the local-space joint transforms for the desired current pose by interpolating between
	 * the transforms at the previous and next keyframes.
	 * @param previousFrame The previous keyframe in the animation.
	 * @param nextFrame The next keyframe in the animation.
	 * @param progression A number between 0 and 1 indicating how far between the previous and next keyframes the current animation time is.
	 * </param>
	 * @return The local-space transforms for all the joints for the desired current pose.
	 * They are returned in a map, indexed by the name of the joint to which they should be applied. </returns>
	 */
	std::map<STxt, Matrix4> InterpolatePoses(const Keyframe &previousFrame, const Keyframe &nextFrame, float progression) const;

	/**
	 * This method applies the current pose to a given joint, and all of its descendants.
	 * It does this by getting the desired local-transform for the
	 * current joint, before applying it to the joint. Before applying the
	 * transformations it needs to be converted from local-space to model-space
	 * (so that they are relative to the model's origin, rather than relative to
	 * the parent joint). This can be done by multiplying the local-transform of
	 * the joint with the model-space transform of the parent joint.
	 *
	 * The same thing is then done to all the child joints.
	 *
	 * Finally the inverse of the joint's bind transform is multiplied with the
	 * model-space transform of the joint. This basically "subtracts" the
	 * joint's original bind (no animation applied) transform from the desired
	 * pose transform. The result of this is then the transform required to move
	 * the joint from its original model-space transform to it's desired
	 * model-space posed transform. This is the transform that needs to be
	 * loaded up to the vertex shader and used to transform the vertices into
	 * the current pose.
	 * @param currentPose A map of the local-space transforms for all the joints for the desired pose. The map is indexed by the name of the joint which the transform corresponds to.
	 * @param joint The current joint which the pose should be applied to.
	 * @param parentTransform The desired model-space transform of the parent joint for the pose.
	 * @param jointMatrices The transforms that get loaded up to the shader and is used to deform the vertices of the "skin".
	 */
	static void CalculateJointPose(const std::map<STxt, Matrix4> &currentPose, const Joint &joint, const Matrix4 &parentTransform, std::vector<Matrix4> &jointMatrices);

	const Animation *GetCurrentAnimation() const { return currentAnimation; }

	/**
	 * Indicates that the entity should carry out the given animation. Resets the animation time so that the new animation starts from the beginning.
	 * @param animation The new animation to carry out.
	 */
	void DoAnimation(Animation *animation);

private:
	Time animationTime;
	Animation *currentAnimation = nullptr;
};
}
