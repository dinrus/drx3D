// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Movie/AnimNode.h>

struct ISound;
class CGotoTrack;
class CAnimEntityNode;
class CCameraTrack;

class CAnimSceneNode : public CAnimNode
{
public:
	CAnimSceneNode(i32k id);
	~CAnimSceneNode();
	static void            Initialize();

	virtual EAnimNodeType  GetType() const override { return eAnimNodeType_Director; }

	virtual void           Animate(SAnimContext& animContext) override;
	virtual void           CreateDefaultTracks() override;
	virtual void           Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks) override;
	virtual void           Activate(bool bActivate) override;

	bool                   GetCameraBoneLinkQuatT(IEntity* pEntity, QuatT& xform, bool bForceAnimationUpdate);

	virtual void           OnReset() override;
	virtual void           OnPause() override;

	virtual u32   GetParamCount() const override;
	virtual CAnimParamType GetParamType(u32 nIndex) const override;

	virtual void           PrecacheStatic(SAnimTime startTime) override;
	virtual void           PrecacheDynamic(SAnimTime time) override;

protected:
	virtual bool GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const override;

private:
	void         ApplyCameraKey(SCameraKey& key, SAnimContext& animContext);
	void         ApplyEventKey(SEventKey& key, SAnimContext& animContext);
	void         ApplyConsoleKey(SConsoleKey& key, SAnimContext& animContext);
	void         ApplySequenceKey(IAnimTrack* pTrack, i32 nPrevKey, i32 nCurrKey, SSequenceKey& key, SAnimContext& animContext);

	void         ApplyGotoKey(CGotoTrack* poGotoTrack, SAnimContext& animContext);

	bool         GetEntityTransform(IAnimSequence* pSequence, IEntity* pEntity, SAnimTime time, Vec3& vCamPos, Quat& qCamRot);
	bool         GetEntityTransform(IEntity* pEntity, SAnimTime time, Vec3& vCamPos, Quat& qCamRot);

	virtual void InitializeTrackDefaultValue(IAnimTrack* pTrack, const CAnimParamType& paramType) override;

	// Cached parameters of node at given time.
	SAnimTime        m_time;
	CCameraTrack*    m_pCurrentCameraTrack;
	i32              m_currentCameraTrackKeyNumber;
	CAnimEntityNode* m_pCamNodeOnHoldForInterp;
	float            m_backedUpFovForInterp;
	SAnimTime        m_lastPrecachePoint;

	//! Last animated key in track.
	i32                     m_lastCameraKey;
	i32                     m_lastEventKey;
	i32                     m_lastConsoleKey;
	i32                     m_lastSequenceKey;
	i32                     m_nLastGotoKey;
	i32                     m_lastCaptureKey;
	bool                    m_bLastCapturingEnded;
	EntityId                m_currentCameraEntityId;
	ICVar*                  m_cvar_t_FixedStep;
};