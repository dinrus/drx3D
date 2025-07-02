// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Input/IInput.h>

class CKinectInputNULL : public IKinectInput
{
public:
	CKinectInputNULL(){}
	~CKinectInputNULL(){}

	virtual bool        Init() override                                                                                                                                             { return false; }
	virtual void        Update() override                                                                                                                                           {}
	virtual bool        IsEnabled() override                                                                                                                                        { return false; }
	virtual tukk GetUserStatusMessage() override                                                                                                                             { return ""; }

	virtual void        RegisterInputListener(IKinectInputListener* pInputListener, tukk name) override                                                                      {}
	virtual void        UnregisterInputListener(IKinectInputListener* pInputListener) override                                                                                      {}
	virtual bool        RegisterArcRail(i32 gripId, i32 railId, const Vec2& vScreenPos, const Vec3& vDir, float fLenght, float fDeadzoneLength, float fToleranceConeAngle) override { return false; }
	virtual void        UnregisterArcRail(i32 gripId) override                                                                                                                      {}
	virtual bool        RegisterHoverTimeRail(i32 gripId, i32 railId, const Vec2& vScreenPos, float fHoverTime, float fTimeTillCommit, SKinGripShape* pGripShape = NULL) override   { return false; }
	virtual void        UnregisterHoverTimeRail(i32 gripId) override                                                                                                                {}
	virtual void        UnregisterAllRails() override                                                                                                                               {}

	virtual bool        GetBodySpaceHandles(SKinBodyShapeHandles& bodyShapeHandles) override                                                                                        { return false; }
	virtual bool        GetSkeletonRawData(u32 iUser, SKinSkeletonRawData& skeletonRawData) const override                                                                       { return false; };
	virtual bool        GetSkeletonDefaultData(u32 iUser, SKinSkeletonDefaultData& skeletonDefaultData) const override                                                           { return false; };
	virtual void        DebugDraw() override                                                                                                                                        {}

	//Skeleton
	virtual void   EnableSeatedSkeletonTracking(bool bValue) override {}
	virtual u32 GetClosestTrackedSkeleton() const override         { return KIN_SKELETON_INVALID_TRACKING_ID; }

	//	Wave
	virtual void  EnableWaveGestureTracking(bool bEnable) override    {};
	virtual float GetWaveGestureProgress(DWORD* pTrackingId) override { return 0.f; }

	// Identity
	virtual bool IdentityDetectedIntentToPlay(DWORD dwTrackingId) override                                    { return false; }
	virtual bool IdentityIdentify(DWORD dwTrackingId, KinIdentifyCallback callbackFunc, uk pData) override { return false; }

	// Speech
	virtual bool SpeechEnable() override                                           { return false; }
	virtual void SpeechDisable() override                                          {}
	virtual void SetEnableGrammar(const string& grammarName, bool enable) override {}
	virtual bool KinSpeechSetEventInterest(u64 ulEvents) override        { return false; }
	virtual bool KinSpeechLoadDefaultGrammar() override                            { return false; }
	virtual bool KinSpeechStartRecognition() override                              { return false; }
	virtual void KinSpeechStopRecognition() override                               {};
};
