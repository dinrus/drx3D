// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/Memory.h>

#include <drx3D/Animation/Pool.h>

namespace Memory
{

/*
   CContext
 */

CContext::CContext() :
	m_pPools(NULL)
{
}

//

void CContext::AddPool(CPool& pool)
{
	if (HasPool(pool))
		return;

	pool.m_pContext = this;
	pool.m_pNext = m_pPools;
	m_pPools = &pool;
}

void CContext::RemovePool(CPool& pool)
{
	if (m_pPools == &pool)
	{
		pool.m_pContext = NULL;
		m_pPools = m_pPools->m_pNext;
		return;
	}

	CPool* pPool = m_pPools;
	while (pPool)
	{
		if (pPool->m_pNext != &pool)
			continue;

		pool.m_pContext = NULL;
		pPool->m_pNext = pPool->m_pNext->m_pNext;
		break;
	}
}

bool CContext::HasPool(CPool& pool)
{
	return pool.m_pContext == this;
}

void CContext::Update()
{
	CPool* pPool = m_pPools;
	while (pPool)
	{
		pPool->Update();

		pPool = pPool->m_pNext;
	}
}

void CContext::GetMemoryUsage(IDrxSizer* pSizer) const
{
	CPool* pPool = m_pPools;
	while (pPool)
	{
		pSizer->AddObject(pPool);
		pPool = pPool->m_pNext;
	}

	pSizer->AddObject(this, sizeof(*this));
}

} //endns Memory

/*
   CAnimationContext
 */

void CAnimationContext::Update()
{
	m_memoryContext.Update();
}

void CAnimationContext::GetMemoryUsage(IDrxSizer* pSizer) const
{
	m_memoryContext.GetMemoryUsage(pSizer);
}
