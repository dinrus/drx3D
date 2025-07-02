// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <drx3D/Render/D3D/Vulkan/VKBase.hpp>
#include <bitset>

namespace NDrxVulkan
{
	class COcclusionQueryUpr
	{
		static i32k kMaxQueryCount = 64;

	public:
		typedef i32  QueryHandle;
		static const QueryHandle InvalidQuery = -1;

	public:
		COcclusionQueryUpr();
		~COcclusionQueryUpr();

		bool Init(VkDevice device);

		QueryHandle CreateQuery();
		void        ReleaseQuery(QueryHandle query);

		void BeginQuery(QueryHandle query, VkCommandBuffer commandBuffer);
		void EndQuery(QueryHandle query, VkCommandBuffer commandBuffer);
		bool GetQueryResults(QueryHandle query, uint64& samplesPassed) const;

	private:
		std::bitset<kMaxQueryCount>        m_bQueryInUse;
		std::array<uint64, kMaxQueryCount> m_fences;
		VkQueryPool                        m_queryPool;
	};

	// A simple inline wrapper class for a single occlusion query.
	// Has to be ref-counted since this is expected by high-level.
	class COcclusionQuery : public CRefCounted
	{
	public:
		COcclusionQuery(COcclusionQueryUpr& manager)
			: m_pUpr(&manager)
			, m_handle(manager.CreateQuery())
		{}

		~COcclusionQuery()
		{
			if (IsValid())
			{
				m_pUpr->ReleaseQuery(m_handle);
			}
		}

		bool IsValid() const
		{
			return m_handle != COcclusionQueryUpr::InvalidQuery;
		}

		void Begin(VkCommandBuffer commandBuffer)
		{
			m_pUpr->BeginQuery(m_handle, commandBuffer);
		}

		void End(VkCommandBuffer commandBuffer)
		{
			m_pUpr->EndQuery(m_handle, commandBuffer);
		}

		bool GetResults(uint64& samplesPassed)
		{
			return m_pUpr->GetQueryResults(m_handle, samplesPassed);
		}

	private:
		COcclusionQueryUpr* const             m_pUpr;
		const COcclusionQueryUpr::QueryHandle m_handle;
	};
}
