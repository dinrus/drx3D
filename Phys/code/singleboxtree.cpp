// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Phys/StdAfx.h>

#include <drx3D/Phys/utils.h>
#include <drx3D/Phys/primitives.h>
#include <drx3D/Phys/bvtree.h>
#include <drx3D/Phys/geometry.h>
#include <drx3D/Phys/singleboxtree.h>

float CSingleBoxTree::Build(CGeometry *pGeom) 
{ 
	m_pGeom = pGeom; 
	return m_Box.size.GetVolume()*8; 
}

void CSingleBoxTree::SetBox(box *pbox)
{
	m_Box.Basis = pbox->Basis;
	m_Box.bOriented = m_Box.Basis.IsIdentity()^1;
	m_Box.center = pbox->center;
	m_Box.size = pbox->size;
}

void CSingleBoxTree::GetNodeBV(BV *&pBV,i32 iNode, i32 iCaller)
{
	pBV = g_BBoxBuf + g_BBoxBufPos++;
	pBV->type = box::type;
	pBV->iNode = 0;
	((BBox*)pBV)->abox.Basis = m_Box.Basis;
	((BBox*)pBV)->abox.bOriented = m_Box.bOriented;
	((BBox*)pBV)->abox.center = m_Box.center;
	((BBox*)pBV)->abox.size = m_Box.size;
}

void CSingleBoxTree::GetNodeBV(BV *&pBV, const Vec3 &sweepdir,float sweepstep, i32 iNode, i32 iCaller)
{
	pBV = g_BBoxBuf + g_BBoxBufPos++;
	pBV->type = box::type;
	pBV->iNode = 0;
	box boxstatic;
	boxstatic.Basis = m_Box.Basis;
	boxstatic.bOriented = m_Box.bOriented;
	boxstatic.center = m_Box.center;
	boxstatic.size = m_Box.size;
	ExtrudeBox(&boxstatic, sweepdir,sweepstep, &((BBox*)pBV)->abox);
}

void CSingleBoxTree::GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, i32 iNode, i32 iCaller)
{
	pBV = g_BBoxBuf + g_BBoxBufPos++;
	pBV->type = box::type;
	pBV->iNode = 0;
	((BBox*)pBV)->abox.Basis = m_Box.Basis*Rw.T();
	((BBox*)pBV)->abox.bOriented = 1;
	((BBox*)pBV)->abox.center = Rw*m_Box.center*scalew + offsw;
	((BBox*)pBV)->abox.size = m_Box.size*scalew;
}

void CSingleBoxTree::GetNodeBV(const Matrix33 &Rw,const Vec3 &offsw,float scalew, BV *&pBV, 
															 const Vec3 &sweepdir,float sweepstep, i32 iNode, i32 iCaller)
{
	pBV = g_BBoxBuf + g_BBoxBufPos++;
	pBV->type = box::type;
	pBV->iNode = 0;
	box boxstatic;
	boxstatic.Basis = m_Box.Basis*Rw.T();
	boxstatic.bOriented = 1;
	boxstatic.center = Rw*m_Box.center*scalew + offsw;
	boxstatic.size = m_Box.size*scalew;
	ExtrudeBox(&boxstatic, sweepdir,sweepstep, &((BBox*)pBV)->abox);
}

i32 CSingleBoxTree::GetNodeContents(i32 iNode, BV *pBVCollider,i32 bColliderUsed,i32 bColliderLocal, 
																		geometry_under_test *pGTest,geometry_under_test *pGTestOp)
{
  return pGTest->pGeometry->GetPrimitiveList(0,m_nPrims, pBVCollider->type,*pBVCollider,bColliderLocal, pGTest,pGTestOp, pGTest->primbuf,pGTest->idbuf);
}

i32 CSingleBoxTree::PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl)
{
	pGTest->pUsedNodesMap = &pGTest->nUsedNodes;
	pGTest->nUsedNodes = 0;
	return 1;
}

void CSingleBoxTree::MarkUsedTriangle(i32 itri, geometry_under_test *pGTest)
{
	i32 bIsConvex = m_bIsConvex | isneg(m_nPrims-2);
	pGTest->nUsedNodes |= bIsConvex;
	pGTest->bCurNodeUsed |= bIsConvex;
}

void CSingleBoxTree::GetBBox(box *pbox)
{
	pbox->Basis = m_Box.Basis;
	pbox->bOriented = m_Box.bOriented;
	pbox->center = m_Box.center;
	pbox->size = m_Box.size;
}


void CSingleBoxTree::GetMemoryStatistics(IDrxSizer *pSizer)
{
	SIZER_COMPONENT_NAME(pSizer, "SingleBox trees");
	pSizer->AddObject(this, sizeof(CSingleBoxTree));
}


void CSingleBoxTree::Save(CMemStream &stm)
{
	stm.Write(m_Box);
	stm.Write(m_nPrims);
}

void CSingleBoxTree::Load(CMemStream &stm, CGeometry *pGeom)
{
	m_pGeom = pGeom;
	stm.Read(m_Box);
	stm.Read(m_nPrims);
}
