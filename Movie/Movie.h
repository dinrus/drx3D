// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>

struct PlayingSequence
{
	//! Sequence playing
	_smart_ptr<IAnimSequence> sequence;

	//! Start/End/Current playing time for this sequence.
	SAnimTime startTime;
	SAnimTime endTime;
	SAnimTime currentTime;
	float     currentSpeed;

	//! Sequence from other sequence's sequence track
	bool trackedSequence;
	bool bSingleFrame;
};

class CLightAnimWrapper : public ILightAnimWrapper
{
public:
	// ILightAnimWrapper interface
	virtual bool Resolve() override;

public:
	static CLightAnimWrapper* Create(tukk name);
	static void ReconstructCache();
	static IAnimSequence* GetLightAnimSet();
	static void SetLightAnimSet(IAnimSequence* pLightAnimSet);
	static void InvalidateAllNodes();

private:
	typedef std::map<string, CLightAnimWrapper*> LightAnimWrapperCache;
	static LightAnimWrapperCache ms_lightAnimWrapperCache;
	static _smart_ptr<IAnimSequence> ms_pLightAnimSet;

private:
	static CLightAnimWrapper* FindLightAnim(tukk name);
	static void CacheLightAnim(tukk name, CLightAnimWrapper* p);
	static void RemoveCachedLightAnim(tukk name);

private:
	CLightAnimWrapper(tukk name);
	virtual ~CLightAnimWrapper();
};

struct IConsoleCmdArgs;
struct ISkeletonAnim;

class CMovieSystem : public IMovieSystem
{
	typedef std::vector<PlayingSequence> PlayingSequences;

public:
	CMovieSystem(ISystem* system);
	virtual ~CMovieSystem();

	virtual void           Release() override                  { delete this; };

	virtual void           SetUser(IMovieUser* pUser) override { m_pUser = pUser; }
	virtual IMovieUser*    GetUser() override                  { return m_pUser; }

	virtual bool           Load(tukk pszFile, tukk pszMission) override;

	virtual ISystem*       GetSystem() override { return m_pSystem; }

	virtual IAnimSequence* CreateSequence(tukk sequence, bool bLoad = false, u32 id = 0) override;
	virtual IAnimSequence* LoadSequence(tukk pszFilePath);
	virtual IAnimSequence* LoadSequence(XmlNodeRef& xmlNode, bool bLoadEmpty = true) override;

	virtual void           AddSequence(IAnimSequence* pSequence) override;
	virtual void           RemoveSequence(IAnimSequence* pSequence) override;
	virtual IAnimSequence* FindSequence(tukk sequence) const override;
	virtual IAnimSequence* FindSequenceById(u32 id) const override;
	virtual IAnimSequence* FindSequenceByGUID(DrxGUID guid) const override;
	virtual IAnimSequence* GetSequence(i32 i) const override;
	virtual i32            GetNumSequences() const override;
	virtual IAnimSequence* GetPlayingSequence(i32 i) const override;
	virtual i32            GetNumPlayingSequences() const override;
	virtual bool           IsCutScenePlaying() const override;

	virtual u32         GrabNextSequenceId() override { return m_nextSequenceId++; }

	virtual bool           AddMovieListener(IAnimSequence* pSequence, IMovieListener* pListener) override;
	virtual bool           RemoveMovieListener(IAnimSequence* pSequence, IMovieListener* pListener) override;

	virtual void           RemoveAllSequences() override;

	//////////////////////////////////////////////////////////////////////////
	// Sequence playback.
	//////////////////////////////////////////////////////////////////////////
	virtual void                                PlaySequence(tukk sequence, IAnimSequence* parentSeq = NULL, bool bResetFX = true,
	                                                         bool bTrackedSequence = false, SAnimTime startTime = SAnimTime::Min(), SAnimTime endTime = SAnimTime::Min()) override;
	virtual void                                PlaySequence(IAnimSequence* seq, IAnimSequence* parentSeq = NULL, bool bResetFX = true,
	                                                         bool bTrackedSequence = false, SAnimTime startTime = SAnimTime::Min(), SAnimTime endTime = SAnimTime::Min()) override;
	virtual void                                PlayOnLoadSequences() override;

	virtual bool                                StopSequence(tukk sequence) override;
	virtual bool                                StopSequence(IAnimSequence* seq) override;
	virtual bool                                AbortSequence(IAnimSequence* seq, bool bLeaveTime = false) override;

	virtual void                                StopAllSequences() override;
	virtual void                                StopAllCutScenes() override;
	void                                        Pause(bool bPause);

	virtual void                                Reset(bool bPlayOnReset, bool bSeekToStart) override;
	virtual void                                StillUpdate() override;
	virtual void                                PreUpdate(const float dt) override;
	virtual void                                PostUpdate(const float dt) override;
	virtual void                                Render() override;

	virtual void                                StartCapture(IAnimSequence* seq, const SCaptureKey& key) override;
	virtual void                                EndCapture() override;
	virtual void                                ControlCapture() override;

	virtual bool                                IsPlaying(IAnimSequence* seq) const override;

	virtual void                                Pause() override;
	virtual void                                Resume() override;

	virtual void                                EnableCameraShake(bool bEnabled) override       { m_bEnableCameraShake = bEnabled; };
	virtual bool                                IsCameraShakeEnabled() const override           { return m_bEnableCameraShake; };

	virtual void                                SetCallback(IMovieCallback* pCallback) override { m_pCallback = pCallback; }
	virtual IMovieCallback*                     GetCallback() override                          { return m_pCallback; }

	virtual void                                Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bRemoveOldNodes = false, bool bLoadEmpty = true) override;

	virtual const SCameraParams&                GetCameraParams() const override { return m_ActiveCameraParams; }
	virtual void                                SetCameraParams(const SCameraParams& Params) override;

	virtual void                                SendGlobalEvent(tukk pszEvent) override;
	virtual void                                SetSequenceStopBehavior(ESequenceStopBehavior behavior) override;
	virtual IMovieSystem::ESequenceStopBehavior GetSequenceStopBehavior() override;

	virtual SAnimTime                           GetPlayingTime(IAnimSequence* pSeq) override;
	virtual bool                                SetPlayingTime(IAnimSequence* pSeq, SAnimTime fTime) override;

	virtual float                               GetPlayingSpeed(IAnimSequence* pSeq) override;
	virtual bool                                SetPlayingSpeed(IAnimSequence* pSeq, float fTime) override;

	virtual bool                                GetStartEndTime(IAnimSequence* pSeq, SAnimTime& fStartTime, SAnimTime& fEndTime) override;
	virtual bool                                SetStartEndTime(IAnimSequence* pSeq, const SAnimTime fStartTime, const SAnimTime fEndTime) override;

	virtual void                                GoToFrame(tukk seqName, float targetFrame) override;

	virtual tukk                         GetOverrideCamName() const override       { return m_mov_overrideCam->GetString(); }

	virtual bool                                IsPhysicsEventsEnabled() const override   { return m_bPhysicsEventsEnabled; }
	virtual void                                EnablePhysicsEvents(bool enable) override { m_bPhysicsEventsEnabled = enable; }

	virtual void                                EnableBatchRenderMode(bool bOn) override  { m_bBatchRenderMode = bOn; }
	virtual bool                                IsInBatchRenderMode() const override      { return m_bBatchRenderMode; }

	void                                        SerializeNodeType(EAnimNodeType& animNodeType, XmlNodeRef& xmlNode, bool bLoading, const uint version, i32 flags);
	virtual void                                SerializeParamType(CAnimParamType& animParamType, XmlNodeRef& xmlNode, bool bLoading, const uint version) override;

	virtual tukk                         GetParamTypeName(const CAnimParamType& animParamType) override;

	void                                        OnCameraCut();

	ILightAnimWrapper*                          CreateLightAnimWrapper(tukk szName) const override;

private:
	void NotifyListeners(IAnimSequence* pSequence, IMovieListener::EMovieEvent event);

	void InternalStopAllSequences(bool bIsAbort, bool bAnimate);
	bool InternalStopSequence(IAnimSequence* seq, bool bIsAbort, bool bAnimate);

	bool FindSequence(IAnimSequence* sequence, PlayingSequences::const_iterator& sequenceIteratorOut) const;
	bool FindSequence(IAnimSequence* sequence, PlayingSequences::iterator& sequenceIteratorOut);

#if !defined(_RELEASE)
	static void GoToFrameCmd(IConsoleCmdArgs* pArgs);
	static void ListSequencesCmd(IConsoleCmdArgs* pArgs);
	static void PlaySequencesCmd(IConsoleCmdArgs* pArgs);
	void        ShowPlayedSequencesDebug();
#endif

	void DoNodeStaticInitialisation();
	void UpdateInternal(const float dt, const bool bPreUpdate);

#ifdef MOVIESYSTEM_SUPPORT_EDITING
	virtual EAnimNodeType           GetNodeTypeFromString(tukk pString) const override;
	virtual CAnimParamType          GetParamTypeFromString(tukk pString) const override;
	virtual DynArray<EAnimNodeType> GetAllNodeTypes() const override;
	virtual tukk             GetDefaultNodeName(EAnimNodeType type) const override;
#endif

	ISystem*        m_pSystem;

	IMovieUser*     m_pUser;
	IMovieCallback* m_pCallback;

	CTimeValue      m_lastUpdateTime;

	typedef std::vector<_smart_ptr<IAnimSequence>> Sequences;
	Sequences        m_sequences;

	PlayingSequences m_playingSequences;

	typedef std::vector<IMovieListener*>                TMovieListenerVec;
	typedef std::map<IAnimSequence*, TMovieListenerVec> TMovieListenerMap;

	// a container which maps sequences to all interested listeners
	// listeners is a vector (could be a set in case we have a lot of listeners, stl::push_back_unique!)
	TMovieListenerMap     m_movieListenerMap;

	bool                  m_bPaused;
	bool                  m_bEnableCameraShake;

	SCameraParams         m_ActiveCameraParams;

	ESequenceStopBehavior m_sequenceStopBehavior;

	bool                  m_bStartCapture;
	bool                  m_bEndCapture;
	bool                  m_bPreEndCapture;
	bool                  m_bIsInGameCutscene;

	IAnimSequence*        m_captureSeq;
	SCaptureKey           m_captureKey;

	float                 m_fixedTimeStepBackUp;
	ICVar*                m_cvar_capture_file_format;
	ICVar*                m_cvar_capture_frame_once;
	ICVar*                m_cvar_capture_folder;
	ICVar*                m_cvar_t_FixedStep;
	ICVar*                m_cvar_capture_frames;
	ICVar*                m_cvar_capture_file_prefix;
	ICVar*                m_cvar_capture_misc_render_buffers;

	static i32            m_mov_NoCutscenes;
	ICVar*                m_mov_overrideCam;

	bool                  m_bPhysicsEventsEnabled;

	bool                  m_bBatchRenderMode;

	// A sequence which turned on the early movie update last time
	u32 m_nextSequenceId;

public:
	static float m_mov_cameraPrecacheTime;
#if !defined(_RELEASE)
	static i32   m_mov_debugCamShake;
	static i32   m_mov_debugSequences;
#endif
};
