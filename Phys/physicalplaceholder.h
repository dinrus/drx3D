// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef physicalplaceholder_h
#define physicalplaceholder_h
#pragma once

#pragma pack(push)
#pragma pack(1)
// Packed and aligned structure ensures bitfield packs properly across
// 4 byte boundaries but stops the compiler implicitly doing byte copies
struct DRX_ALIGN(4) pe_gridthunk {
	uint64 inext : 20; // int64 for tighter packing
	uint64 iprev : 20;
	uint64 inextOwned : 20;
	uint64 iSimClass : 3;
	uint64 bFirstInCell : 1;
	u8 BBox[4];
	i32 BBoxZ0 : 16;
	i32 BBoxZ1 : 16;
	class CPhysicalPlaceholder *pent;
};
#pragma pack(pop)

class CPhysicalEntity;
i32k NO_GRID_REG = -1<<14;
i32k GRID_REG_PENDING = NO_GRID_REG+1;
i32k GRID_REG_LAST = NO_GRID_REG+2;

class CPhysicalPlaceholder : public IPhysicalEntity {
public:
	CPhysicalPlaceholder()
		: m_pForeignData(nullptr)
		, m_iForeignData(0)
		, m_iForeignFlags(0)
		, m_iGThunk0(0)
		, m_pEntBuddy(nullptr)
		, m_bProcessed(0)
		, m_id(0)
		, m_bOBBThunks(0)
		, m_iSimClass(0)
		, m_lockUpdate(0)
	{
		static_assert(DRX_ARRAY_COUNT(m_BBox) == 2, "Неполноценный размер массива!");
		m_BBox[0].zero();
		m_BBox[1].zero();

		static_assert(DRX_ARRAY_COUNT(m_ig) == 2, "Неполноценный размер массива!");
		m_ig[0].x=m_ig[1].x=m_ig[0].y=m_ig[1].y = GRID_REG_PENDING;
	}

	virtual CPhysicalEntity *GetEntity();
	virtual CPhysicalEntity *GetEntityFast() { return (CPhysicalEntity*)m_pEntBuddy; }
	virtual bool IsPlaceholder() const { return true; };

	virtual i32 AddRef() { return 0; }
	virtual i32 Release() { return 0; }

	virtual pe_type GetType() const;
	virtual i32 SetParams(pe_params* params,i32 bThreadSafe=1);
	virtual i32 GetParams(pe_params* params) const;
	virtual i32 GetStatus(pe_status* status) const;
	virtual i32 Action(pe_action* action,i32 bThreadSafe=1);

	virtual i32 AddGeometry(phys_geometry *pgeom, pe_geomparams* params,i32 id=-1,i32 bThreadSafe=1);
	virtual void RemoveGeometry(i32 id,i32 bThreadSafe=1);

	virtual uk GetForeignData(i32 itype=0) const { return m_iForeignData==itype ? m_pForeignData : 0; }
	virtual i32 GetiForeignData() const { return m_iForeignData; }

	virtual i32 GetStateSnapshot(class CStream &stm, float time_back=0, i32 flags=0);
	virtual i32 GetStateSnapshot(TSerialize ser, float time_back=0, i32 flags=0);
	virtual i32 SetStateFromSnapshot(class CStream &stm, i32 flags=0);
	virtual i32 SetStateFromSnapshot(TSerialize ser, i32 flags=0);
	virtual i32 SetStateFromTypedSnapshot(TSerialize ser, i32 type, i32 flags=0);
	virtual i32 PostSetStateFromSnapshot();
	virtual i32 GetStateSnapshotTxt(char *txtbuf,i32 szbuf, float time_back=0);
	virtual void SetStateFromSnapshotTxt(tukk txtbuf,i32 szbuf);
	virtual u32 GetStateChecksum();
	virtual void SetNetworkAuthority(i32 authoritive, i32 paused);

	virtual void StartStep(float time_interval);
	virtual i32 Step(float time_interval);
	virtual i32 DoStep(float time_interval,i32 iCaller) { return 1; }
	virtual void StepBack(float time_interval);
	virtual IPhysicalWorld *GetWorld() const;

	virtual void GetMemoryStatistics(IDrxSizer *pSizer) const {};

	Vec3 m_BBox[2];

	uk m_pForeignData;
	i32 m_iForeignData  : 16;
	i32 m_iForeignFlags : 16;

	struct vec2dpacked {
		i32 x : 16;
		i32 y : 16;
	};
	vec2dpacked m_ig[2];
	i32 m_iGThunk0;
#ifdef MULTI_GRID
	struct SEntityGrid *m_pGrid = nullptr;
#endif

	CPhysicalPlaceholder *m_pEntBuddy;
	 u32 m_bProcessed;
	i32 m_id : 23;
	i32 m_bOBBThunks : 1;
	i32 m_iSimClass : 8;
	mutable i32 m_lockUpdate;
};

#endif
