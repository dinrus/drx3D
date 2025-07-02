// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/RendElements/OpticsElement.h>

class COpticsGroup : public COpticsElement
{

protected:
	std::vector<IOpticsElementBasePtr> children;
	void _init();

public:

	COpticsGroup(tukk name = "[Unnamed_Group]") : COpticsElement(name) { _init(); };
	COpticsGroup(tukk name, COpticsElement* elem, ...);
	virtual ~COpticsGroup(){}

	COpticsGroup&       Add(IOpticsElementBase* pElement);
	void                Remove(i32 i) override;
	void                RemoveAll() override;
	virtual i32         GetElementCount() const override;
	IOpticsElementBase* GetElementAt(i32 i) const override;

	void                AddElement(IOpticsElementBase* pElement) override { Add(pElement);    }
	void                InsertElement(i32 nPos, IOpticsElementBase* pElement) override;
	void                SetElementAt(i32 i, IOpticsElementBase* elem);
	void                Invalidate() override;

	bool                IsGroup() const override { return true; }
	void                validateChildrenGlobalVars(const SAuxParams& aux);

	virtual void        GetMemoryUsage(IDrxSizer* pSizer) const override;
	virtual EFlareType  GetType() override { return eFT_Group; }
	virtual void        validateGlobalVars(const SAuxParams& aux) override;

	bool                PreparePrimitives(const COpticsElement::SPreparePrimitivesContext& context) override;


#if defined(FLARES_SUPPORT_EDITING)
	virtual void InitEditorParamGroups(DynArray<FuncVariableGroup>& groups);
#endif

public:
	static COpticsGroup predef_simpleCamGhost;
	static COpticsGroup predef_cheapCamGhost;
	static COpticsGroup predef_multiGlassGhost;
};
