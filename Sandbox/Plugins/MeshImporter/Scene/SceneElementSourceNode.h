// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "SceneElementCommon.h"

namespace FbxTool
{

struct SNode;
class CScene;

} //endns FbxTool

class CSceneElementSourceNode : public CSceneElementCommon
{
public:
	CSceneElementSourceNode(CScene* pScene, i32 id);
	virtual ~CSceneElementSourceNode() {}

	const FbxTool::SNode*  GetNode() const;

	void SetSceneAndNode(FbxTool::CScene* pFbxScene, const FbxTool::SNode* pNode);

	void Serialize(Serialization::IArchive& ar);

	virtual ESceneElementType GetType() const override;
private:
	const FbxTool::SNode* m_pNode;
	FbxTool::CScene* m_pFbxScene;
};

