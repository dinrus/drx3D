// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//
// The audition map keeps track of which 'listeners' need to be notified in the world
// when certain sound stimuli managed to reach them.

#pragma once

#include <drx3D/AI/AuditionMapDefinitions.h>

#include <drx3D/AI/AuditionMapRayCastUpr.h>

#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/CoreX/Containers/DrxArray.h>

namespace Perception
{

#if AUDITION_MAP_STORE_DEBUG_NAMES_FOR_LISTENER_INSTANCE_ID
// More information for a registered listener instance.
struct SListenerInstanceInfo
{
	string debugName;
};
#endif

// All sorts of information about a listener in the world.
struct SListener
{
	SListener()
		: boundingSphereAroundEars(Vec3(ZERO), 0.0f)
	{
	}

	explicit SListener(const SListenerParams& source)
		: params(source)
		, boundingSphereAroundEars(Vec3(ZERO), 0.0f)
	{
	}

	SListenerParams params;
	Sphere         boundingSphereAroundEars;
	string         name;
};

// All sorts of information about a global listener in the world.
struct SGlobalListener
{
	SGlobalListener() {}
	explicit SGlobalListener(const SGlobalListenerParams& params)
		: params(params)
	{}

	SGlobalListenerParams params;
};

// This is the audio equivalent of the VisionMap. It basically tracks 'listeners' in the world that
// get notified when a sound stimulus was within their hearing range.
class CAuditionMap : public IAuditionMap
{
public:
	CAuditionMap();
	virtual ~CAuditionMap() override {}

	// TODO: Not sure why we use entity IDs here, this should be a generic ID value so that the audition
	// map can be used by anything (identical to how the Vision Map does things), see SListenerInstanceId.
	virtual bool RegisterListener(EntityId entityId, const SListenerParams& listenerParams) override;
	virtual void UnregisterListner(EntityId entityId) override;
	virtual void ListenerChanged(EntityId entityId, const SListenerParams& newListenerParams, u32k changesMask) override;

	virtual SListenerInstanceId RegisterGlobalListener(const SGlobalListenerParams& globalListenerParams, tukk name) override;
	virtual void UnregisterGlobalListener(const SListenerInstanceId& listenerInstanceId) override;

	virtual void OnSoundEvent(const SSoundStimulusParams& soundParams, tukk szDebugText) override;
	virtual void SetSoundObstructionOnHitCallback(const IAuditionMap::SoundObstructionOnHitCallback& callback) override;

	void Update(float deltaTime);

protected:

	friend class AuditionHelpers::CAuditionMapRayCastUpr;

	// Stimulus delivery
	void DeliverStimulusToListener(const SSoundStimulusParams& soundParams, const SListener& listener, const size_t receivingEarIndex);
	void DeliverStimulusToListener(const SSoundStimulusParams& soundParams, const EntityId listenerEntityId, const size_t receivingEarIndex);

	// Global listener events
	void NotifyGlobalListeners_StimulusProcessedAndReachedOneOrMoreEars(const SSoundStimulusParams& soundParams) const;

	IAuditionMap::SoundObstructionOnHitCallback& GetSoundObstructionOnHitCallback() { return m_soundObstructionOnHitCallback; }

private:

	// Stimulus processing
	inline float ComputeStimulusRadius(const SListener& listener, const SSoundStimulusParams& soundParams) const;
	bool         ShouldListenerAcknowledgeSound(const SListener& listener, const SSoundStimulusParams& soundParams) const;
	bool         IsAnyEarWithinStimulusRange(const SListener& listener, const SSoundStimulusParams& soundParams, size_t* pAnyReceivingEarIndex = nullptr) const;
	void         OnSoundEvent_IgnoreAllObstructionsHandling(const SSoundStimulusParams& soundParams);
	void         OnSoundEvent_RayCastWithLinearFallOff(const SSoundStimulusParams& soundParams);

	// Listener management
	static void ReconstructBoundingSphereAroundEars(SListener& listener);

	// Global Listener management
	SListenerInstanceId CreateListenerInstanceId(tukk szName);
	void                ReleaseListenerInstanceId(const SListenerInstanceId& listenerInstanceId);

private:

	// TODO: Hash all these listeners into the new Octree template that SebastienL has recently written?
	typedef VectorMap<EntityId, SListener> Listeners;
	Listeners m_listeners;

	AuditionHelpers::CAuditionMapRayCastUpr m_rayCastUpr;
	SoundObstructionOnHitCallback m_soundObstructionOnHitCallback;

	u32 m_listenerInstanceIdGenerator;

	typedef std::unordered_map<SListenerInstanceId, SGlobalListener, stl::hash_uint32> GlobalListeners;
	GlobalListeners m_globalListeners;

#if AUDITION_MAP_STORE_DEBUG_NAMES_FOR_LISTENER_INSTANCE_ID
	typedef std::unordered_map<SListenerInstanceId, SListenerInstanceInfo, stl::hash_uint32> RegisteredListenerInstanceInfos;
	RegisteredListenerInstanceInfos m_registeredListenerInstanceInfos;
#endif
};

} //!namespace AuditionMap
