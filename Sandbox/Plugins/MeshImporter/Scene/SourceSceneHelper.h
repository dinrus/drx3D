// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <memory>

namespace FbxTool
{
struct SNode;
class CScene;
};

class CScene;
class CSceneElementSourceNode;

class CSceneModelCommon;
class CSceneViewCommon;

CSceneElementSourceNode* CreateSceneFromSourceScene(CScene& scene, FbxTool::CScene& sourceScene);

CSceneElementSourceNode* FindSceneElementOfNode(CScene& scene, const FbxTool::SNode* pFbxNode);

void SelectSceneElementWithNode(CSceneModelCommon* pSceneModel, CSceneViewCommon* pSceneView, const FbxTool::SNode* pNode);

