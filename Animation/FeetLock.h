// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/IAnimationPoseModifier.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

struct SFeetData
{
	QuatT  m_WorldEndEffector;
	u32 m_IsEndEffector;
	void   GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}
};

class CFeetPoseStore :
	public IAnimationPoseModifier
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IAnimationPoseModifier)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CFeetPoseStore, "AnimationPoseModifier_FeetPoseStore", "4095cfb0-96b5-494f-864d-3c007b71d31d"_drx_guid)

	virtual ~CFeetPoseStore() {}

	// IAnimationPoseModifier
public:
	virtual bool Prepare(const SAnimationPoseModifierParams& params) override { return true; }
	virtual bool Execute(const SAnimationPoseModifierParams& params) override;
	virtual void Synchronize() override                                       {}

	void         GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_pFeetData);
	}
public://private:
	SFeetData* m_pFeetData;
};

class CFeetPoseRestore :
	public IAnimationPoseModifier
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(IAnimationPoseModifier)
	DRXINTERFACE_END()

	DRXGENERATE_CLASS_GUID(CFeetPoseRestore, "AnimationPoseModifier_FeetPoseRestore", "90662f0e-d05a-4bf4-8fb6-9924b5da2872"_drx_guid)

	virtual ~CFeetPoseRestore() {}

	// IAnimationPoseModifier
public:
	virtual bool Prepare(const SAnimationPoseModifierParams& params) override { return true; }
	virtual bool Execute(const SAnimationPoseModifierParams& params) override;
	virtual void Synchronize() override                                       {}

	void         GetMemoryUsage(IDrxSizer* pSizer) const override
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_pFeetData);
	}
public://private:
	SFeetData* m_pFeetData;
};

class CFeetLock
{
private:
	IAnimationPoseModifierPtr m_store;
	IAnimationPoseModifierPtr m_restore;

public:
	CFeetLock();

public:
	void                    Reset()   {}
	IAnimationPoseModifier* Store()   { return m_store.get(); }
	IAnimationPoseModifier* Restore() { return m_restore.get(); }

private:
	SFeetData m_FeetData[MAX_FEET_AMOUNT];
};
