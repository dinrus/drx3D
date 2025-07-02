// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Sandbox/DrxSignal.h>

#include <vector>
#include <memory>

class CSceneElementCommon;

class CScene
{
public:
	CScene();
	~CScene();

	template<typename T>
	T* NewElement()
	{
		i32 id = 0;
		if (!m_freeIDs.empty())
		{
			id = m_freeIDs.back();
			m_freeIDs.pop_back();
		}
		else
		{
			id = m_elements.size();
		}

		m_elements.emplace_back(new T(this, id));
		return (T*)m_elements.back().get();
	}

	void DeleteSingleElement(CSceneElementCommon* pElement);  // Deletes single element, excluding its children.
	void DeleteSubtree(CSceneElementCommon* pElement);  // Deletes element with its entire subtree.

	i32 GetElementCount() const;
	CSceneElementCommon* GetElementByIndex(i32 index);

	void Clear();

	CDrxSignal<void(CSceneElementCommon* pOldParent, CSceneElementCommon* pChild)> signalHierarchyChanged;
	CDrxSignal<void(CSceneElementCommon*)> signalPropertiesChanged;
private:
	void DeleteSingleElementUnchecked(CSceneElementCommon* pElement);
	void DeleteSubtreeUnchecked(CSceneElementCommon* pElement);

	std::vector<std::unique_ptr<CSceneElementCommon>> m_elements;
	std::vector<i32> m_freeIDs;
};
