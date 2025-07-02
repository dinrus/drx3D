// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef tetrlattice_h
#define tetrlattice_h
#pragma once

enum ltension_type { LPull,LPush,LShift,LTwist,LBend };
enum lvtx_flags { lvtx_removed=1,lvtx_removed_new=2,lvtx_processed=4,lvtx_inext_log2=8 };
enum ltet_flags { ltet_removed=1,ltet_removed_new=2,ltet_processed=4,ltet_inext_log2=8 };

struct STetrahedron {
	i32 flags;
	float M,Minv;
  float Vinv;
	Matrix33 Iinv;
	Vec3 Pext,Lext;
	float area;
	i32 ivtx[4];
	i32 ibuddy[4];
	float fracFace[4];
	i32 idxface[4];
	i32 idx;
};

struct SCGTetr {
	Vec3 dP,dL;
	float Minv;
	Matrix33 Iinv;
};

enum lface_flags { lface_processed=1 };

struct SCGFace {
	i32 itet,iface;
	Vec3 rv,rw;
	Vec3 dv,dw;
	Vec3 dP,dL;
	Vec3 P,L;
	SCGTetr *pTet[2];
	Vec3 r0,r1;
	Matrix33 vKinv,wKinv;
	i32 flags;
};


class CTetrLattice : public ITetrLattice {
public:
	CTetrLattice(IPhysicalWorld *pWorld);
	CTetrLattice(CTetrLattice *src, i32 bCopyData);
	~CTetrLattice();
	virtual void Release() { delete this; }

	CTetrLattice *CreateLattice(const Vec3 *pVtx,i32 nVtx, i32k *pTets,i32 nTets);
	void SetMesh(CTriMesh *pMesh);
	void SetGrid(const box &bbox);
	void SetIdMat(i32 id) { m_idmat = id; }

	virtual i32 SetParams(pe_params *_params);
	virtual i32 GetParams(pe_params *_params);

	void Subtract(IGeometry *pGeonm, const geom_world_data *pgwd1,const geom_world_data *pgwd2);
	i32 CheckStructure(float time_interval,const Vec3 &gravity, const plane *pGround,i32 nPlanes,pe_explosion *pexpl, i32 maxIters=100000,i32 bLogTension=0);
	void Split(CTriMesh **pChunks,i32 nChunks, CTetrLattice **pLattices);
	i32 Defragment();
	virtual void DrawWireframe(IPhysRenderer *pRenderer, geom_world_data *gwd, i32 idxColor);
	float GetLastTension(i32 &itype) { itype=m_imaxTension; return m_maxTension; }
	i32 AddImpulse(const Vec3 &pt, const Vec3 &impulse,const Vec3 &momentum, const Vec3 &gravity,float worldTime);

	virtual IGeometry *CreateSkinMesh(i32 nMaxTrisPerBVNode);
	virtual i32 CheckPoint(const Vec3 &pt, i32 *idx, float *w);

	i32 GetFaceByBuddy(i32 itet,i32 itetBuddy) {
		i32 i,ibuddy=0,imask;
		for(i=1;i<4;i++) {
			imask = -iszero(m_pTetr[itet].ibuddy[i]-itetBuddy);
			ibuddy = ibuddy&~imask | i&imask;
		}
		return ibuddy;
	}
	Vec3 GetTetrCenter(i32 i) {
		return (m_pVtx[m_pTetr[i].ivtx[0]]+m_pVtx[m_pTetr[i].ivtx[1]]+m_pVtx[m_pTetr[i].ivtx[2]]+m_pVtx[m_pTetr[i].ivtx[3]])*0.25f;
	}
	template<class T> i32 GetFaceIdx(i32 itet, i32 iface, T* idx) {
		for(i32 i=0,j=3-iface,dir=(iface&1)*2-1; i<3; (j+=dir)&=3)
			idx[i++] = m_pTetr[itet].ivtx[j];
		return 3;
	}

	IPhysicalWorld *m_pWorld;

	CTriMesh *m_pMesh;
	Vec3 *m_pVtx;
	i32 m_nVtx;
	STetrahedron *m_pTetr;
	i32 m_nTetr;
	i32 *m_pVtxFlags;
	i32 m_nMaxCracks;
	i32 m_idmat;
	float m_maxForcePush,m_maxForcePull,m_maxForceShift;
	float m_maxTorqueTwist,m_maxTorqueBend;
	float m_crackWeaken;
	float m_density;
	i32 m_nRemovedTets;
	i32 *m_pVtxRemap;
	i32 m_flags;
	float m_maxTension;
	i32 m_imaxTension;
	float m_lastImpulseTime;

	Matrix33 m_RGrid;
	Vec3 m_posGrid;
	Vec3 m_stepGrid,m_rstepGrid;
	Vec3i m_szGrid,m_strideGrid;
	i32 *m_pGridTet0,*m_pGrid;

	static SCGFace *g_Faces;
	static SCGTetr *g_Tets;
	static i32 g_nFacesAlloc,g_nTetsAlloc;
};


class CBreakableGrid2d : public IBreakableGrid2d {
public:
	CBreakableGrid2d() { m_pt=0; m_pTris=0; m_pCellDiv=0; m_nTris=0; m_nTris0=1<<31; m_pCellQueue=new i32[m_szCellQueue=32]; }
	~CBreakableGrid2d() {
		if (m_pt) delete[] m_pt;
		if (m_pTris) delete[] m_pTris;
		if (m_pCellDiv) delete[] m_pCellDiv;
		if (m_pCellQueue) delete[] m_pCellQueue;
	}
	void Generate(Vec2 *ptsrc,i32 npt, const Vec2i &nCells, i32 bStaticBorder, i32 seed=-1);

	virtual i32 *BreakIntoChunks(const Vec2 &pt, float r, Vec2 *&ptout, i32 maxPatchTris,float jointhresh,i32 seed=-1,
		float filterAng=0.0f,float ry=0.0f);
	virtual grid *GetGridData() { return &m_coord; }
	virtual bool IsEmpty() { return m_nTris==0; }
	virtual void Release() { delete this; }
	virtual float GetFracture() { return (float)(m_nTris0-m_nTris)/(float)m_nTris0; }

	void MarkCellInterior(i32 i);
	i32 get_neighb(i32 iTri,i32 iEdge);
	void get_edge_ends(i32 iTri,i32 iEdge, i32 &iend0,i32 &iend1);
	i32 CropSpikes(i32 imin,i32 imax, i32 *queue,i32 szQueue, i32 flags,i32 flagsNew, float thresh);

	virtual void GetMemoryStatistics(IDrxSizer *pSizer) const {
		SIZER_COMPONENT_NAME(pSizer, "brekable2d grid");
		if (m_pt) pSizer->AddObject(m_pt, m_coord.size.x*m_coord.size.y*(sizeof(m_pt[0])+sizeof(m_pTris[0])*2+sizeof(m_pCellDiv[0])));
		if (m_pCellQueue) pSizer->AddObject(m_pCellQueue, m_szCellQueue);
	}

	enum tritypes { TRI_AVAILABLE=1<<29, TRI_FIXED=1<<27, TRI_EMPTY=1<<26, TRI_STABLE=1<<25, TRI_PROCESSED=1<<24 };
	primitives::grid m_coord;
	Vec2 *m_pt;
	i32 *m_pTris;
	char *m_pCellDiv;
	i32 m_nTris,m_nTris0;
	i32 *m_pCellQueue;
	i32 m_szCellQueue;
};

#endif
