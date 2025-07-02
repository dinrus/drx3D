// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __CCULLTHREAD__
#define __CCULLTHREAD__

#include <drx3D/CoreX/Thread/IJobUpr.h>

namespace NAsyncCull
{

class DRX_ALIGN(128) CCullThread: public DinrusX3dEngBase
{
	bool m_Enabled;

	bool m_Active;                                      // used to verify that the cull job is running and no new jobs are added after the job has finished

public:
	enum PrepareStateT { IDLE, PREPARE_STARTED, PREPARE_DONE, CHECK_REQUESTED, CHECK_STARTED };
	PrepareStateT m_nPrepareState;
	DrxCriticalSection m_FollowUpLock;
	SRenderingPassInfo m_passInfoForCheckOcclusion;
	u32 m_nRunningReprojJobs;
	u32 m_nRunningReprojJobsAfterMerge;
	i32 m_bCheckOcclusionRequested;
private:
	uk m_pCheckOcclusionJob;
	JobUpr::SJobState m_JobStatePrepareOcclusionBuffer;
	JobUpr::SJobState m_PrepareBufferSync;
	Matrix44A m_MatScreenViewProj;
	Matrix44A m_MatScreenViewProjTransposed;
	Vec3 m_ViewDir;
	Vec3 m_Position;
	float m_NearPlane;
	float m_FarPlane;
	float m_NearestMax;

	PodArray<u8> m_OCMBuffer;
	u8* m_pOCMBufferAligned;
	u32 m_OCMMeshCount;
	u32 m_OCMInstCount;
	u32 m_OCMOffsetInstances;

	template<class T>
	T Swap(T& rData)
	{
		// #if IS_LOCAL_MACHINE_BIG_ENDIAN
		PREFAST_SUPPRESS_WARNING(6326)
		switch (sizeof(T))
		{
		case 1:
			break;
		case 2:
			SwapEndianBase(reinterpret_cast<u16*>(&rData));
			break;
		case 4:
			SwapEndianBase(reinterpret_cast<u32*>(&rData));
			break;
		case 8:
			SwapEndianBase(reinterpret_cast<uint64*>(&rData));
			break;
		default:
			UNREACHABLE();
		}
		//#endif
		return rData;
	}

	void RasterizeZBuffer(u32 PolyLimit);
	void OutputMeshList();

public:

	void CheckOcclusion();
	void PrepareOcclusion();

	void PrepareOcclusion_RasterizeZBuffer();
	void PrepareOcclusion_ReprojectZBuffer();
	void PrepareOcclusion_ReprojectZBufferLine(i32 nStartLine, i32 nNumLines);
	void PrepareOcclusion_ReprojectZBufferLineAfterMerge(i32 nStartLine, i32 nNumLines);
	void Init();

	bool LoadLevel(tukk pFolderName);
	void UnloadLevel();

	bool TestAABB(const AABB &rAABB, float fEntDistance, float fVerticalExpand = 0.0f);
	bool TestQuad(const Vec3 &vCenter, const Vec3 &vAxisX, const Vec3 &vAxisY);

	CCullThread();
	~CCullThread();

#ifndef _RELEASE
	void CoverageBufferDebugDraw();
#endif

	void PrepareCullbufferAsync(const CCamera &rCamera);
	void CullStart(const SRenderingPassInfo &passInfo);
	void CullEnd();

	bool IsActive() const        { return m_Active; }
	void SetActive(bool bActive) { m_Active = bActive; }

	Vec3 GetViewDir()            { return m_ViewDir; };

};

}

#endif
