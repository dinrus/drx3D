// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/VKOcclusionQueryUpr.hpp>

namespace NDrxVulkan
{

COcclusionQueryUpr::COcclusionQueryUpr()
	: m_queryPool(VK_NULL_HANDLE)
	, m_bQueryInUse(0)
{
	m_fences.fill(0);
}

COcclusionQueryUpr::~COcclusionQueryUpr()
{
	vkDestroyQueryPool(GetDeviceObjectFactory().GetVKDevice()->GetVkDevice(), m_queryPool, nullptr);

	for (i32 i = 0; i < kMaxQueryCount; ++i)
		GetDeviceObjectFactory().ReleaseFence(m_fences[i]);
}

bool COcclusionQueryUpr::Init(VkDevice device)
{
	VkQueryPoolCreateInfo poolInfo;
	poolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.flags = 0;
	poolInfo.queryType = VK_QUERY_TYPE_OCCLUSION;
	poolInfo.queryCount = kMaxQueryCount;
	poolInfo.pipelineStatistics = 0;

	if (vkCreateQueryPool(device, &poolInfo, nullptr, &m_queryPool) == VK_SUCCESS)
		return true;

	return false;
}

COcclusionQueryUpr::QueryHandle COcclusionQueryUpr::CreateQuery()
{
	for (i32 query = 0; query < kMaxQueryCount; ++query)
	{
		if (!m_bQueryInUse[query])
			return query;
	}

	return InvalidQuery;
}

void COcclusionQueryUpr::ReleaseQuery(COcclusionQueryUpr::QueryHandle query)
{
	DRX_ASSERT(query >= 0 && query < kMaxQueryCount);
	m_bQueryInUse[query] = false;
}

void COcclusionQueryUpr::BeginQuery(COcclusionQueryUpr::QueryHandle query, VkCommandBuffer commandBuffer)
{
	DRX_ASSERT(query >= 0 && query < kMaxQueryCount);

	vkCmdResetQueryPool(commandBuffer, m_queryPool, query, 1);
	vkCmdBeginQuery(commandBuffer, m_queryPool, query, VK_QUERY_CONTROL_PRECISE_BIT);
}

void COcclusionQueryUpr::EndQuery(COcclusionQueryUpr::QueryHandle query, VkCommandBuffer commandBuffer)
{
	DRX_ASSERT(query >= 0 && query < kMaxQueryCount);

	vkCmdEndQuery(commandBuffer, m_queryPool, query);
	m_fences[query] = GetDeviceObjectFactory().GetVKScheduler()->InsertFence();
}

bool COcclusionQueryUpr::GetQueryResults(COcclusionQueryUpr::QueryHandle query, uint64& samplesPassed) const
{
	if (GetDeviceObjectFactory().GetVKScheduler()->TestForFence(m_fences[query]) != S_OK)
	{
		GetDeviceObjectFactory().GetVKScheduler()->FlushToFence(m_fences[query]);
		return false;
	}

	VkResult querySuccess = vkGetQueryPoolResults(GetDeviceObjectFactory().GetVKDevice()->GetVkDevice(), m_queryPool, query, 1, sizeof(uint64), &samplesPassed, sizeof(uint64), VK_QUERY_RESULT_64_BIT);
	return querySuccess == VK_SUCCESS;
}

}
