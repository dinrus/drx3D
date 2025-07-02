// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CAttachmentPROW;
class CAttachmentBONE;

namespace Command
{

class CEvaluationContext;
class CState;

enum
{
	eClearPoseBuffer = 0,
	eAddPoseBuffer,     //reads content from m_SourceBuffer, multiplies the pose by a blend weight, and adds the result to the m_TargetBuffer

	eSampleAddAnimFull,
	eScaleUniformFull,
	eNormalizeFull,

	eSampleAddAnimPart, // Layer-Sampling for Override and Additive. This command adds a sample partial-body animation and its per-joint Blend-Weights to a destination buffer.
	ePerJointBlending,  // Layer-Blending for Override and Additive. This command is using Blend-Weigths per joint, which can be different for positions and orientations

	eJointMask,
	ePoseModifier,

	//just for debugging
	eSampleAddPoseFull, //used to playback the frames in a CAF-file which stored is in GlobalAnimationHeaderAIM
	eVerifyFull,

	eUpdateRedirectedJoint,
	eUpdatePendulumRow,
	ePrepareAllRedirectedTransformations,
	eGenerateProxyModelRelativeTransformations,

	eComputeAbsolutePose,
	eProcessAnimationDrivenIk,
	ePhysicsSync,
};

//this command deletes all previous entries in a pose-buffer (no matter if Temp or Target-Buffer)
struct ClearPoseBuffer
{
	enum { ID = eClearPoseBuffer };

	u8 m_nCommand;
	u8 m_TargetBuffer;
	u8 m_nJointStatus;
	u8 m_nPoseInit;

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

struct AddPoseBuffer
{
	enum { ID = eAddPoseBuffer };

	u8 m_nCommand;
	u8 m_SourceBuffer;
	u8 m_TargetBuffer;
	u8 m_IsEmpty; //temporary
	f32   m_fWeight;

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

struct SampleAddAnimFull
{
	enum { ID = eSampleAddAnimFull };
	enum
	{
		Flag_ADMotion  = 1,
		Flag_TmpBuffer = 8,
	};

	u8 m_nCommand;
	u8 m_flags;
	i16 m_nEAnimID;
	f32   m_fETimeNew; //this is a percentage value between 0-1
	f32   m_fWeight;

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

struct ScaleUniformFull
{
	enum { ID = eScaleUniformFull };

	u8 m_nCommand;
	u8 m_TargetBuffer;
	u8 _PADDING0;
	u8 _PADDING1;
	f32   m_fScale;

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

struct NormalizeFull
{
	enum { ID = eNormalizeFull };

	u8 m_nCommand;
	u8 m_TargetBuffer;
	u8 _PADDING0;
	u8 _PADDING1;

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

struct SampleAddAnimPart
{
	enum { ID = eSampleAddAnimPart };

	u8 m_nCommand;
	u8 m_TargetBuffer;
	u8 m_SourceBuffer;
	u8 _PADDING1;

	i32 m_nEAnimID;

	f32   m_fAnimTime; //this is a percentage value between 0-1
	f32   m_fWeight;

#if defined(USE_PROTOTYPE_ABS_BLENDING)
	strided_pointer<i32k>   m_maskJointIDs;
	strided_pointer<const float> m_maskJointWeights;
	i32                          m_maskNumJoints;
#endif //!defined(USE_PROTOTYPE_ABS_BLENDING)

	void Execute(const CState& state, CEvaluationContext& context) const;
};

struct PerJointBlending
{
	enum { ID = ePerJointBlending };

	u8 m_nCommand;
	u8 m_SourceBuffer;
	u8 m_TargetBuffer;
	u8 m_BlendMode; //0=Override / 1=Additive

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

//this is only for debugging of uncompiled aim-pose CAFs
struct SampleAddPoseFull
{
	enum { ID = eSampleAddPoseFull };
	u8 m_nCommand;
	u8 m_flags;
	i16 m_nEAnimID;
	f32   m_fETimeNew; //this is a percentage value between 0-1
	f32   m_fWeight;
	void  Execute(const CState& state, CEvaluationContext& context) const;
};

struct PoseModifier
{
	enum { ID = ePoseModifier };

	u8                   m_nCommand;
	u8                   m_TargetBuffer;
	u8                   _PADDING0;
	u8                   _PADDING1;
	IAnimationPoseModifier* m_pPoseModifier;

	void                    Execute(const CState& state, CEvaluationContext& context) const;
};

struct JointMask
{
	enum { ID = eJointMask };

	u8         m_nCommand;
	u8         m_count;
	u8         _PADDING0;
	u8         _PADDING1;
	u32k* m_pMask;

	void          Execute(const CState& state, CEvaluationContext& context) const;
};

struct VerifyFull
{
	enum { ID = eVerifyFull };

	u8 m_nCommand;
	u8 m_TargetBuffer;
	u8 _PADDING0;
	u8 _PADDING1;

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

struct UpdateRedirectedJoint
{
	enum { ID = eUpdateRedirectedJoint };
	u8            m_nCommand;
	u8            _PADDING[3];
	CAttachmentBONE* m_attachmentBone;

	void             Execute(const CState& state, CEvaluationContext& context) const;
};

struct UpdatePendulumRow
{
	enum { ID = eUpdatePendulumRow };
	u8            m_nCommand;
	u8            _PADDING[3];
	CAttachmentPROW* m_attachmentPendulumRow;

	void             Execute(const CState& state, CEvaluationContext& context) const;
};

struct PrepareAllRedirectedTransformations
{
	enum { ID = ePrepareAllRedirectedTransformations };
	u8 m_nCommand;
	u8 _PADDING[3];

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

struct GenerateProxyModelRelativeTransformations
{
	enum { ID = eGenerateProxyModelRelativeTransformations };
	u8 m_nCommand;
	u8 _PADDING[3];

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

struct ComputeAbsolutePose
{
	enum { ID = eComputeAbsolutePose };
	u8 m_nCommand;
	u8 _PADDING[3];

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

struct ProcessAnimationDrivenIk
{
	enum { ID = eProcessAnimationDrivenIk };
	u8 m_nCommand;
	u8 _PADDING[3];

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

struct PhysicsSync
{
	enum { ID = ePhysicsSync };
	u8 m_nCommand;
	u8 _PADDING[3];

	void  Execute(const CState& state, CEvaluationContext& context) const;
};

} //endns Command
