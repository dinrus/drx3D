// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/String/DrxString.h>

#include <vector>

enum class ESceneElementType : i32;

class CScene;

class CSceneElementCommon
{
public:
	CSceneElementCommon(CScene* pScene, i32 id);
	virtual ~CSceneElementCommon() {}

	i32                    GetId() const;
	CSceneElementCommon*   GetParent() const;
	CSceneElementCommon*   GetChild(i32 index) const;
	i32                    GetNumChildren() const;
	bool                   IsLeaf() const;
	string                GetName() const;
	i32                    GetSiblingIndex() const;
	CScene*     GetScene();

	void                   SetName(const string& name);
	void                   SetName(tukk szName);
	void                   SetSiblingIndex(i32 index);

	void                   AddChild(CSceneElementCommon* pChild);
	CSceneElementCommon*   RemoveChild(i32 index);

	virtual ESceneElementType GetType() const = 0;

	static void MakeRoot(CSceneElementCommon* pSceneElement);  // Remove element from its parent.
private:
	string                           m_name;

	std::vector<CSceneElementCommon*> m_children;
	CSceneElementCommon*              m_pParent;
	i32                               m_siblingIndex;

	CScene*                m_pScene;
	i32                               m_id;
};

