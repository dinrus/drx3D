// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "../stdafx.h"
#include "../RenderLock.h"
#include "../DrxThreading/DrxAtomics.h"
#include "../DrxCore/Assert/DrxAssert.h"

i32 CScopedRenderLock::s_renderLock = 0;

CScopedRenderLock::CScopedRenderLock()
{
	i32 res = DrxInterlockedIncrement(alias_cast<volatile i32*>(&s_renderLock));
	assert(res > 0);
	m_bOwnsLock = (res == 1);
}

//////////////////////////////////////////////////////////////////////////
CScopedRenderLock::~CScopedRenderLock()
{
	i32 res = DrxInterlockedDecrement(alias_cast<volatile i32*>(&s_renderLock));
	assert(res >= 0);
}

void CScopedRenderLock::Lock()
{
	i32 res = DrxInterlockedIncrement(alias_cast<volatile i32*>(&s_renderLock));
	assert(res > 0);
}

void CScopedRenderLock::Unlock()
{
	i32 res = DrxInterlockedDecrement(alias_cast<volatile i32*>(&s_renderLock));
	assert(res >= 0);
}


CScopedRenderLock::operator bool()
{
	return m_bOwnsLock;
}
