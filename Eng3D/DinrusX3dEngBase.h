// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   drx3denginebase.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание: Access to external stuff used by 3d engine. Most 3d engine classes
//               are derived from this base class to access other interfaces
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _DinrusX3dEngBase_h_
#define _DinrusX3dEngBase_h_

#include "3DEngineMemory.h"

struct ISystem;
struct IRenderer;
struct ILog;
struct IPhysicalWorld;
struct ITimer;
struct IConsole;
struct I3DEngine;
struct CVars;
struct CVisAreaUpr;
namespace pfx2{
struct IParticleSystem;
}
class CTerrain;
class CIndirectLighting;
class CObjUpr;
class C3DEngine;
class CParticleUpr;
class CDecalUpr;
class CRainUpr;
class CSkyLightUpr;
class CWaterWaveUpr;
class CRenderMeshMerger;
class CMergedMeshesUpr;
class CGeomCacheUpr;
class CBreezeGenerator;
class CMatMan;
class CClipVolumeUpr;

#define DISTANCE_TO_THE_SUN 1000000

#if !defined(_RELEASE) || defined(ENABLE_STATOSCOPE_RELEASE)
	#define OBJMAN_STREAM_STATS
#endif

struct DinrusX3dEngBase
{
	static ISystem*                 m_pSystem;
#if !defined(DEDICATED_SERVER)
	static IRenderer*               m_pRenderer;
#else
	static IRenderer* const         m_pRenderer;
#endif
	static ITimer*                  m_pTimer;
	static ILog*                    m_pLog;
	static IPhysicalWorld*          m_pPhysicalWorld;
	static IConsole*                m_pConsole;
	static C3DEngine*               m_p3DEngine;
	static CVars*                   m_pCVars;
	static IDrxPak*                 m_pDrxPak;
	static CObjUpr*             m_pObjUpr;
	static CTerrain*                m_pTerrain;
	static IParticleUpr*        m_pPartUpr;
	static IOpticsUpr*          m_pOpticsUpr;
	static CDecalUpr*           m_pDecalUpr;
	static CVisAreaUpr*         m_pVisAreaUpr;
	static CClipVolumeUpr*      m_pClipVolumeUpr;
	static CMatMan*                 m_pMatMan;
	static CSkyLightUpr*        m_pSkyLightUpr;
	static CWaterWaveUpr*       m_pWaterWaveUpr;
	static CRenderMeshMerger*       m_pRenderMeshMerger;
	static CMergedMeshesUpr*    m_pMergedMeshesUpr;
	static CBreezeGenerator*        m_pBreezeGenerator;
	static IStreamedObjectListener* m_pStreamListener;
#if defined(USE_GEOM_CACHES)
	static CGeomCacheUpr*       m_pGeomCacheUpr;
#endif

	static bool              m_bProfilerEnabled;
	static threadID          m_nMainThreadId;
	static bool              m_bLevelLoadingInProgress;
	static bool              m_bIsInRenderScene;
	static bool              m_bAsyncOctreeUpdates;
	static bool              m_bRenderTypeEnabled[eERType_TypesNum];
	static i32               m_mergedMeshesPoolSize;

	static ESystemConfigSpec m_LightConfigSpec;
#if DRX_PLATFORM_DESKTOP
	static bool              m_bEditor;
#else
	static const bool        m_bEditor = false;
#endif
	static i32               m_arrInstancesCounter[eERType_TypesNum];

	// components access
	ILINE static ISystem*            GetSystem()                 { return m_pSystem; }
	ILINE static IRenderer*          GetRenderer()               { return m_pRenderer; }
	ILINE static ITimer*             GetTimer()                  { return m_pTimer; }
	ILINE static ILog*               GetLog()                    { return m_pLog; }

	inline static IPhysicalWorld*    GetPhysicalWorld()          { return m_pPhysicalWorld; }
	inline static IConsole*          GetConsole()                { return m_pConsole; }
	inline static C3DEngine*         Get3DEngine()               { return m_p3DEngine; }
	inline static CObjUpr*       GetObjUpr()             { return m_pObjUpr; };
	inline static CTerrain*          GetTerrain()                { return m_pTerrain; };
	inline static CVars*             GetCVars()                  { return m_pCVars; }
	inline static CVisAreaUpr*   GetVisAreaUpr()         { return m_pVisAreaUpr; }
	inline static IDrxPak*           GetPak()                    { return m_pDrxPak; }
	inline static CMatMan*           GetMatMan()                 { return m_pMatMan; }
	inline static CWaterWaveUpr* GetWaterWaveUpr()       { return m_pWaterWaveUpr; };
	inline static CRenderMeshMerger* GetSharedRenderMeshMerger() { return m_pRenderMeshMerger; };
	inline static CTemporaryPool*    GetTemporaryPool()          { return CTemporaryPool::Get(); };

#if defined(USE_GEOM_CACHES)
	inline static CGeomCacheUpr* GetGeomCacheUpr() { return m_pGeomCacheUpr; };
#endif

	inline static i32 GetMergedMeshesPoolSize()                               { return m_mergedMeshesPoolSize; }
	ILINE static bool IsRenderNodeTypeEnabled(EERType rnType)                 { return m_bRenderTypeEnabled[(i32)rnType]; }
	ILINE static void SetRenderNodeTypeEnabled(EERType rnType, bool bEnabled) { m_bRenderTypeEnabled[(i32)rnType] = bEnabled; }

	static float      GetCurTimeSec();
	static float      GetCurAsyncTimeSec();

	static void       PrintMessage(tukk szText, ...) PRINTF_PARAMS(1, 2);
	static void       PrintMessagePlus(tukk szText, ...) PRINTF_PARAMS(1, 2);
	static void       PrintComment(tukk szText, ...) PRINTF_PARAMS(1, 2);

	// Validator warning.
	static void    Warning(tukk format, ...) PRINTF_PARAMS(1, 2);
	static void    Error(tukk format, ...) PRINTF_PARAMS(1, 2);
	static void    FileWarning(i32 flags, tukk file, tukk format, ...) PRINTF_PARAMS(3, 4);

	CRenderObject* GetIdentityCRenderObject(const SRenderingPassInfo &passInfo)
	{
		CRenderObject* pCRenderObject = passInfo.GetIRenderView()->AllocateTemporaryRenderObject();
		if (!pCRenderObject)
			return NULL;
		pCRenderObject->SetMatrix(Matrix34::CreateIdentity(), passInfo);
		return pCRenderObject;
	}

	static bool IsValidFile(tukk sFilename);
	static bool IsResourceLocked(tukk sFilename);

	static bool IsPreloadEnabled();

	IMaterial*  MakeSystemMaterialFromShader(tukk sShaderName, SInputShaderResources* Res = NULL);
	static void DrawBBoxLabeled(const AABB& aabb, const Matrix34& m34, const ColorB& col, tukk format, ...) PRINTF_PARAMS(4, 5);
	static void DrawBBox(const Vec3& vMin, const Vec3& vMax, ColorB col = Col_White);
	static void DrawBBox(const AABB& box, ColorB col = Col_White);
	static void DrawLine(const Vec3& vMin, const Vec3& vMax, ColorB col = Col_White);
	static void DrawSphere(const Vec3& vPos, float fRadius, ColorB color = ColorB(255, 255, 255, 255));
	static void DrawQuad(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& v3, ColorB color);

	i32&        GetInstCount(EERType eType)                            { return m_arrInstancesCounter[eType]; }

	u32      GetMinSpecFromRenderNodeFlags(uint64 dwRndFlags) const { return (dwRndFlags & ERF_SPEC_BITS_MASK) >> ERF_SPEC_BITS_SHIFT; }
	static bool CheckMinSpec(u32 nMinSpec);

	static bool IsEscapePressed();

	size_t      fread(
	  uk buffer,
	  size_t elementSize,
	  size_t count,
	  FILE* stream)
	{
		size_t res = ::fread(buffer, elementSize, count, stream);
		if (res != count)
			Error("fread() failed");
		return res;
	}

	i32 fseek(
	  FILE* stream,
	  long offset,
	  i32 whence
	  )
	{
		i32 res = ::fseek(stream, offset, whence);
		if (res != 0)
			Error("fseek() failed");
		return res;
	}

};

#endif // _DinrusX3dEngBase_h_
