// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef TransformationPinning_h
#define TransformationPinning_h

class DRX_ALIGN(32) CTransformationPinning :
	public ITransformationPinning
{
public:
	struct TransformationPinJoint
	{
		enum Type
		{
			Copy		= 'C',
			Feather		= 'F',
			Inherit		= 'I'
		};
	};

	DRXINTERFACE_BEGIN()
		DRXINTERFACE_ADD(IAnimationPoseModifier)
		DRXINTERFACE_ADD(ITransformationPinning)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS(CTransformationPinning, "AnimationPoseModifier_TransformationPin", 0xcc34ddea972e47da, 0x93f9cdcb98c28c8e)

public:

public:
	virtual void SetBlendWeight(float factor) override;
	virtual void SetJoint(u32 jntID) override;
	virtual void SetSource(ICharacterInstance* source) override;

	// IAnimationPoseModifier
public:
	virtual bool Prepare(const SAnimationPoseModifierParams& params) override { return true; }
	virtual bool Execute(const SAnimationPoseModifierParams& params) override;
	virtual void Synchronize() override {}

	void GetMemoryUsage( IDrxSizer *pSizer ) const override
	{
		pSizer->AddObject(this, sizeof(*this));
	}
private:
	float m_factor;

	u32 m_jointID;
	char *m_jointTypes;
	u32 m_numJoints;
	ICharacterInstance* m_source;
	bool m_jointsInitialised;

	void Init(const SAnimationPoseModifierParams& params);

};

#endif // TransformationPinning_h
