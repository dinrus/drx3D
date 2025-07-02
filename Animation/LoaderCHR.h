// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/ModelAnimationSet.h>
#include <drx3D/Animation/ModelMesh.h>
#include <drx3D/Sys/IStreamEngine.h>
#include <drx3D/Animation/ParamLoader.h>

class CSkin;
class CDefaultSkeleton;

class DrxCHRLoader : public IStreamCallback
{

public:
	DrxCHRLoader()
	{
		m_pModelSkel = 0;
		m_pModelSkin = 0;
	}
	~DrxCHRLoader()
	{
#ifndef _RELEASE
		if (m_pStream)
			__debugbreak();
#endif
		ClearModel();
	}

	bool        BeginLoadCHRRenderMesh(CDefaultSkeleton* pSkel, const DynArray<CCharInstance*>& pCharInstances, EStreamTaskPriority estp);
	bool        BeginLoadSkinRenderMesh(CSkin* pSkin, i32 nRenderLod, EStreamTaskPriority estp);
	void        AbortLoad();

	static void ClearCachedLodSkeletonResults();

	// the controller manager for the new model; this remains the same during the whole lifecycle the file without extension
	string m_strGeomFileNameNoExt;

public: // IStreamCallback Members
	virtual void StreamAsyncOnComplete(IReadStream* pStream, unsigned nError);
	virtual void StreamOnComplete(IReadStream* pStream, unsigned nError);

private:
	struct SmartContentCGF
	{
		CContentCGF* pCGF;
		SmartContentCGF(CContentCGF* pCGF) : pCGF(pCGF) {}
		~SmartContentCGF() { if (pCGF) g_pI3DEngine->ReleaseChunkfileContent(pCGF); }
		CContentCGF* operator->() { return pCGF; }
		CContentCGF& operator*()  { return *pCGF; }
		operator CContentCGF*() const { return pCGF; }
		operator bool() const { return pCGF != NULL; }
	};

private:

	void                    EndStreamSkel(IReadStream* pStream);
	void                    EndStreamSkinAsync(IReadStream* pStream);
	void                    EndStreamSkinSync(IReadStream* pStream);

	void                    PrepareMesh(CMesh* pMesh);
	void                    PrepareRenderChunks(DynArray<RChunk>& arrRenderChunks, CMesh* pMesh);

	void                    ClearModel();

private:
	CDefaultSkeleton*        m_pModelSkel;
	CSkin*                   m_pModelSkin;
	IReadStreamPtr           m_pStream;
	DynArray<RChunk>         m_arrNewRenderChunks;
	_smart_ptr<IRenderMesh>  m_pNewRenderMesh;
	DynArray<CCharInstance*> m_RefByInstances;
};
