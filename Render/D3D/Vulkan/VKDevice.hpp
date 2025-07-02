// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <vector>

#include <drx3D/Render/D3D/Vulkan/VKBase.hpp>
#include <drx3D/Render/D3D/Vulkan/VKHeap.hpp>
#include <drx3D/Render/D3D/Vulkan/VKBufferResource.hpp>
#include <drx3D/Render/D3D/Vulkan/VKImageResource.hpp>
#include <drx3D/Render/D3D/Vulkan/VKSampler.hpp>
#include <drx3D/Render/D3D/Vulkan/VKResourceView.hpp>

#include <drx3D/Render/D3D/Vulkan/VKCommandList.hpp>
#include <drx3D/Render/D3D/Vulkan/VKCommandScheduler.hpp>

#include <drx3D/Render/D3D/Vulkan/VKOcclusionQueryUpr.hpp>

namespace NDrxVulkan
{
#define CMDQUEUE_TYPE_GRAPHICS 0
#define CMDQUEUE_TYPE_COMPUTE 1
#define CMDQUEUE_TYPE_TRANSFER 2
#define CMDQUEUE_TYPE_PRESENT 3
#define CMDQUEUE_TYPE_COUNT 4

class CInstance;
struct SPhysicalDeviceInfo;

class CDevice : public CRefCounted
{

protected:
	struct SQueueConfiguration
	{
		uint32_t queueFamilyIndex  [CMDQUEUE_TYPE_COUNT];
		uint32_t queueIndexInFamily[CMDQUEUE_TYPE_COUNT];
	};

public:
	static _smart_ptr<CDevice> Create(const SPhysicalDeviceInfo* pDeviceInfo, VkAllocationCallbacks* hostAllocator, const std::vector<tukk >& layersToEnable, const std::vector<tukk >& extensionsToEnable);

	CDevice(const SPhysicalDeviceInfo* pDeviceInfo, VkAllocationCallbacks* hostAllocator, VkDevice Device);
	~CDevice();

	CCommandScheduler& GetScheduler() { return m_Scheduler; }
	const CCommandScheduler& GetScheduler() const { return m_Scheduler; }

	const VkDevice& GetVkDevice() const { return m_device; }
	const VkPipelineCache& GetVkPipelineCache() const { return m_pipelineCache; }

	const SPhysicalDeviceInfo* GetPhysicalDeviceInfo() const { return m_pDeviceInfo; }

	VkDescriptorPool GetDescriptorPool() const { return m_descriptorPool; }
	CHeap& GetHeap() { return m_heap; }
	const CHeap& GetHeap() const { return m_heap; }

	COcclusionQueryUpr& GetOcclusionQueries() { return m_occlusionQueries; }
	const COcclusionQueryUpr& GetOcclusionQueries() const { return m_occlusionQueries; }

	// Vk spec requires "The following Vulkan objects must not be destroyed while any command buffers using the object are recording or pending execution"
	// - VkEvent
	// - VkQueryPool
	// - VkBufferView       done
	// - VkImageView        done
	// - VkPipeline
	// - VkSampler          done
	// - VkDescriptorPool
	// - VkFramebuffer      done
	// - VkRenderPass       done
	// - VkCommandPool
	// - VkDeviceMemory     done
	// - VkDescriptorSet
	void DeferDestruction(CSampler&& resource);
	void DeferDestruction(CBufferView&& view);
	void DeferDestruction(CImageView&& view);
	void DeferDestruction(VkRenderPass renderPass, VkFramebuffer frameBuffer);
	void DeferDestruction(VkPipeline pipeline);

	// Ticks the deferred destruction for the above types.
	// After kDeferTicks the deferred objects will be actually destroyed.
	void TickDestruction();

	template<class CResource> VkResult CommitResource(EHeapType HeapHint, CResource* pInputResource) threadsafe;
	template<class CResource, class VkCreateInfo> VkResult CreateCommittedResource(EHeapType HeapHint, const VkCreateInfo& createInfo, CResource** ppOutputResource) threadsafe;
	template<class CResource> VkResult DuplicateCommittedResource(CResource* pInputResource, CResource** ppOutputResource) threadsafe;
	template<class CResource> VkResult SubstituteUsedCommittedResource(const FVAL64 (&fenceValues)[CMDQUEUE_NUM], CResource** ppSubstituteResource) threadsafe;
	template<class CResource> VkResult CreateOrReuseStagingResource(CResource* pInputResource, VkDeviceSize minSize, CBufferResource** ppStagingResource, bool bUpload) threadsafe;
	template<class CResource, class VkCreateInfo> VkResult CreateOrReuseCommittedResource(EHeapType HeapHint, const VkCreateInfo& createInfo, CResource** ppOutputResource) threadsafe;
	template<class CResource> void ReleaseLater(const FVAL64 (&fenceValues)[CMDQUEUE_NUM], CResource* pObject, bool bReusable = true) threadsafe;
	template<class CResource> void FlushReleaseHeap(const UINT64 (&completedFenceValues)[CMDQUEUE_NUM], const UINT64 (&pruneFenceValues)[CMDQUEUE_NUM]) threadsafe;

	void FlushReleaseHeaps(const UINT64 (&completedFenceValues)[CMDQUEUE_NUM], const UINT64 (&pruneFenceValues)[CMDQUEUE_NUM]) threadsafe;
	void FlushAndWaitForGPU();

private:
	const SPhysicalDeviceInfo* m_pDeviceInfo;
	VkAllocationCallbacks m_Allocator;
	VkDevice m_device;
	VkPipelineCache m_pipelineCache;
	VkDescriptorPool m_descriptorPool;
	CHeap m_heap;

	struct SRenderPass
	{
		VkDevice self;
		CAutoHandle<VkRenderPass>  renderPass;
		CAutoHandle<VkFramebuffer> frameBuffer;
		SRenderPass(SRenderPass&&) = default;
		~SRenderPass();
	};

	struct SPipeline
	{
		VkDevice self;
		CAutoHandle<VkPipeline> pipeline;
		SPipeline(SPipeline&&) = default;
		~SPipeline();
	};

	static const uint32_t        kDeferTicks = 2; // One for "currently recording", one for "currently executing on GPU", may need to be adjusted?
	DrxCriticalSection           m_deferredLock;
	std::vector<CBufferView>     m_deferredBufferViews[kDeferTicks];
	std::vector<CImageView>      m_deferredImageViews[kDeferTicks];
	std::vector<CSampler>        m_deferredSamplers[kDeferTicks];
	std::vector<SRenderPass>     m_deferredRenderPasses[kDeferTicks];
	std::vector<SPipeline>       m_deferredPipelines[kDeferTicks];

	// Objects that should be released when they are not in use anymore
	static DrxCriticalSectionNonRecursive m_ReleaseHeapTheadSafeScope[3];
	static DrxCriticalSectionNonRecursive m_RecycleHeapTheadSafeScope[3];

	template<class CResource> DrxCriticalSectionNonRecursive& GetReleaseHeapCriticalSection();
	template<class CResource> DrxCriticalSectionNonRecursive& GetRecycleHeapCriticalSection();

	struct ReleaseInfo
	{
		THash      hHash;
		UINT32     bFlags;
		UINT64     fenceValues[CMDQUEUE_NUM];
	};

	template<class CResource>
	struct RecycleInfo
	{
		CResource* pObject;
		UINT64     fenceValues[CMDQUEUE_NUM];
	};

	template<class CResource> using TReleaseHeap = std::unordered_map<CResource*, ReleaseInfo>;
	template<class CResource> using TRecycleHeap = std::unordered_map<THash, std::deque<RecycleInfo<CResource>>>;

	TReleaseHeap<CBufferResource> m_BufferReleaseHeap;
	TRecycleHeap<CBufferResource> m_BufferRecycleHeap;

	TReleaseHeap<CDynamicOffsetBufferResource> m_DynamicOffsetBufferReleaseHeap;
	TRecycleHeap<CDynamicOffsetBufferResource> m_DynamicOffsetBufferRecycleHeap;

	TReleaseHeap<CImageResource> m_ImageReleaseHeap;
	TRecycleHeap<CImageResource> m_ImageRecycleHeap;

	template<class CResource> TReleaseHeap<CResource>& GetReleaseHeap();
	template<class CResource> TRecycleHeap<CResource>& GetRecycleHeap();

	CCommandScheduler      m_Scheduler;
	COcclusionQueryUpr m_occlusionQueries;

};

}
