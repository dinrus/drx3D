// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _SCREEN_EFFECTS_H_
#define _SCREEN_EFFECTS_H_

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Act/IViewSystem.h>

struct IBlendType;
struct IBlendedEffect;
class	 CBlendGroup;
struct IActor;

//-ScreenEffects------------------------
class CScreenEffects : public ISystemEventListener, public IViewSystemListener
{

public:

	//Some prefenided group IDs
	enum ScreenEffectsGroupId
	{
		eSFX_GID_RBlur			= 1,
		eSFX_GID_ZoomIn			= 2,
		eSFX_GID_ZoomOut		= 3,
		eSFX_GID_HitReaction= 4,
		eSFX_GID_MotionBlur = 5,
		eSFX_GID_Last
	};

	enum CameraShakeGroupId
	{
		eCS_GID_Default = 1,
		eCS_GID_Player = 2,
		eCS_GID_Weapon = 3,
		eCS_GID_HitRecoil = 4,
	};

	CScreenEffects();
	virtual ~CScreenEffects();

	void Reset();

	void ResetAllBlendGroups(bool resetScreen = false);
	
	//Update functions
	void Update(float frameTime);
	void PostUpdate(float frameTime);

	// Camera shake
	void CamShake(Vec3 rotateShake, Vec3 shiftShake, float freq, float shakeTime, float randomness = 0, CameraShakeGroupId shakeID=eCS_GID_Default);

	// List of effect calls (add custom ones here)....
	void ProcessExplosionEffect(float blurRadius, const Vec3& explosionPos);
	void ProcessZoomInEffect();
	void ProcessZoomOutEffect();
	void ProcessSlidingFX();
	void ProcessSprintingFX(bool sprinting, bool isInAir);
	//~ List of effects

	void GetMemoryStatistics(IDrxSizer * s);

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	// IViewSystemListener
	virtual bool OnBeginCutScene(IAnimSequence* pSeq, bool bResetFX) override { return true; }
	virtual bool OnEndCutScene(IAnimSequence* pSeq) override { return true; }
	virtual bool OnCameraChange(const SCameraParams& cameraParams) override;
	// ~IViewSystemListener

private:

	// Update x/y coords
	void SetUpdateCoords(tukk coordsXname, tukk coordsYname, Vec3 pos);

	void EnableBlends(bool enable) {m_enableBlends = enable;};
	void EnableBlends(bool enable, i32 blendGroup);

	// Clear a blend group (deletes running blends)
	void ClearBlendGroup(i32 blendGroup, bool resetScreen = false);
	void ClearAllBlendGroups(bool resetScreen = false);

	// Reset a blend group (do not delete the group)
	void ResetBlendGroup(i32 blendGroup, bool resetScreen = false);
	void ResetScreen();

	void ResetGameEffectPools();

	i32 GetUniqueID();

	// Start a blend
	void StartBlend(IBlendedEffect *effect, IBlendType *blendType, float speed, i32 blendGroup);
	bool HasJobs(i32 blendGroup);


	// Maps blend group IDs to blend groups
	std::map<i32, CBlendGroup*> m_blends;
	std::map<i32, bool> m_enabledGroups;
	i32     m_curUniqueID;
	bool    m_enableBlends;
	bool    m_updatecoords;
	string  m_coordsXname;
	string  m_coordsYname;
	Vec3    m_coords3d;
};

#endif