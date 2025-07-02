#include <drx3D/Graphics/Graphics.h>

#include <cstring>
#include <X/glslang/SPIRV/GlslangToSpv.h>
#include <X/glslang/Public/ShaderLang.h>
#include <drx3D/Devices/Windows.h>
#include <drx3D/Graphics/Subrender.h>

namespace drx3d {
Graphics::Graphics() :
	elapsedPurge(5s),
	instance(std::make_unique<Instance>()),
	physicalDevice(std::make_unique<PhysicalDevice>(*instance)),
	logicalDevice(std::make_unique<LogicalDevice>(*instance, *physicalDevice)) {
	Windows::Get()->OnAddWindow().connect(this, [this](Window *window, bool added) {
		surfaces.emplace_back(std::make_unique<Surface>(*instance, *physicalDevice, *logicalDevice, *window));
	});

	CreatePipelineCache();

	if (!glslang::InitializeProcess())
		throw drx::Exc("Не удалось инициализовать процесс glslang");
}

Graphics::~Graphics() {
	auto graphicsQueue = logicalDevice->GetGraphicsQueue();

	renderer = nullptr;
	swapchains.clear();
	
	CheckVk(vkQueueWaitIdle(graphicsQueue));

	glslang::FinalizeProcess();

	vkDestroyPipelineCache(*logicalDevice, pipelineCache, nullptr);

	commandPools.clear();

	for (auto &perSurfaceBuffer : perSurfaceBuffers) {
		for (std::size_t i = 0; i < perSurfaceBuffer->flightFences.size(); i++) {
			vkDestroyFence(*logicalDevice, perSurfaceBuffer->flightFences[i], nullptr);
			vkDestroySemaphore(*logicalDevice, perSurfaceBuffer->renderCompletes[i], nullptr);
			vkDestroySemaphore(*logicalDevice, perSurfaceBuffer->presentCompletes[i], nullptr);
		}

		perSurfaceBuffer->commandBuffers.clear();
	}
}

void Graphics::Update() {
	if (!renderer || Windows::Get()->GetWindow(0)->IsIconified()) return;

	if (!renderer->started) {
		ResetRenderStages();
		renderer->Start();
		renderer->started = true;
	}

	renderer->Update();

	for (auto [id, swapchain] : Enumerate(swapchains)) {
		auto &perSurfaceBuffer = perSurfaceBuffers[id];
		auto acquireResult = swapchain->AcquireNextImage(perSurfaceBuffer->presentCompletes[perSurfaceBuffer->currentFrame], perSurfaceBuffer->flightFences[perSurfaceBuffer->currentFrame]);

		if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapchain();
			return;
		}

		if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
			Log::Error("Не удалось обрести swap chain image!\n");
			return;
		}

		Pipeline::Stage stage;

		for (auto &renderStage : renderer->renderStages) {
			renderStage->Update();

			if (!StartRenderpass(id, *renderStage))
				return;

			auto &commandBuffer = perSurfaceBuffer->commandBuffers[swapchain->GetActiveImageIndex()];

			for (const auto &subpass : renderStage->GetSubpasses()) {
				stage.second = subpass.GetBinding();

				// Renders subpass subrender pipelines.
				renderer->subrenderHolder.RenderStage(stage, *commandBuffer);

				if (subpass.GetBinding() != renderStage->GetSubpasses().back().GetBinding())
					vkCmdNextSubpass(*commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
			}

			EndRenderpass(id, *renderStage);
			stage.first++;
		}
	}

	// Purges unused command pools.
	if (elapsedPurge.GetElapsed() != 0) {
		for (auto it = commandPools.begin(); it != commandPools.end();) {
			if ((*it).second.use_count() <= 1) {
				it = commandPools.erase(it);
				continue;
			}

			++it;
		}
	}
}

STxt Graphics::StringifyResultVk(VkResult result) {
	switch (result) {
	case VK_SUCCESS:
		return "Успешно";
	case VK_NOT_READY:
		return "Либо fence, либо query ещё не завершён";
	case VK_TIMEOUT:
		return "Операция wait не закончилась за отведённое время";
	case VK_EVENT_SET:
		return "Поступил сигнал о каком-то событии";
	case VK_EVENT_RESET:
		return "Не поступил сигнал о каком-то событии";
	case VK_INCOMPLETE:
		return "Возвращённый массив слишком мал для итога";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "Неудавшееся размещение в памяти хоста";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "Неудавшееся размещение в памяти устройства";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "Инициализация объекта не завершена по причине, связанной с реализацией";
	case VK_ERROR_DEVICE_LOST:
		return "Потеряно логическое или физическое устройство";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "Неудачный маппинг объекта памяти";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "Требуемый слой отсутствует или не может быть загружен";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "Требуемое расширение не поддерживается";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "Требуемая фича не поддерживается";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "Требуемая версия Vulkan не поддерживается драйвером или несовместима по иным причинам";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "Уже создано слишком много объектов данного типа";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "Требуемый формат не поддерживается на этом устройстве";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "Поверхность более недоступна";
		//case VK_ERROR_OUT_OF_POOL_MEMORY:
		//	return "A allocation failed due to having no more space in the descriptor pool";
	case VK_SUBOPTIMAL_KHR:
		return "A swapchain no longer matches the surface properties exactly, but can still be used";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "Поверхность изменена таким образом, что более несовместима с swapchain";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "The display used by a swapchain does not use the same presentable image layout";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "Требуемое окно уже подключено к VkSurfaceKHR, или к какому-то иному, не-Vulkan, API";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "Контролирующий слой обнаружил ошибку";
	default:
		return "Неизвестная ошибка Vulkan";
	}
}

void Graphics::CheckVk(VkResult result) {
	if (result >= 0) return;

	auto failure = StringifyResultVk(result);

	throw drx::Exc("Ошибка Vulkan: " + failure);
}

void Graphics::CaptureScreenshot(const std::filesystem::path &filename, std::size_t id) const {
#ifdef DRX3D_DEBUG
	auto debugStart = Time::Now();
#endif

	auto size = Windows::Get()->GetWindow(0)->GetSize();

	VkImage dstImage;
	VkDeviceMemory dstImageMemory;
	auto supportsBlit = Image::CopyImage(swapchains[id]->GetActiveImage(), dstImage, dstImageMemory, surfaces[id]->GetFormat().format, {size.x, size.y, 1},
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0, 0);

	// Get layout of the image (including row pitch).
	VkImageSubresource imageSubresource = {};
	imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresource.mipLevel = 0;
	imageSubresource.arrayLayer = 0;

	VkSubresourceLayout dstSubresourceLayout;
	vkGetImageSubresourceLayout(*logicalDevice, dstImage, &imageSubresource, &dstSubresourceLayout);

	Bitmap bitmap(std::make_unique<uint8_t[]>(dstSubresourceLayout.size), size);

	uk data;
	vkMapMemory(*logicalDevice, dstImageMemory, dstSubresourceLayout.offset, dstSubresourceLayout.size, 0, &data);
	std::memcpy(bitmap.GetData().get(), data, static_cast<size_t>(dstSubresourceLayout.size));
	vkUnmapMemory(*logicalDevice, dstImageMemory);

	// Frees temp image and memory.
	vkFreeMemory(*logicalDevice, dstImageMemory, nullptr);
	vkDestroyImage(*logicalDevice, dstImage, nullptr);

	// Writes the screenshot bitmap to the file.
	bitmap.Write(filename);

#ifdef DRX3D_DEBUG
	Log::Out("Screenshot ", filename, " created in ", (Time::Now() - debugStart).AsMilliseconds<float>(), "ms\n");
#endif
}

const RenderStage *Graphics::GetRenderStage(uint32_t index) const {
	if (renderer)
		return renderer->GetRenderStage(index);
	return nullptr;
}

const Descriptor *Graphics::GetAttachment(const STxt &name) const {
	if (auto it = attachments.find(name); it != attachments.end())
		return it->second;
	return nullptr;
}

const std::shared_ptr<CommandPool> &Graphics::GetCommandPool(const std::thread::id &threadId) {
	if (auto it = commandPools.find(threadId); it != commandPools.end())
		return it->second;
	// TODO: Cleanup and fix crashes
	return commandPools.emplace(threadId, std::make_shared<CommandPool>(threadId)).first->second;
}

void Graphics::CreatePipelineCache() {
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	CheckVk(vkCreatePipelineCache(*logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
}

void Graphics::ResetRenderStages() {
	RecreateSwapchain();

	for (const auto [id, swapchain] : Enumerate(swapchains)) {
		auto &perSurfaceBuffer = perSurfaceBuffers[id];
		if (perSurfaceBuffer->flightFences.size() != swapchain->GetImageCount())
			RecreateCommandBuffers(id);

		for (const auto &renderStage : renderer->renderStages)
			renderStage->Rebuild(*swapchain);
	}

	RecreateAttachmentsMap();
}

void Graphics::RecreateSwapchain() {
	vkDeviceWaitIdle(*logicalDevice);

	VkExtent2D displayExtent = {Windows::Get()->GetWindow(0)->GetSize().x, Windows::Get()->GetWindow(0)->GetSize().y};
#ifdef DRX3D_DEBUG
	if (swapchains.empty()) {
		for (auto &swapchain : swapchains) {
			Log::Out("Recreating swapchain old (", swapchain->GetExtent().width, ", ", swapchain->GetExtent().height, ") new (", displayExtent.width, ", ", displayExtent.height, ")\n");
		}
	}
#endif
	swapchains.resize(surfaces.size());
	perSurfaceBuffers.resize(surfaces.size());
	for (const auto [id, surface] : Enumerate(surfaces)) {
		swapchains[id] = std::make_unique<Swapchain>(*physicalDevice, *surface, *logicalDevice, displayExtent, swapchains[id].get());
		perSurfaceBuffers[id] = std::make_unique<PerSurfaceBuffers>();
		RecreateCommandBuffers(id);
	}
}

void Graphics::RecreateCommandBuffers(std::size_t id) {
	auto &swapchain = swapchains[id];
	auto &perSurfaceBuffer = perSurfaceBuffers[id];

	for (std::size_t i = 0; i < perSurfaceBuffer->flightFences.size(); i++) {
		vkDestroyFence(*logicalDevice, perSurfaceBuffer->flightFences[i], nullptr);
		vkDestroySemaphore(*logicalDevice, perSurfaceBuffer->renderCompletes[i], nullptr);
		vkDestroySemaphore(*logicalDevice, perSurfaceBuffer->presentCompletes[i], nullptr);
	}

	perSurfaceBuffer->presentCompletes.resize(swapchain->GetImageCount());
	perSurfaceBuffer->renderCompletes.resize(swapchain->GetImageCount());
	perSurfaceBuffer->flightFences.resize(swapchain->GetImageCount());
	perSurfaceBuffer->commandBuffers.resize(swapchain->GetImageCount());

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (std::size_t i = 0; i < perSurfaceBuffer->flightFences.size(); i++) {
		CheckVk(vkCreateSemaphore(*logicalDevice, &semaphoreCreateInfo, nullptr, &perSurfaceBuffer->presentCompletes[i]));

		CheckVk(vkCreateSemaphore(*logicalDevice, &semaphoreCreateInfo, nullptr, &perSurfaceBuffer->renderCompletes[i]));

		CheckVk(vkCreateFence(*logicalDevice, &fenceCreateInfo, nullptr, &perSurfaceBuffer->flightFences[i]));

		perSurfaceBuffer->commandBuffers[i] = std::make_unique<CommandBuffer>(false);
	}
}

void Graphics::RecreatePass(std::size_t id, RenderStage &renderStage) {
	auto graphicsQueue = logicalDevice->GetGraphicsQueue();
	auto &perSurfaceBuffer = perSurfaceBuffers[id];

	VkExtent2D displayExtent = {Windows::Get()->GetWindow(0)->GetSize().x, Windows::Get()->GetWindow(0)->GetSize().y};

	CheckVk(vkQueueWaitIdle(graphicsQueue));

	for (const auto [id, swapchain] : Enumerate(swapchains)) {
		if (renderStage.HasSwapchain() && (perSurfaceBuffer->framebufferResized || !swapchain->IsSameExtent(displayExtent)))
			RecreateSwapchain();

		renderStage.Rebuild(*swapchain);
	}
	RecreateAttachmentsMap(); // TODO: Maybe not recreate on a single change.
}

void Graphics::RecreateAttachmentsMap() {
	attachments.clear();

	for (const auto &renderStage : renderer->renderStages)
		attachments.insert(renderStage->descriptors.begin(), renderStage->descriptors.end());
}

bool Graphics::StartRenderpass(std::size_t id, RenderStage &renderStage) {
	if (renderStage.IsOutOfDate()) {
		RecreatePass(id, renderStage);
		return false;
	}

	auto &swapchain = swapchains[id];
	auto &perSurfaceBuffer = perSurfaceBuffers[id];
	auto &commandBuffer = perSurfaceBuffer->commandBuffers[swapchain->GetActiveImageIndex()];

	if (!commandBuffer->IsRunning())
		commandBuffer->Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

	VkRect2D renderArea = {};
	renderArea.offset = {renderStage.GetRenderArea().GetOffset().x, renderStage.GetRenderArea().GetOffset().y};
	renderArea.extent = {renderStage.GetRenderArea().GetExtent().x, renderStage.GetRenderArea().GetExtent().y};

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(renderArea.extent.width);
	viewport.height = static_cast<float>(renderArea.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset = renderArea.offset;
	scissor.extent = renderArea.extent;
	vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);

	auto clearValues = renderStage.GetClearValues();

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = *renderStage.GetRenderpass();
	renderPassBeginInfo.framebuffer = renderStage.GetActiveFramebuffer(swapchain->GetActiveImageIndex());
	renderPassBeginInfo.renderArea = renderArea;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();
	vkCmdBeginRenderPass(*commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	return true;
}

void Graphics::EndRenderpass(std::size_t id, RenderStage &renderStage) {
	auto presentQueue = logicalDevice->GetPresentQueue();
	auto &swapchain = swapchains[id];
	auto &perSurfaceBuffer = perSurfaceBuffers[id];
	auto &commandBuffer = perSurfaceBuffer->commandBuffers[swapchain->GetActiveImageIndex()];

	vkCmdEndRenderPass(*commandBuffer);

	if (!renderStage.HasSwapchain())
		return;

	commandBuffer->End();
	commandBuffer->Submit(perSurfaceBuffer->presentCompletes[perSurfaceBuffer->currentFrame], perSurfaceBuffer->renderCompletes[perSurfaceBuffer->currentFrame], perSurfaceBuffer->flightFences[perSurfaceBuffer->currentFrame]);

	auto presentResult = swapchain->QueuePresent(presentQueue, perSurfaceBuffer->renderCompletes[perSurfaceBuffer->currentFrame]);
	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR) { // || framebufferResized
		perSurfaceBuffer->framebufferResized = true; // false
		//RecreateSwapchain();
	} else if (presentResult != VK_SUCCESS) {
		CheckVk(presentResult);
		Log::Error("Failed to present swap chain image!\n");
	}

	perSurfaceBuffer->currentFrame = (perSurfaceBuffer->currentFrame + 1) % swapchain->GetImageCount();
}
}
