// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxVKShaderReflection.hpp>
#include <drx3D/Render/../../../../../Code/Tools/HLSLCrossCompiler/src/reflect.inl>

#include <sstream>
#include <fstream>

HRESULT D3D10CreateBlob(size_t NumBytes, ID3DBlob** ppBuffer)
{
	*ppBuffer = new CDrxVKBlob(NumBytes);
	return S_OK;
}

HRESULT D3DDisassemble(ukk pShader, size_t BytecodeLength, u32 nFlags, ID3DBlob** ppComments, ID3DBlob** ppDisassembly)
{
	return S_OK;
}

void DumpWarning(tukk str)
{
	DrxLogAlways("[Vk Compiler] Warning: %s", str);
	fwrite(str, sizeof(char), strlen(str), stderr);
}

void DumpError(tukk str)
{
	DRX_ASSERT_MESSAGE(false, "[Vk Compiler] ERROR: %s", str);
	DrxLogAlways("[Vk Compiler] ERROR: %s", str);
	fwrite(str, sizeof(char), strlen(str), stderr);
}
 
bool ShellExecute(const STxt& file, const STxt& parameters, const STxt& workingDir = "")
{
#ifdef WIN32
	HANDLE childStdInRead;
	HANDLE childStdInWrite;
	HANDLE childStdOutRead;
	HANDLE childStdOutWrite;

	SECURITY_ATTRIBUTES securityAttributes;
	securityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttributes.bInheritHandle = true;
	securityAttributes.lpSecurityDescriptor = nullptr;

	CreatePipe(&childStdInRead, &childStdInWrite, &securityAttributes, 0);
	SetHandleInformation(childStdInWrite, HANDLE_FLAG_INHERIT, 0);

	CreatePipe(&childStdOutRead, &childStdOutWrite, &securityAttributes, 0);
	SetHandleInformation(childStdOutRead, HANDLE_FLAG_INHERIT, 0);

	PROCESS_INFORMATION processInfo = { 0 };
	STARTUPINFO startUpInfo = { 0 };

	startUpInfo.cb = sizeof(STARTUPINFO);
	startUpInfo.hStdInput = childStdInRead;
	startUpInfo.hStdOutput = childStdOutWrite;
	startUpInfo.hStdError = childStdOutWrite;
	startUpInfo.dwFlags |= STARTF_USESTDHANDLES;

	char parametersTemp[4096];
	sprintf_s(parametersTemp, 4096, "%s %s", file.c_str(), parameters.c_str());

	STxt output;

	CreateProcess(nullptr, parametersTemp, nullptr, nullptr, true, CREATE_NO_WINDOW, nullptr, workingDir.size() == 0 ? nullptr : workingDir.c_str(), &startUpInfo, &processInfo);
	while (WaitForSingleObject(processInfo.hProcess, 100) == WAIT_TIMEOUT)
	{
		u64 size = GetFileSize(childStdOutRead, nullptr);

		if (size != 0)
		{
			STxt outputTemp;
			outputTemp.resize(size);

			u64 readBytes;
			ReadFile(childStdOutRead, &outputTemp[0], size, &readBytes, nullptr);

			output += outputTemp;
		}
	}

	u64 size = GetFileSize(childStdOutRead, nullptr);

	if (size != 0)
	{
		STxt outputTemp;
		outputTemp.resize(size);

		u64 readBytes;
		ReadFile(childStdOutRead, &outputTemp[0], size, &readBytes, nullptr);

		output += outputTemp;
	}

	u64 exitCode;
	GetExitCodeProcess(processInfo.hProcess, &exitCode);

	if (exitCode != 0)
	{
		// convert 'ERROR' to 'error' as remote compiler 
		// doesn't report upper case version
		size_t curPos = output.find("ERROR:", 0);
		while (curPos != STxt::npos)
		{
			output.replace(curPos, 5, "error");
			curPos = output.find("ERROR:", curPos + 1);
		}

		DumpError(output.c_str());
	}
	else
	{
		if (output.size() > 0)
			DumpWarning(output.c_str());
	}

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	CloseHandle(childStdInRead);
	CloseHandle(childStdInWrite);
	CloseHandle(childStdOutRead);
	CloseHandle(childStdOutWrite);

	return exitCode == 0;
#else
	DrxFatalError("Cannot run the process! ShellExecute is implemented ONLY for Windows Platform.");
	return false;
#endif
}

static tukk GetGLSLANGTargetName(tukk pDxTarget)
{
	if (strncmp(pDxTarget, "vs", 2) == 0)
	{
		return "vert";
	}
	else if (strncmp(pDxTarget, "ps", 2) == 0)
	{
		return "frag";
	}
	else if (strncmp(pDxTarget, "gs", 2) == 0)
	{
		return "geom";
	}
	else if (strncmp(pDxTarget, "ds", 2) == 0)
	{
		return "tesc";
	}
	else if (strncmp(pDxTarget, "hs", 2) == 0)
	{
		return "tese";
	}
	else if (strncmp(pDxTarget, "cs", 2) == 0)
	{
		return "comp";
	}

	return "invalid";
}

#define INPUT_HLSL_FORMAT                    ".in"
#define OUTPUT_SPIRV_FORMAT                  ".out"
#define OUTPUT_HUMAN_READABLE_SPIRV_FORMAT   ".h_spv"
HRESULT D3DCompile(_In_reads_bytes_(SrcDataSize) LPCVOID pSrcData, _In_ SIZE_T SrcDataSize, _In_opt_ LPCSTR pSourceName, CONST D3D_SHADER_MACRO* pDefines,
                   _In_opt_ ID3DInclude* pInclude, _In_opt_ LPCSTR pEntrypoint, _In_ LPCSTR pTarget, _In_ UINT Flags1, _In_ UINT Flags2, _Out_ ID3DBlob** ppCode,
                   _Always_(_Outptr_opt_result_maybenull_) ID3DBlob** ppErrorMsgs)
{
	static const STxt tmpShaderPath = "%USER%/shaders/temp/";
	if (!DrxDirectoryExists(tmpShaderPath.c_str()))
	{
		gEnv->pDrxPak->MakeDir(tmpShaderPath.c_str());
	}
	static char tmpShaderPathAbsloutePath[2048];
	gEnv->pDrxPak->AdjustFileName(tmpShaderPath.c_str(), tmpShaderPathAbsloutePath, IDrxPak::FLAGS_FOR_WRITING);
	
	STxt shaderFileLocationWithoutFormat = tmpShaderPathAbsloutePath;
	shaderFileLocationWithoutFormat += pSourceName;
	shaderFileLocationWithoutFormat += "_";
	shaderFileLocationWithoutFormat += pEntrypoint;

	std::ofstream shaderFile;
	shaderFile.open(shaderFileLocationWithoutFormat + INPUT_HLSL_FORMAT);
	if (!shaderFile.good())
	{
		DRX_ASSERT_MESSAGE(shaderFile.good(), ("Cannot create " + shaderFileLocationWithoutFormat + INPUT_HLSL_FORMAT + " shader file.").c_str());
		return E_FAIL;
	}

	shaderFile.write((tukk)pSrcData, SrcDataSize);
	shaderFile.close();

	STxt vkShaderCompiler(CRendererCVars::CV_r_VkShaderCompiler->GetString());
	if (vkShaderCompiler == STR_VK_SHADER_COMPILER_HLSLCC)
	{
		char params[1001];
		drx_sprintf(params, "%s %s \"%s%s\" \"%s%s\"",
			pEntrypoint,
			pTarget,
			shaderFileLocationWithoutFormat.c_str(), OUTPUT_SPIRV_FORMAT,
			shaderFileLocationWithoutFormat.c_str(), INPUT_HLSL_FORMAT
			);

		ShellExecute("%ENGINE%\\..\\Tools\\RemoteShaderCompiler\\Compiler\\SPIRV\\V002\\HLSL2SPIRV.exe", params, "%ENGINE%\\..\\Tools\\RemoteShaderCompiler\\Compiler\\SPIRV\\V002");
	}
	else if (vkShaderCompiler == STR_VK_SHADER_COMPILER_DXC)
	{
		bool showWarnings = false;
		
		char params[1001];
		drx_sprintf(params, " -spirv -O3 -Zpr \"%s%s\" -Fo \"%s%s\" -Fc \"%s%s\" -T %s -E \"%s\" %s %s",
			shaderFileLocationWithoutFormat.c_str(), INPUT_HLSL_FORMAT,
			shaderFileLocationWithoutFormat.c_str(), OUTPUT_SPIRV_FORMAT,
			shaderFileLocationWithoutFormat.c_str(), OUTPUT_HUMAN_READABLE_SPIRV_FORMAT,
			pTarget,
			pEntrypoint,
			strncmp(pTarget, "vs", 2) == 0 ? "-fvk-invert-y" : "",
			showWarnings ? "" : "-no-warnings");

		ShellExecute("%ENGINE%\\..\\Tools\\RemoteShaderCompiler\\Compiler\\SPIRV\\V003\\dxc\\dxc.exe", params);
	}
	else if (vkShaderCompiler == STR_VK_SHADER_COMPILER_GLSLANG)
	{
		STxt targetEnv = "vulkan1.0";
		
		char params[1001];
		drx_sprintf(params, " -D -fhlsl_functionality1 \"%s%s\" -o \"%s%s\" --target-env %s -S %s -e %s -V100 %s",
			shaderFileLocationWithoutFormat.c_str(), INPUT_HLSL_FORMAT,
			shaderFileLocationWithoutFormat.c_str(), OUTPUT_SPIRV_FORMAT,
			targetEnv.c_str(),
			GetGLSLANGTargetName(pTarget),
			pEntrypoint,
			strncmp(pTarget, "vs", 2) == 0 ? "--invert-y" : "");

		ShellExecute("%ENGINE%\\..\\Tools\\RemoteShaderCompiler\\Compiler\\SPIRV\\V003\\glslang\\glslangValidator.exe", params);
	}

	std::ifstream spirvShaderFile(shaderFileLocationWithoutFormat + OUTPUT_SPIRV_FORMAT, std::ios::binary);
	if (!spirvShaderFile.good())
	{
		DRX_ASSERT_MESSAGE(false, (shaderFileLocationWithoutFormat + OUTPUT_SPIRV_FORMAT + " cannot be opened").c_str());
		return E_FAIL;
	}

	STxt spirvSrcStr(
		(std::istreambuf_iterator<char>(spirvShaderFile)),
		std::istreambuf_iterator<char>()
	);

	D3DCreateBlob(spirvSrcStr.size(), ppCode);
	memcpy((*ppCode)->GetBufferPointer(), spirvSrcStr.c_str(), spirvSrcStr.size());

	return S_OK;
}

HRESULT D3DReflect(ukk pShaderBytecode, size_t BytecodeLength, UINT pInterface, uk * ppReflector)
{
	*ppReflector = new CDrxVKShaderReflection(pShaderBytecode, BytecodeLength);
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

CDrxVKBlob::CDrxVKBlob(size_t numBytes)
{
	m_pData = new u8[numBytes];
	m_size = numBytes;
}

CDrxVKBlob::~CDrxVKBlob()
{
	delete[] m_pData;
}

//////////////////////////////////////////////////////////////////////

CDrxVKShaderReflection::CDrxVKShaderReflection(ukk pShaderBytecode, size_t BytecodeLength)
{
	DRX_ASSERT(pShaderBytecode && BytecodeLength > 0 && BytecodeLength % 4 == 0);

	u32k* pByteCodeStart = reinterpret_cast<u32k*>(pShaderBytecode);
	u32k* pByteCodeEnd = pByteCodeStart + BytecodeLength / 4;

	STxt vkShaderCompiler(CRendererCVars::CV_r_VkShaderCompiler->GetString());
	m_pCompiler = stl::make_unique<spirv_cross::Compiler>(std::vector<u32>(pByteCodeStart, pByteCodeEnd));
	m_shaderResources = m_pCompiler->get_shader_resources();

	// prepare input data
	if (m_pCompiler->get_execution_model() == spv::ExecutionModelVertex)
	{
		for (auto& input : m_shaderResources.stage_inputs)
		{
			uint32_t location = m_pCompiler->get_decoration(input.id, spv::DecorationLocation);
			uint32_t set = m_pCompiler->get_decoration(input.id, spv::DecorationDescriptorSet);
			uint32_t binding = m_pCompiler->get_decoration(input.id, spv::DecorationBinding);

			SInputParameter inputParam;
			inputParam.semanticIndex = 0;

			if (vkShaderCompiler == STR_VK_SHADER_COMPILER_HLSLCC)
			{
				if (sscanf_s(input.name.c_str(), "dcl_Input%d_%[a-zA-Z]%d", &inputParam.attributeLocation, inputParam.semanticName, sizeof(inputParam.semanticName), &inputParam.semanticIndex) == 3)
				{
					UnformatVariableName(inputParam.semanticName);
					m_shaderInputs.push_back(inputParam);
				}
			}
			else if (vkShaderCompiler == STR_VK_SHADER_COMPILER_DXC)
			{
				if(sscanf_s(input.name.c_str(), "in_var_%[a-zA-Z]%d", inputParam.semanticName, sizeof(inputParam.semanticName), &inputParam.semanticIndex) >= 1)
				{
					inputParam.attributeLocation = location;
					m_shaderInputs.push_back(inputParam);
				}
			}
			else if (vkShaderCompiler == STR_VK_SHADER_COMPILER_GLSLANG)
			{
				STxt semanticsName = m_pCompiler->get_decoration_string(input.id, spv::DecorationHlslSemanticGOOGLE);
				if (sscanf_s(semanticsName.c_str(), "%[a-zA-Z]%d", inputParam.semanticName, sizeof(inputParam.semanticName), &inputParam.semanticIndex) >= 1)
				{
					inputParam.attributeLocation = location;
					m_shaderInputs.push_back(inputParam);
				}

				uint32_t no = m_pCompiler->get_decoration(input.id, spv::DecorationHlslCounterBufferGOOGLE);
			}
		}
	}

	// NOTE: just for completeness. most of this stuff is untested!!
	for (auto pResourceList : { &m_shaderResources.sampled_images, &m_shaderResources.separate_images, &m_shaderResources.separate_samplers })
	{
		for (UINT localListIndex = 0; localListIndex < pResourceList->size(); ++localListIndex)
		{
			spirv_cross::Resource& resource = pResourceList->at(localListIndex);

			const spirv_cross::SPIRType& resourceType = m_pCompiler->get_type(resource.type_id);
			u32 descriptorIndex = m_pCompiler->get_decoration(resource.base_type_id, spv::DecorationBinding);
			u32 setIndex = m_pCompiler->get_decoration(resource.base_type_id, spv::DecorationDescriptorSet);

			struct isEqual
			{
				tukk p;
				isEqual(tukk s) : p(s) {}
				bool operator() ( const SResourceBinding& b) {
					return !strcmp(b.semanticName, p); }
			};

			// semanticType:
			// "ConstantBuffer";
			// "Texture";
			// "Sampler";
			// "Unordered";
			// "Buffer";
			SResourceBinding resourceBindingT;
			SResourceBinding resourceBindingS;

			resourceBindingT.semanticType = D3D10_SIT_TEXTURE;
			resourceBindingS.semanticType = D3D10_SIT_SAMPLER;

			if (CRendererCVars::CV_r_VkShaderCompiler && strcmp(CRendererCVars::CV_r_VkShaderCompiler->GetString(), STR_VK_SHADER_COMPILER_HLSLCC))
			{
				char semanticTypeT[128];
				char semanticTypeS[128];

				if (sscanf_s(resource.name.c_str(), "dcl_%[a-zA-Z]%d_%[a-zA-Z]_%[a-zA-Z]%d_%[a-zA-Z]",
					semanticTypeT, sizeof(semanticTypeT), &resourceBindingT.bindPoint, resourceBindingT.semanticName, sizeof(resourceBindingT.semanticName),
					semanticTypeS, sizeof(semanticTypeS), &resourceBindingS.bindPoint, resourceBindingS.semanticName, sizeof(resourceBindingS.semanticName)) == 6)
				{
					UnformatVariableName(resourceBindingT.semanticName);
					UnformatVariableName(resourceBindingS.semanticName);

					if (std::find_if(m_shaderBindings.begin(), m_shaderBindings.end(), isEqual(resourceBindingT.semanticName)) == m_shaderBindings.end())
						m_shaderBindings.push_back(resourceBindingT);
					if (std::find_if(m_shaderBindings.begin(), m_shaderBindings.end(), isEqual(resourceBindingS.semanticName)) == m_shaderBindings.end())
						m_shaderBindings.push_back(resourceBindingS);
				}

				else if (sscanf_s(resource.name.c_str(), "dcl_%[a-zA-Z]%d_%[a-zA-Z]",
					semanticTypeT, sizeof(semanticTypeT), &resourceBindingT.bindPoint, resourceBindingT.semanticName, sizeof(resourceBindingT.semanticName)) == 3)
				{
					UnformatVariableName(resourceBindingT.semanticName);

					if (std::find_if(m_shaderBindings.begin(), m_shaderBindings.end(), isEqual(resourceBindingT.semanticName)) == m_shaderBindings.end())
						m_shaderBindings.push_back(resourceBindingT);
				}
				else
				{
					SResourceBinding resourceBinding;
					resourceBinding.bindPoint = 0;
					strcpy(resourceBinding.semanticName, resource.name.c_str());
					switch (resourceType.basetype)
					{
					case spirv_cross::SPIRType::SampledImage:
					case spirv_cross::SPIRType::Sampler:
						resourceBinding.semanticType = D3D10_SIT_SAMPLER;
						break;
					case spirv_cross::SPIRType::Image:
						resourceBinding.semanticType = D3D10_SIT_TEXTURE;
						break;
					default:
						VK_NOT_IMPLEMENTED;
					}

					if (std::find_if(m_shaderBindings.begin(), m_shaderBindings.end(), isEqual(resourceBinding.semanticName)) == m_shaderBindings.end())
						m_shaderBindings.push_back(resourceBinding);
				}
			}
			else
			{

				auto type = m_pCompiler->get_type(resource.type_id);
				auto binding = m_pCompiler->get_decoration(resource.id, spv::DecorationBinding);
				auto set = m_pCompiler->get_decoration(resource.id, spv::DecorationDescriptorSet);

				resourceBindingT.bindPoint = binding;
				resourceBindingS.bindPoint = binding;

				strcpy(resourceBindingT.semanticName, resource.name.c_str());
				strcpy(resourceBindingS.semanticName, resource.name.c_str());

				if (type.basetype == spirv_cross::SPIRType::BaseType::SampledImage)
				{
					if (std::find_if(m_shaderBindings.begin(), m_shaderBindings.end(), isEqual(resourceBindingT.semanticName)) == m_shaderBindings.end())
						m_shaderBindings.push_back(resourceBindingT);
					if (std::find_if(m_shaderBindings.begin(), m_shaderBindings.end(), isEqual(resourceBindingS.semanticName)) == m_shaderBindings.end())
						m_shaderBindings.push_back(resourceBindingS);
				}
				else if (type.basetype == spirv_cross::SPIRType::BaseType::Image)
				{
					if (std::find_if(m_shaderBindings.begin(), m_shaderBindings.end(), isEqual(resourceBindingT.semanticName)) == m_shaderBindings.end())
						m_shaderBindings.push_back(resourceBindingT);
				}
				else if (type.basetype == spirv_cross::SPIRType::BaseType::Sampler)
				{
					if (std::find_if(m_shaderBindings.begin(), m_shaderBindings.end(), isEqual(resourceBindingS.semanticName)) == m_shaderBindings.end())
						m_shaderBindings.push_back(resourceBindingS);
				}
			}
		}
	}

	m_constantBuffers.resize(m_shaderResources.uniform_buffers.size());
}

HRESULT STDMETHODCALLTYPE CDrxVKShaderReflection::GetDesc(D3D11_SHADER_DESC* pDesc)
{
	ZeroStruct(*pDesc);
	pDesc->ConstantBuffers = m_shaderResources.uniform_buffers.size();
	pDesc->InputParameters = m_shaderInputs.size();
	pDesc->BoundResources =  m_shaderBindings.size();

	return S_OK;
}

ID3D11ShaderReflectionConstantBuffer* STDMETHODCALLTYPE CDrxVKShaderReflection::GetConstantBufferByIndex(UINT Index)
{
	DRX_ASSERT(Index < m_shaderResources.uniform_buffers.size());
	if (!m_constantBuffers[Index])
	{
		m_constantBuffers[Index].reset(new CDrxVKShaderReflectionConstantBuffer(this, m_shaderResources.uniform_buffers[Index]));
	}
	return m_constantBuffers[Index].get();
}

ID3D11ShaderReflectionConstantBuffer* STDMETHODCALLTYPE CDrxVKShaderReflection::GetConstantBufferByName(LPCSTR Name)
{
	VK_NOT_IMPLEMENTED
	return nullptr;
}

HRESULT STDMETHODCALLTYPE CDrxVKShaderReflection::GetResourceBindingDesc(UINT ResourceIndex, D3D11_SHADER_INPUT_BIND_DESC* pDesc)
{
	DRX_ASSERT(ResourceIndex < m_shaderBindings.size());
	const auto& input = m_shaderBindings[ResourceIndex];

	ZeroStruct(*pDesc);
	pDesc->Name = input.semanticName;
	pDesc->Type = (D3D_SHADER_INPUT_TYPE)input.semanticType;
	pDesc->Dimension = D3D10_SRV_DIMENSION_UNKNOWN;
	pDesc->BindCount = 1;
	pDesc->BindPoint = input.bindPoint;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDrxVKShaderReflection::GetInputParameterDesc(UINT ParameterIndex, D3D11_SIGNATURE_PARAMETER_DESC* pDesc)
{
	DRX_ASSERT(ParameterIndex < m_shaderInputs.size());
	const auto& input = m_shaderInputs[ParameterIndex];

	ZeroStruct(*pDesc);
	pDesc->SemanticName = input.semanticName;
	pDesc->SemanticIndex = input.semanticIndex;
	pDesc->ReadWriteMask = 0xFF;
	pDesc->AttributeLocation = input.attributeLocation;

	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDrxVKShaderReflection::GetOutputParameterDesc(UINT ParameterIndex, D3D11_SIGNATURE_PARAMETER_DESC* pDesc)
{
	VK_NOT_IMPLEMENTED
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CDrxVKShaderReflection::GetPatchConstantParameterDesc(UINT ParameterIndex, D3D11_SIGNATURE_PARAMETER_DESC* pDesc)
{
	VK_NOT_IMPLEMENTED
	return S_OK;
}

ID3D11ShaderReflectionVariable* STDMETHODCALLTYPE CDrxVKShaderReflection::GetVariableByName(LPCSTR Name)
{
	VK_NOT_IMPLEMENTED
	return nullptr;
}

HRESULT STDMETHODCALLTYPE CDrxVKShaderReflection::GetResourceBindingDescByName(LPCSTR Name, D3D11_SHADER_INPUT_BIND_DESC* pDesc)
{
	VK_NOT_IMPLEMENTED
	return S_OK;
}

UINT STDMETHODCALLTYPE CDrxVKShaderReflection::GetMovInstructionCount()
{
	VK_NOT_IMPLEMENTED
	return S_OK;
}
UINT STDMETHODCALLTYPE CDrxVKShaderReflection::GetMovcInstructionCount()
{
	VK_NOT_IMPLEMENTED
	return S_OK;
}
UINT STDMETHODCALLTYPE CDrxVKShaderReflection::GetConversionInstructionCount()
{
	VK_NOT_IMPLEMENTED
	return S_OK;
}
UINT STDMETHODCALLTYPE CDrxVKShaderReflection::GetBitwiseInstructionCount()
{
	VK_NOT_IMPLEMENTED
	return S_OK;
}
D3D_PRIMITIVE STDMETHODCALLTYPE CDrxVKShaderReflection::GetGSInputPrimitive()
{
	VK_NOT_IMPLEMENTED
	return D3D_PRIMITIVE_UNDEFINED;
}
BOOL STDMETHODCALLTYPE CDrxVKShaderReflection::IsSampleFrequencyShader()
{
	VK_NOT_IMPLEMENTED
	return S_OK;
}
UINT STDMETHODCALLTYPE CDrxVKShaderReflection::GetNumInterfaceSlots()
{
	VK_NOT_IMPLEMENTED
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CDrxVKShaderReflection::GetMinFeatureLevel(D3D_FEATURE_LEVEL* pLevel)
{
	VK_NOT_IMPLEMENTED
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

CDrxVKShaderReflectionVariable::CDrxVKShaderReflectionVariable(CDrxVKShaderReflection* pShaderReflection, const spirv_cross::Resource& constantBuffer, u32 memberIndex, bool bInUse)
	: m_pShaderReflection(pShaderReflection)
	, m_constantBuffer(constantBuffer)
	, m_memberIndex(memberIndex)
	, m_bInUse(bInUse)
{
	spirv_cross::Compiler& compiler = *pShaderReflection->m_pCompiler;

	const STxt& variableName = compiler.get_member_name(constantBuffer.base_type_id, memberIndex);

	if (CRendererCVars::CV_r_VkShaderCompiler && strcmp(CRendererCVars::CV_r_VkShaderCompiler->GetString(), STR_VK_SHADER_COMPILER_HLSLCC) == 0)
	{
		strncpy(m_name, variableName.c_str() + 1 /* skip ShaderTypePrefix() */, sizeof(m_name));
		UnformatVariableName(m_name);
	}
	else
	{
		strncpy(m_name, variableName.c_str(), sizeof(m_name));
	}
}

HRESULT STDMETHODCALLTYPE CDrxVKShaderReflectionVariable::GetDesc(D3D11_SHADER_VARIABLE_DESC* pDesc)
{
	spirv_cross::Compiler& compiler = *m_pShaderReflection->m_pCompiler;
	const spirv_cross::SPIRType& structType = compiler.get_type(m_constantBuffer.type_id);

	ZeroStruct(*pDesc);
	pDesc->Name = m_name;
	pDesc->StartOffset = compiler.get_member_decoration(m_constantBuffer.base_type_id, m_memberIndex, spv::DecorationOffset);
	pDesc->Size = compiler.get_declared_struct_member_size(structType, m_memberIndex);
	pDesc->uFlags = m_bInUse ? D3D10_SVF_USED : 0;

	return S_OK;
}

ID3D11ShaderReflectionType* STDMETHODCALLTYPE CDrxVKShaderReflectionVariable::GetType()
{
	if (!m_pType)
	{
		spirv_cross::Compiler& compiler = *m_pShaderReflection->m_pCompiler;
		const spirv_cross::SPIRType& structType = compiler.get_type(m_constantBuffer.type_id);

		DRX_ASSERT(m_memberIndex < structType.member_types.size());
		uint32_t memberTypeId = structType.member_types[m_memberIndex];

		m_pType.reset(new CDrxVKShaderReflectionType(m_pShaderReflection, memberTypeId));
	}
	return m_pType.get();
}

ID3D11ShaderReflectionConstantBuffer* STDMETHODCALLTYPE CDrxVKShaderReflectionVariable::GetBuffer()
{
	VK_NOT_IMPLEMENTED
	return nullptr;
}

UINT STDMETHODCALLTYPE CDrxVKShaderReflectionVariable::GetInterfaceSlot(UINT uArrayIndex)
{
	VK_NOT_IMPLEMENTED
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

CDrxVKShaderReflectionType::CDrxVKShaderReflectionType(CDrxVKShaderReflection* pShaderReflection, uint32_t typeId)
{
	spirv_cross::Compiler& compiler = *pShaderReflection->m_pCompiler;
	const auto& memberType = compiler.get_type(typeId);

	ZeroStruct(m_Desc);

	switch (memberType.basetype)
	{
	case spirv_cross::SPIRType::Boolean:
	case spirv_cross::SPIRType::Char:
	case spirv_cross::SPIRType::Int:
	case spirv_cross::SPIRType::UInt:
	case spirv_cross::SPIRType::Int64:
	case spirv_cross::SPIRType::UInt64:
	case spirv_cross::SPIRType::Float:
	case spirv_cross::SPIRType::Double:
		m_Desc.Class = memberType.vecsize > 1 ? D3D10_SVC_VECTOR : D3D10_SVC_SCALAR;
		break;
	default:
		m_Desc.Class = D3D10_SVC_OBJECT;
		break;
	}
}

HRESULT STDMETHODCALLTYPE CDrxVKShaderReflectionType::GetDesc(D3D11_SHADER_TYPE_DESC* pDesc)
{
	*pDesc = m_Desc;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////

CDrxVKShaderReflectionConstantBuffer::CDrxVKShaderReflectionConstantBuffer(CDrxVKShaderReflection* pShaderReflection, const spirv_cross::Resource& resource)
	: m_pShaderReflection(std::move(pShaderReflection))
	, m_resource(resource)
{
	spirv_cross::Compiler& compiler = *m_pShaderReflection->m_pCompiler;

	m_usedVariables = compiler.get_active_buffer_ranges(m_resource.id);

	STxt name = !m_resource.name.empty() ? m_resource.name : compiler.get_fallback_name(m_resource.base_type_id);
	i32 bindPoint = compiler.get_decoration(m_resource.id, spv::DecorationBinding);
	i32 set = compiler.get_decoration(m_resource.id, spv::DecorationDescriptorSet);
	//i32 space = compiler.get_decoration(m_resource.id, spv::DecorationSpecId);

	
	STxt vkShaderCompiler(CRendererCVars::CV_r_VkShaderCompiler->GetString());

	if (vkShaderCompiler == STR_VK_SHADER_COMPILER_HLSLCC)
	{
		if (sscanf_s(name.c_str(), "dcl_ConstantBuffer%d_%[a-zA-Z]", &bindPoint, m_name, sizeof(m_name)) == 2)
		{
			UnformatVariableName(m_name);
		}
	}
	else if (vkShaderCompiler == STR_VK_SHADER_COMPILER_DXC)
	{
		if (sscanf_s(name.c_str(), "type_%[a-zA-Z_]", m_name, sizeof(m_name)) != 1)
		{
			DRX_ASSERT_MESSAGE(false, "Constant buffer name format is not covered.");
		}
	}
	else if (vkShaderCompiler == STR_VK_SHADER_COMPILER_GLSLANG)
	{
		drx_strcpy(m_name, name.c_str());
	}

	const spirv_cross::SPIRType& structType = compiler.get_type(m_resource.type_id);
	m_variables.resize(structType.member_types.size());
}

HRESULT STDMETHODCALLTYPE CDrxVKShaderReflectionConstantBuffer::GetDesc(D3D11_SHADER_BUFFER_DESC* pDesc)
{
	spirv_cross::Compiler& compiler = *m_pShaderReflection->m_pCompiler;
	const spirv_cross::SPIRType& structType = compiler.get_type(m_resource.type_id);

	ZeroStruct(*pDesc);
	pDesc->Name = m_name;
	pDesc->Type = D3D_CT_CBUFFER;
	pDesc->Variables = structType.member_types.size();
	if (pDesc->Variables > 0)
	{
		pDesc->Size = compiler.get_declared_struct_size(structType);
	}

	return S_OK;
}

ID3D11ShaderReflectionVariable* STDMETHODCALLTYPE CDrxVKShaderReflectionConstantBuffer::GetVariableByIndex(UINT Index)
{
	if (!m_variables[Index])
	{
		spirv_cross::Compiler& compiler = *m_pShaderReflection->m_pCompiler;
		const spirv_cross::SPIRType& structType = compiler.get_type(m_resource.type_id);

		DRX_ASSERT(Index < structType.member_types.size());

		bool bInUse = false;
		for (auto& range : m_usedVariables)
		{
			if (range.index == Index)
			{
				bInUse = true;
				break;
			}
		}

		m_variables[Index].reset(new CDrxVKShaderReflectionVariable(m_pShaderReflection, m_resource, Index, bInUse));
	}

	return m_variables[Index].get();
}

ID3D11ShaderReflectionVariable* STDMETHODCALLTYPE CDrxVKShaderReflectionConstantBuffer::GetVariableByName(LPCSTR Name)
{
	VK_NOT_IMPLEMENTED
	return nullptr;
}
