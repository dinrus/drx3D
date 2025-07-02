// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
$Id$
$DateTime$
Описание: Climb-able Ledge system 

-------------------------------------------------------------------------
История:
- 20:04:2009: Created by Michelle Martin
- 27:02:2012: Rewritten by Benito G.R.

*************************************************************************/

#ifndef __LEDGEMANAGER_H__
#define __LEDGEMANAGER_H__


#define LEDGE_MANAGER_EDITING_ENABLED	1
#define MAX_LEDGE_ENTITIES 1024

//////////////////////////////////////////////////////////////////////////
/// Ledge definition structures, types and flags

// these flags are exported with exported levels. Do NOT introduce new flags in the middle of these
// or you'll require ALL levels to be re-exported to ensure the correct flags are present
enum
{
	kLedgeFlag_none								= 0,
	kLedgeFlag_static						  = BIT(0),
	kLedgeFlag_enabled						= BIT(1),
	kLedgeFlag_useVault						= BIT(2),
	kLedgeFlag_isThin							= BIT(3),
	kLedgeFlag_isWindow						= BIT(4),
	kLedgeFlag_isDoubleSided			= BIT(5),
	kLedgeFlag_endCrouched				= BIT(6),
	kLedgeFlag_endFalling					= BIT(7),
	kLedgeFlag_useHighVault				= BIT(8),
	kledgeFlag_usableByMarines		= BIT(9),
	kledgeRunTimeOnlyFlag_p0IsEndOrCorner	= BIT(10),		// these flags are only ever injected into ledge flags at runtime on demand.
	kledgeRunTimeOnlyFlag_p1IsEndOrCorner	= BIT(11)			// they are not part of the flags loaded in from the file. Rather derived from markers on the fly.
};

typedef u16 ELedgeFlagBitfield;

struct LedgeId
{
private:
	
	enum { object_bits_shift  = 6, }; 
	enum { segment_bits_shift = 1, };

	enum { object_mask  = (BIT(10) - 1) << object_bits_shift };		// must add up to a 16 bit
	enum { segment_mask = (BIT(5) - 1)  << segment_bits_shift };
	enum { side_mask    = BIT(0) };

public:

	enum { invalid_id   = 0xFFFF };  // The last available ledge on the list (we hit the limit if we reach this value)

	LedgeId()
		: m_id( invalid_id )
	{

	}

	LedgeId( u16 id )
		: m_id( id )
	{

	}

	LedgeId( u16 objectIdx, u16 segmentIdx, u16 side )
	{
		DRX_ASSERT( objectIdx < 1024 );
		DRX_ASSERT( segmentIdx < 32 );
		DRX_ASSERT( side < 2 );

		m_id = (objectIdx << object_bits_shift) | ((segmentIdx << segment_bits_shift) & segment_mask) | (side & side_mask);
	}

	operator bool() const
	{
		return IsValid();
	}

	operator u16() const
	{
		return m_id;
	}

	ILINE bool IsValid() const
	{
		return (m_id != invalid_id);
	}

	ILINE u16 GetLedgeObjectIdx() const
	{
		return ((m_id & object_mask) >> object_bits_shift);
	}

	ILINE u16 GetSubSegmentIdx() const
	{
		return ((m_id & segment_mask) >> segment_bits_shift);
	}

	ILINE u16 GetSide() const
	{
		return (m_id & side_mask);
	}

	ILINE bool operator==(u16 id) const
	{
		return (m_id == id);
	}

private:

	u16	m_id;
};

struct SLedgeInfo
{
	SLedgeInfo()
		: valid( false )
		, cornerEndAdjustAmount(0.0f)
	{

	}

	SLedgeInfo( EntityId _entityId, const Vec3& p0, const Vec3& p1, const Vec3& facing, const ELedgeFlagBitfield _flags, const float _cornerEndAdjustAmount )
		: entityId(_entityId)
		, direction( facing )
		, flags( _flags )
		, valid( true )
		, cornerEndAdjustAmount(_cornerEndAdjustAmount)
	{
		points[0] = p0;
		points[1] = p1;
	}
	
	ILINE Vec3 GetPosition() const
	{
		return ((points[0] + points[1]) * 0.5f);
	}

	ILINE Vec3 GetFacingDirection() const
	{
		return direction;
	}

	ILINE Vec3 GetSegment() const
	{
		return (points[1] - points[0]);
	}

	ILINE bool IsValid( ) const
	{
		return valid;
	}

	ILINE float GetCornerEndAdjustAmount() const
	{
		return cornerEndAdjustAmount;
	}

	ILINE bool AreFlagsSet( ELedgeFlagBitfield checkFlags ) const
	{
		return ((flags & checkFlags) == checkFlags);
	}

	ILINE bool AreAnyFlagsSet( ELedgeFlagBitfield checkFlags ) const
	{
		return ((flags & checkFlags) != 0);
	}

	ILINE EntityId GetEntityId() const
	{
		return entityId;
	}

private:

	EntityId entityId;
	Vec3   points[2];
	Vec3   direction;
	float cornerEndAdjustAmount;
	ELedgeFlagBitfield flags;
	bool   valid;
};

struct SLedgeMarker
{
	Vec3 m_worldPosition;
	Vec3 m_facingDirection;
	bool m_endOrCorner;

	// IMPORTANT: Update STRUCT definition in .cpp file for exporting if you change this struct
	AUTO_STRUCT_INFO;
};

enum
{
	LedgeSide_In = 0,
	LedgeSide_Out,
	LedgeSide_Count
};

struct SLedgeObject
{
	SLedgeObject()
		: m_entityId(0)
		, m_markersStartIdx(0)
		, m_markersCount(0)
		, m_ledgeCornerEndAdjustAmount(0.0f)
	{
		m_ledgeFlags[LedgeSide_In] = m_ledgeFlags[LedgeSide_Out] = kLedgeFlag_none;
	}

	ILINE bool NeedsToBeSerialized() const { return (m_entityId != 0); }

	EntityId	m_entityId;

	u16		m_markersStartIdx;
	u16    m_markersCount;

	ELedgeFlagBitfield m_ledgeFlags[LedgeSide_Count];
	float			m_ledgeCornerEndAdjustAmount;

	// IMPORTANT: Update STRUCT definition in .cpp file for exporting if you change this struct

	AUTO_STRUCT_INFO;
};

struct SLevelLedges
{
	SLevelLedges()
		: m_pLedgeObjects(NULL)
		, m_pMarkers(NULL)
		, m_ledgeCount(0)
		, m_markerCount(0)
	{

	}

	~SLevelLedges()
	{
		Release();
	}

	void Allocate( u32k ledgeCount, u32k markerCount )
	{
		Release();

		if ( (ledgeCount > 0) && (markerCount > 0) )
		{
			m_pLedgeObjects = new SLedgeObject[ledgeCount];
			m_pMarkers      = new SLedgeMarker[markerCount];

			m_ledgeCount  = ledgeCount;
			m_markerCount = markerCount;
		}
	}

	void Release()
	{
		SAFE_DELETE_ARRAY(m_pLedgeObjects);
		SAFE_DELETE_ARRAY(m_pMarkers);
		m_ledgeCount = m_markerCount = 0;
	}

	SLedgeObject* FindLedgeForEntity( const EntityId entityId )
	{
		if (entityId != 0)
		{
			u32 objectIdx = 0;
			while ((objectIdx < m_ledgeCount) && (m_pLedgeObjects[objectIdx].m_entityId != entityId))
			{
				++objectIdx;
			}

			if (objectIdx < m_ledgeCount)
			{
				return &m_pLedgeObjects[objectIdx];
			}
		}

		return NULL;
	}

	SLedgeObject* m_pLedgeObjects;
	SLedgeMarker* m_pMarkers;
	
	u32				m_ledgeCount;
	u32				m_markerCount;        
};

//////////////////////////////////////////////////////////////////////////
/// Editor only ledge representation

#if LEDGE_MANAGER_EDITING_ENABLED

struct SLedgeObjectEditor
{
	SLedgeObjectEditor()
		: m_entityId(0)
		, m_ledgeCornerEndAdjustAmount(0.0f)
		, m_ledgeCornerMaxAngle(0.0f)
	{
		m_ledgeFlags[LedgeSide_In] = m_ledgeFlags[LedgeSide_Out] = kLedgeFlag_none;
	}

	~SLedgeObjectEditor()
	{
		stl::free_container(m_markers);
	}

	ILINE bool operator==( EntityId ledgeId ) const
	{
		return m_entityId == ledgeId;
	}

	ILINE bool operator!=( EntityId ledgeId ) const
	{
		return m_entityId != ledgeId;
	}

	void DebugDraw( IRenderAuxGeom* pRenderAux, const Sphere& visibleArea ) const;

	EntityId	m_entityId;

	typedef std::vector<SLedgeMarker> TMarkers;
	TMarkers  m_markers;

	ELedgeFlagBitfield m_ledgeFlags[LedgeSide_Count];
	float m_ledgeCornerMaxAngle;
	float m_ledgeCornerEndAdjustAmount;
};

typedef DrxFixedArray<SLedgeObjectEditor, MAX_LEDGE_ENTITIES> TLedgeObjectsEditorContainer;

class CLedgeUprEdit
{
public:
	
	CLedgeUprEdit( bool inEditor )
		: m_inEditorMode ( inEditor )
	{
		RegisterCVars();
	}

	ILINE bool IsInEditorMode( ) const
	{
		return m_inEditorMode;
	}

	ILINE void Reset()
	{
		m_ledgeObjects.clear();
	}

	void RegisterLedge( EntityId entityId, const SLedgeMarker* pMarkersArray, u32k markerCount, ELedgeFlagBitfield ledgeInFlags, ELedgeFlagBitfield ledgeOutFlags, float ledgeCornerMaxAngle, float ledgeCornerEndAdjustAmount );
	void UnregisterLedge( EntityId entityId );

	void UpdateLedgeMarkers( const EntityId entityId, const SLedgeMarker* pMarkersArray, u32k markerCount );
	void EnableLedge( const EntityId entityId, bool enable );

	LedgeId FindNearestLedge(const Vec3 &referencePosition, const Vec3 &testDirection, const float maxDistance, const float angleRange, const float extendedAngleRange) const;
	SLedgeInfo GetLedgeById( const LedgeId& ledgeId ) const;

	void Export( tukk fileName ) const;

	void DebugDraw() const;
	void OnDisplayHelpersChanged( bool displayHelpers );

private:
	
	void RegisterCVars();

	TLedgeObjectsEditorContainer m_ledgeObjects;
	bool m_inEditorMode;

	//////////////////////////////////////////////////////////////////////////
	// CVars
	i32   g_LedgeGrabUpr_DebugDrawInEditor;
	float g_LedgeGrabUpr_DebugDrawInEditor_Distance;

	i32		m_lastDebugDrawValue;
};

#else

class CLedgeUprEdit
{
public:

	CLedgeUprEdit( bool inEditor ){}

	ILINE bool IsInEditorMode( ) const { return false; }
	ILINE void Reset() {}

	ILINE void RegisterLedge( EntityId entityId, const SLedgeMarker* pMarkersArray, u32k markerCount, ELedgeFlagBitfield ledgeInFlags, ELedgeFlagBitfield ledgeOutFlags, float ledgeCornerMaxAngle, float ledgeCornerEndAdjustAmount ) {}
	ILINE void UnregisterLedge( EntityId entityId ) {}

	ILINE void UpdateLedgeMarkers( const EntityId entityId, const SLedgeMarker* pMarkersArray, u32k markerCount ) {}
	ILINE void EnableLedge( const EntityId entityId, bool enable ) {}

	ILINE LedgeId FindNearestLedge(const Vec3 &vPos, const Vec3 &vDir, float fMaxDistance = 2.0f, float fAngleRange = DEG2RAD(35.0f), float fExtendedAngleRange = DEG2RAD(50.0f)) const
	{
		return LedgeId( 0, 0, 0 );
	}

	ILINE SLedgeInfo GetLedgeById( const LedgeId& ledgeId ) const
	{
		return SLedgeInfo( );
	}

	ILINE void Export( tukk fileName ) const { };

	ILINE void DebugDraw( ) const { }
	ILINE void OnDisplayHelpersChanged( bool displayHelpers ) { }
};

#endif

//////////////////////////////////////////////////////////////////////////
/// Class which manages all ledge markers in the level
/// Editor:
///   While in editor the data is stored in a slight different format, less efficient, but easier for editing purposes
/// In game:
///   Data is structured in an efficient way for game usage, all editor functionality is turned off

class CLedgeUpr
{
public:

	CLedgeUpr();
	~CLedgeUpr();

	ILINE CLedgeUprEdit* GetEditorUpr()
	{
		return m_editorUpr.IsInEditorMode() ? &m_editorUpr : NULL;
	}

	ILINE const CLedgeUprEdit* GetEditorUpr() const
	{
		return m_editorUpr.IsInEditorMode() ? &m_editorUpr : NULL; 
	}

	void Reset();
	void Load( tukk fileName );

	void UpdateLedgeMarkers( const EntityId entityId, const SLedgeMarker* pMakersArray, u32k markerCount );
	void EnableLedge( const EntityId entityId, bool enable );

	SLedgeInfo	GetLedgeById( const LedgeId& ledgeId ) const;
	LedgeId			FindNearestLedge(const Vec3 &vPos, const Vec3 &vDir, float fMaxDistance = 2.0f, float fAngleRange = DEG2RAD(35.0f), float fExtendedAngleRange = DEG2RAD(50.0f)) const;
	Vec3				FindVectorToClosestPointOnLedge(const Vec3 &vPoint, const SLedgeInfo &ledgeInfo) const;

	void	Serialize(TSerialize ser);

	void DebugDraw() const;

private:

	void RegisterCVars();

	// Data used in pure game mode
	SLevelLedges      m_levelLedges;

	// Data used in editor
	CLedgeUprEdit m_editorUpr;

	//////////////////////////////////////////////////////////////////////////
	// CVars
	i32   g_LedgeGrabUpr_DebugDraw;
	float g_LedgeGrabUpr_DebugDraw_Distance;
};

#endif // __LEDGEMANAGER_H__