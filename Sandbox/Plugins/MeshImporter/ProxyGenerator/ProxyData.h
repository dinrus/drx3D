// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

#include <memory>
#include <vector>

struct SPhysProxies;

namespace FbxTool
{

class CScene;
struct SNode;

} //endns FbxTool

struct IStatObj;

// Holds proxy data and maps it to nodes of FBX scene.
class CProxyData
{
public:
	CProxyData();
	~CProxyData();

	void SetScene(const FbxTool::CScene* pFbxScene);

	SPhysProxies* NewPhysProxies();

	void AddNodePhysProxies(const FbxTool::SNode* pFbxNode, SPhysProxies* pPhysProxies);

	const SPhysProxies* GetPhysProxiesByIndex(const FbxTool::SNode* pFbxNode, i32 id) const;
	i32 GetPhysProxiesCount(const FbxTool::SNode* pFbxNode) const;

	void RemovePhysProxies(SPhysProxies* pPhysProxies);
	void RemovePhysGeometry(phys_geometry* pPhysGeom);

	uint64 HasPhysProxies(const FbxTool::SNode* pFbxNode) const;

	CDrxSignal<void(SPhysProxies*)> signalPhysProxiesCreated;
	CDrxSignal<void(SPhysProxies*, phys_geometry*)> signalPhysGeometryCreated;
	CDrxSignal<void(phys_geometry* pOld, phys_geometry* pNew)> signalPhysGeometryAboutToBeReused;
	CDrxSignal<void(SPhysProxies*)> signalPhysProxiesAboutToBeRemoved;
	CDrxSignal<void(phys_geometry*)> signalPhysGeometryAboutToBeRemoved;
private:
	const FbxTool::CScene* m_pFbxScene;

	std::vector<std::unique_ptr<SPhysProxies>> m_physProxies;
	std::vector<std::vector<SPhysProxies*>> m_physProxiesNodeMap;  // Maps physics proxies to nodes. Indexed by node id.
};

