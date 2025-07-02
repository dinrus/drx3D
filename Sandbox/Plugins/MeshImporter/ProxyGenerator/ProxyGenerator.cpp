// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "PhysicsProxies.h"
#include "ProxyData.h"
#include "ProxyGenerator.h"

#include <QViewport.h>

#include <Drx3DEngine/I3DEngine.h>
#include <Drx3DEngine/IIndexedMesh.h>

#include <DrxPhysics/IPhysicsDebugRenderer.h>

namespace Private_ProxyGenerator
{

template<class T> Matrix34 locTransform(const T* obj)
{
	Matrix34 res;
	res.Set(Vec3(obj->scale[0], obj->scale[1], obj->scale[2]), Quat(obj->rotation[3], obj->rotation[0], obj->rotation[1], obj->rotation[2]), Vec3(obj->translation[0], obj->translation[1], obj->translation[2]));
	return res;
}

} //endns Private_ProxyGenerator

CProxyGenerator::CProxyGenerator(CProxyData* pProxyData)
	: m_pProxyData(pProxyData)
	, m_hitShift(0)
	, m_hitmask(0)
	, m_proxyIsland(-1)
{
	DRX_ASSERT(pProxyData);
}

CProxyData* CProxyGenerator::GetProxyData()
{
	return m_pProxyData;
}

void CProxyGenerator::CloseHoles(SPhysProxies* pProx)
{
	DRX_ASSERT(pProx);

	mesh_data* md = (mesh_data*)pProx->pSrc->pMesh->GetData();
	uint64 islandMap = pProx->params.islandMap & (1ull << md->nIslands) - 1 & (~pProx->pSrc->closedIsles | pProx->pSrc->restoredIsles);
	for (i32 i = 0, j; i < md->nIslands; i++)
		if (islandMap & 1ull << i & pProx->pSrc->restoredIsles)
		{
			for (j = md->pIslands[i].itri; md->pTri2Island[j].inext != 0x7fff; j = md->pTri2Island[j].inext)
				;
			md->pTri2Island[j].inext = pProx->pSrc->isleTriClosed[i];
			for (j = pProx->pSrc->isleTriClosed[i]; j != 0x7fff; md->pMats[j] = 0, md->pIslands[i].nTris++, j = md->pTri2Island[j].inext)
				;
			pProx->pSrc->restoredIsles &= ~(1ull << i);
			islandMap &= ~(1ull << i);
		}
	if (islandMap)
	{
		IGeometry::SProxifyParams prp;
		prp.findMeshes = 0;
		prp.findPrimLines = 0;
		prp.findPrimSurfaces = 0;
		prp.closeHoles = 1;
		prp.islandMap = islandMap;
		IGeometry** pGeoms;
		pProx->pSrc->pMesh->Proxify(pGeoms, &prp);
		for (i32 i = 0, j, n; i < md->nIslands; i++)
			if (pProx->params.islandMap & 1ull << i)
			{
				for (j = md->pIslands[i].itri, n = 0; j != 0x7fff && n < pProx->pSrc->isleTriCount[i]; j = md->pTri2Island[j].inext, n++)
					;
				pProx->pSrc->isleTriClosed[i] = j;
				pProx->pSrc->closedIsles |= 1ull << i;
			}
	}
	signalProxyIslandsChanged(pProx);
}

void CProxyGenerator::ReopenHoles(SPhysProxies* pProx)
{
	DRX_ASSERT(pProx);

	uint64 mask = pProx->pSrc->closedIsles & pProx->params.islandMap & ~pProx->pSrc->restoredIsles;
	mesh_data* md = (mesh_data*)pProx->pSrc->pMesh->GetData();
	for (i32 i = 0, j, n; mask; mask &= ~(1ull << i++))
		if (mask & 1ull << i)
		{
			for (j = md->pIslands[i].itri, n = 1; md->pTri2Island[j].inext != 0x7fff && n < pProx->pSrc->isleTriCount[i]; j = md->pTri2Island[j].inext, n++)
				;
			for (i32 j0 = md->pTri2Island[j].inext; j0 != 0x7fff; md->pMats[j0] = -1, j0 = md->pTri2Island[j0].inext)
				;
			md->pTri2Island[j].inext = 0x7fff;
			md->pIslands[i].nTris = n;
			pProx->pSrc->restoredIsles |= 1ull << i;
		}
	signalProxyIslandsChanged(pProx);
}

void CProxyGenerator::SelectAll(SPhysProxies* pProx)
{
	DRX_ASSERT(pProx);

	pProx->params.islandMap = -1ll;
	signalProxyIslandsChanged(pProx);
}

void CProxyGenerator::SelectNone(SPhysProxies* pProx)
{
	DRX_ASSERT(pProx);

	pProx->params.islandMap = 0;
	signalProxyIslandsChanged(pProx);
}

static i32 AssembleMeshes(const FbxTool::SNode* pNode, std::vector<Vec3>& pos, std::vector<Vec3_tpl<index_t>>& faces, const Matrix34& mtxParent)
{
	using namespace Private_ProxyGenerator;

	Matrix34 mtxNode = mtxParent* locTransform(pNode);
	i32 n = 0;
	for (i32 i = 0; i < pNode->numMeshes; i++)
		for (i32 j = 0; j < pNode->ppMeshes[i]->displayMeshes.size(); j++, n++)
		{
			const CMesh* pMesh = pNode->ppMeshes[i]->displayMeshes[j].get()->pEngineMesh.get();
			Matrix34 mtxMesh = mtxNode * locTransform(pNode->ppMeshes[i]);
			i32 vtx0 = pos.size(), tri0 = faces.size(), nVtx = pMesh->GetVertexCount(), nTris = pMesh->GetFaceCount();
			pos.resize(pos.size() + nVtx);
			faces.resize(faces.size() + nTris);
			for (i32 idx = 0; idx < nVtx; pos[vtx0 + idx] = mtxMesh * pMesh->m_pPositions[idx], idx++)
				;
			for (i32 idx = 0; idx < nTris; faces[tri0 + idx] = Vec3i(pMesh->m_pFaces[idx].v[0], pMesh->m_pFaces[idx].v[1], pMesh->m_pFaces[idx].v[2]) + Vec3i(vtx0), idx++)
				;
		}
	for (i32 i = 0; i < pNode->numChildren; n += AssembleMeshes(pNode->ppChildren[i++], pos, faces, mtxNode))
		;
	return n;
}

SPhysProxies* CProxyGenerator::AddPhysProxies(const FbxTool::SNode* pFbxNode, const Matrix34& mtxGlobal)
{
	SPhysProxies* pProx = m_pProxyData->NewPhysProxies();
	pProx->params.qForced.SetIdentity();
	pProx->ready = false;
	i32k physProxyCount = m_pProxyData->GetPhysProxiesCount(pFbxNode);
	if (physProxyCount > 0)
	{
		const SPhysProxies* const pOther = m_pProxyData->GetPhysProxiesByIndex(pFbxNode, physProxyCount - 1);
		pProx->pSrc = pOther->pSrc;
		pProx->nMeshes = pOther->nMeshes;
	}
	else
	{
		pProx->pSrc = std::make_shared<SPhysSrcMesh>();
		std::vector<Vec3> pos;
		auto& faces = pProx->pSrc->faces;
		i32 nMeshes = AssembleMeshes(pFbxNode, pos, faces, mtxGlobal);
		char name[64];
		sprintf(name, "Auto Proxies (%d source mesh%s)", nMeshes, nMeshes == 1 ? "" : "es");
		pProx->nMeshes = nMeshes;
		std::vector<char> mats;
		mats.resize(faces.size(), 0);
		pProx->pSrc->pMesh = std::unique_ptr<IGeometry>(gEnv->pPhysicalWorld->GetGeomManager()->CreateMesh(pos.data(), &faces.data()->x, mats.data(), 0, faces.size(), mesh_OBB | mesh_shared_idx));
		const mesh_data* pmd = (const mesh_data*)pProx->pSrc->pMesh->GetData();
		pProx->pSrc->isleTriCount.reserve(pmd->nIslands);
		pProx->pSrc->isleTriClosed.resize(pmd->nIslands);
		for (i32 i = 0; i < pmd->nIslands; pProx->pSrc->isleTriCount.push_back(pmd->pIslands[i++].nTris))
			;
		pProx->pSrc->closedIsles = pProx->pSrc->restoredIsles = 0;
		if (nMeshes == 1)
			MARK_UNUSED pProx->params.qForced;
	}
	m_pProxyData->AddNodePhysProxies(pFbxNode, pProx);
	return pProx;
}

phys_geometry* CProxyGenerator::AddProxyGeom(SPhysProxies* pPhysProxies, IGeometry* pProxyGeom, bool replaceIfPresent)
{
	i32 sIdx = -1;
	const mesh_data* mdOld, * mdNew = (const mesh_data*)pProxyGeom->GetData();
	IGeomManager* pGeoman = gEnv->pPhysicalWorld->GetGeomManager();
	phys_geometry* pPhysGeom = pGeoman->RegisterGeometry(pProxyGeom, 0);
	pPhysGeom->nMats = PHYS_GEOM_TYPE_DEFAULT;
	i32 type = pProxyGeom->GetType();
	if (type == GEOM_TRIMESH)
	{
		mesh_data* pmd = (mesh_data*)pProxyGeom->GetData();
		memset(pmd->pMats, 0, pmd->pMats ? pmd->nTris : 0);
	}
	if (replaceIfPresent && pProxyGeom->GetType() == GEOM_TRIMESH)
	{
		for (i32 i = 0; i < pPhysProxies->proxyGeometries.size(); i++)
		{
			IGeometry* pGeomOld = pPhysProxies->proxyGeometries[i]->pGeom;
			mdOld = (const mesh_data*)pGeomOld->GetData();
			if ((pGeomOld->GetCenter() - pProxyGeom->GetCenter()).len2() < 1e-3f && mdOld->nTris == mdNew->nTris)
			{
				m_pProxyData->signalPhysGeometryAboutToBeReused(pPhysProxies->proxyGeometries[i], pPhysGeom);
				pGeoman->UnregisterGeometry(pPhysProxies->proxyGeometries[i]);
				pPhysProxies->proxyGeometries[i] = pPhysGeom;
				return pPhysProxies->proxyGeometries[i];
			}
		}
	}
	pPhysProxies->ready = true;
	pPhysProxies->proxyGeometries.push_back(pPhysGeom);
	m_pProxyData->signalPhysGeometryCreated(pPhysProxies, pPhysGeom);
	return pPhysGeom;
}

void CProxyGenerator::GenerateProxies(SPhysProxies* pProx, std::vector<phys_geometry*>& newPhysGeoms)
{
	DRX_ASSERT(pProx);

	IGeometry** pGeoms;
	pProx->params.storeVox = 1;
	if (i32 nGeoms = pProx->pSrc->pMesh->Proxify(pGeoms, &pProx->params))
	{
		for (i32 i = 0; i < nGeoms; ++i)
		{
			const auto oldSize = pProx->proxyGeometries.size();
			phys_geometry* physGeom = AddProxyGeom(pProx, pGeoms[i]);
			if (oldSize != pProx->proxyGeometries.size())
			{
				newPhysGeoms.push_back(physGeom);
			}
		}
		gEnv->pPhysicalWorld->GetPhysUtils()->DeletePointer(pGeoms);
		m_hitShift = 0;
		signalProxyIslandsChanged(pProx);
	}
}

void CProxyGenerator::ResetProxies(SPhysProxies* pProx)
{
	DRX_ASSERT(pProx);
	
	while (!pProx->proxyGeometries.empty())
	{
		m_pProxyData->RemovePhysGeometry(pProx->proxyGeometries.back());
	}
	pProx->ready = false;
	pProx->params.skipPrimMask = 0;
	signalProxyIslandsChanged(pProx);
}

void CProxyGenerator::ResetAndRegenerate(SPhysProxies* pProx, phys_geometry* pPhysGeom, std::vector<phys_geometry*>& newPhysGeoms)
{
	DRX_ASSERT(pProx);

	pProx->params.storeVox = 1;
	pProx->params.skipPrimMask = 1ull << pPhysGeom->pGeom->GetiForeignData();

	while (!pProx->proxyGeometries.empty())
	{
		m_pProxyData->RemovePhysGeometry(pProx->proxyGeometries.back());
	}

	GenerateProxies(pProx, newPhysGeoms);
}

void CProxyGenerator::Reset()
{
	m_proxyIsland = -1;
	m_hitShift = 0;
	m_hitmask = 0;
}

IGeometry* HideMeshIslands(IGeometry* pMesh, uint64 mask)
{
	mesh_data* pmd = (mesh_data*)pMesh->GetData();
	for (i32 i = 0; i < pmd->nIslands; i++)
		if ((pmd->pMats[pmd->pIslands[i].itri] & 1) != (mask >> i & 1))
		{
			for (i32 j = pmd->pIslands[i].itri; j != 0x7fff; pmd->pMats[j] = -(char)(mask >> i & 1), j = pmd->pTri2Island[j].inext)
				;
		}
	return pMesh;
}

void CProxyGenerator::Render(SPhysProxies* pProx, phys_geometry* pProxyGeom, const SRenderContext& rc, const SShowParams& showParams)
{
	DRX_ASSERT(pProx);

	IPhysRenderer* pPhysRnd = gEnv->pSystem->GetIPhysRenderer();
	ICVar* pdist = gEnv->pConsole->GetCVar("p_wireframe_dist");
	gEnv->pSystem->GetIPhysicsDebugRenderer()->UpdateCamera(*rc.viewport->Camera());
	float dist0;
	if (pdist)
	{
		dist0 = pdist->GetFVal();
		pdist->Set(1e10f);
	}
	if (!pProx->ready || showParams.sourceVisibility != EVisibility::Hidden)
	{
		HideMeshIslands(pProx->pSrc->pMesh.get(), ~pProx->params.islandMap)->DrawWireframe(pPhysRnd, 0, 0, 2 | (pProx->ready && showParams.sourceVisibility == EVisibility::Wireframe) << 8);
	}
	if (!pProx->ready)
	{
		HideMeshIslands(pProx->pSrc->pMesh.get(), pProx->params.islandMap | 1ull << m_proxyIsland)->DrawWireframe(pPhysRnd, 0, 0, 2 | 1 << 8);
		HideMeshIslands(pProx->pSrc->pMesh.get(), ~(1ull << m_proxyIsland))->DrawWireframe(pPhysRnd, 0, 0, 5);
	}
	HideMeshIslands(pProx->pSrc->pMesh.get(), 0);
	i32 nMeshes = 0;
	for (phys_geometry* physGeom : pProx->proxyGeometries)
	{
		IGeometry* pGeom = physGeom->pGeom;
		EVisibility show = showParams.primitivesVisibility;
		if (pGeom->GetType() == GEOM_TRIMESH)
		{
			show = showParams.meshesVisibility;
			nMeshes++;
		}
		if (show != EVisibility::Hidden)
			pGeom->DrawWireframe(pPhysRnd, 0, 0, (physGeom == pProxyGeom) * 6 | (show == EVisibility::Wireframe) << 8);
	}
	if (nMeshes && pProx->ready && showParams.bShowVoxels)
	{
		pProx->pSrc->pMesh->DrawWireframe(pPhysRnd, 0, -1, 6);
	}
	pdist ? (pdist->Set(dist0), 0) : 0;
	return;
}

void MarkMeshChildren(SPhysProxies* pProx)
{
	for (auto& physGeom : pProx->proxyGeometries)
	{
		if (physGeom->pGeom->GetType() == GEOM_TRIMESH)
		{
			((mesh_data*)physGeom->pGeom->GetData())->flags |= mesh_transient;
		}
	}
}

// replace changed mesh proxies, removes ones that are no longer present after re-proxification
void RemoveMarkedChildren(CProxyData* pProxyData, SPhysProxies* pProx)
{
	auto eraseIt = std::remove_if(pProx->proxyGeometries.begin(), pProx->proxyGeometries.end(), [](phys_geometry* physGeom)
	{
		IGeometry* pGeom = physGeom->pGeom;
		return pGeom->GetType() == GEOM_TRIMESH && ((const mesh_data*)pGeom->GetData())->flags & mesh_transient;
	});

	std::vector<phys_geometry*> killList;
	for (auto it = eraseIt; it != pProx->proxyGeometries.end(); ++it)
	{
		killList.push_back(*it);
	}

	for (auto& k : killList)
	{
		pProxyData->RemovePhysGeometry(k);
	}
}

void CProxyGenerator::OnMouse(SPhysProxies* pProx, const SMouseEvent& ev, const CCamera& cam)
{
	DRX_ASSERT(pProx);

	if ((ev.type == SMouseEvent::TYPE_PRESS || ev.type == SMouseEvent::TYPE_MOVE) && pProx)
	{
		Vec2 sz(cam.GetViewSurfaceX(), cam.GetViewSurfaceZ());
		Vec3 dir = Vec3((Vec2(ev.x, ev.y) * 2 - sz) * (tan(cam.GetFov() * 0.5f) / sz.y));
		dir.z = 1;
		primitives::ray ray;
		ray.origin = cam.GetMatrix().GetTranslation();
		ray.dir = cam.GetMatrix().TransformVector(Quat(1 / sqrt2, Vec3(-1 / sqrt2, 0, 0)) * dir);
		primitives::box bbox;
		pProx->pSrc->pMesh->GetBBox(&bbox);
		Vec3 dirloc = bbox.Basis * ray.dir;
		ray.dir *= ray.dir * (bbox.center + Vec3(sgnnz(dirloc.x), sgnnz(dirloc.y), sgnnz(dirloc.z)) * bbox.Basis - ray.origin);
		static IGeometry* pRay = gEnv->pPhysicalWorld->GetGeomManager()->CreatePrimitive(primitives::ray::type, &ray);
		pRay->SetData(&ray);
		geom_contact* pcont;
		m_hitShift += ev.type == SMouseEvent::TYPE_PRESS && ev.button == 4; // use Mclick cycle hitShift
		m_proxyIsland = -1;
		if (i32 ncont = pProx->pSrc->pMesh->Intersect(pRay, 0, 0, 0, pcont))
		{
			// hittest the source mesh for proxy generation
			if (!pProx->ready)
			{
				// if proxies haven't been generated yet, toggle individual mesh islands (components) on Lclicks
				const mesh_data* pmd = (const mesh_data*)pProx->pSrc->pMesh->GetData();
				i32 i = ncont - 1, j = -1, isle = -1, isle1;
				uint64 hitmask = 0;
				for (i = 0; i < ncont; hitmask |= 1ull << pmd->pTri2Island[pcont[i++].iPrim[0]].isle)
					;
				m_hitShift &= -iszero((int64)(hitmask - m_hitmask));
				m_hitmask = hitmask;
				m_proxyIsland = pmd->pTri2Island[pcont[--i].iPrim[0]].isle; // use hitShift to cycle if hittest detects several hits
				for (; i >= 0 && j < m_hitShift; isle1 = pmd->pTri2Island[pcont[i].iPrim[0]].isle, j += isle1 != isle, isle = isle1, i--)
					;
				m_proxyIsland += isle - m_proxyIsland & ~(i >> 31);
				m_hitShift &= ~(i >> 31);
				if (ev.type == SMouseEvent::TYPE_PRESS && ev.button == 1)
				{
					// toggle a mesh island's usage for proxygen
					pProx->params.islandMap ^= 1ull << m_proxyIsland;
					signalProxyIslandsChanged(pProx);
				}
			}
			else if (ev.type == SMouseEvent::TYPE_PRESS && ev.button == 1)
			{
				// if proxies are ready, use Lclicks to toggle individual voxels to be used as vertices in proxy meshes
				IGeometry::SProxifyParams params = pProx->params;
				params.storeVox = 0;
				params.reuseVox = 1;
				params.flipCurCell = 1;
				params.findPrimLines = 0;
				params.findPrimSurfaces = 0;
				params.convexHull = 0;
				params.forceBBox = 0;
				params.islandMap = -1ll;
				IGeometry** pGeoms;
				i32 nGeoms = pProx->pSrc->pMesh->Proxify(pGeoms, &params); // Proxify will reuse the last voxel map, but toggle some voxels in it
				if (!nGeoms)
					return;

				// TODO: Marking children as transient does not work properly.
				MarkMeshChildren(pProx);
				RemoveMarkedChildren(m_pProxyData, pProx);

				for (i32 i = 0; i < nGeoms; AddProxyGeom(pProx, pGeoms[i++], true))
					;
			}
		}
	}
}
