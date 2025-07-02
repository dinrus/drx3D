// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Network/StdAfx.h>
#include  <drx3D/Network/WorkQueue.h>

void CWorkQueue::FlushEmpty()
{
	DoNothingOp nothing;
	m_jobQueue.Flush(nothing);
	Empty();
}
