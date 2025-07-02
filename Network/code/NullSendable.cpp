// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  implements communications between two machines, with
               context setup
   -------------------------------------------------------------------------
   История:
   - 26/07/2004   : Created by Craig Tiller
*************************************************************************/

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/NullSendable.h>

#if USE_SYSTEM_ALLOCATOR
void CNullSendable::DeleteThis()
{
	ASSERT_GLOBAL_LOCK;
	delete this;
}

CNullSendable* CNullSendable::Alloc(u32 flags, ENetReliabilityType reliability)
{
	ASSERT_GLOBAL_LOCK;
	return new CNullSendable(flags | eMPF_DontAwake, reliability);
}
#else
typedef stl::PoolAllocator<sizeof(CNullSendable), stl::PoolAllocatorSynchronizationSinglethreaded> TNullAllocatorPool;
static TNullAllocatorPool* pNullAllocator;

void CNullSendable::DeleteThis()
{
	ASSERT_GLOBAL_LOCK;
	//	TMemHdl hdl = m_memhdl;
	this->~CNullSendable();
	MMM().FreeHdl(m_memhdl);
}

CNullSendable* CNullSendable::Alloc(u32 flags, ENetReliabilityType reliability)
{
	ASSERT_GLOBAL_LOCK;
	if (!pNullAllocator)
		pNullAllocator = new TNullAllocatorPool;
	TMemHdl hdl = MMM().AllocHdl(sizeof(CNullSendable));
	CNullSendable* p = new(MMM().PinHdl(hdl))CNullSendable(flags | eMPF_DontAwake, reliability);
	p->m_memhdl = hdl;
	return p;
}
#endif
