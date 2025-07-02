// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "StdAfx.h"
#include "Scene.h"
#include "SceneElementCommon.h"

CScene::CScene() {}

CScene::~CScene() {}

i32 CScene::GetElementCount() const
{
	return m_elements.size();
}

CSceneElementCommon* CScene::GetElementByIndex(i32 index)
{
	return m_elements[index].get();
}

void CScene::DeleteSingleElementUnchecked(CSceneElementCommon* pElement)
{
	const auto it = std::find_if(m_elements.begin(), m_elements.end(), [pElement](std::unique_ptr<CSceneElementCommon>& other)
	{
		return other.get() == pElement;
	});
	if (m_elements.end() != it)
	{
		m_freeIDs.push_back(pElement->GetId());
		m_elements.erase(it);
	}
}

void CScene::DeleteSingleElement(CSceneElementCommon* pElement)
{
	DRX_ASSERT(!pElement->GetParent() && !pElement->GetNumChildren());
	DeleteSingleElementUnchecked(pElement);
}

void CScene::DeleteSubtreeUnchecked(CSceneElementCommon* pElement)
{
	for (i32 i = 0; i < pElement->GetNumChildren(); ++i)
	{
		DeleteSubtreeUnchecked(pElement->GetChild(i));
	}
	DeleteSingleElementUnchecked(pElement);
}

void CScene::DeleteSubtree(CSceneElementCommon* pElement)
{
	DRX_ASSERT(!pElement->GetParent());
	DeleteSubtreeUnchecked(pElement);
}

void CScene::Clear()
{
	m_elements.clear();
	m_freeIDs.clear();
}

