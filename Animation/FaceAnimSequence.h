// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/FaceEffector.h>
#include <drx3D/Animation/LipSync.h>

class CFacialInstance;
class CFacialAnimationContext;
class CFacialAnimSequence;
class CFacialAnimation;

//////////////////////////////////////////////////////////////////////////
class CFacialAnimChannelInterpolator : public spline::CSplineKeyInterpolator<spline::HermiteKey<float>>
{
public:
	void CleanupKeys(float errorMax);
	void SmoothKeys(float sigma);
	void RemoveNoise(float sigma, float threshold);
};

class CFacialCameraPathPositionInterpolator : public spline::CSplineKeyInterpolator<spline::TCBSplineKey<Vec3>>
{
};

class CFacialCameraPathOrientationInterpolator : public spline::CSplineKeyInterpolator<spline::HermiteKey<Quat>>
{
};

//////////////////////////////////////////////////////////////////////////
class CFacialAnimChannel : public IFacialAnimChannel, public _i_reference_target_t
{
public:
	CFacialAnimChannel(i32 index);
	~CFacialAnimChannel();

	//////////////////////////////////////////////////////////////////////////
	// IFacialAnimChannel
	//////////////////////////////////////////////////////////////////////////
	virtual void                        SetIdentifier(CFaceIdentifierHandle ident);
	virtual const CFaceIdentifierHandle GetIdentifier();

#ifdef FACE_STORE_ASSET_VALUES
	virtual void        SetName(tukk name);
	virtual tukk GetName() const   { return m_name.GetString(); }
	virtual tukk GetEffectorName() { return m_effectorName.GetString(); }
#endif

	virtual void                        SetEffectorIdentifier(CFaceIdentifierHandle ident) { m_effectorName = ident; }
	virtual const CFaceIdentifierHandle GetEffectorIdentifier()                            { return m_effectorName; }

	// Associate animation channel with the group.
	virtual void                SetParent(IFacialAnimChannel* pParent) { m_pParent = (CFacialAnimChannel*)pParent; };
	// Get group of this animation channel.
	virtual IFacialAnimChannel* GetParent()                            { return m_pParent; };

	virtual void                SetEffector(IFacialEffector* pEffector);
	virtual IFacialEffector*    GetEffector()           { return m_pEffector; };

	virtual void                SetFlags(u32 nFlags) { m_nFlags = nFlags; };
	virtual u32              GetFlags()              { return m_nFlags; };

	// Retrieve interpolator spline used to animated channel value.
	virtual ISplineInterpolator* GetInterpolator(i32 i) { return m_splines[i]; }
	virtual ISplineInterpolator* GetLastInterpolator()  { return (!m_splines.empty() ? m_splines.back() : 0); }
	virtual void                 AddInterpolator();
	virtual void                 DeleteInterpolator(i32 i);
	virtual i32                  GetInterpolatorCount() const { return m_splines.size(); }

	virtual void                 CleanupKeys(float fErrorMax);
	virtual void                 SmoothKeys(float sigma);
	virtual void                 RemoveNoise(float sigma, float threshold);
	//////////////////////////////////////////////////////////////////////////

	u32                      GetInstanceChannelId() const           { return m_nInstanceChannelId; }
	void                        SetInstanceChannelId(u32 nChanelId) { m_nInstanceChannelId = nChanelId; }

	float                       Evaluate(float t);
	bool                        HaveEffector() const { return m_pEffector != 0; }

	void                        CreateInterpolator();
	_smart_ptr<CFacialEffector> GetEffectorPtr() { return m_pEffector; }

	void                        GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_nFlags);
		pSizer->AddObject(m_nInstanceChannelId);
		pSizer->AddObject(m_name);
		pSizer->AddObject(m_effectorName);
		pSizer->AddObject(m_pParent);
		pSizer->AddObject(m_pEffector);
		pSizer->AddObject(m_splines);
	}
private:
	u32                                       m_nFlags;
	u32                                       m_nInstanceChannelId;
	CFaceIdentifierStorage                       m_name;
	CFaceIdentifierHandle                        m_effectorName;
	_smart_ptr<CFacialAnimChannel>               m_pParent;
	_smart_ptr<CFacialEffector>                  m_pEffector;
	std::vector<CFacialAnimChannelInterpolator*> m_splines;
};

//////////////////////////////////////////////////////////////////////////
class CFacialAnimSequenceInstance : public _i_reference_target_t
{
public:
	struct ChannelInfo
	{
		i32                         nChannelId;
		_smart_ptr<CFacialEffector> pEffector;
		bool                        bUse;
		i32                         nBalanceChannelListIndex; // Index into m_balanceChannelIndices

		void                        GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(nChannelId);
			pSizer->AddObject(pEffector);
			pSizer->AddObject(bUse);
			pSizer->AddObject(nBalanceChannelListIndex);
		}
	};
	typedef std::vector<ChannelInfo> Channels;

	Channels m_channels;
	Channels m_proceduralChannels;

	struct BalanceChannelEntry
	{
		i32   nChannelIndex;
		float fEvaluatedBalance; // Place to store temporary evaluation of spline - could be on stack, but would require dynamic allocation per frame/instance.
		i32   nMorphIndexStartIndex;
		i32   nMorphIndexCount;

		void  GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(this, sizeof(*this));
		}
	};
	typedef std::vector<BalanceChannelEntry> BalanceEntries;
	BalanceEntries           m_balanceChannelEntries;

	std::vector<i32>         m_balanceChannelStateIndices;

	i32                      m_nValidateID;
	CFacialAnimationContext* m_pAnimContext;

	// Phonemes related.
	i32 m_nCurrentPhoneme;
	i32 m_nCurrentPhonemeChannelId;

	//////////////////////////////////////////////////////////////////////////
	CFacialAnimSequenceInstance()
	{
		m_nValidateID = 0;
		m_pAnimContext = 0;
		m_nCurrentPhoneme = -1;
		m_nCurrentPhonemeChannelId = -1;
	}
	~CFacialAnimSequenceInstance()
	{
		if (m_pAnimContext)
			UnbindChannels();
	}

	//////////////////////////////////////////////////////////////////////////
	void BindChannels(CFacialAnimationContext* pContext, CFacialAnimSequence* pSequence);
	void UnbindChannels();

	void GetMemoryUsage(IDrxSizer* pSizer) const;

private:
	void BindProceduralChannels(CFacialAnimationContext* pContext, CFacialAnimSequence* pSequence);
	void UnbindProceduralChannels();
};

class CProceduralChannel
{
public:
	CFacialAnimChannelInterpolator* GetInterpolator()                                  { return &m_interpolator; }

	void                            SetEffectorIdentifier(CFaceIdentifierHandle ident) { m_effectorName = ident; }
	CFaceIdentifierHandle           GetEffectorIdentifier() const                      { return m_effectorName; }

	void                            GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_interpolator);
		pSizer->AddObject(m_effectorName);
	}

	void swap(CProceduralChannel& b)
	{
		using std::swap;

		m_interpolator.swap(b.m_interpolator);
		swap(m_effectorName, b.m_effectorName);
	}

private:
	CFacialAnimChannelInterpolator m_interpolator;
	CFaceIdentifierHandle          m_effectorName;
};

class CProceduralChannelSet
{
public:
	enum ChannelType
	{
		ChannelTypeHeadUpDown,
		ChannelTypeHeadRightLeft,

		ChannelType_count
	};

	CProceduralChannelSet();
	CProceduralChannel* GetChannel(ChannelType type) { return &m_channels[type]; }

	void                swap(CProceduralChannelSet& b)
	{
		for (i32 i = 0; i < ChannelType_count; ++i)
			m_channels[i].swap(b.m_channels[i]);
	}

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_channels);
	}
private:
	CProceduralChannel m_channels[ChannelType_count];
};

//////////////////////////////////////////////////////////////////////////
class CFacialAnimSoundEntry : public IFacialAnimSoundEntry
{
public:
	CFacialAnimSoundEntry();

	virtual void             SetSoundFile(tukk sSoundFile);
	virtual tukk      GetSoundFile();

	virtual IFacialSentence* GetSentence() { return m_pSentence; };

	virtual float            GetStartTime();
	virtual void             SetStartTime(float time);

	void                     ValidateSentence();
	bool                     IsSentenceInvalid();

	_smart_ptr<CFacialSentence> m_pSentence;
	string                      m_sound;
	i32                         m_nSentenceValidateID;
	float                       m_startTime;

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_pSentence);
		pSizer->AddObject(m_sound);
		pSizer->AddObject(m_nSentenceValidateID);
		pSizer->AddObject(m_startTime);
	};
};

//////////////////////////////////////////////////////////////////////////
class CFacialAnimSkeletalAnimationEntry : public IFacialAnimSkeletonAnimationEntry
{
public:
	CFacialAnimSkeletalAnimationEntry();

	virtual void        SetName(tukk skeletonAnimationFile);
	virtual tukk GetName() const;

	virtual void        SetStartTime(float time);
	virtual float       GetStartTime() const;
	virtual void        SetEndTime(float time);
	virtual float       GetEndTime() const;

	string m_animationName;
	float  m_startTime;
	float  m_endTime;

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_animationName);
		pSizer->AddObject(m_startTime);
		pSizer->AddObject(m_endTime);
	};
};

//////////////////////////////////////////////////////////////////////////
class CFacialAnimSequence : public IFacialAnimSequence, public IStreamCallback
{
public:
	CFacialAnimSequence(CFacialAnimation* pFaceAnim);
	~CFacialAnimSequence();

	virtual void AddRef() { ++m_nRefCount; };
	virtual void Release();

	//////////////////////////////////////////////////////////////////////////
	// IFacialAnimSequence
	//////////////////////////////////////////////////////////////////////////
	virtual bool                               StartStreaming(tukk sFilename);

	virtual void                               SetName(tukk sNewName);
	virtual tukk                        GetName()                 { return m_name; }

	virtual void                               SetFlags(i32 nFlags)      { m_data.m_nFlags = nFlags; };
	virtual i32                                GetFlags()                { return m_data.m_nFlags; };

	virtual Range                              GetTimeRange()            { return m_data.m_timeRange; };
	virtual void                               SetTimeRange(Range range) { m_data.m_timeRange = range; };

	virtual i32                                GetChannelCount()         { return m_data.m_channels.size(); };
	virtual IFacialAnimChannel*                GetChannel(i32 nIndex);
	virtual IFacialAnimChannel*                CreateChannel();
	virtual IFacialAnimChannel*                CreateChannelGroup();
	virtual void                               RemoveChannel(IFacialAnimChannel* pChannel);

	virtual i32                                GetSoundEntryCount();
	virtual void                               InsertSoundEntry(i32 index);
	virtual void                               DeleteSoundEntry(i32 index);
	virtual IFacialAnimSoundEntry*             GetSoundEntry(i32 index);

	virtual void                               Animate(const QuatTS& rAnimLocationNext, CFacialAnimSequenceInstance* pInstance, float fTime);

	virtual i32                                GetSkeletonAnimationEntryCount();
	virtual void                               InsertSkeletonAnimationEntry(i32 index);
	virtual void                               DeleteSkeletonAnimationEntry(i32 index);
	virtual IFacialAnimSkeletonAnimationEntry* GetSkeletonAnimationEntry(i32 index);

	virtual void                               SetJoystickFile(tukk joystickFile);
	virtual tukk                        GetJoystickFile() const;

	virtual void                               Serialize(XmlNodeRef& xmlNode, bool bLoading, ESerializationFlags flags);

	virtual void                               MergeSequence(IFacialAnimSequence* pMergeSequence, const Functor1wRet<tukk , MergeCollisionAction>& collisionStrategy);

	virtual ISplineInterpolator*               GetCameraPathPosition()    { return &m_data.m_cameraPathPosition; }
	virtual ISplineInterpolator*               GetCameraPathOrientation() { return &m_data.m_cameraPathOrientation; }
	virtual ISplineInterpolator*               GetCameraPathFOV()         { return &m_data.m_cameraPathFOV; }

	//////////////////////////////////////////////////////////////////////////
	// IStreamCallback
	//////////////////////////////////////////////////////////////////////////
	virtual void StreamAsyncOnComplete(IReadStream* pStream, unsigned nError);
	virtual void StreamOnComplete(IReadStream* pStream, unsigned nError);

	//////////////////////////////////////////////////////////////////////////

	i32                    GetValidateId() const       { return m_data.m_nValidateID; };

	CProceduralChannelSet& GetProceduralChannelSet()   { return m_data.m_proceduralChannels; }

	bool                   IsInMemory() const          { return m_bInMemory; };
	void                   SetInMemory(bool bInMemory) { m_bInMemory = bInMemory; };

	void                   GetMemoryUsage(IDrxSizer* pSizer) const;

private:
	CFacialAnimSequence(const CFacialAnimSequence&);
	CFacialAnimSequence& operator=(const CFacialAnimSequence&);

private:
	friend class CFacialAnimation;

	typedef std::vector<_smart_ptr<CFacialAnimChannel>> Channels;

	struct Data
	{
		Data()
		{
			m_timeRange.Set(0, 1);
			m_nFlags = 0;
			m_nValidateID = 0;
			m_nProceduralChannelsValidateID = 0;
			m_nSoundEntriesValidateID = 0;
		}

		// Validate id insure that sequence instances are valid.
		i32    m_nValidateID;

		i32    m_nFlags;

		string m_joystickFile;
		std::vector<CFacialAnimSkeletalAnimationEntry> m_skeletonAnimationEntries;

		Channels                                 m_channels;
		Range                                    m_timeRange;
		std::vector<CFacialAnimSoundEntry>       m_soundEntries;

		i32                                      m_nProceduralChannelsValidateID;
		i32                                      m_nSoundEntriesValidateID;

		CProceduralChannelSet                    m_proceduralChannels;

		CFacialCameraPathPositionInterpolator    m_cameraPathPosition;
		CFacialCameraPathOrientationInterpolator m_cameraPathOrientation;
		CFacialAnimChannelInterpolator           m_cameraPathFOV;

		friend void swap(Data& a, Data& b)
		{
			using std::swap;

			swap(a.m_nValidateID, b.m_nValidateID);
			swap(a.m_nFlags, b.m_nFlags);
			swap(a.m_timeRange, b.m_timeRange);
			a.m_joystickFile.swap(b.m_joystickFile);
			a.m_skeletonAnimationEntries.swap(b.m_skeletonAnimationEntries);
			a.m_channels.swap(b.m_channels);
			a.m_soundEntries.swap(b.m_soundEntries);
			a.m_cameraPathPosition.swap(b.m_cameraPathPosition);
			a.m_cameraPathOrientation.swap(b.m_cameraPathOrientation);
			a.m_cameraPathFOV.swap(b.m_cameraPathFOV);
			swap(a.m_nProceduralChannelsValidateID, b.m_nProceduralChannelsValidateID);
			swap(a.m_nSoundEntriesValidateID, b.m_nSoundEntriesValidateID);
			a.m_proceduralChannels.swap(b.m_proceduralChannels);
		}

		IFacialAnimChannel* CreateChannel()
		{
			i32 index = i32(m_channels.size());
			CFacialAnimChannel* pChannel = new CFacialAnimChannel(index);
			pChannel->CreateInterpolator();
			m_channels.push_back(pChannel);
			m_nValidateID++;
			return pChannel;
		}

		IFacialAnimChannel* CreateChannelGroup()
		{
			i32 index = i32(m_channels.size());
			CFacialAnimChannel* pChannel = new CFacialAnimChannel(index);
			pChannel->SetFlags(pChannel->GetFlags() | IFacialAnimChannel::FLAG_GROUP);
			m_channels.push_back(pChannel);
			m_nValidateID++;
			return pChannel;
		}

	};

private:
	void SerializeLoad(Data& data, XmlNodeRef& xmlNode, ESerializationFlags flags);
	void SerializeChannelSave(IFacialAnimChannel* pChannel, XmlNodeRef& node);
	void SerializeChannelLoad(Data& data, IFacialAnimChannel* pChannel, XmlNodeRef& node);

	void UpdateProceduralChannels();
	void GenerateProceduralChannels(Data& data);
	void UpdateSoundEntriesValidateID();

	i32               m_nRefCount;

	string            m_name;
	CFacialAnimation* m_pFaceAnim;

	bool              m_bInMemory;

	Data              m_data;

	IReadStreamPtr    m_pStream;
	Data*             m_pStreamingData;
};
