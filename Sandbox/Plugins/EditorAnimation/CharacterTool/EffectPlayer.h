// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <IEditor.h>

struct IParticleEffect;
struct IParticleEmitter;
struct ISkeletonAnim;
struct ISkeletonPose;
struct IDefaultSkeleton;
struct SRendParams;
struct SRenderingPassInfo;

namespace CharacterTool {

class EffectPlayer
	: public IEditorNotifyListener
{
public:
	EffectPlayer();
	~EffectPlayer();

	void SetSkeleton(ISkeletonAnim* pSkeletonAnim, ISkeletonPose* pSkeletonPose, IDefaultSkeleton* pIDefaultSkeleton);
	void Update(const QuatT& rPhysEntity);
	void SpawnEffect(tukk effectName, tukk boneName, const Vec3& offset, const Vec3& dir);
	void KillAllEffects();
	void Render(SRendParams& params, const SRenderingPassInfo& passInfo);

	// IEditorNotifyListener
	void OnEditorNotifyEvent(EEditorNotifyEvent event) override;
	// ~IEditorNotifyListener

private:
	struct EffectEntry
	{
		EffectEntry(const _smart_ptr<IParticleEffect>& pEffect, const _smart_ptr<IParticleEmitter>& pEmitter, i32 boneID, const Vec3& offset, const Vec3& dir);
		~EffectEntry();

		_smart_ptr<IParticleEffect>  pEffect;
		_smart_ptr<IParticleEmitter> pEmitter;
		i32                          boneID;
		Vec3                         offset;
		Vec3                         dir;
	};

	void GetEffectTM(Matrix34& tm, i32 boneID, const Vec3& offset, const Vec3& dir);
	bool IsPlayingEffect(tukk effectName);

	ISkeletonAnim*        m_pSkeleton2;
	ISkeletonPose*        m_pSkeletonPose;
	IDefaultSkeleton*     m_pIDefaultSkeleton;
	DynArray<EffectEntry> m_effects;
};

}

