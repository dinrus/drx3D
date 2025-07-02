// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/VertexCommandBuffer.h>

#include <drx3D/CoreX/Thread/IJobUpr_JobDelegator.h>
#include <drx3D/Animation/ModelMesh.h>
#include <drx3D/Animation/VertexData.h>
#include <drx3D/Animation/VertexCommand.h>

/*
   CVertexCommandBufferAllocationCounter
 */

CVertexCommandBufferAllocationCounter::CVertexCommandBufferAllocationCounter() :
	m_count(0)
{
}

CVertexCommandBufferAllocationCounter::~CVertexCommandBufferAllocationCounter()
{
}

// CVertexCommandBufferAllocator

uk CVertexCommandBufferAllocationCounter::Allocate(const uint length)
{
	m_count += length;
	return NULL;
}

/*
   CVertexCommandBufferAllocatorStatic
 */

CVertexCommandBufferAllocatorStatic::CVertexCommandBufferAllocatorStatic(ukk pMemory, const uint length) :
	m_pMemory((u8*)pMemory),
	m_memoryLeft(length)
{
}

CVertexCommandBufferAllocatorStatic::~CVertexCommandBufferAllocatorStatic()
{
}

// CVertexCommandBufferAllocator

uk CVertexCommandBufferAllocatorStatic::Allocate(const uint length)
{
	if (length > m_memoryLeft)
		return NULL;

	uk pAllocation = m_pMemory;
	m_pMemory += length;
	m_memoryLeft -= length;
	return pAllocation;
}

/*
   CVertexCommandBuffer
 */

void CVertexCommandBuffer::Process(CVertexData& vertexData)
{
	i32 length = m_commandsLength;
	if (!length)
		return;

	u8k* pCommands = m_pCommands;
	while (length > sizeof(VertexCommand))
	{
		VertexCommand* pCommand = (VertexCommand*)pCommands;
		pCommands += pCommand->length;
		length -= i32(pCommand->length);
		pCommand->Execute(*pCommand, vertexData);
	}
}

/*
   SVertexAnimationJobData
 */

DECLARE_JOB("VertexAnimation", TVertexAnimation, SVertexAnimationJob::Execute);

void SVertexAnimationJob::Execute(i32)
{
	if (commandBufferLength)
		commandBuffer.Process(vertexData);

	if (m_previousRenderMesh)
	{
		m_previousRenderMesh->UnlockStream(VSF_GENERAL);
		m_previousRenderMesh->UnLockForThreadAccess();
	}

	m_previousRenderMesh = NULL;

	DrxInterlockedDecrement(pRenderMeshSyncVariable);
}

void SVertexAnimationJob::Begin(JobUpr::SJobState* pJob)
{
	TVertexAnimation job(0);
	job.SetClassInstance(this);
	job.SetPriorityLevel(JobUpr::eRegularPriority);
	job.RegisterJobState(pJob);
	job.Run();
}
