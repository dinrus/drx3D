// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   DeferredRenderUtils.h :

   Revision история:
* Created by Nick Kasyan
   =============================================================================*/

#ifndef __DEFERRED_RENDER_UTILS_H__
#define __DEFERRED_RENDER_UTILS_H__

#define SDeferMeshVert SVF_P3F_C4B_T2F

typedef stl::aligned_vector<SVF_P3F_C4B_T2F, DRX_PLATFORM_ALIGNMENT> t_arrDeferredMeshVertBuff;
typedef stl::aligned_vector<u16         , DRX_PLATFORM_ALIGNMENT> t_arrDeferredMeshIndBuff;

class CDeferredRenderUtils
{
public:
	static void CreateUnitFrustumMesh(i32 tessx, i32 tessy, t_arrDeferredMeshIndBuff& indBuff, t_arrDeferredMeshVertBuff& vertBuff);
	static void CreateUnitFrustumMeshTransformed(SRenderLight* pLight, ShadowMapFrustum* pFrustum, i32 nAxis, i32 tessx, i32 tessy, t_arrDeferredMeshIndBuff& indBuff, t_arrDeferredMeshVertBuff& vertBuff);
	static void CreateUnitSphere(i32 rec, /*SRenderLight* pLight, i32 depth, */ t_arrDeferredMeshIndBuff& indBuff, t_arrDeferredMeshVertBuff& vertBuff);

	static void CreateSimpleLightFrustumMesh(t_arrDeferredMeshIndBuff& indBuff, t_arrDeferredMeshVertBuff& vertBuff);
	static void CreateSimpleLightFrustumMeshTransformed(ShadowMapFrustum* pFrustum, i32 nFrustNum, t_arrDeferredMeshIndBuff& indBuff, t_arrDeferredMeshVertBuff& vertBuff);
	static void CreateUnitBox(t_arrDeferredMeshIndBuff& indBuff, t_arrDeferredMeshVertBuff& vertBuff);

	static void CreateQuad(t_arrDeferredMeshIndBuff& indBuff, t_arrDeferredMeshVertBuff& vertBuff);

	CDeferredRenderUtils();
	~CDeferredRenderUtils();
private:
	static void SphereTess(Vec3& v0, Vec3& v1, Vec3& v2, t_arrDeferredMeshIndBuff& indBuff, t_arrDeferredMeshVertBuff& vertBuff);
	static void SphereTessR(Vec3& v0, Vec3& v1, Vec3& v2, i32 depth, t_arrDeferredMeshIndBuff& indBuff, t_arrDeferredMeshVertBuff& vertBuff);

};

#endif
