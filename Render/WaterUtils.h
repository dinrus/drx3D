// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   ShadowUtils.h :

   Revision история:
* Created by Tiago Sousa
   =============================================================================*/

#ifndef __WATERUTILS_H__
#define __WATERUTILS_H__

class CWaterSim;

// todo. refactor me - should be more general

class CWater
{
public:

	CWater() : m_pWaterSim(0)
	{
	}

	~CWater()
	{
		Release();
	}

	// Create/Initialize simulation
	void Create(float fA, float fWind, float fWorldSizeX, float fWorldSizeY);
	void Release();
	void SaveToDisk(tukk pszFileName);

	// Update water simulation
	void Update(i32 nFrameID, float fTime, bool bOnlyHeight = false, uk pRawPtr = NULL);

	// Get water simulation data
	Vec3      GetPositionAt(i32 x, i32 y) const;
	float     GetHeightAt(i32 x, i32 y) const;
	Vec4*     GetDisplaceGrid() const;

	i32k GetGridSize() const;

	void      GetMemoryUsage(IDrxSizer* pSizer) const;

	bool      NeedInit() const { return m_pWaterSim == NULL; }
private:

	CWaterSim* m_pWaterSim;
};

static CWater* WaterSimMgr()
{
	return gRenDev->m_pWaterSimMgr;
}

#endif
