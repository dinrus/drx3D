// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DeviceObjectHelpers.h>

#include <drx3D/Render/D3D/DriverD3D.h>   // CD3D9Renderer

extern CD3D9Renderer gcpRendD3D;

#include <drx3D/Render/D3D/D3DHWShader.h> // CHWShader_D3D

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EShaderStage SDeviceObjectHelpers::GetShaderInstanceInfo(THwShaderInfo& result, ::CShader* pShader, const CDrxNameTSCRC& technique, uint64 rtFlags, u32 mdFlags, u32 mdvFlags, const UPipelineState pipelineState[eHWSC_Num], bool bAllowTesselation)
{
	if (SShaderTechnique* pShaderTechnique = pShader->mfFindTechnique(technique))
	{
		if (pShaderTechnique->m_Passes.empty())
			return EShaderStage_None;

		SShaderPass& shaderPass = pShaderTechnique->m_Passes[0];
		
		// Shader pointers are consecutive
		CHWShader** pHWShaders = &shaderPass.m_VShader;

		// Compile time evaluable, should produce no code
		DRX_ASSERT(eHWSC_Vertex   == (&shaderPass.m_VShader - &shaderPass.m_VShader));
		DRX_ASSERT(eHWSC_Pixel    == (&shaderPass.m_PShader - &shaderPass.m_VShader));
		DRX_ASSERT(eHWSC_Geometry == (&shaderPass.m_GShader - &shaderPass.m_VShader));
		DRX_ASSERT(eHWSC_Domain   == (&shaderPass.m_DShader - &shaderPass.m_VShader));
		DRX_ASSERT(eHWSC_Hull     == (&shaderPass.m_HShader - &shaderPass.m_VShader));
		DRX_ASSERT(eHWSC_Compute  == (&shaderPass.m_CShader - &shaderPass.m_VShader));

		EShaderStage validShaderStages = EShaderStage_None;

		for (EHWShaderClass shaderStage = eHWSC_Vertex; shaderStage < eHWSC_Num; shaderStage = EHWShaderClass(shaderStage + 1))
		{
			if (!bAllowTesselation && (shaderStage == eHWSC_Hull || shaderStage == eHWSC_Domain))
			{
				continue;
			}

			CHWShader_D3D* pHWShaderD3D = reinterpret_cast<CHWShader_D3D*>(pHWShaders[shaderStage]);
			result[shaderStage].pHwShader = pHWShaderD3D;
			result[shaderStage].technique = technique;

			// Special case for nvidia multires shading: auto geometry shader requires vertex shader instance
			if (CVrProjectionUpr::IsMultiResEnabledStatic())
			{
				if (shaderStage == eHWSC_Geometry && pHWShaderD3D)
				{
					// TODO: do this without global variables!! (Needed in mfGenerateScript)
					//CHWShader_D3D::s_pCurInstVS = reinterpret_cast<CHWShader_D3D::SHWSInstance*>(result[eHWSC_Vertex].pHwShaderInstance);
					CHWShader_D3D::s_pCurHWVS = result[eHWSC_Vertex].pHwShader;

					//if (!CHWShader_D3D::s_pCurInstVS || !CHWShader_D3D::s_pCurHWVS)
						//continue;
				}
			}

			if (pHWShaderD3D)
			{
				SShaderCombIdent Ident;
				Ident.m_LightMask = 0;
				Ident.m_RTMask = rtFlags & pHWShaderD3D->m_nMaskAnd_RT | pHWShaderD3D->m_nMaskOr_RT;
				Ident.m_MDMask = mdFlags & (shaderStage != eHWSC_Pixel ? 0xFFFFFFFF : ~HWMD_TEXCOORD_FLAG_MASK);
				Ident.m_MDVMask = ((shaderStage != eHWSC_Pixel) ? mdvFlags : 0) | CParserBin::m_nPlatform;
				Ident.m_GLMask = pHWShaderD3D->m_nMaskGenShader;
				Ident.m_pipelineState = pipelineState ? pipelineState[shaderStage] : UPipelineState();

				bool isShaderValid = false;
				if (auto pInstance = pHWShaderD3D->mfGetInstance(Ident, 0))
				{
					if (pHWShaderD3D->CheckActivation(pShader, pInstance, 0))
					{
						if (pInstance->m_Handle.m_pShader->m_bDisabled)
						{
							result[shaderStage].pHwShader = nullptr;
						}
						else
						{
							result[shaderStage].pHwShaderInstance = pInstance;
							result[shaderStage].pDeviceShader = pInstance->m_Handle.m_pShader->GetHandle();

							validShaderStages |= SHADERSTAGE_FROM_SHADERCLASS(shaderStage);
						}

						isShaderValid = true;
					}
				}
				
				if (!isShaderValid)
				{
					return EShaderStage_None;
				}
			}
		}

		return validShaderStages;
	}

	return EShaderStage_None;
}

bool SDeviceObjectHelpers::CheckTessellationSupport(SShaderItem& shaderItem)
{
	// TTYPE_ZPREPASS doesn't support tessellation
	EShaderTechniqueID techniquesInUse[] = { TTYPE_GENERAL, TTYPE_Z, TTYPE_SHADOWGEN };
	for (auto techType : techniquesInUse)
	{
		if (CheckTessellationSupport(shaderItem, techType))
		{
			return true;
		}
	}
	return false;
}

bool SDeviceObjectHelpers::CheckTessellationSupport(SShaderItem& shaderItem, EShaderTechniqueID techniqueId)
{
	bool bResult = false;
	::CShader* pShader = reinterpret_cast<::CShader*>(shaderItem.m_pShader);
	if (pShader)
	{
		auto pShaderTechnique = pShader->GetTechnique(shaderItem.m_nTechnique, techniqueId, true);
		if (pShaderTechnique)
		{
			SShaderPass& shaderPass = pShaderTechnique->m_Passes[0];
			bResult = (shaderPass.m_DShader && shaderPass.m_HShader);
		}
	}
	return bResult;
}

void SDeviceObjectHelpers::UpdateBuffer(CConstantBuffer* pBuffer, ukk src, size_t size, size_t offset, u32 numDataBlocks)
{
	if (pBuffer)
	{
		pBuffer->UpdateBuffer(src, size, offset, numDataBlocks);
	}
}

SDeviceObjectHelpers::CShaderConstantUpr::CShaderConstantUpr()
{
}

SDeviceObjectHelpers::CShaderConstantUpr::CShaderConstantUpr(CShaderConstantUpr&& other)
{
	std::swap(m_constantBuffers, other.m_constantBuffers);
	std::swap(m_pShaderReflection, other.m_pShaderReflection);

	other.m_constantBuffers.clear();
	other.m_pShaderReflection.reset();
}

void SDeviceObjectHelpers::CShaderConstantUpr::Reset()
{
	m_constantBuffers.clear();
}

bool SDeviceObjectHelpers::CShaderConstantUpr::AllocateShaderReflection(::CShader* pShader, const CDrxNameTSCRC& technique, uint64 rtFlags, EShaderStage shaderStages)
{
	DRX_ASSERT(!m_pShaderReflection);
	m_pShaderReflection = stl::make_unique<SShaderReflection>();

	if (SShaderTechnique* pShaderTechnique = pShader->mfFindTechnique(technique))
	{
		if (!pShaderTechnique->m_Passes.empty())
		{
			SShaderPass& shaderPass = pShaderTechnique->m_Passes[0];

			// Shader pointers are consecutive
			CHWShader** pHWShaders = &shaderPass.m_VShader;
			
			// Compile time evaluable, should produce no code
			DRX_ASSERT(eHWSC_Vertex   == (&shaderPass.m_VShader - &shaderPass.m_VShader));
			DRX_ASSERT(eHWSC_Pixel    == (&shaderPass.m_PShader - &shaderPass.m_VShader));
			DRX_ASSERT(eHWSC_Geometry == (&shaderPass.m_GShader - &shaderPass.m_VShader));
			DRX_ASSERT(eHWSC_Domain   == (&shaderPass.m_DShader - &shaderPass.m_VShader));
			DRX_ASSERT(eHWSC_Hull     == (&shaderPass.m_HShader - &shaderPass.m_VShader));
			DRX_ASSERT(eHWSC_Compute  == (&shaderPass.m_CShader - &shaderPass.m_VShader));

			// Shader stages are ordered by usage-frequency and loop exists according to usage-frequency (VS+PS fast, etc.)
			i32 validShaderStages = shaderStages;
			for (EHWShaderClass shaderClass = eHWSC_Vertex; validShaderStages; shaderClass = EHWShaderClass(shaderClass + 1), validShaderStages >>= 1)
			{
				if (validShaderStages & 1)
				{
					DRX_ASSERT(pHWShaders[shaderClass]);
					DRX_ASSERT_MESSAGE(m_pShaderReflection->bufferCount < MaxReflectedBuffers, "Maximum reflected buffer count exceeded. Feel free to increase if necessary");

				#ifndef RELEASE
					// check for overlap with another CB
					for (auto& cb : m_constantBuffers)
					{
						if (cb.shaderSlot == ReflectedBufferShaderSlot && (cb.shaderStages & SHADERSTAGE_FROM_SHADERCLASS(shaderClass)))
						{
							DRX_ASSERT_MESSAGE(false, "Overlapping constant buffer bindings detected! Cannot assign typed constant buffer " \
							                          "to 'ReflectedBufferShaderSlot' when shader reflection is used.");
						}
					}
				#endif
					
					SConstantBufferBindInfo bindInfo;
					bindInfo.shaderSlot = ReflectedBufferShaderSlot;
					bindInfo.shaderStages = SHADERSTAGE_FROM_SHADERCLASS(shaderClass);
					bindInfo.pBuffer = gcpRendD3D->m_DevBufMan.GetNullConstantBuffer();
					m_constantBuffers.push_back(std::move(bindInfo));

					SShaderReflection::SBufferUpdateContext& updateContext = m_pShaderReflection->bufferUpdateContexts[m_pShaderReflection->bufferCount++];
					updateContext.bufferIndex = m_constantBuffers.size() - 1;
					updateContext.pShaderInstance = nullptr;
					updateContext.shaderClass = shaderClass;
				}
			}
		}
	}

	return true;
}

void SDeviceObjectHelpers::CShaderConstantUpr::InitShaderReflection(CDeviceGraphicsPSO& pipelineState)
{
	DRX_ASSERT(m_pShaderReflection);

	// get maximum buffer size
	i32 maxVectorCount = 0;
	for (i32 i = 0, end = m_pShaderReflection->bufferCount; i < end; ++i)
	{
		auto& updateContext = m_pShaderReflection->bufferUpdateContexts[i];
		CHWShader_D3D::SHWSInstance* pInstance = reinterpret_cast<CHWShader_D3D::SHWSInstance*>(pipelineState.m_pHwShaderInstances[updateContext.shaderClass]);
		maxVectorCount = max(maxVectorCount, pInstance->m_nMaxVecs[eConstantBufferShaderSlot_PerDraw]);
	}

	DrxStackAllocWithSizeVectorCleared(Vec4, maxVectorCount, zeroMem, CDeviceBufferUpr::AlignBufferSizeForStreaming);

	// allocate constant buffers and fill with zeros
	bool allBuffersValid = true;
	for (i32 i = 0, end = m_pShaderReflection->bufferCount; i < end; ++i)
	{
		auto& updateContext = m_pShaderReflection->bufferUpdateContexts[i];
		DRX_ASSERT(updateContext.bufferIndex >= 0);
		DRX_ASSERT(pipelineState.m_pHwShaderInstances[updateContext.shaderClass]);
		
		CHWShader_D3D::SHWSInstance* pInstance = reinterpret_cast<CHWShader_D3D::SHWSInstance*>(pipelineState.m_pHwShaderInstances[updateContext.shaderClass]);
		DRX_ASSERT(pInstance->m_nMaxVecs[eConstantBufferShaderSlot_PerDraw] > 0);           // No per batch shader constants. Shader reflection not required.

		updateContext.pShaderInstance = pInstance;

		const size_t bufferSize = sizeof(Vec4) * pInstance->m_nMaxVecs[eConstantBufferShaderSlot_PerDraw];
		const size_t updateSize = CDeviceBufferUpr::AlignBufferSizeForStreaming(bufferSize);

		if (bufferSize)
		{
			m_constantBuffers[updateContext.bufferIndex].pBuffer = gcpRendD3D->m_DevBufMan.CreateConstantBuffer(bufferSize);
			allBuffersValid &= m_constantBuffers[updateContext.bufferIndex].pBuffer->UpdateBuffer(zeroMem, updateSize);
		}
	}

	m_pShaderReflection->bValid = m_pShaderReflection->bufferCount > 0 && allBuffersValid;
}

void SDeviceObjectHelpers::CShaderConstantUpr::InitShaderReflection(CDeviceComputePSO& pipelineState)
{
	DRX_ASSERT(m_pShaderReflection);
	DRX_ASSERT(pipelineState.m_pHwShaderInstance);
	DRX_ASSERT(m_pShaderReflection->bufferCount == 1);

	auto& updateContext = m_pShaderReflection->bufferUpdateContexts[0];
	CHWShader_D3D::SHWSInstance* pInstance = reinterpret_cast<CHWShader_D3D::SHWSInstance*>(pipelineState.m_pHwShaderInstance);
	updateContext.pShaderInstance = pInstance;

	DRX_ASSERT(pInstance->m_nMaxVecs[eConstantBufferShaderSlot_PerDraw] > 0);           // No per batch shader constants. Shader reflection not required.

	if (pInstance->m_nMaxVecs[eConstantBufferShaderSlot_PerDraw] > 0)
	{
		DrxStackAllocWithSizeVectorCleared(Vec4, pInstance->m_nMaxVecs[eConstantBufferShaderSlot_PerDraw], zeroMem, CDeviceBufferUpr::AlignBufferSizeForStreaming);

		const size_t bufferSize = sizeof(Vec4) * pInstance->m_nMaxVecs[eConstantBufferShaderSlot_PerDraw];
		const size_t updateSize = CDeviceBufferUpr::AlignBufferSizeForStreaming(bufferSize);

		m_constantBuffers[updateContext.bufferIndex].pBuffer = gcpRendD3D->m_DevBufMan.CreateConstantBuffer(bufferSize);
		m_constantBuffers[updateContext.bufferIndex].pBuffer->UpdateBuffer(zeroMem, updateSize);

		m_pShaderReflection->bValid = true;
	}
}

void SDeviceObjectHelpers::CShaderConstantUpr::ReleaseShaderReflection()
{
	ReleaseReflectedBuffers();
	m_pShaderReflection.reset();
}

void SDeviceObjectHelpers::CShaderConstantUpr::ReleaseReflectedBuffers()
{
	if (m_pShaderReflection)
	{
		// remove any buffers that were used for shader reflection
		i32 bufferCount = m_constantBuffers.size();
		for (i32 i = 0, end = m_pShaderReflection->bufferCount; i < end; ++i)
		{
			const auto& updateContext = m_pShaderReflection->bufferUpdateContexts[i];
			m_constantBuffers[updateContext.bufferIndex].shaderStages = EShaderStage_None;
			--bufferCount;
		}

		std::vector<SConstantBufferBindInfo> buffers;

		if (bufferCount > 0)
		{
			buffers.reserve(bufferCount);

			for (auto& cb : m_constantBuffers)
			{
				if (cb.shaderStages != EShaderStage_None)
				{
					buffers.push_back(std::move(cb));
				}
			}
		}

		m_constantBuffers = std::move(buffers);
	}
}

bool SDeviceObjectHelpers::CShaderConstantUpr::IsBufferUsedForReflection(i32 bufferIndex) const
{
	if (m_pShaderReflection)
	{
		static_assert(MaxReflectedBuffers == 2, "Fixme: the following statement works for MaxReflectedBuffers==2");

		return
			m_pShaderReflection->bufferUpdateContexts[0].bufferIndex == bufferIndex ||
			m_pShaderReflection->bufferUpdateContexts[1].bufferIndex == bufferIndex;

	}

	return false;
}

void SDeviceObjectHelpers::CShaderConstantUpr::BeginNamedConstantUpdate()
{
	DRX_ASSERT_MESSAGE(m_pShaderReflection, "Shader reflection not initialized. If you get this on a CRenderPrimitive, please make sure the primitive " \
	                                        "has eFlags_ReflectShaderConstants and CRenderPrimitive::Compile() has been executed successfully");

	for (i32 i = 0, end = m_pShaderReflection->bufferCount; i < end; ++i)
	{
		auto& updateContext = m_pShaderReflection->bufferUpdateContexts[i];
		const SConstantBufferBindInfo& cb = m_constantBuffers[updateContext.bufferIndex];

		if (!cb.pBuffer->IsNullBuffer())
		{
			DRX_ASSERT(updateContext.pMappedData == nullptr);
			updateContext.pMappedData = (Vec4*)cb.pBuffer->BeginWrite();
		}
	}
}

void SDeviceObjectHelpers::CShaderConstantUpr::EndNamedConstantUpdate(const D3DViewPort* pVP)
{
	DRX_ASSERT(m_pShaderReflection);

	for (i32 i = 0, end = m_pShaderReflection->bufferCount; i < end; ++i)
	{
		auto& updateContext = m_pShaderReflection->bufferUpdateContexts[i];
		const SConstantBufferBindInfo& cb = m_constantBuffers[updateContext.bufferIndex];

		if (!cb.pBuffer->IsNullBuffer())
		{
			DRX_ASSERT(updateContext.pMappedData != nullptr);
			auto pShaderInstance = reinterpret_cast<CHWShader_D3D::SHWSInstance*>(updateContext.pShaderInstance);

			// update generic per batch constants
			if (pShaderInstance->m_nParams[0] >= 0)
			{
				SCGParamsGroup& Group = CGParamUpr::s_Groups[pShaderInstance->m_nParams[0]];
				CHWShader_D3D::mfSetParameters(Group.pParams, Group.nParams, eHWSC_Num, -1, (Vec4*)updateContext.pMappedData, cb.pBuffer->m_size, pVP);
			}

			cb.pBuffer->EndWrite();
			updateContext.pMappedData = nullptr;
		}
	}
}

bool SDeviceObjectHelpers::CShaderConstantUpr::SetTypedConstantBuffer(EConstantBufferShaderSlot shaderSlot, CConstantBuffer* pBuffer, EShaderStage shaderStages)
{
	for (i32 i = 0, end = m_constantBuffers.size(); i < end; ++i)
	{
		auto& cb = m_constantBuffers[i];
		if (cb.shaderSlot == shaderSlot && cb.shaderStages == shaderStages)
		{
			if (cb.pBuffer != pBuffer)
			{
				DRX_ASSERT(!IsBufferUsedForReflection(i));

				cb.pBuffer = pBuffer;
				return true;
			}
			return false;
		}
	}

	m_constantBuffers.push_back(SConstantBufferBindInfo());
	SConstantBufferBindInfo& cb = m_constantBuffers.back();
	cb.shaderSlot = shaderSlot;
	cb.shaderStages = shaderStages;
	cb.pBuffer = pBuffer;

	return true;
}

bool SDeviceObjectHelpers::CShaderConstantUpr::SetNamedConstant(const CDrxNameR& paramName, const Vec4 param, EHWShaderClass shaderClass)
{
	return SetNamedConstantArray(paramName, &param, 1, shaderClass);
}

bool SDeviceObjectHelpers::CShaderConstantUpr::SetNamedConstantArray(const CDrxNameR& paramName, const Vec4 params[], u32 numParams, EHWShaderClass shaderClass)
{
	DRX_ASSERT_MESSAGE(m_pShaderReflection, "Flag eFlags_ReflectConstantBuffersFromShader might be required for pass");
	if (!m_pShaderReflection->bValid)
		return false;

	static_assert(MaxReflectedBuffers == 2, "Fixme: the following statement works for MaxReflectedBuffers==2");

	auto& updateContext = m_pShaderReflection->bufferUpdateContexts[0].shaderClass == shaderClass 
		? m_pShaderReflection->bufferUpdateContexts[0] 
		: m_pShaderReflection->bufferUpdateContexts[1];

	DRX_ASSERT(updateContext.pMappedData != nullptr);

	if (auto pShaderInstance = reinterpret_cast<CHWShader_D3D::SHWSInstance*>(updateContext.pShaderInstance))
	{
		for (auto& bindVar : pShaderInstance->m_pBindVars)
		{
			if (paramName == bindVar.m_Name)
			{
				memcpy(&updateContext.pMappedData[bindVar.m_dwBind], params, numParams * sizeof(Vec4));
				return true;
			}
		}
	}

	return false;
}
