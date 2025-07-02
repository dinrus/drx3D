// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __INCLUDE_DRX3DENGINE_CLIPVOLUME_H
#define __INCLUDE_DRX3DENGINE_CLIPVOLUME_H

struct IBSPTree3D;

class CClipVolume : public IClipVolume
{
public:
	CClipVolume();

	////////////// IClipVolume implementation //////////////
	virtual void         GetClipVolumeMesh(_smart_ptr<IRenderMesh>& renderMesh, Matrix34& worldTM) const;
	virtual const AABB&  GetClipVolumeBBox() const;

	virtual u8 GetStencilRef() const      { return m_nStencilRef; }
	virtual uint  GetClipVolumeFlags() const { return m_nFlags; }
	virtual bool  IsPointInsideClipVolume(const Vec3& vPos) const;
	////////////////////////////

	void SetName(tukk szName);
	void SetStencilRef(i32 nStencilRef) { m_nStencilRef = nStencilRef; }

	float GetMaxViewDist() const;

	void Update(_smart_ptr<IRenderMesh> pRenderMesh, IBSPTree3D* pBspTree, const Matrix34& worldTM, u8 viewDistRatio, u32 flags);

	void GetMemoryUsage(class IDrxSizer* pSizer) const;

private:
	char                    m_sName[64];
	Matrix34                m_WorldTM;
	Matrix34                m_InverseWorldTM;
	_smart_ptr<IRenderMesh> m_pRenderMesh;
	IBSPTree3D*             m_pBspTree;
	AABB                    m_BBoxWS;
	AABB                    m_BBoxLS;
	u32                  m_nFlags;
	u8                   m_nStencilRef;
	u8                   m_viewDistRatio;
};

#endif //__INCLUDE_DRX3DENGINE_CLIPVOLUME_H
