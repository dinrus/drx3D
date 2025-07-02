// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/IEntityCoverUserComponent.h>
#include <drx3D/AI/MovementRequestID.h>
#include <drx3D/AI/MovementBlock.h>
#include <drx3D/AI/ICoverSystem.h>

struct MovementRequestResult;
struct MovementRequest;

namespace sxema
{
struct IEnvRegistrar;
}

class CEntityAICoverUserComponent final : public IEntityCoverUserComponent
{
public:
	struct SCoverLeftSignal
	{
		static void ReflectType(sxema::CTypeDesc<SCoverLeftSignal>& typeInfo);
		CoverID coverId;
	};
	struct SCoverEnteredSignal
	{
		static void ReflectType(sxema::CTypeDesc<SCoverEnteredSignal>& typeInfo);
		CoverID coverId;
	};
	struct SCoverCompromisedSignal
	{
		static void ReflectType(sxema::CTypeDesc<SCoverCompromisedSignal>& typeInfo);
		CoverID coverId;
	};
	struct SMoveToCoverFailedSignal
	{
		static void ReflectType(sxema::CTypeDesc<SMoveToCoverFailedSignal>& typeInfo);
		CoverID coverId;
	};
	struct SRefreshEyesSignal
	{
		static void ReflectType(sxema::CTypeDesc<SRefreshEyesSignal>& typeInfo);
	};

	static void ReflectType(sxema::CTypeDesc<CEntityAICoverUserComponent>& desc);
	static void Register(sxema::IEnvRegistrar& registrar);

	CEntityAICoverUserComponent();
	virtual ~CEntityAICoverUserComponent();

	// IEntityComponent
	virtual void Initialize() override;
	virtual void OnShutDown() override;

	virtual void   ProcessEvent(const SEntityEvent& event) override;
	virtual uint64 GetEventMask() const override;
	// ~IEntityComponent

	// IEntityCoverUserComponent
	virtual void MoveToCover(const CoverID& coverId) override;
	virtual void SetCallbacks(const SCoverCallbacks& callbacks) override { m_callbacks = callbacks; }
	virtual void SetRefreshEyesCustomFunction(RefreshEyesCustomFunction functor) override { m_refreshEyesCustomFunction = functor; }

	virtual bool IsInCover() const  override { return m_pCoverUser->GetState().Check(ICoverUser::EStateFlags::InCover); }
	virtual bool IsMovingToCover() const  override { return m_pCoverUser->GetState().Check(ICoverUser::EStateFlags::MovingToCover); }
	virtual bool IsCoverCompromised() const  override { return m_pCoverUser->IsCompromised(); }

	virtual bool GetCurrentCoverPosition(Vec3& position) const override;
	virtual bool GetCurrentCoverNormal(Vec3& normal) const override;
	// ~IEntityCoverUserComponent

	void SetState(ICoverUser::StateFlags state);

	void SetCurrentCover(const CoverID& coverId) { DRX_ASSERT(m_pCoverUser); m_pCoverUser->SetCoverID(coverId); }
	void ReserveNextCover(const CoverID& coverId) { DRX_ASSERT(m_pCoverUser); m_pCoverUser->SetNextCoverID(coverId); }

private:
	void Start();
	void Stop();
	bool IsGameOrSimulation() const;

	// Callbacks
	void CreatePlanStartBlocks(DynArray<Movement::BlockPtr>& blocks, const MovementRequest& request);
	void CreatePlanEndBlocks(DynArray<Movement::BlockPtr>& blocks, const MovementRequest& request);
	void MovementRequestCompleted(const MovementRequestResult& result);

	void OnSetCoverCompromised(CoverID coverId, ICoverUser* pCoverUser);
	void FillCoverEyes(DynArray<Vec3>& eyesContainer);
	// ~Callbacks

	void CancelMovementRequest();

	void SetCoverBlacklisted(const CoverID& coverID, bool blacklist, float time);
	bool IsCoverBlackListed(const CoverID& coverId) const;

	void ProcessCoverEnteredEvent(const CoverID& coverID);
	void ProcessCoverLeftEvent(const CoverID& coverID);
	void ProcessCoverCompromisedEvent(const CoverID& coverID);
	void ProcessMoveToCoverFailedEvent(const CoverID& coverID);

	//////////////////////////////////////////////////////////////////////////
	//TODO: Remove in the future
	void AddCoverEye(const Vec3& eyePos);
	CoverID GetRandomCoverId(float radius) const;
	//////////////////////////////////////////////////////////////////////////

	// Properties
	float m_blackListTime = 10.0f;
	float m_effectiveCoverHeight = 0.85f;
	float m_distanceToCover = 0.5f;
	float m_inCoverRadius = 0.3f;

	ICoverUser* m_pCoverUser;
	
	MovementRequestID m_moveToCoverRequestId;

	// Registered callbacks
	SCoverCallbacks m_callbacks;

	// Registered custom functions
	IEntityCoverUserComponent::RefreshEyesCustomFunction m_refreshEyesCustomFunction;

	DynArray<Vec3>* m_pCoverEyesTemp;
};
