// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "DriverD3D.h"

static u32 MapResourceFlags(NDrxVulkan::CMemoryResource* pResource)
{
	u32 result = 0;
	if (pResource->GetFlag(NDrxVulkan::kResourceFlagCpuReadable))
	{
		result |= CDeviceObjectFactory::USAGE_CPU_READ;
	}
	if (pResource->GetFlag(NDrxVulkan::kResourceFlagCpuWritable))
	{
		result |= CDeviceObjectFactory::USAGE_CPU_WRITE;
	}
	if (pResource->GetFlag(NDrxVulkan::kResourceFlagShaderReadable))
	{
		result |= CDeviceObjectFactory::BIND_SHADER_RESOURCE;
	}
	if (pResource->GetFlag(NDrxVulkan::kResourceFlagShaderWritable))
	{
		result |= CDeviceObjectFactory::BIND_UNORDERED_ACCESS;
	}
	if (pResource->GetFlag(NDrxVulkan::kBufferFlagVertices))
	{
		result |= CDeviceObjectFactory::BIND_VERTEX_BUFFER;
	}
	else if (pResource->GetFlag(NDrxVulkan::kBufferFlagIndices))
	{
		result |= CDeviceObjectFactory::BIND_INDEX_BUFFER;
	}
	else if (pResource->AsBuffer())
	{
		result |= CDeviceObjectFactory::BIND_CONSTANT_BUFFER;
	}
	if (pResource->GetFlag(NDrxVulkan::kImageFlagColorAttachment))
	{
		result |= CDeviceObjectFactory::BIND_RENDER_TARGET;
	}
	if (pResource->GetFlag(NDrxVulkan::kImageFlagDepthAttachment) || pResource->GetFlag(NDrxVulkan::kImageFlagStencilAttachment))
	{
		result |= CDeviceObjectFactory::BIND_DEPTH_STENCIL;
	}
	return result;
}

////////////////////////////////////////////////////////////////////////////
// ResourceView API
CDeviceResourceView* CDeviceResource::CreateResourceView(const SResourceView pView)
{
	NDrxVulkan::CMemoryResource* const pResource = GetNativeResource();
	VK_ASSERT(pResource && "Attempt to create view on nullptr");
	NDrxVulkan::CImageResource* const pImage = pResource->AsImage();
	NDrxVulkan::CBufferResource* const pBuffer = pResource->AsBuffer();
	VK_ASSERT((m_bFilterable ? pImage != nullptr : pBuffer != nullptr) && "Filterable property requires an image resource");

	DXGI_FORMAT dxgiFormat = DXGI_FORMAT(pView.m_Desc.nFormat);
	if (pView.m_Desc.bSrgbRead && DeviceFormats::IsSRGBReadable(dxgiFormat))
	{
		dxgiFormat = DeviceFormats::ConvertToSRGB(dxgiFormat);
	}
	VkFormat format = NDrxVulkan::ConvertFormat(dxgiFormat);

	if (pImage)
	{
		const SResourceView zeroView(0ULL);
		const SResourceView oneView(~0ULL);
		const bool bAllMips = pView.m_Desc.nMipCount == zeroView.m_Desc.nMipCount || pView.m_Desc.nMipCount == oneView.m_Desc.nMipCount;
		const bool bAllSlices = pView.m_Desc.nSliceCount == zeroView.m_Desc.nSliceCount || pView.m_Desc.nSliceCount == oneView.m_Desc.nSliceCount;
		const bool bMultiSampled = pView.m_Desc.bMultisample != 0;
		VK_ASSERT(bMultiSampled == pResource->GetFlag(NDrxVulkan::kImageFlagMultiSampled) && "Cannot create non-multi-sampled view on a multi-sampled resource");

		uint32_t firstMip = pView.m_Desc.nMostDetailedMip;
		uint32_t numMips = bAllMips ? pImage->GetMipCount() - firstMip : static_cast<uint32_t>(pView.m_Desc.nMipCount);
		uint32_t firstSlice = pView.m_Desc.nFirstSlice;
		uint32_t numSlices = bAllSlices ? pImage->GetSliceCount() - firstSlice : static_cast<uint32_t>(pView.m_Desc.nSliceCount);

		if (pImage->GetFlag(NDrxVulkan::kResourceFlagNull))
		{
			// In case this is a null-resource, ignore all view properties.
			// This is required because CDeviceTexture will associate random (and likely incompatible) layouts.
			dxgiFormat = DXGI_FORMAT_UNKNOWN;
			format = pImage->GetFormat();
			firstMip = 0U;
			numMips = 1U;
			firstSlice = 0U;
			numSlices = m_eTT == eTT_Cube || eTT_CubeArray ? 6U : 1U;
		}

		const VkImageViewType viewType =
			m_eTT == eTT_1D        ? (numSlices == 1 ? VK_IMAGE_VIEW_TYPE_1D   : VK_IMAGE_VIEW_TYPE_1D_ARRAY  ) :
			m_eTT == eTT_2D        ? (numSlices == 1 ? VK_IMAGE_VIEW_TYPE_2D   : VK_IMAGE_VIEW_TYPE_2D_ARRAY  ) :
			m_eTT == eTT_2DArray   ? (numSlices == 1 ? VK_IMAGE_VIEW_TYPE_2D   : VK_IMAGE_VIEW_TYPE_2D_ARRAY  ) :
			m_eTT == eTT_Cube      ? (numSlices <= 6 ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) :
			m_eTT == eTT_CubeArray ? (numSlices <= 6 ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) :
			                                           VK_IMAGE_VIEW_TYPE_3D;

		NDrxVulkan::EImageSwizzle swizzle = NDrxVulkan::kImageSwizzleRGBA;
		switch (dxgiFormat)
		{
		case DXGI_FORMAT_A8_UNORM:
			swizzle = NDrxVulkan::kImageSwizzle000A;
			break;
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			if (format != pImage->GetFormat())
			{
				swizzle = NDrxVulkan::kImageSwizzleBGR1;
			}
			break;
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			if (format != pImage->GetFormat())
			{
				swizzle = NDrxVulkan::kImageSwizzleBGRA;
			}
			break;
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			swizzle = NDrxVulkan::kImageSwizzleDD00;
			break;
		case DXGI_FORMAT_R32_FLOAT:
			if (pImage->GetFormat() == VK_FORMAT_D32_SFLOAT)
			{
				swizzle = NDrxVulkan::kImageSwizzleDD00;
			}
			break;
		case DXGI_FORMAT_R16_UNORM:
			if (pImage->GetFormat() == VK_FORMAT_D16_UNORM)
			{
				swizzle = NDrxVulkan::kImageSwizzleDD00;
			}
			break;
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			if (pImage->GetFormat() == VK_FORMAT_S8_UINT
			    || pImage->GetFormat() == VK_FORMAT_D16_UNORM_S8_UINT
			    || pImage->GetFormat() == VK_FORMAT_D24_UNORM_S8_UINT
			    || pImage->GetFormat() == VK_FORMAT_D32_SFLOAT_S8_UINT)
			{
				swizzle = NDrxVulkan::kImageSwizzleSS00;
			}
			break;
		}

		NDrxVulkan::CImageView* const pResult = new NDrxVulkan::CImageView(pImage, format, viewType, firstMip, numMips, firstSlice, numSlices, swizzle);
		if (pResult->GetHandle())
		{
			return pResult;
		}
		pResult->Release();
	}
	else if (pBuffer)
	{
		const uint64_t offsetAndSize = pView.m_Desc.nOffsetAndSize;
		const uint32_t shift = 46U - static_cast<uint32_t>(pView.m_Desc.nOffsetBits);
		uint32_t offset = static_cast<uint32_t>(offsetAndSize >> shift);
		uint32_t size = static_cast<uint32_t>(offsetAndSize & MASK64(shift));

		if (pResource->GetFlag(NDrxVulkan::kResourceFlagNull))
		{
			return GetDeviceObjectFactory().GetVkNullBufferView(format == VK_FORMAT_UNDEFINED);
		}
		else
		{
			VK_ASSERT(size != 0 && "Cannot create 0-byte view on non-null-buffer");
			offset *= pBuffer->GetStride();
			size *= pBuffer->GetStride();
		}

		NDrxVulkan::CBufferView* const pResult = new NDrxVulkan::CBufferView(pBuffer, offset, size, format);
		VkBufferView view;
		if (pResult->IsStructured() || (pResult->FillInfo(view), view != VK_NULL_HANDLE))
		{
			return pResult;
		}

		// Edge case, unable to create non-structured buffer view.
		pResult->Release();
	}
	else
	{
		VK_ASSERT("Unsupported resource type for view creation");
	}
	return nullptr;
}

SResourceLayout CDeviceResource::GetLayout() const
{
	SResourceLayout result;
	if (NDrxVulkan::CBufferResource* pBuffer = m_pNativeResource->AsBuffer())
	{
		result.m_elementCount = pBuffer->GetElementCount();
		result.m_byteCount = pBuffer->GetElementCount() * pBuffer->GetStride();
	}
	else if (NDrxVulkan::CImageResource* pImage = m_pNativeResource->AsImage())
	{
		result.m_elementCount = pImage->GetWidth() * pImage->GetHeight() * pImage->GetDepth() * pImage->GetSliceCount();
		result.m_byteCount = static_cast<buffer_size_t>(pImage->GetSize()); // Includes tiling/alignment requirements.
	}
	result.m_eFlags = MapResourceFlags(m_pNativeResource);
	return result;
}

CDeviceResource::ESubstitutionResult CDeviceResource::SubstituteUsedResource()
{
	NDrxVulkan::CMemoryResource* pResource = GetNativeResource();
	const auto& fenceUpr = pResource->GetDevice()->GetScheduler().GetFenceUpr();
	NDrxVulkan::CDynamicOffsetBufferResource* pDynBuf = pResource->AsDynamicOffsetBuffer();
	NDrxVulkan::CBufferResource             * pBuffer = pResource->AsBuffer();
	NDrxVulkan::CImageResource              * pImage  = pResource->AsImage();
	VkResult hVkResult = VK_SUCCESS;

	// NOTE: Poor man's resource tracking (take current time as last-used moment)
	if (pDynBuf)
		hVkResult = pDynBuf->GetDevice()->SubstituteUsedCommittedResource(fenceUpr.GetCurrentValues(), &pDynBuf);
	else if (pBuffer)
		hVkResult = pBuffer->GetDevice()->SubstituteUsedCommittedResource(fenceUpr.GetCurrentValues(), &pBuffer);
	else if (pImage)
		hVkResult = pImage ->GetDevice()->SubstituteUsedCommittedResource(fenceUpr.GetCurrentValues(), &pImage);

	if (hVkResult == VK_NOT_READY) // NOT_SUBSTITUTED
		return eSubResult_Kept;
	if (hVkResult != VK_SUCCESS) // Other Error
		return eSubResult_Failed;

	if (pDynBuf)
		m_pNativeResource = pDynBuf;
	else if (pBuffer)
		m_pNativeResource = pBuffer;
	else if (pImage)
		m_pNativeResource = pImage;

	if (pDynBuf || pBuffer)
	{
		auto* const pPreviousBuffer = static_cast<NDrxVulkan::CBufferResource*>(pResource);
		pBuffer->SetStrideAndElementCount(pPreviousBuffer->GetStride(), pPreviousBuffer->GetElementCount());
	}

	ReleaseResourceViews();
	AllocatePredefinedResourceViews();

	// TODO: also call this when it is moved to CDeviceResource
	// InvalidateDeviceResource(u32 dirtyFlags);

	return eSubResult_Substituted;
}

////////////////////////////////////////////////////////////////////////////

SBufferLayout CDeviceBuffer::GetLayout() const
{
	NDrxVulkan::CBufferResource* const pBuffer = m_pNativeResource->AsBuffer();
	VK_ASSERT(pBuffer && "Invalid cast to buffer");

	SBufferLayout result;
	result.m_eFormat = DXGI_FORMAT_UNKNOWN; // Vk buffers are not typed, that's a property of the view.
	result.m_elementCount = pBuffer->GetElementCount();
	result.m_elementSize = static_cast<u16>(pBuffer->GetStride());
	result.m_eFlags = MapResourceFlags(pBuffer);
	return result;
}

SResourceMemoryAlignment CDeviceBuffer::GetAlignment() const
{
	SResourceMemoryAlignment Alignment = { 0 };
	SBufferLayout Layout = GetLayout();

	Alignment.typeStride = Layout.m_elementSize;
	Alignment.rowStride = Layout.m_elementSize * Layout.m_elementCount;
	Alignment.planeStride = Layout.m_elementSize * Layout.m_elementCount;
	Alignment.volumeStride = Layout.m_elementSize * Layout.m_elementCount;

	return Alignment;
}

SResourceDimension CDeviceBuffer::GetDimension() const
{
	SResourceDimension Dimension = { 0 };
	SBufferLayout Layout = GetLayout();

	Dimension.Width = Layout.m_elementSize * Layout.m_elementCount;
	Dimension.Height = 1;
	Dimension.Depth = 1;
	Dimension.Subresources = 1;

	return Dimension;
}

////////////////////////////////////////////////////////////////////////////

STextureLayout CDeviceTexture::GetLayout() const
{
	STextureLayout Layout;

	Layout.m_eSrcFormat =
	Layout.m_eDstFormat = CRendererResources::s_hwTexFormatSupport.GetClosestFormatSupported(DeviceFormats::ConvertToTexFormat(m_eNativeFormat), Layout.m_pPixelFormat);
	Layout.m_eTT = m_eTT;
	Layout.m_eFlags = m_eFlags;
	Layout.m_bIsSRGB = m_bIsSrgb;

	NDrxVulkan::CImageResource* const pImage = m_pNativeResource ? m_pNativeResource->AsImage() : nullptr;

	if (pImage)
	{
		Layout.m_nWidth = static_cast<u16>(pImage->GetWidth());
		Layout.m_nHeight = static_cast<u16>(pImage->GetHeight());
		Layout.m_nDepth = static_cast<u16>(pImage->GetDepth());
		Layout.m_nArraySize = static_cast<u8>(pImage->GetSliceCount());
		Layout.m_nMips = static_cast<int8>(pImage->GetMipCount());
		Layout.m_eFlags |= MapResourceFlags(pImage);
	}

	return Layout;
}

STextureLayout CDeviceTexture::GetLayout(D3DBaseView* pView)
{
	STextureLayout Layout = {};

	NDrxVulkan::CImageResource* const pImage = pView->GetResource()->AsImage();
	const NDrxVulkan::CImageView* const pImageView = pImage ? static_cast<NDrxVulkan::CImageView*>(pView) : nullptr;

	if (pImage && pImageView)
	{
		u32 nWidth = pImage->GetWidth();
		u32 nHeight = pImage->GetHeight();
		u32 nDepth = pImage->GetDepth();
		u32 nMips = pImageView->GetMipCount();
		u32 nSlices = pImageView->GetSliceCount();
		u32 nFirstMip = pImageView->GetFirstMip();
		u32 nFlags = MapResourceFlags(pImage);

		DXGI_FORMAT format = NDrxVulkan::ConvertFormat(pImage->GetFormat());
		ETEX_Format eTF = DeviceFormats::ConvertToTexFormat(format);
		ETEX_Type eTT = eTT_MaxTexType;

		if (pImage->GetFlag(NDrxVulkan::kImageFlagCube))
			eTT = (nSlices <= 6 ? eTT_Cube : eTT_CubeArray);
		else if (pImage->GetFlag(NDrxVulkan::kImageFlag2D))
			eTT = (nSlices <= 1 ? eTT_2D : eTT_2DArray);
		else if (pImage->GetFlag(NDrxVulkan::kImageFlag3D))
			eTT = eTT_3D;
		assert(eTT != eTT_MaxTexType);

		nWidth  = std::max(nWidth  >> nFirstMip, 1U);
		nHeight = std::max(nHeight >> nFirstMip, 1U);
		nDepth  = std::max(nDepth  >> nFirstMip, 1U);

		Layout.m_eSrcFormat =
		Layout.m_eDstFormat = CRendererResources::s_hwTexFormatSupport.GetClosestFormatSupported(eTF, Layout.m_pPixelFormat);
		Layout.m_eTT = eTT;
		Layout.m_eFlags = nFlags;
		Layout.m_nWidth = nWidth;
		Layout.m_nHeight = nHeight;
		Layout.m_nDepth = nDepth;
		Layout.m_nMips = nMips;
		Layout.m_nArraySize = nSlices;
	}

	return Layout;
}

SResourceMemoryAlignment CDeviceTexture::GetAlignment(u8 mip /*= 0*/, u8 slices /*= 0*/) const
{
	SResourceMemoryAlignment Alignment = { 0 };
	STextureLayout Layout = GetLayout();

	if (!(Layout.m_nWidth  = Layout.m_nWidth  >> mip)) Layout.m_nWidth  = 1;
	if (!(Layout.m_nHeight = Layout.m_nHeight >> mip)) Layout.m_nHeight = 1;
	if (!(Layout.m_nDepth  = Layout.m_nDepth  >> mip)) Layout.m_nDepth  = 1;

	Alignment.typeStride   = CTexture::TextureDataSize(              1,                1,               1, 1, 1, DeviceFormats::ConvertToTexFormat(m_eNativeFormat), eTM_None);
	Alignment.rowStride    = CTexture::TextureDataSize(Layout.m_nWidth,                1,               1, 1, 1, DeviceFormats::ConvertToTexFormat(m_eNativeFormat), eTM_None);
	Alignment.planeStride  = CTexture::TextureDataSize(Layout.m_nWidth, Layout.m_nHeight,               1, 1, 1, DeviceFormats::ConvertToTexFormat(m_eNativeFormat), eTM_None);
	Alignment.volumeStride = CTexture::TextureDataSize(Layout.m_nWidth, Layout.m_nHeight, Layout.m_nDepth, 1, 1, DeviceFormats::ConvertToTexFormat(m_eNativeFormat), eTM_None);

	return Alignment;
}

SResourceDimension CDeviceTexture::GetDimension(u8 mip /*= 0*/, u8 slices /*= 0*/) const
{
	SResourceDimension Dimension = { 0 };
	STextureLayout Layout = GetLayout();

	if (!(Layout.m_nWidth  = Layout.m_nWidth  >> mip)) Layout.m_nWidth  = 1;
	if (!(Layout.m_nHeight = Layout.m_nHeight >> mip)) Layout.m_nHeight = 1;
	if (!(Layout.m_nDepth  = Layout.m_nDepth  >> mip)) Layout.m_nDepth  = 1;

	Dimension.Width  = Layout.m_nWidth;
	Dimension.Height = Layout.m_nHeight;
	Dimension.Depth  = Layout.m_nDepth;
	Dimension.Subresources = (slices ? slices : Layout.m_nArraySize) * (Layout.m_nMips - mip);

	return Dimension;
}

#ifdef DEVRES_USE_STAGING_POOL

void CDeviceTexture::DownloadToStagingResource(u32 nSubRes, StagingHook cbTransfer)
{
	NDrxVulkan::CImageResource* const pImage = Get2DTexture(); // On Vulkan, also safe if 1D or 3D
	SResourceMemoryMapping mapping;
	CDeviceObjectFactory::SelectStagingLayout(pImage, nSubRes, mapping);

	NDrxVulkan::CBufferResource* pStaging = static_cast<NDrxVulkan::CBufferResource*>(m_pStagingResource[0]);
	const bool bCreateStaging = !pStaging || pStaging->GetStride() * pStaging->GetElementCount() < mapping.MemoryLayout.volumeStride;
	if (bCreateStaging)
	{
		VK_ASSERT(cbTransfer && "Invalid persistent download staging resource size");
		if (pImage->GetDevice()->CreateOrReuseStagingResource(pImage, mapping.MemoryLayout.volumeStride, &pStaging, false) != VK_SUCCESS)
		{
			VK_ASSERT(false && "Skipping resource download because no staging buffer is available");
			return;
		}
	}

	GetDeviceObjectFactory().GetCoreCommandList().GetCopyInterface()->DownloadImage(pImage, pStaging, mapping);

	if (cbTransfer)
	{
		// Full sync is unavoidable, we need to wait for the copy just recorded above to finish on the GPU before read-back can occur.
		pImage->GetDevice()->FlushAndWaitForGPU();

		if (bCreateStaging)
		{
			void* const pMappedMemory = pStaging->Map();
			if (pMappedMemory)
			{
				cbTransfer(pMappedMemory, mapping.MemoryLayout.rowStride, mapping.MemoryLayout.planeStride);
				pStaging->Unmap();
			}
			else
			{
				VK_ASSERT(false && "Unable to map staging resource used for download");
			}

			pStaging->Release();
		}
		else
		{
			cbTransfer(m_pStagingMemory[0], mapping.MemoryLayout.rowStride, mapping.MemoryLayout.planeStride);
		}
	}
	else
	{
		HRESULT ret = GetDeviceObjectFactory().IssueFence(m_hStagingFence[0]);
		VK_ASSERT(ret == S_OK && "Failed to issue fence");
	}
}

void CDeviceTexture::DownloadToStagingResource(u32 nSubRes)
{
	VK_ASSERT(m_pStagingResource[0] && "Cannot issue non-callback download without a persistent staging buffer");

	DownloadToStagingResource(nSubRes, nullptr);
}

void CDeviceTexture::UploadFromStagingResource(u32 nSubRes, StagingHook cbTransfer)
{
	NDrxVulkan::CImageResource* const pImage = Get2DTexture(); // On Vulkan, also safe if 1D or 3D
	SResourceMemoryMapping mapping;
	CDeviceObjectFactory::SelectStagingLayout(pImage, nSubRes, mapping);

	NDrxVulkan::CBufferResource* pStaging = static_cast<NDrxVulkan::CBufferResource*>(m_pStagingResource[1]);
	const bool bCreateStaging = !pStaging || pStaging->GetStride() * pStaging->GetElementCount() < mapping.MemoryLayout.volumeStride;
	if (bCreateStaging)
	{
		VK_ASSERT(cbTransfer && "Invalid persistent upload staging resource size");
		if (pImage->GetDevice()->CreateOrReuseStagingResource(pImage, mapping.MemoryLayout.volumeStride, &pStaging, true) != VK_SUCCESS)
		{
			VK_ASSERT(false && "Skipping resource upload because no staging buffer is available");
			return;
		}
	}

	if (cbTransfer)
	{
		if (bCreateStaging)
		{
			void* const pMappedMemory = pStaging->Map();
			if (pMappedMemory)
			{
				cbTransfer(pMappedMemory, mapping.MemoryLayout.rowStride, mapping.MemoryLayout.planeStride);
				pStaging->Unmap();
			}
			else
			{
				VK_ASSERT(false && "Unable to map staging resource used for download");
			}
		}
		else
		{
			// We have to wait for a previous UploadFromStaging/DownloadToStaging to have finished on the GPU, before we can access the staging resource again on CPU.
			GetDeviceObjectFactory().SyncFence(m_hStagingFence[1], true, true);

			cbTransfer(m_pStagingMemory[1], mapping.MemoryLayout.rowStride, mapping.MemoryLayout.planeStride);
		}
	}

	GetDeviceObjectFactory().GetCoreCommandList().GetCopyInterface()->UploadImage(pStaging, pImage, mapping);

	if (cbTransfer && bCreateStaging)
	{
		pStaging->Release();
	}
	else
	{
		HRESULT ret = GetDeviceObjectFactory().IssueFence(m_hStagingFence[1]);
		VK_ASSERT(ret == S_OK && "Failed to issue fence");
	}
}

void CDeviceTexture::UploadFromStagingResource(u32 nSubRes)
{
	VK_ASSERT(m_pStagingResource[1] && "Cannot issue non-callback upload without a persistent staging buffer");

	UploadFromStagingResource(nSubRes, nullptr);
}

void CDeviceTexture::AccessCurrStagingResource(u32 nSubRes, bool forUpload, StagingHook cbTransfer)
{
	NDrxVulkan::CImageResource* const pImage = Get2DTexture(); // On Vulkan, also safe if 1D or 3D
	VK_ASSERT(pImage && m_pStagingResource[forUpload] && m_pStagingMemory[forUpload] && "No persistent staging buffer associated");

	// We have to wait for a previous UploadFromStaging/DownloadToStaging to have finished on the GPU, before we can access the staging resource again on CPU.
	GetDeviceObjectFactory().SyncFence(m_hStagingFence[forUpload], true, true);

	SResourceMemoryMapping mapping;
	CDeviceObjectFactory::SelectStagingLayout(pImage, nSubRes, mapping);
	cbTransfer(m_pStagingMemory[forUpload], mapping.MemoryLayout.rowStride, mapping.MemoryLayout.planeStride);
}

bool CDeviceTexture::AccessCurrStagingResource(u32 nSubRes, bool forUpload)
{
	return GetDeviceObjectFactory().SyncFence(m_hStagingFence[forUpload], false, false) == S_OK;
}

#endif

//=============================================================================

#if DEVRES_USE_PINNING
void CDeviceTexture::WeakPin()
{
}

void CDeviceTexture::Pin()
{
}

void CDeviceTexture::Unpin()
{
}
#endif
