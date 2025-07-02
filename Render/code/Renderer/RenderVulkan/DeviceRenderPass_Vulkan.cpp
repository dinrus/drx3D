// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DriverD3D.h>
#include <drx3D/Render/xxhash.h>
#include <drx3D/Render/D3D/Vulkan/VKInstance.hpp>
#include <drx3D/Render/D3D/Vulkan/VKBufferResource.hpp>
#include <drx3D/Render/D3D/Vulkan/VKImageResource.hpp>
#include <drx3D/Render/D3D/Vulkan/VKSampler.hpp>
#include <drx3D/Render/D3D/Vulkan/VKExtensions.hpp>

#include <drx3D/Render/Renderer.h>

#include <drx3D/Render/DeviceResourceSet_Vulkan.h>	
#include <drx3D/Render/DeviceRenderPass_Vulkan.h>

using namespace NDrxVulkan;


////////////////////////////////////////////////////////////////////////////
CDeviceRenderPass::CDeviceRenderPass()
	: m_renderPassHandle(VK_NULL_HANDLE)
	, m_frameBufferHandle(VK_NULL_HANDLE)
{
	m_frameBufferExtent.width = 0;
	m_frameBufferExtent.height = 0;
}

CDeviceRenderPass::~CDeviceRenderPass()
{
	GetDevice()->DeferDestruction(m_renderPassHandle, m_frameBufferHandle);
}

bool CDeviceRenderPass::UpdateImpl(const CDeviceRenderPassDesc& passDesc)
{
	// render pass handle cannot change as PSOs would become invalid
	if (m_renderPassHandle == VK_NULL_HANDLE)
		InitVkRenderPass(passDesc);

	InitVkFrameBuffer(passDesc);

	return m_renderPassHandle != VK_NULL_HANDLE && m_frameBufferHandle != VK_NULL_HANDLE;
}

bool CDeviceRenderPass::InitVkRenderPass(const CDeviceRenderPassDesc& passDesc)
{
	i32 attachmentCount = 0;
	i32 colorAttachmentCount = 0;
	VkAttachmentDescription attachments[CDeviceRenderPassDesc::MaxRendertargetCount + 1];
	VkAttachmentReference colorAttachments[CDeviceRenderPassDesc::MaxRendertargetCount];
	VkAttachmentReference depthStencilAttachment;

	// NOTE: the following code deliberately falls back to CTexture::GetDstFormat as we cannot guarantee
	// the existence of device texture at this point.

	const auto& renderTargets = passDesc.GetRenderTargets();
	const auto& depthTarget = passDesc.GetDepthTarget();

	for (i32 i = 0; i < renderTargets.size(); ++i)
	{
		if (!renderTargets[i].pTexture)
			break;

		DXGI_FORMAT deviceFormat = renderTargets[i].GetResourceFormat();

		attachments[attachmentCount].flags = 0;
		attachments[attachmentCount].format = DXGIFormatToVKFormat(deviceFormat);
		attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[attachmentCount].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		colorAttachments[attachmentCount].attachment = i;
		colorAttachments[attachmentCount].layout = attachments[attachmentCount].initialLayout;

		++attachmentCount;
		++colorAttachmentCount;
	}

	if (depthTarget.pTexture)
	{
		DXGI_FORMAT deviceFormat = depthTarget.GetResourceFormat();


		attachments[attachmentCount].flags = 0;
		attachments[attachmentCount].format = DXGIFormatToVKFormat(deviceFormat);
		attachments[attachmentCount].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[attachmentCount].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[attachmentCount].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[attachmentCount].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[attachmentCount].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[attachmentCount].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		depthStencilAttachment.attachment = attachmentCount;
		depthStencilAttachment.layout = attachments[attachmentCount].initialLayout;

		++attachmentCount;
	}

	VkSubpassDescription subpass;
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = colorAttachmentCount;
	subpass.pColorAttachments = colorAttachments;
	subpass.pResolveAttachments = nullptr;
	subpass.pDepthStencilAttachment = depthTarget.pTexture ? &depthStencilAttachment : nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo renderPassCreateInfo;
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.pNext = nullptr;
	renderPassCreateInfo.flags = 0;
	renderPassCreateInfo.pAttachments = attachments;
	renderPassCreateInfo.attachmentCount = attachmentCount;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 0;
	renderPassCreateInfo.pDependencies = nullptr;

	if (vkCreateRenderPass(GetDevice()->GetVkDevice(), &renderPassCreateInfo, nullptr, &m_renderPassHandle) == VK_SUCCESS)
		return true;

	return false;
}

bool CDeviceRenderPass::InitVkFrameBuffer(const CDeviceRenderPassDesc& passDesc)
{
	m_RenderTargetCount = 0;
	m_pDepthStencilTarget = nullptr;

	i32 renderTargetCount;
	std::array<CImageView*, CDeviceRenderPassDesc::MaxRendertargetCount> rendertargetViews;
	auto& renderTargets = passDesc.GetRenderTargets();

	CImageView* pDepthStencilView = nullptr;
	auto& depthTarget = passDesc.GetDepthTarget();

	if (!passDesc.GetDeviceRendertargetViews(rendertargetViews, renderTargetCount))
		return false;

	if (!passDesc.GetDeviceDepthstencilView(pDepthStencilView))
		return false;

	VkImageView imageViews[CDeviceRenderPassDesc::MaxRendertargetCount + 1];
	i32 imageViewCount = 0;

	for (i32 i = 0; i < renderTargetCount; ++i)
	{
		imageViews[imageViewCount++] = rendertargetViews[i]->GetHandle();
		m_renderTargets[m_RenderTargetCount++] = renderTargets[i].pTexture->GetDevTexture()->Get2DTexture();
	}

	if (pDepthStencilView)
	{
		imageViews[imageViewCount++] = pDepthStencilView->GetHandle();
		m_pDepthStencilTarget = depthTarget.pTexture->GetDevTexture()->Get2DTexture();
	}

	if (imageViewCount > 0)
	{
		CTexture* pTex = renderTargetCount > 0 ? renderTargets[0].pTexture : depthTarget.pTexture;
		m_frameBufferExtent.width = u32(pTex->GetWidth());
		m_frameBufferExtent.height = u32(pTex->GetHeight());

		VkFramebufferCreateInfo frameBufferCreateInfo;

		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = nullptr;
		frameBufferCreateInfo.flags = 0;
		frameBufferCreateInfo.renderPass = m_renderPassHandle;
		frameBufferCreateInfo.attachmentCount = imageViewCount;
		frameBufferCreateInfo.pAttachments = imageViews;
		frameBufferCreateInfo.width = m_frameBufferExtent.width;
		frameBufferCreateInfo.height = m_frameBufferExtent.height;
		frameBufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(GetDevice()->GetVkDevice(), &frameBufferCreateInfo, nullptr, &m_frameBufferHandle) == VK_SUCCESS)
			return true;
	}

	return false;
}

