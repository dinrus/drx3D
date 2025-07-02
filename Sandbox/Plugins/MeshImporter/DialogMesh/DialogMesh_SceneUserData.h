// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

namespace FbxTool
{

struct SMesh;
struct SNode;
class CScene;

} //endns FbxTool

namespace DialogMesh
{

//! Data stored for a FBX scene.
class CSceneUserData
{
public:
	void Init(FbxTool::CScene* pFbxScene);

	bool SelectSkin(const FbxTool::SMesh* pMesh);
	bool SelectAnySkin(const FbxTool::SNode* pNode);
	void DeselectSkin();
	const FbxTool::SMesh* GetSelectedSkin() const;

	CDrxSignal<void()> sigSelectedSkinChanged;
private:
	const FbxTool::SMesh* m_pSelectedSkin;  //!< There can be at most one skin selected.
};

} //endns DialogMesh

