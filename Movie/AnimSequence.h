// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __animsequence_h__
#define __animsequence_h__

#pragma once

#include <drx3D/Movie/IMovieSystem.h>

class CAnimSequence : public IAnimSequence
{
public:
	CAnimSequence(IMovieSystem* pMovieSystem, u32 id);

	// Movie system.
	IMovieSystem*                GetMovieSystem() const { return m_pMovieSystem; };

	virtual void                 SetName(tukk name) override;
	virtual tukk          GetName() const override;
	virtual u32               GetId() const override                        { return m_id; }

	SAnimTime                    GetTime() const                               { return m_time; }

	virtual float                GetFixedTimeStep() const override             { return m_fixedTimeStep; }
	virtual void                 SetFixedTimeStep(float dt) override           { m_fixedTimeStep = dt; }

	virtual void                 SetOwner(IAnimSequenceOwner* pOwner) override { m_pOwner = pOwner; }
	virtual IAnimSequenceOwner*  GetOwner() const override                     { return m_pOwner; }

	virtual void                 SetActiveDirector(IAnimNode* pDirectorNode) override;
	virtual IAnimNode*           GetActiveDirector() const override;

	virtual void                 SetFlags(i32 flags) override;
	virtual i32                  GetFlags() const override;
	virtual i32                  GetCutSceneFlags(const bool localFlags = false) const override;

	virtual void                 SetParentSequence(IAnimSequence* pParentSequence) override;
	virtual const IAnimSequence* GetParentSequence() const override;
	virtual bool                 IsAncestorOf(const IAnimSequence* pSequence) const override;

	virtual void                 SetTimeRange(TRange<SAnimTime> timeRange) override;
	virtual TRange<SAnimTime>    GetTimeRange() override { return m_timeRange; };

	virtual i32                  GetNodeCount() const override;
	virtual IAnimNode*           GetNode(i32 index) const override;

	virtual IAnimNode*           FindNodeByName(tukk sNodeName, const IAnimNode* pParentDirector) override;
	virtual IAnimNode*           FindNodeById(i32 nNodeId);

	virtual void                 Reset(bool bSeekToStart) override;
	virtual void                 Pause() override;
	virtual void                 Resume() override;
	virtual bool                 IsPaused() const override;

	virtual void                 OnStart();
	virtual void                 OnStop();

	//! Add animation node to sequence.
	virtual bool       AddNode(IAnimNode* node) override;
	virtual IAnimNode* CreateNode(EAnimNodeType nodeType) override;
	virtual IAnimNode* CreateNode(XmlNodeRef node) override;
	virtual void       RemoveNode(IAnimNode* node) override;
	virtual void       RemoveAll() override;

	virtual void       Activate() override;
	virtual bool       IsActivated() const override { return m_bActive; }
	virtual void       Deactivate() override;

	virtual void       PrecacheData(SAnimTime startTime) override;
	virtual void       PrecacheStatic(const SAnimTime startTime);
	virtual void       PrecacheDynamic(SAnimTime time);
	virtual void       PrecacheEntity(IEntity* pEntity);

	virtual void       StillUpdate() override;
	virtual void       Animate(const SAnimContext& animContext) override;
	virtual void       Render() override;

	virtual void       Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks = true, u32 overrideId = 0, bool bResetLightAnimSet = false) override;

	//! Add/remove track events in sequence
	virtual bool AddTrackEvent(tukk szEvent) override;
	virtual bool RemoveTrackEvent(tukk szEvent) override;
	virtual bool RenameTrackEvent(tukk szEvent, tukk szNewEvent) override;
	virtual bool MoveUpTrackEvent(tukk szEvent) override;
	virtual bool MoveDownTrackEvent(tukk szEvent) override;
	virtual void ClearTrackEvents() override;

	//! Get the track events in the sequence
	virtual i32         GetTrackEventsCount() const override;
	virtual char const* GetTrackEvent(i32 iIndex) const override;

	//! Call to trigger a track event
	virtual void TriggerTrackEvent(tukk event, tukk param = NULL) override;

	//! Track event listener
	virtual void    AddTrackEventListener(ITrackEventListener* pListener) override;
	virtual void    RemoveTrackEventListener(ITrackEventListener* pListener) override;

	virtual DrxGUID GetGUID() const override { return m_guid; }

	//! Sequence audio trigger
	virtual void                         SetAudioTrigger(const SSequenceAudioTrigger& audioTrigger) override { m_audioTrigger = audioTrigger; }
	virtual const SSequenceAudioTrigger& GetAudioTrigger() const override                                    { return m_audioTrigger; }

private:
	void ComputeTimeRange();
	void NotifyTrackEvent(ITrackEventListener::ETrackEventReason reason,
	                      tukk event, tukk param = NULL);

	void ExecuteAudioTrigger(const DrxAudio::ControlId audioTriggerId);

	// Create a new animation node.
	IAnimNode* CreateNodeInternal(EAnimNodeType nodeType, u32 nNodeId = -1);

	bool       AddNodeNeedToRender(IAnimNode* pNode);
	void       RemoveNodeNeedToRender(IAnimNode* pNode);

	typedef std::vector<_smart_ptr<IAnimNode>> AnimNodes;
	AnimNodes         m_nodes;
	AnimNodes         m_nodesNeedToRender;

	DrxGUID           m_guid;
	u32            m_id;
	string            m_name;
	mutable string    m_fullNameHolder;
	TRange<SAnimTime> m_timeRange;
	TrackEvents       m_events;

	// Listeners
	typedef std::list<ITrackEventListener*> TTrackEventListeners;
	TTrackEventListeners  m_listeners;

	i32                   m_flags;

	bool                  m_precached;
	bool                  m_bResetting;

	IAnimSequence*        m_pParentSequence;

	IMovieSystem*         m_pMovieSystem;
	bool                  m_bPaused;
	bool                  m_bActive;

	u32                m_lastGenId;

	IAnimSequenceOwner*   m_pOwner;

	IAnimNode*            m_pActiveDirector;

	SAnimTime             m_time;
	float                 m_fixedTimeStep;

	SSequenceAudioTrigger m_audioTrigger;

	VectorSet<IEntity*>   m_precachedEntitiesSet;
};

#endif // __animsequence_h__
