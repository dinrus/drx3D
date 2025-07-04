// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CProxyGenerator;
class CScene;
class CSceneModelCommon;
class CSceneElementCommon;
class CSceneElementPhysProxies;
class CSceneElementProxyGeom;

struct SPhysProxies;
struct phys_geometry;

class QMenu;
class QModelIndex;

void AddProxyGeometries(CSceneElementPhysProxies* pPhysProxiesElement, CProxyGenerator* pProxyGenerator);

void AddProxyGenerationContextMenu(QMenu* pMenu, CSceneModelCommon* pSceneModel, const QModelIndex& index, CProxyGenerator* pProxyGenerator);

CSceneElementPhysProxies* GetSelectedPhysProxiesElement(CSceneElementCommon* pSelectedElement, bool includeGeoms = false);

CSceneElementPhysProxies* FindSceneElementOfPhysProxies(CScene& scene, const SPhysProxies* pPhysProxies);
CSceneElementProxyGeom* FindSceneElementOfProxyGeom(CScene& scene, const phys_geometry* pProxyGeom);


