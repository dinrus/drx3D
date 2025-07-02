// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Renderer/IFlares.h>

class COpticsReference : public IOpticsElementBase
{
public:

#if defined(FLARES_SUPPORT_EDITING)
	DynArray<FuncVariableGroup> GetEditorParamGroups();
#endif

	COpticsReference(tukk name);
	~COpticsReference(){}

	EFlareType          GetType() override                    { return eFT_Reference; }
	bool                IsGroup() const override              { return false; }

	tukk         GetName() const override              { return m_name.c_str();  }
	void                SetName(tukk ch_name) override { m_name = ch_name; }
	void                Load(IXmlNode* pNode) override;

	IOpticsElementBase* GetParent() const override            { return NULL;  }

	bool                IsEnabled() const override            { return true;  }
	void                SetEnabled(bool b) override           {}
	void                AddElement(IOpticsElementBase* pElement) override;
	void                InsertElement(i32 nPos, IOpticsElementBase* pElement) override;
	void                Remove(i32 i) override;
	void                RemoveAll() override;
	i32                 GetElementCount() const override;
	IOpticsElementBase* GetElementAt(i32 i) const override;

	void                GetMemoryUsage(IDrxSizer* pSizer) const override;
	void                Invalidate() override;

	void                RenderPreview(const SLensFlareRenderParam* pParam, const Vec3& vPos) override;
	void                DeleteThis() override;

public:
	string                             m_name;
	std::vector<IOpticsElementBasePtr> m_OpticsList;
};
