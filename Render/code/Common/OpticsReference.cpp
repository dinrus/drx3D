// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/OpticsReference.h>

#if defined(FLARES_SUPPORT_EDITING)
DynArray<FuncVariableGroup> COpticsReference::GetEditorParamGroups()
{
	return DynArray<FuncVariableGroup>();
}
#endif

COpticsReference::COpticsReference(tukk name) : m_name(name)
{
}

void COpticsReference::Load(IXmlNode* pNode)
{
}

void COpticsReference::AddElement(IOpticsElementBase* pElement)
{
	m_OpticsList.push_back(pElement);
}

void COpticsReference::InsertElement(i32 nPos, IOpticsElementBase* pElement)
{
	if (nPos < 0 || nPos >= (i32)m_OpticsList.size())
		return;
	m_OpticsList.insert(m_OpticsList.begin() + nPos, pElement);
}

void COpticsReference::Remove(i32 i)
{
	if (i < 0 || i >= (i32)m_OpticsList.size())
		return;
	m_OpticsList.erase(m_OpticsList.begin() + i);
}

void COpticsReference::RemoveAll()
{
	m_OpticsList.clear();
}

i32 COpticsReference::GetElementCount() const
{
	return m_OpticsList.size();
}

IOpticsElementBase* COpticsReference::GetElementAt(i32 i) const
{
	if (i < 0 || i >= (i32)m_OpticsList.size())
		return NULL;
	return m_OpticsList[i];
}

void COpticsReference::GetMemoryUsage(IDrxSizer* pSizer) const
{
	for (i32 i = 0, iSize(m_OpticsList.size()); i < iSize; ++i)
		m_OpticsList[i]->GetMemoryUsage(pSizer);
}

void COpticsReference::Invalidate()
{
	for (i32 i = 0, iSize(m_OpticsList.size()); i < iSize; ++i)
		m_OpticsList[i]->Invalidate();
}

void COpticsReference::RenderPreview(const SLensFlareRenderParam* pParam, const Vec3& vPos)
{
	for (i32 i = 0, iSize(m_OpticsList.size()); i < iSize; ++i)
		m_OpticsList[i]->RenderPreview(pParam, vPos);
}

void COpticsReference::DeleteThis()
{
	delete this;
}
