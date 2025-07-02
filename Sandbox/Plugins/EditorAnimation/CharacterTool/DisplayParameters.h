// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Math.h>
#include "QViewportSettings.h"
#include "Serialization.h"

namespace CharacterTool
{

enum CharacterMovement
{
	CHARACTER_MOVEMENT_INPLACE,
	CHARACTER_MOVEMENT_REPEATED,
	CHARACTER_MOVEMENT_CONTINUOUS
};

enum CompressionPreview
{
	COMPRESSION_PREVIEW_COMPRESSED,
	COMPRESSION_PREVIEW_BOTH
};

struct DisplayAnimationOptions
{
	CharacterMovement  movement;
	CompressionPreview compressionPreview;
	bool               showTrail;
	bool               showLocator;
	bool               animationEventGizmos;
	bool               showDccToolOrigin;
	// state variables
	bool               resetCharacter;
	bool               resetGrid;
	bool               updateCameraTarget;

	DisplayAnimationOptions()
		: movement(CHARACTER_MOVEMENT_INPLACE)
		, compressionPreview(COMPRESSION_PREVIEW_COMPRESSED)
		, showTrail(true)
		, showLocator(false)
		, animationEventGizmos(true)
		, showDccToolOrigin(false)
		, resetCharacter(false)
		, resetGrid(false)
		, updateCameraTarget(true)
	{
	}

	void Serialize(IArchive& ar);
};

struct DisplayRenderingOptions
{
	bool showEdges;

	DisplayRenderingOptions()
		: showEdges(false)
	{
	}

	void Serialize(Serialization::IArchive& ar);
};

struct DisplaySkeletonOptions
{
	string jointFilter;
	bool   showJoints;
	bool   showJointNames;
	float  showJointNamesFontSize;
	bool   showSkeletonBoundingBox;

	DisplaySkeletonOptions()
		: showJoints(false)
		, showJointNames(false)
		, showJointNamesFontSize(1.0f)
		, showSkeletonBoundingBox(false)
	{
	}

	void Serialize(Serialization::IArchive& ar);
};

struct DisplaySecondaryAnimationOptions
{
	bool showDynamicProxies;
	bool showAuxiliaryProxies;
	bool showClothProxies;
	bool showRagdollProxies;

	DisplaySecondaryAnimationOptions()
		: showDynamicProxies(false)
		, showAuxiliaryProxies(false)
		, showClothProxies(false)
		, showRagdollProxies(false)
	{
	}

	void Serialize(Serialization::IArchive& ar);
};

struct DisplayPhysicsOptions
{
	enum EDbgRenderProxy	{ DISABLED, SOLID, TRANSLUCENT };
	enum EDbgRenderLimits	{ NONE, ALL, SELECTED };

	EDbgRenderProxy  showPhysicalProxies;
	EDbgRenderLimits showRagdollJointLimits;
	i32              selectedBone;

	DisplayPhysicsOptions()
		: showPhysicalProxies(DISABLED)
		, showRagdollJointLimits(NONE)
		, selectedBone(-1)
	{
	}

	void Serialize(Serialization::IArchive& ar);
};

struct DisplayFollowJointOptions
{
	string jointName;
	bool   lockAlign;
	bool   usePosition;
	bool   useRotation;

	DisplayFollowJointOptions()
		: lockAlign(false)
		, usePosition(true)
		, useRotation(false)
	{
	}

	void Serialize(Serialization::IArchive& ar);
};

struct DisplayOptions
{
	bool                             attachmentGizmos;
	bool                             poseModifierGizmos;
	DisplayAnimationOptions          animation;
	DisplayRenderingOptions          rendering;
	DisplaySkeletonOptions           skeleton;
	DisplaySecondaryAnimationOptions secondaryAnimation;
	DisplayPhysicsOptions            physics;
	DisplayFollowJointOptions        followJoint;
	SViewportSettings                viewport;

	DisplayOptions()
		: attachmentGizmos(true)
		, poseModifierGizmos(true)
	{
	}

	void Serialize(Serialization::IArchive& ar);
};

}

