// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#if !defined(_RELEASE)
	#define MATERIAL_EFFECTS_DEBUG
#endif

#include <drx3D/FlowGraph/IFlowSystem.h> // <> required for Interfuscator
#include <drx3D/CoreX/ParticleSys/IParticles.h>
#include <drx3D/CoreX/Math/Drx_Color.h>

struct IEntityClass;
struct ISurfaceType;

//////////////////////////////////////////////////////////////////////////
enum EMFXPlayFlags
{
	eMFXPF_Disable_Delay = BIT(0),
	eMFXPF_Audio         = BIT(1),
	eMFXPF_Decal         = BIT(2),
	eMFXPF_Particles     = BIT(3),
	eMFXPF_Flowgraph     = BIT(4),
	eMFXPF_ForceFeedback = BIT(5),
	eMFXPF_All           = (eMFXPF_Audio | eMFXPF_Decal | eMFXPF_Particles | eMFXPF_Flowgraph | eMFXPF_ForceFeedback),
};

#define MFX_INVALID_ANGLE (gf_PI2 + 1)

//////////////////////////////////////////////////////////////////////////

typedef Functor1<tukk > EnumerateMaterialEffectsDataCallback;

//////////////////////////////////////////////////////////////////////////
struct SMFXAudioEffectRtpc
{
	SMFXAudioEffectRtpc()
	{
		rtpcName = "";
		rtpcValue = 0.0f;
	}
	tukk rtpcName;
	float       rtpcValue;
};

//////////////////////////////////////////////////////////////////////////
struct SMFXEmitterParameter
{
	SMFXEmitterParameter()
	{}

	template<typename T>
	SMFXEmitterParameter(tukk szName, T value)
		: name(szName)
		, value(value)
	{}

	bool operator==(const SMFXEmitterParameter& rhs) const
	{
		return name == rhs.name && value.index() == rhs.value.index();
	}

	DrxFixedStringT<32> name;
	IParticleAttributes::TValue value;
};
typedef std::vector<SMFXEmitterParameter> TMFXEmitterParameters;

//////////////////////////////////////////////////////////////////////////
struct SMFXRunTimeEffectParams
{
	static i32k MAX_AUDIO_RTPCS = 4;
	static i32k MAX_PARTICLE_PARAMS = 4;

	SMFXRunTimeEffectParams()
		: playSoundFP(false)
		, playflags(eMFXPF_All)
		, fLastTime(0.0f)
		, src(0)
		, trg(0)
		, srcSurfaceId(0)
		, trgSurfaceId(0)
		, srcRenderNode(0)
		, trgRenderNode(0)
		, partID(0)
		, pos(ZERO)
		, decalPos(ZERO)
		, normal(0.0f, 0.0f, 1.0f)
		, angle(MFX_INVALID_ANGLE)
		, scale(1.0f)
		, audioProxyEntityId(0)
		, audioProxyId(DrxAudio::DefaultAuxObjectId)
		, audioProxyOffset(ZERO)
		, numAudioRtpcs(0)
		, numParticleParams(0)
		, fDecalPlacementTestMaxSize(1000.f)
	{
		dir[0].Set(0.0f, 0.0f, -1.0f);
		dir[1].Set(0.0f, 0.0f, 1.0f);
	}

	bool AddAudioRtpc(tukk name, float val)
	{
		if (numAudioRtpcs < MAX_AUDIO_RTPCS)
		{
			audioRtpcs[numAudioRtpcs].rtpcName = name;
			audioRtpcs[numAudioRtpcs].rtpcValue = val;
			++numAudioRtpcs;
			return true;
		}
		return false;
	}

	void ResetAudioRtpcs()
	{
		numAudioRtpcs = 0;
	}

	bool AddParticleParams(const SMFXEmitterParameter& param)
	{
		if (numParticleParams < MAX_PARTICLE_PARAMS)
		{
			particleParams[numParticleParams] = param;
			++numParticleParams;
			return true;
		}
		return false;
	}

	void ResetParticleParams()
	{
		numParticleParams = 0;
	}

public:
	u16       playSoundFP; //!< Sets 1p/3p audio switch.
	u16       playflags;   //!< See EMFXPlayFlags.
	float        fLastTime;   //!< Last time this effect was played.
	float        fDecalPlacementTestMaxSize;

	EntityId     src;
	EntityId     trg;
	i32          srcSurfaceId;
	i32          trgSurfaceId;
	IRenderNode* srcRenderNode;
	IRenderNode* trgRenderNode;
	i32          partID;

	Vec3         pos;
	Vec3         decalPos;
	Vec3         dir[2];
	Vec3         normal;
	float        angle;
	float        scale;

	// Audio related.
	EntityId              audioProxyEntityId; //!< If set, uses this Entity's audio proxy to execute audio triggers. otherwise creates independent sound.
	DrxAudio::AuxObjectId audioProxyId;       //!< If set, uses the specified audio proxy of the entity, otherwise the default proxy id will be used.
	Vec3                  audioProxyOffset;   //!< In case of audio proxy, uses this offset.

	SMFXAudioEffectRtpc   audioRtpcs[MAX_AUDIO_RTPCS];
	u32                numAudioRtpcs;

	SMFXEmitterParameter  particleParams[MAX_PARTICLE_PARAMS];
	u32                numParticleParams;
};

struct SMFXBreakageParams
{
	enum EBreakageRequestFlags
	{
		eBRF_Matrix           = BIT(0),
		eBRF_HitPos           = BIT(1),
		eBRF_HitImpulse       = BIT(2),
		eBRF_Velocity         = BIT(3),
		eBRF_ExplosionImpulse = BIT(4),
		eBRF_Mass             = BIT(5),
		eBFR_Entity           = BIT(6),
	};

	SMFXBreakageParams() :
		m_flags(0),
		m_worldTM(IDENTITY),
		m_vHitPos(ZERO),
		m_vHitImpulse(IDENTITY),
		m_vVelocity(ZERO),
		m_fExplosionImpulse(1.0f),
		m_fMass(0.0f),
		m_nEntityId(0)
	{
	}

	void SetMatrix(const Matrix34& worldTM)
	{
		m_worldTM = worldTM;
		SetFlag(eBRF_Matrix);
	}

	const Matrix34& GetMatrix() const
	{
		return m_worldTM;
	}

	void SetHitPos(const Vec3& vHitPos)
	{
		m_vHitPos = vHitPos;
		SetFlag(eBRF_HitPos);
	}

	const Vec3& GetHitPos() const
	{
		return m_vHitPos;
	}

	void SetHitImpulse(const Vec3& vHitImpulse)
	{
		m_vHitImpulse = vHitImpulse;
		SetFlag(eBRF_HitImpulse);
	}

	const Vec3& GetHitImpulse() const
	{
		return m_vHitImpulse;
	}

	void SetVelocity(const Vec3& vVelocity)
	{
		m_vVelocity = vVelocity;
		SetFlag(eBRF_Velocity);
	}

	const Vec3& GetVelocity() const
	{
		return m_vVelocity;
	}

	void SetExplosionImpulse(float fExplosionImpulse)
	{
		m_fExplosionImpulse = fExplosionImpulse;
		SetFlag(eBRF_ExplosionImpulse);
	}

	float GetExplosionImpulse() const
	{
		return m_fExplosionImpulse;
	}

	void SetMass(float fMass)
	{
		m_fMass = fMass;
		SetFlag(eBRF_Mass);
	}

	float GetMass() const
	{
		return m_fMass;
	}

	void SetEntityId(EntityId id)
	{
		m_nEntityId = id;
		SetFlag(eBFR_Entity);
	}

	EntityId GetEntityId() const
	{
		return m_nEntityId;
	}

	bool CheckFlag(EBreakageRequestFlags flag) const
	{
		return (m_flags & flag) != 0;
	}

protected:
	void SetFlag(EBreakageRequestFlags flag)
	{
		m_flags |= flag;
	}

	void ClearFlag(EBreakageRequestFlags flag)
	{
		m_flags &= ~flag;
	}

	u32   m_flags;
	Matrix34 m_worldTM;
	Vec3     m_vHitPos;
	Vec3     m_vHitImpulse;
	Vec3     m_vVelocity;
	float    m_fExplosionImpulse;
	float    m_fMass;
	EntityId m_nEntityId;
};

class IMFXParticleParams
{
public:
	IMFXParticleParams()
		: name(NULL)
		, userdata(NULL)
		, scale(1.0f)
	{
	}

	tukk name;
	tukk userdata;
	float       scale;
};

class SMFXParticleListNode
{
public:
	static SMFXParticleListNode* Create();
	void                         Destroy();
	static void                  FreePool();

	IMFXParticleParams    m_particleParams;
	SMFXParticleListNode* pNext;

private:
	SMFXParticleListNode()
	{
		pNext = NULL;
	}
	~SMFXParticleListNode() {}
};

class IMFXAudioParams
{
	static const uint MAX_SWITCH_DATA_ELEMENTS = 4;

public:

	struct SSwitchData
	{
		SSwitchData()
			: switchName(NULL)
			, switchStateName(NULL)
		{
		}

		tukk switchName;
		tukk switchStateName;
	};

	IMFXAudioParams()
		: triggerName(NULL)
	{

	}
	tukk triggerName;

	DrxFixedArray<SSwitchData, MAX_SWITCH_DATA_ELEMENTS> triggerSwitches;
};

class SMFXAudioListNode
{
public:
	static SMFXAudioListNode* Create();
	void                      Destroy();
	static void               FreePool();

	IMFXAudioParams    m_audioParams;
	SMFXAudioListNode* pNext;

private:
	SMFXAudioListNode()
		: pNext(NULL)
	{
	}

	~SMFXAudioListNode()
	{
	}
};

class IMFXFlowGraphParams
{
public:
	IMFXFlowGraphParams()
	{
		name = NULL;
	}
	tukk name;
};

class SMFXFlowGraphListNode
{
public:
	static SMFXFlowGraphListNode* Create();
	void                          Destroy();
	static void                   FreePool();

	IMFXFlowGraphParams    m_flowGraphParams;
	SMFXFlowGraphListNode* pNext;

private:
	SMFXFlowGraphListNode()
	{
		pNext = NULL;
	}
	~SMFXFlowGraphListNode() {}
};

class IMFXDecalParams
{
public:
	IMFXDecalParams()
	{
		filename = 0;
		material = 0;
		minscale = 1.f;
		maxscale = 1.f;
		rotation = -1.f;
		lifetime = 10.0f;
		assemble = false;
		forceedge = false;
	}
	tukk filename;
	tukk material;
	float       minscale;
	float       maxscale;
	float       rotation;
	float       lifetime;
	bool        assemble;
	bool        forceedge;
};

class SMFXDecalListNode
{
public:
	static SMFXDecalListNode* Create();
	void                      Destroy();
	static void               FreePool();

	IMFXDecalParams    m_decalParams;
	SMFXDecalListNode* pNext;

private:
	SMFXDecalListNode()
	{
		pNext = 0;
	}
	~SMFXDecalListNode() {}
};

class IMFXForceFeedbackParams
{
public:
	IMFXForceFeedbackParams()
		: forceFeedbackEventName(NULL)
		, intensityFallOffMinDistanceSqr(0.0f)
		, intensityFallOffMaxDistanceSqr(0.0f)
	{

	}

	tukk forceFeedbackEventName;
	float       intensityFallOffMinDistanceSqr;
	float       intensityFallOffMaxDistanceSqr;
};

class SMFXForceFeedbackListNode
{
public:
	static SMFXForceFeedbackListNode* Create();
	void                              Destroy();
	static void                       FreePool();

	IMFXForceFeedbackParams    m_forceFeedbackParams;
	SMFXForceFeedbackListNode* pNext;

private:
	SMFXForceFeedbackListNode()
		: pNext(NULL)
	{

	}
	~SMFXForceFeedbackListNode() {}
};

struct SMFXResourceList;
typedef _smart_ptr<SMFXResourceList> SMFXResourceListPtr;

struct SMFXResourceList
{
public:
	SMFXParticleListNode*      m_particleList;
	SMFXAudioListNode*         m_audioList;
	SMFXDecalListNode*         m_decalList;
	SMFXFlowGraphListNode*     m_flowGraphList;
	SMFXForceFeedbackListNode* m_forceFeedbackList;

	void                       AddRef()  { ++m_refs; }
	void                       Release() { if (--m_refs <= 0) Destroy(); }

	static SMFXResourceListPtr Create();
	static void                FreePool();

private:
	i32          m_refs;

	virtual void Destroy();

	SMFXResourceList() : m_refs(0)
	{
		m_particleList = 0;
		m_audioList = 0;
		m_decalList = 0;
		m_flowGraphList = 0;
		m_forceFeedbackList = 0;
	}
	virtual ~SMFXResourceList()
	{
		while (m_particleList != 0)
		{
			SMFXParticleListNode* next = m_particleList->pNext;
			m_particleList->Destroy();
			m_particleList = next;
		}
		while (m_audioList != 0)
		{
			SMFXAudioListNode* next = m_audioList->pNext;
			m_audioList->Destroy();
			m_audioList = next;
		}
		while (m_decalList != 0)
		{
			SMFXDecalListNode* next = m_decalList->pNext;
			m_decalList->Destroy();
			m_decalList = next;
		}
		while (m_flowGraphList != 0)
		{
			SMFXFlowGraphListNode* next = m_flowGraphList->pNext;
			m_flowGraphList->Destroy();
			m_flowGraphList = next;
		}
		while (m_forceFeedbackList != 0)
		{
			SMFXForceFeedbackListNode* next = m_forceFeedbackList->pNext;
			m_forceFeedbackList->Destroy();
			m_forceFeedbackList = next;
		}
	}
};

typedef u16 TMFXEffectId;
static const TMFXEffectId InvalidEffectId = 0;

struct SMFXCustomParamValue
{
	float fValue;
};

struct SAnimFXEventInfo
{
	SAnimFXEventInfo()
		: szName(nullptr)
		, uniqueId(0u)
	{}

	SAnimFXEventInfo(tukk szName, u32 uniqueId)
		: szName(szName)
		, uniqueId(uniqueId)
	{}

	tukk szName;
	u32      uniqueId;
};

struct IAnimFXEvents
{
	// Library count
	virtual u32      GetLibrariesCount() const = 0;
	virtual tukk GetLibraryName(u32 index) const = 0;
	// Events
	virtual i32         GetAnimFXEventsCount() const = 0;
	virtual void        GetAnimFXEventInfo(i32 eventIndex, SAnimFXEventInfo& typeInfo) const = 0;
	virtual void        GetAnimFXEventInfoById(u32 uniqueId, SAnimFXEventInfo& typeInfo) const = 0;
	// Events parameters
	virtual i32         GetAnimFXEventParamCount(i32 eventIndex) const = 0;
	virtual void        GetAnimFXEventParam(i32 eventIndex, i32 paramIndex, SAnimFXEventInfo& extraParam) const = 0;
	virtual tukk GetAnimFXEventParamName(u32 eventTypeUid, u32 eventParamUid) const = 0;

protected:
	virtual ~IAnimFXEvents() {}
};

struct IMaterialEffectsListener
{
	virtual void OnPostLoadFXLibraries() = 0;

protected:
	virtual ~IMaterialEffectsListener() {}
};

//////////////////////////////////////////////////////////////////////////
//! The material effects system is responsible for triggering effects (such as particles) when the surface types of two materials interact (typically due to a physical collision).
struct IMaterialEffects
{
	// <interfuscator:shuffle>
	virtual ~IMaterialEffects(){}
	virtual void                LoadFXLibraries() = 0;
	virtual void                Reset(bool bCleanup) = 0;
	virtual void                ClearDelayedEffects() = 0;
	virtual TMFXEffectId        GetEffectIdByName(tukk libName, tukk effectName) = 0;
	virtual TMFXEffectId        GetEffectId(i32 surfaceIndex1, i32 surfaceIndex2) = 0;
	virtual TMFXEffectId        GetEffectId(tukk customName, i32 surfaceIndex2) = 0;
	virtual TMFXEffectId        GetEffectId(IEntityClass* pEntityClass, i32 surfaceIndex2) = 0;
	virtual SMFXResourceListPtr GetResources(TMFXEffectId effectId) const = 0;
	virtual void                PreLoadAssets() = 0;
	virtual bool                ExecuteEffect(TMFXEffectId effectId, SMFXRunTimeEffectParams& runtimeParams) = 0;
	virtual void                StopEffect(TMFXEffectId effectId) = 0;
	virtual i32                 GetDefaultSurfaceIndex() = 0;
	virtual i32                 GetDefaultCanopyIndex() = 0;

	virtual bool                PlayBreakageEffect(ISurfaceType* pSurfaceType, tukk breakageType, const SMFXBreakageParams& mfxBreakageParams) = 0;

	virtual void                SetCustomParameter(TMFXEffectId effectId, tukk customParameter, const SMFXCustomParamValue& customParameterValue) = 0;

	virtual void                CompleteInit() = 0;

	virtual void                ReloadMatFXFlowGraphs() = 0;
	virtual size_t              GetMatFXFlowGraphCount() const = 0;
	virtual IFlowGraphPtr       GetMatFXFlowGraph(i32 index, string* pFileName = NULL) const = 0;
	virtual IFlowGraphPtr       LoadNewMatFXFlowGraph(const string& filename) = 0;

	virtual void                EnumerateEffectNames(EnumerateMaterialEffectsDataCallback& callback, tukk szLibraryName) const = 0;
	virtual void                EnumerateLibraryNames(EnumerateMaterialEffectsDataCallback& callback) const = 0;
	virtual void                AddListener(IMaterialEffectsListener* pListener, tukk name) = 0;
	virtual void                RemoveListener(IMaterialEffectsListener* pListener) = 0;
	virtual void                LoadFXLibraryFromXMLInMemory(tukk szName, XmlNodeRef root) = 0;
	virtual void                UnloadFXLibrariesWithPrefix(tukk szName) = 0;
	virtual void                SetAnimFXEvents(IAnimFXEvents* pAnimEvents) = 0;
	virtual IAnimFXEvents*      GetAnimFXEvents() = 0;
	// </interfuscator:shuffle>
};
