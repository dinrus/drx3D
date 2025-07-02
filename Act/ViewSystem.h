// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "View.h"
#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/Act/ILevelSystem.h>

class CViewSystem : public IViewSystem, public IMovieUser, public ILevelSystemListener
{

private:

	using TViewMap = std::map<u32, CView*>;
	using TViewIdVector = std::vector<u32>;

public:

	//IViewSystem
	virtual IView* CreateView() override;
	virtual void   RemoveView(IView* pView) override;
	virtual void   RemoveView(u32 viewId) override;

	virtual void   SetActiveView(IView* pView) override;
	virtual void   SetActiveView(u32 viewId) override;

	//utility functions
	virtual IView*       GetView(u32 viewId) const override;
	virtual IView*       GetActiveView() const override;
	virtual bool         IsClientActorViewActive() const override;

	virtual u32 GetViewId(IView* pView) const override;
	virtual u32 GetActiveViewId() const override;

	virtual void         Serialize(TSerialize ser) override;
	virtual void         PostSerialize() override;

	virtual IView*       GetViewByEntityId(EntityId id, bool forceCreate) override;

	virtual float        GetDefaultZNear() const override { return m_fDefaultCameraNearZ; };
	virtual void         SetBlendParams(float fBlendPosSpeed, float fBlendRotSpeed, bool performBlendOut) override
	{
		m_fBlendInPosSpeed = fBlendPosSpeed;
		m_fBlendInRotSpeed = fBlendRotSpeed;
		m_bPerformBlendOut = performBlendOut;
	};
	virtual void SetOverrideCameraRotation(bool bOverride, Quat rotation) override;
	virtual bool IsPlayingCutScene() const override                         { return m_cutsceneCount > 0; }

	virtual void SetDeferredViewSystemUpdate(bool const bDeferred) override { m_useDeferredViewSystemUpdate = bDeferred; }
	virtual bool UseDeferredViewSystemUpdate() const override               { return m_useDeferredViewSystemUpdate; }
	virtual void SetControlAudioListeners(bool const bActive) override;
	virtual void UpdateAudioListeners() override;
	//~IViewSystem

	//IMovieUser
	virtual void SetActiveCamera(const SCameraParams& Params) override;
	virtual void BeginCutScene(IAnimSequence* pSeq, u64 dwFlags, bool bResetFX) override;
	virtual void EndCutScene(IAnimSequence* pSeq, u64 dwFlags) override;
	virtual void SendGlobalEvent(tukk pszEvent) override;
	//~IMovieUser

	// ILevelSystemListener
	virtual void OnLevelNotFound(tukk levelName) override                    {}
	virtual void OnLoadingStart(ILevelInfo* pLevel) override;
	virtual void OnLoadingLevelEntitiesStart(ILevelInfo* pLevel) override           {}
	virtual void OnLoadingComplete(ILevelInfo* pLevel) override                     {}
	virtual void OnLoadingError(ILevelInfo* pLevel, tukk error) override     {}
	virtual void OnLoadingProgress(ILevelInfo* pLevel, i32 progressAmount) override {}
	virtual void OnUnloadComplete(ILevelInfo* pLevel) override;
	//~ILevelSystemListener

	explicit CViewSystem(ISystem* const pSystem);
	virtual ~CViewSystem() override;

	void Release() { delete this; };
	void Update(float frameTime);

	//void RegisterViewClass(tukk name, IView *(*func)());

	bool AddListener(IViewSystemListener* pListener) override
	{
		return stl::push_back_unique(m_listeners, pListener);
	}

	bool RemoveListener(IViewSystemListener* pListener) override
	{
		return stl::find_and_erase(m_listeners, pListener);
	}

	void GetMemoryUsage(IDrxSizer* s) const;

	void ClearAllViews();

	bool ShouldApplyHmdOffset() const { return m_bApplyHmdOffset != 0; }

private:

	void RemoveViewById(u32 viewId);
	void ClearCutsceneViews();
	void DebugDraw();

	ISystem* const m_pSystem;

	//TViewClassMap	m_viewClasses;
	TViewMap      m_views;
	TViewIdVector m_cutsceneViewIdVector;

	// Listeners
	std::vector<IViewSystemListener*> m_listeners;

	u32                      m_activeViewId;
	u32                      m_nextViewIdToAssign; // next id which will be assigned
	u32                      m_preSequenceViewId;  // viewId before a movie cam dropped in

	u32                      m_cutsceneViewId;
	u32                      m_cutsceneCount;

	bool                              m_bActiveViewFromSequence;

	bool                              m_bOverridenCameraRotation;
	Quat                              m_overridenCameraRotation;
	float                             m_fCameraNoise;
	float                             m_fCameraNoiseFrequency;

	float                             m_fDefaultCameraNearZ;
	float                             m_fBlendInPosSpeed;
	float                             m_fBlendInRotSpeed;
	bool                              m_bPerformBlendOut;
	i32                               m_nViewSystemDebug;

	i32                               m_bApplyHmdOffset;

	bool                              m_useDeferredViewSystemUpdate;
};
