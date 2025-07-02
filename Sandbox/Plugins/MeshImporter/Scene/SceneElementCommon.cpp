// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "SceneElementCommon.h"
#include "Scene.h"

CSceneElementCommon::CSceneElementCommon(CScene* pScene, i32 id)
	: m_name()
	, m_pParent(nullptr)
	, m_siblingIndex(0)
	, m_pScene(pScene)
	, m_id(id)
{
	DRX_ASSERT(pScene);
}

i32 CSceneElementCommon::GetId() const
{
	return m_id;
}

CSceneElementCommon* CSceneElementCommon::GetParent() const
{
	return m_pParent;
}

CSceneElementCommon* CSceneElementCommon::GetChild(i32 index) const
{
	return index >= 0 && index < m_children.size() ? m_children[index] : nullptr;
}

i32 CSceneElementCommon::GetNumChildren() const
{
	return (i32)m_children.size();
}

bool CSceneElementCommon::IsLeaf() const
{
	return m_children.empty();
}

string CSceneElementCommon::GetName() const
{
	return m_name;
}

i32 CSceneElementCommon::GetSiblingIndex() const
{
	return m_siblingIndex;
}

CScene* CSceneElementCommon::GetScene()
{
	return m_pScene;
}

void CSceneElementCommon::SetSiblingIndex(i32 index)
{
	m_siblingIndex = index;
}

void CSceneElementCommon::SetName(tukk szName)
{
	assert(szName && *szName);
	m_name = szName;
}

void CSceneElementCommon::SetName(const string& name)
{
	m_name = name;
}

void CSceneElementCommon::AddChild(CSceneElementCommon* pChild)
{
	assert(pChild);
	assert(!pChild->m_pParent && pChild->m_siblingIndex == 0);

	pChild->m_pParent = this;
	pChild->m_siblingIndex = (i32)m_children.size();

	m_children.push_back(pChild);

	m_pScene->signalHierarchyChanged(nullptr, pChild);
}

CSceneElementCommon* CSceneElementCommon::RemoveChild(i32 index)
{
	if (index < 0 || index >= m_children.size())
	{
		return nullptr;
	}
	CSceneElementCommon* const pOrphan = m_children[index];
	pOrphan->m_pParent = nullptr;
	pOrphan->m_siblingIndex = -1;
	m_children.erase(m_children.begin() + index);

	for (i32 i = 0; i < m_children.size(); ++i)
	{
		m_children[i]->SetSiblingIndex(i);
	}

	m_pScene->signalHierarchyChanged(this, pOrphan);

	return pOrphan;
}

void CSceneElementCommon::MakeRoot(CSceneElementCommon* pSceneElement)
{
	if (!pSceneElement->GetParent())
	{
		return;
	}
	pSceneElement->GetParent()->RemoveChild(pSceneElement->GetSiblingIndex());
}

