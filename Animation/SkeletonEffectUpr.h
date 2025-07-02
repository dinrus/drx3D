// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct ISkeletonAnim;
class CCharInstance;

class CSkeletonEffectUpr
{
public:
	CSkeletonEffectUpr();
	~CSkeletonEffectUpr();

	void   Update(ISkeletonAnim* pSkeletonAnim, ISkeletonPose* pSkeletonPose, const QuatTS& entityLoc);
	void   SpawnEffect(CCharInstance* pCharInstance, const AnimEventInstance& animEvent, const QuatTS& entityLoc);

	void   KillAllEffects();
	size_t SizeOfThis()
	{
		return m_particlesEffects.capacity() * sizeof(EffectEntry);
	}
	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_particlesEffects);
	}

private:
	struct EffectEntry
	{
		EffectEntry(_smart_ptr<IParticleEffect> pEffect, _smart_ptr<IParticleEmitter> pEmitter, i32 boneID, const Vec3& offset, const Vec3& dir);
		~EffectEntry();
		void GetMemoryUsage(IDrxSizer* pSizer) const {}

		_smart_ptr<IParticleEffect>  pEffect;
		_smart_ptr<IParticleEmitter> pEmitter;
		i32                          boneID;
		Vec3                         offset;
		Vec3                         dir;
	};

	void SpawnEffectParticles(CCharInstance* pCharInstance, const AnimEventInstance& animEvent, const QuatTS& entityLoc);
	void SpawnEffectAudio(CCharInstance* pCharInstance, const AnimEventInstance& animEvent, const QuatTS& entityLoc);

	void GetEffectJointLocation(ISkeletonPose* pSkeletonPose, QuatTS& loc, i32 boneID, const Vec3& offset, const Vec3& dir, const QuatTS& entityLoc);

	bool IsPlayingParticlesEffect(tukk effectName);
	DynArray<EffectEntry> m_particlesEffects;
};
