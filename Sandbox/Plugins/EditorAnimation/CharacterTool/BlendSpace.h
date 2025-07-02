// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <vector>
#include "Serialization.h"
#include <drx3D/CoreX/String/NameCRCHelper.h>

struct IAnimationSet;

namespace CharacterTool
{

using std::vector;

struct BlendSpaceDimension
{
	i32  parameterId;
	float  minimal;
	float  maximal;
	u8  cellCount;
	float  debugVisualScale;
	float  startKey;
	float  endKey;
	bool   locked;

	BlendSpaceDimension()
		: parameterId(eMotionParamID_INVALID)
		, minimal(0)
		, maximal(1)
		, cellCount(8)
		, debugVisualScale(1.0f)
		, startKey(0.0f)
		, endKey(1.0f)
		, locked(false)
	{
	}

	void Serialize(IArchive& ar);

};
typedef vector<BlendSpaceDimension> BlendSpaceDimensions;

struct BlendSpaceAnnotation
{
	vector<DrxGUID> exampleGuids;

	void Serialize(IArchive& ar);
};
typedef vector<BlendSpaceAnnotation> BlendSpaceAnnotations;

struct BlendSpaceExample
{
	struct SParameter
	{
		float value;
		bool userDefined;
		bool useDirectlyForDeltaMotion; //!< Serialized, but not exposed in GUI.
	};

	DrxGUID runtimeGuid; //<! GUID used to identify this object at runtime, not serialized to file.

	string animation;
	SParameter parameters[eMotionParamID_COUNT];
	float  playbackScale;

	BlendSpaceExample()
		: runtimeGuid(DrxGUID::Create())
		, animation()
		, parameters()
		, playbackScale(1.0f)
	{
	}

	void Serialize(IArchive& ar);
};
typedef vector<BlendSpaceExample> BlendSpaceExamples;

struct BlendSpaceAdditionalExtraction
{
	i32 parameterId;

	BlendSpaceAdditionalExtraction()
		: parameterId(eMotionParamID_INVALID)
	{
	}

	void Serialize(IArchive& ar);
};
typedef vector<BlendSpaceAdditionalExtraction> BlendSpaceAdditionalExtractions;

struct BlendSpacePseudoExample
{
	DrxGUID runtimeGuid; //<! GUID used to identify this object at runtime, not serialized to file.

	DrxGUID guid0;
	DrxGUID guid1;
	float weight0;
	float weight1;

	BlendSpacePseudoExample()
		: runtimeGuid(DrxGUID::Create())
		, guid0(DrxGUID::Null())
		, guid1(DrxGUID::Null())
		, weight0(0.5f)
		, weight1(0.5f)
	{
	}

	void Serialize(IArchive& ar);
};
typedef vector<BlendSpacePseudoExample> BlendSpacePseudoExamples;

struct BlendSpaceMotionCombination
{
	string animation;
};
typedef vector<BlendSpaceMotionCombination> BlendSpaceMotionCombinations;

inline bool Serialize(IArchive& ar, BlendSpaceMotionCombination& anim, tukk name, tukk label)
{
	return ar(AnimationAlias(anim.animation), name, label);
}

struct BlendSpaceJoint
{
	string name;

	bool operator<(const BlendSpaceJoint& rhs) const { return name < rhs.name; }
};

inline bool Serialize(IArchive& ar, BlendSpaceJoint& joint, tukk name, tukk label)
{
	return ar(JointName(joint.name), name, label);
}

struct BlendSpaceVirtualExample1D
{
	u8 i0, i1;
	float w0, w1;

	void  Serialize(IArchive& ar)
	{
		ar(i0, "i0", "^i0");
		ar(i1, "i1", "^i1");
		ar(w0, "w0", "^w0");
		ar(w1, "w1", "^w1");
	}
};

struct BlendSpaceVirtualExample2D
{
	u8 i0, i1, i2, i3;
	float w0, w1, w2, w3;

	void  Serialize(IArchive& ar)
	{
		ar(i0, "i0", "^i0");
		ar(i1, "i1", "^i1");
		ar(i2, "i2", "^i2");
		ar(i3, "i3", "^i3");
		ar(w0, "w0", "^w0");
		ar(w1, "w1", "^w1");
		ar(w2, "w2", "^w2");
		ar(w3, "w3", "^w3");
	}
};

struct BlendSpaceVirtualExample3D
{
	u8 i0, i1, i2, i3, i4, i5, i6, i7;
	float w0, w1, w2, w3, w4, w5, w6, w7;

	void  Serialize(IArchive& ar)
	{
		ar(i0, "i0", "^i0");
		ar(i1, "i1", "^i1");
		ar(i2, "i2", "^i2");
		ar(i3, "i3", "^i3");
		ar(i4, "i4", "^i4");
		ar(i5, "i5", "^i5");
		ar(i6, "i6", "^i6");
		ar(i7, "i7", "^i7");
		ar(w0, "w0", "^w0");
		ar(w1, "w1", "^w1");
		ar(w2, "w2", "^w2");
		ar(w3, "w3", "^w3");
		ar(w4, "w4", "^w4");
		ar(w5, "w5", "^w5");
		ar(w6, "w6", "^w6");
		ar(w7, "w7", "^w7");
	}
};

struct BlendSpace
{
	float                               m_threshold;
	bool                                m_idleToMove;
	BlendSpaceDimensions                m_dimensions;
	BlendSpaceExamples                  m_examples;
	BlendSpacePseudoExamples            m_pseudoExamples;
	BlendSpaceAdditionalExtractions     m_additionalExtraction;
	BlendSpaceAnnotations               m_annotations;
	vector<BlendSpaceMotionCombination> m_motionCombinations;
	vector<BlendSpaceJoint>             m_joints;
	vector<BlendSpaceVirtualExample1D>  m_virtualExamples1d;
	vector<BlendSpaceVirtualExample2D>  m_virtualExamples2d;
	vector<BlendSpaceVirtualExample3D>  m_virtualExamples3d;

	bool       LoadFromXml(string& errorMessage, XmlNodeRef root, IAnimationSet* pAnimationSet);
	XmlNodeRef SaveToXml() const;

	BlendSpace()
		: m_idleToMove(false)
		, m_threshold(-99999.f)
	{
	}

	void Serialize(IArchive& ar);

};

struct CombinedBlendSpaceDimension
{
	i32    parameterId;
	bool   locked;
	float  parameterScale;
	bool   chooseBlendSpace;

	CombinedBlendSpaceDimension()
		: parameterId(eMotionParamID_INVALID)
		, locked(false)
		, parameterScale(1.0f)
		, chooseBlendSpace(false)
	{
	}

	void Serialize(IArchive& ar);
};
typedef vector<CombinedBlendSpaceDimension> CombinedBlendSpaceDimensions;

struct BlendSpaceReference
{
	string path;
};
typedef vector<BlendSpaceReference> BlendSpaceReferences;

bool Serialize(IArchive& ar, BlendSpaceReference& ref, tukk name, tukk label);

struct CombinedBlendSpace
{
	bool                            m_idleToMove;
	CombinedBlendSpaceDimensions    m_dimensions;
	BlendSpaceAdditionalExtractions m_additionalExtraction;
	BlendSpaceReferences            m_blendSpaces;
	BlendSpaceMotionCombinations    m_motionCombinations;
	vector<BlendSpaceJoint>         m_joints;

	CombinedBlendSpace()
		: m_idleToMove(false)
	{
	}

	void       Serialize(IArchive& ar);
	bool       LoadFromXml(string& errorMessage, XmlNodeRef root, IAnimationSet* pAnimationSet);
	XmlNodeRef SaveToXml() const;
};

}

