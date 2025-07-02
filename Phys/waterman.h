// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef waterman_h
#define waterman_h
#pragma once


struct SWaterTile : SWaterTileBase {
	SWaterTile() {}
	SWaterTile(i32 _nCells) { i32 n=sqr(nCells=_nCells); ph=new float[n]; pvel=new Vec3[n]; mv=new Vec2[n]; m=new float[n]; norm=new uchar[n]; bActive=0; }
	~SWaterTile() { delete[] ph; delete[] pvel; delete[] mv; delete[] m; delete[] norm; }
	SWaterTile *Activate(i32 bActivate=1) {	if (bActive && !bActivate) zero(); bActive=bActivate; return this; }
	void zero() { i32 n=sqr(nCells); memset(ph,0,n*sizeof(ph[0])); memset(pvel,0,n*sizeof(pvel[0])); memset(pvel,0,n*sizeof(pvel[0])); memset(norm,0,sizeof(norm[0])); }
	i32 cell_used(i32 i) { return isneg((i32)norm[i]-255); }

	Vec2 *mv;
	float *m;
	uchar *norm;
	i32 nCells;
};


class CWaterMan {
public:
	CWaterMan(class CPhysicalWorld *pWorld, IPhysicalEntity *pArea=0);
	~CWaterMan();
	i32 SetParams(pe_params *_params);
	i32 GetParams(pe_params *_params);
	i32 GetStatus(pe_status *_status);
	void OnWaterInteraction(class CPhysicalEntity *pent);
	void OnWaterHit(const Vec3 &pthit,const Vec3 &vel);
	void SetViewerPos(const Vec3& newpos,i32 iCaller=0);
	void UpdateWaterLevel(const plane& waterPlane, float dt=0);
	void OffsetWaterLevel(float diff,float dt);
	void GenerateSurface(const Vec2i* BBox, Vec3 *pvtx,Vec3_tpl<index_t> *pTris, const Vec2 *ptbrd,i32 nbrd, const Vec3 &offs=Vec3(ZERO), Vec2 *pttmp=0, float tmax=0);
	void TimeStep(float dt);
	i32 IsActive() { for(i32 i=0;i<sqr(m_nTiles*2+1);i++) if (m_pTiles[i] && m_pTiles[i]->bActive) return 1; return 0; }
	void Reset() { for(i32 i=0;i<sqr(m_nTiles*2+1);i++) if (m_pTiles[i]) m_pTiles[i]->Activate(0); m_doffs=0; }
	void DrawHelpers(IPhysRenderer *pRenderer);
	void GetMemoryStatistics(IDrxSizer *pSizer) const;

	i32 GetCellIdx(Vec2i pt, i32 &idxtile) {
		float rncells = m_cellSz*m_rtileSz;
		Vec2i itile(float2int(pt.x*rncells),float2int(pt.y*rncells));
		idxtile = max(0,min(sqr(m_nTiles+1)-1, itile.x+m_nTiles+(itile.y+m_nTiles)*(m_nTiles*2+1)));
		return pt.x-itile.x*m_nCells+(m_nCells>>1)+(pt.y-itile.y*m_nCells+(m_nCells>>1))*m_nCells;
	}
	float GetHeight(Vec2i pt) {
		i32 itile,icell=GetCellIdx(pt,itile);
		if (!m_pTiles[itile])
			return 0;
		return m_pTiles[itile]->ph[icell];
	}
	float GetHeight(Vec2 pt) {
		Vec2i ipt(float2int(pt.x*m_rcellSz), float2int(pt.y*m_rcellSz));
		float x=pt.x*m_rcellSz-ipt.x+0.5f, y=pt.y*m_rcellSz-ipt.y;
		i32 iflip = isneg(1-x-y);	float flip=1-iflip*2;
		return GetHeight(ipt)*max(1-x-y,0.0f) + GetHeight(ipt+Vec2i(1,0))*(x*flip+iflip) + GetHeight(ipt+Vec2i(0,1))*(y*flip+iflip) + GetHeight(ipt+Vec2i(1,1))*max(x+y-1,0.0f);
	}
	inline void advect_cell(const Vec2i& itile, const Vec2i& ic, const Vec2& vel, float m, grid* rgrid);
	inline SWaterTile* get_tile(const Vec2i& itile,const Vec2i& ic,i32 &i1);

	class CPhysicalWorld *m_pWorld;
	IPhysicalEntity *m_pArea;
	CWaterMan *m_next,*m_prev;
	i32 m_bActive;
	float m_timeSurplus,m_dt;
	i32 m_nTiles,m_nCells;
	float m_tileSz,m_rtileSz;
	float m_cellSz,m_rcellSz;
	float m_waveSpeed;
	float m_dampingCenter,m_dampingRim;
	float m_minhSpread,m_minVel;
	float m_kResistance,m_depth,m_hlimit;
	float m_doffs;
	Vec3 m_origin;
	Vec3 m_waterOrigin,m_posViewer;
	i32 m_ix,m_iy;
	Matrix33 m_R;
	SWaterTile **m_pTiles,**m_pTilesTmp;
	char *m_pCellMask;
	Vec2 *m_pCellNorm;
	i32 *m_pCellQueue,m_szCellQueue;
	mutable  i32 m_lockUpdate;
};

#endif
