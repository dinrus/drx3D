// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Memory/PoolAllocator.h>

// Class responsible for managing potentially visible render nodes.
class CVisibleRenderNodesUpr
{
public:
	static i32 const MAX_NODES_CHECK_PER_FRAME = 500;
	static i32 const MAX_DELETE_BUFFERS = 3;

	struct Statistics
	{
		i32 numFree;
		i32 numUsed;
	};

public:
	CVisibleRenderNodesUpr();
	~CVisibleRenderNodesUpr();

	SRenderNodeTempData* AllocateTempData(i32 lastSeenFrame);

	// Set last frame for this rendering pass.
	// Return if last frame changed and rendering should be done this frame for this pass.
	static bool SetLastSeenFrame(SRenderNodeTempData* pTempData, const SRenderingPassInfo& passInfo);

	// Iteratively update array of visible nodes checking if they are expired
	void       UpdateVisibleNodes(i32 currentFrame, i32 maxNodesToCheck = MAX_NODES_CHECK_PER_FRAME);
	void       InvalidateAll();
	void       OnRenderNodeDeleted(IRenderNode* pRenderNode);
	Statistics GetStatistics() const;

	void       ClearAll();

private:
	void OnRenderNodeVisibilityChange( IRenderNode *pRenderNode,bool bVisible );

private:
	std::vector<SRenderNodeTempData*> m_visibleNodes;
	i32                               m_lastStartUpdateNode;

	i32                               m_firstAddedNode;

	i32                               m_currentNodesToDelete;
	std::vector<SRenderNodeTempData*> m_toDeleteNodes[MAX_DELETE_BUFFERS];

	i32                               m_lastUpdateFrame;

	struct PoolSyncCriticalSection
	{
		void Lock()   { m_cs.Lock(); }
		void Unlock() { m_cs.Unlock(); }
		DrxCriticalSectionNonRecursive m_cs;
	};
	stl::TPoolAllocator<SRenderNodeTempData, PoolSyncCriticalSection> m_pool;

	DrxCriticalSectionNonRecursive m_accessLock;
};
