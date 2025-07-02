// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __animnode_h__
#define __animnode_h__

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/Movie/Movie.h>

struct SAudioInfo
{
	SAudioInfo()
		: audioKeyStart(-1)
		, audioKeyStop(-1)
	{}

	void Reset()
	{
		audioKeyStart = -1;
		audioKeyStop = -1;
	}

	i32 audioKeyStart;
	i32 audioKeyStop;
};

/*!
    Base class for all Animation nodes,
    can host multiple animation tracks, and execute them other time.
    Animation node is reference counted.
 */
class CAnimNode : public IAnimNode
{
public:
	struct SParamInfo
	{
		SParamInfo() : name(""), valueType(eAnimValue_Float), flags(ESupportedParamFlags(0)) {};
		SParamInfo(tukk name, CAnimParamType paramType, EAnimValue valueType, ESupportedParamFlags flags)
			: name(name), paramType(paramType), valueType(valueType), flags(flags) {};

		tukk          name;      // parameter name.
		CAnimParamType       paramType; // parameter id.
		EAnimValue           valueType; // value type, defines type of track to use for animating this parameter.
		ESupportedParamFlags flags;     // combination of flags from ESupportedParamFlags.
	};

public:
	CAnimNode(i32k id);

	virtual void                            SetName(tukk name) override    { m_name = name; };
	virtual tukk                     GetName() override                    { return m_name; };

	void                                    SetSequence(IAnimSequence* pSequence) { m_pSequence = pSequence; }
	// Return Animation Sequence that owns this node.
	virtual IAnimSequence*                  GetSequence() override                { return m_pSequence; };

	virtual void                            SetFlags(i32 flags) override;
	virtual i32                             GetFlags() const override;

	virtual void                            OnStart()  {}
	virtual void                            OnReset()  {}
	virtual void                            OnPause()  {}
	virtual void                            OnResume() {}
	virtual void                            OnStop()   {}

	virtual bool                            IsParamValid(const CAnimParamType& paramType) const override;
	virtual tukk                     GetParamName(const CAnimParamType& param) const override;
	virtual EAnimValue                      GetParamValueType(const CAnimParamType& paramType) const override;
	virtual IAnimNode::ESupportedParamFlags GetParamFlags(const CAnimParamType& paramType) const override;
	virtual u32                    GetParamCount() const override      { return 0; };

	virtual void                            SetTarget(IAnimNode* node)          {};
	virtual IAnimNode*                      GetTarget() const                   { return 0; };

	virtual void                            StillUpdate() override              {}

	virtual void                            PrecacheStatic(SAnimTime startTime) {} // Called when the Sequence is explicitly precached
	virtual void                            PrecacheDynamic(SAnimTime time)     {} // Called while playing

	virtual void                            Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks) override;

	virtual void                            SetNodeOwner(IAnimNodeOwner* pOwner) override;
	virtual IAnimNodeOwner*                 GetNodeOwner() override { return m_pOwner; };

	// Called by sequence when needs to activate a node.
	virtual void             Activate(bool bActivate) override;

	virtual void             SetParent(IAnimNode* pParent) override { m_pParentNode = pParent; };
	virtual IAnimNode*       GetParent() const override             { return m_pParentNode; };
	virtual IAnimNode*       HasDirectorAsParent() const override;

	virtual i32              GetTrackCount() const override;
	virtual IAnimTrack*      GetTrackByIndex(i32 nIndex) const override;
	virtual IAnimTrack*      GetTrackForParameter(const CAnimParamType& paramType) const override;
	virtual IAnimTrack*      GetTrackForParameter(const CAnimParamType& paramType, u32 index) const override;

	virtual u32           GetTrackParamIndex(const IAnimTrack* pTrack) const override;

	virtual void             SetTrack(const CAnimParamType& paramType, IAnimTrack* track) override;
	virtual IAnimTrack*      CreateTrack(const CAnimParamType& paramType) override;
	virtual void             InitializeTrackDefaultValue(IAnimTrack* pTrack, const CAnimParamType& paramType) override {}
	virtual void             SetTimeRange(TRange<SAnimTime> timeRange) override;
	virtual void             AddTrack(IAnimTrack* pTrack) override;
	virtual bool             RemoveTrack(IAnimTrack* pTrack) override;
	virtual void             SerializeAnims(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks);
	virtual void             CreateDefaultTracks() override {};

	virtual void             PostLoad();

	i32                      GetId() const                       { return m_id; }
	tukk              GetNameFast() const                 { return m_name.c_str(); }

	virtual void             Render() override                   {}

	virtual void             UpdateDynamicParams() override      {}

	virtual IAnimEntityNode* QueryEntityNodeInterface() override { return NULL; }
	virtual IAnimCameraNode* QueryCameraNodeInterface() override { return NULL; }

protected:
	virtual bool    GetParamInfoFromType(const CAnimParamType& paramType, SParamInfo& info) const { return false; };

	i32             NumTracks() const                                                             { return (i32)m_tracks.size(); }
	IAnimTrack*     CreateTrackInternal(const CAnimParamType& paramType, EAnimValue valueType);

	IAnimTrack*     CreateTrackInternalVector4(const CAnimParamType& paramType) const;
	IAnimTrack*     CreateTrackInternalQuat(const CAnimParamType& paramType) const;
	IAnimTrack*     CreateTrackInternalVector(const CAnimParamType& paramType, const EAnimValue animValue) const;
	IAnimTrack*     CreateTrackInternalFloat(const CAnimParamType& paramType) const;

	virtual bool    NeedToRender() const override { return false; }

	virtual DrxGUID GetGUID() const override      { return m_guid; }

protected:
	DrxGUID                             m_guid;
	i32                                 m_id;
	string                              m_name;
	IAnimSequence*                      m_pSequence;
	IAnimNodeOwner*                     m_pOwner;
	IAnimNode*                          m_pParentNode;
	i32                                 m_nLoadedParentNodeId;
	i32                                 m_flags;
	u32                        m_bIgnoreSetParam : 1; // Internal flags.

	std::vector<_smart_ptr<IAnimTrack>> m_tracks;

private:
	static bool TrackOrder(const _smart_ptr<IAnimTrack>& left, const _smart_ptr<IAnimTrack>& right);
};

//////////////////////////////////////////////////////////////////////////
class CAnimNodeGroup : public CAnimNode
{
public:
	CAnimNodeGroup(i32k id) : CAnimNode(id) { SetFlags(GetFlags() | eAnimNodeFlags_CanChangeName); }
	EAnimNodeType          GetType() const                         { return eAnimNodeType_Group; }

	virtual CAnimParamType GetParamType(u32 nIndex) const { return eAnimParamType_Invalid; }
};

#endif // __animnode_h__
