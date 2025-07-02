// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __INCLUDE_DRX3DENGINE_CLIPVOLUMEMANAGER_H
#define __INCLUDE_DRX3DENGINE_CLIPVOLUMEMANAGER_H

class CClipVolume;

class CClipVolumeUpr : public DinrusX3dEngBase
{
	struct SClipVolumeInfo
	{
		CClipVolume* m_pVolume         = nullptr;
		i32          m_updateFrameId   = 0;
		float        m_currentViewDist = std::numeric_limits<float>::max();
		bool         m_bActive         = false;

		SClipVolumeInfo(CClipVolume* pVolume) : m_pVolume(pVolume) {}
		bool operator==(const SClipVolumeInfo& other) const { return m_pVolume == other.m_pVolume; }
	};

public:
	static u8k InactiveVolumeStencilRef    = 0xFE;
	static u8k AffectsEverythingStencilRef = 0xFF;

	virtual ~CClipVolumeUpr();

	virtual IClipVolume* CreateClipVolume();
	virtual bool         DeleteClipVolume(IClipVolume* pClipVolume);
	virtual bool         UpdateClipVolume(IClipVolume* pClipVolume, _smart_ptr<IRenderMesh> pRenderMesh, IBSPTree3D* pBspTree, const Matrix34& worldTM, u8 viewDistRatio, bool bActive, u32 flags, tukk szName);
	void                 TrimDeletedClipVolumes(i32 trimFrameId = std::numeric_limits<i32>::max());

	void                 PrepareVolumesForRendering(const SRenderingPassInfo& passInfo);

	void                 UpdateEntityClipVolume(const Vec3& pos, IRenderNode* pRenderNode);

	bool                 IsClipVolumeRequired(IRenderNode* pRenderNode) const;
	CClipVolume*         GetClipVolumeByPos(const Vec3& pos, const IClipVolume* pIgnoreVolume = NULL) const;

	void GetMemoryUsage(class IDrxSizer* pSizer) const;
	size_t GetClipVolumeCount() const { return m_clipVolumes.size(); }

private:
	std::vector<SClipVolumeInfo> m_clipVolumes;
	std::vector<SClipVolumeInfo> m_deletedClipVolumes;

};

#endif //__INCLUDE_DRX3DENGINE_CLIPVOLUMEMANAGER_H
