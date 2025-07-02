// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Animation/VertexData.h>

class CVertexCommandBufferAllocator
{
public:
	virtual ~CVertexCommandBufferAllocator() {}

public:
	virtual uk Allocate(const uint length) = 0;
};

//

class CVertexCommandBufferAllocationCounter :
	public CVertexCommandBufferAllocator
{
public:
	CVertexCommandBufferAllocationCounter();
	virtual ~CVertexCommandBufferAllocationCounter();

public:
	ILINE uint GetCount() const { return m_count; }

	// CVertexCommandBufferAllocator
public:
	virtual uk Allocate(const uint length);

private:
	uint m_count;
};

//

class CVertexCommandBufferAllocatorStatic :
	public CVertexCommandBufferAllocator
{
public:
	CVertexCommandBufferAllocatorStatic(ukk pMemory, const uint length);
	~CVertexCommandBufferAllocatorStatic();

	// CVertexCommandBufferAllocator
public:
	virtual uk Allocate(const uint length);

private:
	u8* m_pMemory;
	uint   m_memoryLeft;
};

//

class CVertexCommandBuffer
{
public:
	bool Initialize(CVertexCommandBufferAllocator& allocator)
	{
		m_pAllocator = &allocator;
		m_pCommands = NULL;
		m_commandsLength = 0;
		return true;
	}

	template<class Type>
	Type* AddCommand();

	void  Process(CVertexData& vertexData);

private:
	CVertexCommandBufferAllocator* m_pAllocator;
	u8k*                   m_pCommands;
	uint                           m_commandsLength;
};

//

template<class Type>
Type* CVertexCommandBuffer::AddCommand()
{
	uint length = sizeof(Type);
	uk pAllocation = m_pAllocator->Allocate(length);
	if (!pAllocation)
		return NULL;

	if (!m_pCommands)
		m_pCommands = (u8k*)pAllocation;

	m_commandsLength += length;

	Type* pCommand = (Type*)pAllocation;
	new(pCommand) Type();
	pCommand->length = length;
	return pCommand;
}

struct SVertexAnimationJob
{
	CVertexData             vertexData;
	CVertexCommandBuffer    commandBuffer;
	uint                    commandBufferLength;

	 i32*           pRenderMeshSyncVariable;
	_smart_ptr<IRenderMesh> m_previousRenderMesh;

public:
	SVertexAnimationJob() : pRenderMeshSyncVariable(nullptr) {}

	void Begin(JobUpr::SJobState* pJob);
	void Execute(i32);
};
