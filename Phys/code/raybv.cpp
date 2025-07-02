// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Phys/StdAfx.h>

#include <drx3D/Phys/utils.h>
#include <drx3D/Phys/primitives.h>
#include <drx3D/Phys/bvtree.h>
#include <drx3D/Phys/geometry.h>
#include <drx3D/Phys/raybv.h>

i32 CRayBV::GetNodeContents(i32 iNode, BV *pBVCollider,i32 bColliderUsed,i32 bColliderLocal, 
														geometry_under_test *pGTest,geometry_under_test *pGTestOp)
{
	return m_pGeom->GetPrimitiveList(0,1, pBVCollider->type,*pBVCollider,bColliderLocal, pGTest,pGTestOp, pGTest->primbuf,pGTest->idbuf);
	//return 1;	// ray should already be in the buffer
}

void CRayBV::GetNodeBV(BV *&pBV, i32 iNode, i32 iCaller)
{
	pBV = &g_BVray;
	g_BVray.iNode = 0;
	g_BVray.type = ray::type;
	g_BVray.aray.origin = m_pray->origin;
	g_BVray.aray.dir = m_pray->dir;
}

void CRayBV::GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, i32 iNode, i32 iCaller)
{
	pBV = &g_BVray;
	g_BVray.iNode = 0;
	g_BVray.type = ray::type;
	g_BVray.aray.origin = Rw*m_pray->origin*scalew + offsw;
	g_BVray.aray.dir = Rw*m_pray->dir*scalew;
}