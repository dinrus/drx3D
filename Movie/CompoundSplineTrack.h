// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#define MAX_SUBTRACKS 4

class CCompoundSplineTrack : public IAnimTrack
{
public:
	CCompoundSplineTrack(const CAnimParamType& paramType, i32 nDims, EAnimValue inValueType, CAnimParamType subTrackParamTypes[MAX_SUBTRACKS]);

	virtual i32                         GetSubTrackCount() const override { return m_nDimensions; }
	virtual IAnimTrack*                 GetSubTrack(i32 nIndex) const override;
	virtual tukk                 GetSubTrackName(i32 nIndex) const override;
	virtual void                        SetSubTrackName(i32 nIndex, tukk name) override;

	virtual EAnimValue                  GetValueType() override           { return m_valueType; }

	virtual CAnimParamType              GetParameterType() const override { return m_paramType; }

	virtual i32                         GetNumKeys() const override;
	virtual bool                        HasKeys() const override;
	virtual void                        RemoveKey(i32 num) override;
	virtual void                        ClearKeys() override;

	virtual i32                         CreateKey(SAnimTime time) override               { assert(0); return 0; };
	virtual tukk                 GetKeyType() const override                      { return STrackKey::GetType(); }
	virtual bool                        KeysDeriveTrackDurationKey() const override      { return false; }
	virtual _smart_ptr<IAnimKeyWrapper> GetWrappedKey(i32 key) override;
	virtual void                        GetKey(i32 index, STrackKey* key) const override { assert(0); };
	virtual SAnimTime                   GetKeyTime(i32 index) const override;
	virtual i32                         FindKey(SAnimTime time) override                 { assert(0); return 0; };
	virtual void                        SetKey(i32 index, const STrackKey* key) override { assert(0); };

	virtual i32                         GetFlags() override                              { return m_flags; };
	virtual bool                        IsMasked(u32k mask) const override       { return false; }
	virtual void                        SetFlags(i32 flags) override                     { m_flags = flags; };

	// Get track value at specified time. Interpolates keys if needed.
	virtual TMovieSystemValue GetValue(SAnimTime time) const override;

	// Get track default value
	virtual TMovieSystemValue GetDefaultValue() const override;

	// Set track default value
	virtual void SetDefaultValue(const TMovieSystemValue& value) override;

	virtual void SetTimeRange(TRange<SAnimTime> timeRange) override;

	virtual void Serialize(Serialization::IArchive& ar) override {}

	virtual bool Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks = true) override;

    virtual bool SerializeKeys(XmlNodeRef& xmlNode, bool bLoading, std::vector<SAnimTime>& keys, const SAnimTime time = SAnimTime(0)) override { return false; };

	virtual i32  NextKeyByTime(i32 key) const override;

	void         SetSubTrackName(i32k i, const string& name)          { assert(i < MAX_SUBTRACKS); m_subTrackNames[i] = name; }

	virtual void GetKeyValueRange(float& fMin, float& fMax) const override { if (GetSubTrackCount() > 0) { m_subTracks[0]->GetKeyValueRange(fMin, fMax); } }
	virtual void SetKeyValueRange(float fMin, float fMax) override
	{
		for (i32 i = 0; i < m_nDimensions; ++i)
		{
			m_subTracks[i]->SetKeyValueRange(fMin, fMax);
		}
	};

	virtual DrxGUID GetGUID() const override { return m_guid; }

protected:
	DrxGUID                m_guid;
	EAnimValue             m_valueType;
	i32                    m_nDimensions;
	_smart_ptr<IAnimTrack> m_subTracks[MAX_SUBTRACKS];
	i32                    m_flags;
	CAnimParamType         m_paramType;
	string                 m_subTrackNames[MAX_SUBTRACKS];

	void  PrepareNodeForSubTrackSerialization(XmlNodeRef& subTrackNode, XmlNodeRef& xmlNode, i32 i, bool bLoading);
	float PreferShortestRotPath(float degree, float degree0) const;
	i32   GetSubTrackIndex(i32& key) const;
};