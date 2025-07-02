// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DrxNameR.h>
#include <drx3D/Render/D3D/DriverD3D.h>
#include <drx3D/Render/Textures/Texture.h>
#include <drx3D/Render/OpticsGroup.h>

#if defined(FLARES_SUPPORT_EDITING)
	#define MFPtr(FUNC_NAME) (Optics_MFPtr)(&COpticsGroup::FUNC_NAME)
void COpticsGroup::InitEditorParamGroups(DynArray<FuncVariableGroup>& groups)
{
	COpticsElement::InitEditorParamGroups(groups);
}
	#undef MFPtr
#endif

void COpticsGroup::_init()
{
	SetSize(1.f);
	SetAutoRotation(true);
}

COpticsGroup::COpticsGroup(tukk name, COpticsElement* ghost, ...) : COpticsElement(name)
{
	_init();

	va_list arg;
	va_start(arg, ghost);

	COpticsElement* curArg;
	while ((curArg = va_arg(arg, COpticsElement*)) != NULL)
		Add(curArg);

	va_end(arg);
}

COpticsGroup& COpticsGroup::Add(IOpticsElementBase* pElement)
{
	children.push_back(pElement);
	((COpticsElement*)&*pElement)->SetParent(this);
	return *this;
}

void COpticsGroup::InsertElement(i32 nPos, IOpticsElementBase* pElement)
{
	children.insert(children.begin() + nPos, pElement);
	((COpticsElement*)&*pElement)->SetParent(this);
}

void COpticsGroup::Remove(i32 i)
{
	children.erase(children.begin() + i);
}

void COpticsGroup::RemoveAll()
{
	children.clear();
}

i32                 COpticsGroup::GetElementCount() const   { return children.size(); }

IOpticsElementBase* COpticsGroup::GetElementAt(i32 i) const { return children.at(i);  }

void                COpticsGroup::SetElementAt(i32 i, IOpticsElementBase* elem)
{
	if (i < 0 || i > GetElementCount())
		return;
	children[i] = elem;
	((COpticsElement*)&*children[i])->SetParent(this);
}

void COpticsGroup::validateGlobalVars(const SAuxParams& aux)
{
	COpticsElement::validateGlobalVars(aux);
	validateChildrenGlobalVars(aux);
}
void COpticsGroup::validateChildrenGlobalVars(const SAuxParams& aux)
{
	for (uint i = 0; i < children.size(); i++)
		((COpticsElement*)GetElementAt(i))->validateGlobalVars(aux);
}

bool COpticsGroup::PreparePrimitives(const COpticsElement::SPreparePrimitivesContext& context)
{
	bool bResult = true;

	for (uint i = 0; i < children.size(); i++)
	{
		if (GetElementAt(i)->IsEnabled())
			bResult &= reinterpret_cast<COpticsElement*>(GetElementAt(i))->PreparePrimitives(context);
	}

	return bResult;
}

void COpticsGroup::GetMemoryUsage(IDrxSizer* pSizer) const
{
	for (i32 i = 0, iChildSize(children.size()); i < iChildSize; ++i)
		children[i]->GetMemoryUsage(pSizer);
	pSizer->AddObject(this, sizeof(*this));
}

void COpticsGroup::Invalidate()
{
	for (i32 i = 0, iChildSize(children.size()); i < iChildSize; ++i)
		children[i]->Invalidate();
}
