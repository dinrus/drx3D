// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLShaderReflection.cpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Definition of the DXGL wrappers for D3D11 shader
//               reflection interfaces
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/CDrxDXGLShaderReflection.hpp>
#include <drx3D/Render/Implementation/GLShader.hpp>

////////////////////////////////////////////////////////////////////////////////
// CDrxDXGLShaderReflectionVariable
////////////////////////////////////////////////////////////////////////////////

struct CDrxDXGLShaderReflectionVariable::Impl
{
	NDrxOpenGL::SShaderReflectionVariable* m_pVariable;
};

CDrxDXGLShaderReflectionVariable::CDrxDXGLShaderReflectionVariable()
	: m_pImpl(new Impl())
{
	DXGL_INITIALIZE_INTERFACE(D3D11ShaderReflectionVariable)
	DXGL_INITIALIZE_INTERFACE(D3D11ShaderReflectionType)
}

CDrxDXGLShaderReflectionVariable::~CDrxDXGLShaderReflectionVariable()
{
	delete m_pImpl;
}

bool CDrxDXGLShaderReflectionVariable::Initialize(uk pvData)
{
	m_pImpl->m_pVariable = static_cast<NDrxOpenGL::SShaderReflectionVariable*>(pvData);
	return true;
}

HRESULT CDrxDXGLShaderReflectionVariable::GetDesc(D3D11_SHADER_VARIABLE_DESC* pDesc)
{
	(*pDesc) = m_pImpl->m_pVariable->m_kDesc;
	return S_OK;
}

ID3D11ShaderReflectionType* CDrxDXGLShaderReflectionVariable::GetType()
{
	ID3D11ShaderReflectionType* pType;
	ToInterface(&pType, this);
	return pType;
}

ID3D11ShaderReflectionConstantBuffer* CDrxDXGLShaderReflectionVariable::GetBuffer()
{
	DXGL_NOT_IMPLEMENTED
	return NULL;
}

UINT CDrxDXGLShaderReflectionVariable::GetInterfaceSlot(UINT uArrayIndex)
{
	DXGL_NOT_IMPLEMENTED
	return 0;
}

HRESULT CDrxDXGLShaderReflectionVariable::GetDesc(D3D11_SHADER_TYPE_DESC* pDesc)
{
	(*pDesc) = m_pImpl->m_pVariable->m_kType;
	return S_OK;
}

ID3D11ShaderReflectionType* CDrxDXGLShaderReflectionVariable::GetMemberTypeByIndex(UINT Index)
{
	DXGL_NOT_IMPLEMENTED
	return NULL;
}

ID3D11ShaderReflectionType* CDrxDXGLShaderReflectionVariable::GetMemberTypeByName(LPCSTR Name)
{
	DXGL_NOT_IMPLEMENTED
	return NULL;
}

LPCSTR CDrxDXGLShaderReflectionVariable::GetMemberTypeName(UINT Index)
{
	DXGL_NOT_IMPLEMENTED
	return NULL;
}

HRESULT CDrxDXGLShaderReflectionVariable::IsEqual(ID3D11ShaderReflectionType* pType)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

ID3D11ShaderReflectionType* CDrxDXGLShaderReflectionVariable::GetSubType()
{
	DXGL_NOT_IMPLEMENTED
	return NULL;
}

ID3D11ShaderReflectionType* CDrxDXGLShaderReflectionVariable::GetBaseClass()
{
	DXGL_NOT_IMPLEMENTED
	return NULL;
}

UINT CDrxDXGLShaderReflectionVariable::GetNumInterfaces()
{
	DXGL_NOT_IMPLEMENTED
	return 0;
}

ID3D11ShaderReflectionType* CDrxDXGLShaderReflectionVariable::GetInterfaceByIndex(UINT uIndex)
{
	DXGL_NOT_IMPLEMENTED
	return NULL;
}

HRESULT CDrxDXGLShaderReflectionVariable::IsOfType(ID3D11ShaderReflectionType* pType)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

HRESULT CDrxDXGLShaderReflectionVariable::ImplementsInterface(ID3D11ShaderReflectionType* pBase)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////
// CDrxDXGLShaderReflectionConstBuffer
////////////////////////////////////////////////////////////////////////////////

struct CDrxDXGLShaderReflectionConstBuffer::Impl
{
	typedef std::vector<_smart_ptr<CDrxDXGLShaderReflectionVariable>> TVariables;
	TVariables m_kVariables;
	NDrxOpenGL::SShaderReflectionConstBuffer* m_pConstBuffer;
};

CDrxDXGLShaderReflectionConstBuffer::CDrxDXGLShaderReflectionConstBuffer()
	: m_pImpl(new Impl())
{
	DXGL_INITIALIZE_INTERFACE(D3D11ShaderReflectionConstantBuffer)
}

CDrxDXGLShaderReflectionConstBuffer::~CDrxDXGLShaderReflectionConstBuffer()
{
	delete m_pImpl;
}

bool CDrxDXGLShaderReflectionConstBuffer::Initialize(uk pvData)
{
	m_pImpl->m_pConstBuffer = static_cast<NDrxOpenGL::SShaderReflectionConstBuffer*>(pvData);

	NDrxOpenGL::SShaderReflectionConstBuffer::TVariables::iterator kVarIter(m_pImpl->m_pConstBuffer->m_kVariables.begin());
	const NDrxOpenGL::SShaderReflectionConstBuffer::TVariables::iterator kVarEnd(m_pImpl->m_pConstBuffer->m_kVariables.end());
	while (kVarIter != kVarEnd)
	{
		_smart_ptr<CDrxDXGLShaderReflectionVariable> spVariable(new CDrxDXGLShaderReflectionVariable());
		m_pImpl->m_kVariables.push_back(spVariable);

		if (!spVariable->Initialize(static_cast<uk>(&*kVarIter)))
			return false;
		++kVarIter;
	}
	return true;
}

HRESULT CDrxDXGLShaderReflectionConstBuffer::GetDesc(D3D11_SHADER_BUFFER_DESC* pDesc)
{
	(*pDesc) = m_pImpl->m_pConstBuffer->m_kDesc;
	return S_OK;
}

ID3D11ShaderReflectionVariable* CDrxDXGLShaderReflectionConstBuffer::GetVariableByIndex(UINT Index)
{
	if (Index >= m_pImpl->m_kVariables.size())
		return NULL;
	ID3D11ShaderReflectionVariable* pVariable;
	CDrxDXGLShaderReflectionVariable::ToInterface(&pVariable, m_pImpl->m_kVariables.at(Index));
	return pVariable;
}

ID3D11ShaderReflectionVariable* CDrxDXGLShaderReflectionConstBuffer::GetVariableByName(LPCSTR Name)
{
	Impl::TVariables::const_iterator kVarIter(m_pImpl->m_kVariables.begin());
	const Impl::TVariables::const_iterator kVarEnd(m_pImpl->m_kVariables.end());
	for (; kVarIter != kVarEnd; ++kVarIter)
	{
		D3D11_SHADER_VARIABLE_DESC kDesc;
		if (FAILED((*kVarIter)->GetDesc(&kDesc)))
			return NULL;
		if (strcmp(kDesc.Name, Name) == 0)
		{
			ID3D11ShaderReflectionVariable* pVariable;
			CDrxDXGLShaderReflectionVariable::ToInterface(&pVariable, kVarIter->get());
			return pVariable;
		}
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// CDrxDXGLShaderReflection
////////////////////////////////////////////////////////////////////////////////

struct CDrxDXGLShaderReflection::Impl
{
	struct SResource
	{
		NDrxOpenGL::SShaderReflectionResource* m_pResource;
	};

	struct SParameter
	{
		NDrxOpenGL::SShaderReflectionParameter* m_pParameter;
	};

	typedef std::vector<_smart_ptr<CDrxDXGLShaderReflectionConstBuffer>> TConstantBuffers;

	TConstantBuffers              m_kConstantBuffers;

	NDrxOpenGL::SShaderReflection m_kReflection;
};

CDrxDXGLShaderReflection::CDrxDXGLShaderReflection()
	: m_pImpl(new Impl())
{
	DXGL_INITIALIZE_INTERFACE(D3D11ShaderReflection)
}

CDrxDXGLShaderReflection::~CDrxDXGLShaderReflection()
{
	delete m_pImpl;
}

bool CDrxDXGLShaderReflection::Initialize(ukk pvData)
{
	if (!InitializeShaderReflectionFromInput(&m_pImpl->m_kReflection, pvData))
		return false;

	NDrxOpenGL::SShaderReflection::TConstantBuffers::iterator kConstBufferIter(m_pImpl->m_kReflection.m_kConstantBuffers.begin());
	const NDrxOpenGL::SShaderReflection::TConstantBuffers::iterator kConstBufferEnd(m_pImpl->m_kReflection.m_kConstantBuffers.end());
	while (kConstBufferIter != kConstBufferEnd)
	{
		_smart_ptr<CDrxDXGLShaderReflectionConstBuffer> spConstBuffer(new CDrxDXGLShaderReflectionConstBuffer());
		if (!spConstBuffer->Initialize(static_cast<uk>(&*kConstBufferIter)))
			return false;
		m_pImpl->m_kConstantBuffers.push_back(spConstBuffer);
		++kConstBufferIter;
	}
	return true;
}

HRESULT CDrxDXGLShaderReflection::GetDesc(D3D11_SHADER_DESC* pDesc)
{
	(*pDesc) = m_pImpl->m_kReflection.m_kDesc;
	return S_OK;
}

ID3D11ShaderReflectionConstantBuffer* CDrxDXGLShaderReflection::GetConstantBufferByIndex(UINT Index)
{
	if (Index >= m_pImpl->m_kConstantBuffers.size())
		return NULL;
	ID3D11ShaderReflectionConstantBuffer* pConstantBuffer;
	CDrxDXGLShaderReflectionConstBuffer::ToInterface(&pConstantBuffer, m_pImpl->m_kConstantBuffers.at(Index));
	return pConstantBuffer;
}

ID3D11ShaderReflectionConstantBuffer* CDrxDXGLShaderReflection::GetConstantBufferByName(LPCSTR Name)
{
	Impl::TConstantBuffers::const_iterator kCBIter(m_pImpl->m_kConstantBuffers.begin());
	const Impl::TConstantBuffers::const_iterator kCBEnd(m_pImpl->m_kConstantBuffers.end());
	for (; kCBIter != kCBEnd; ++kCBIter)
	{
		D3D11_SHADER_BUFFER_DESC kDesc;
		if (FAILED((*kCBIter)->GetDesc(&kDesc)))
			return NULL;
		if (strcmp(kDesc.Name, Name) == 0)
		{
			ID3D11ShaderReflectionConstantBuffer* pConstantBuffer;
			CDrxDXGLShaderReflectionConstBuffer::ToInterface(&pConstantBuffer, kCBIter->get());
			return pConstantBuffer;
		}
	}
	return NULL;
}

HRESULT CDrxDXGLShaderReflection::GetResourceBindingDesc(UINT ResourceIndex, D3D11_SHADER_INPUT_BIND_DESC* pDesc)
{
	if (ResourceIndex >= m_pImpl->m_kReflection.m_kResources.size())
		return E_FAIL;
	*pDesc = m_pImpl->m_kReflection.m_kResources[ResourceIndex].m_kDesc;
	return S_OK;
}

HRESULT CDrxDXGLShaderReflection::GetInputParameterDesc(UINT ParameterIndex, D3D11_SIGNATURE_PARAMETER_DESC* pDesc)
{
	if (ParameterIndex >= m_pImpl->m_kReflection.m_kInputs.size())
		return E_FAIL;
	*pDesc = m_pImpl->m_kReflection.m_kInputs[ParameterIndex].m_kDesc;
	return S_OK;
}

HRESULT CDrxDXGLShaderReflection::GetOutputParameterDesc(UINT ParameterIndex, D3D11_SIGNATURE_PARAMETER_DESC* pDesc)
{
	if (ParameterIndex >= m_pImpl->m_kReflection.m_kOutputs.size())
		return E_FAIL;
	*pDesc = m_pImpl->m_kReflection.m_kOutputs[ParameterIndex].m_kDesc;
	return S_OK;
}

HRESULT CDrxDXGLShaderReflection::GetPatchConstantParameterDesc(UINT ParameterIndex, D3D11_SIGNATURE_PARAMETER_DESC* pDesc)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

ID3D11ShaderReflectionVariable* CDrxDXGLShaderReflection::GetVariableByName(LPCSTR Name)
{
	DXGL_NOT_IMPLEMENTED
	return NULL;
}

HRESULT CDrxDXGLShaderReflection::GetResourceBindingDescByName(LPCSTR Name, D3D11_SHADER_INPUT_BIND_DESC* pDesc)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

UINT CDrxDXGLShaderReflection::GetMovInstructionCount()
{
	DXGL_NOT_IMPLEMENTED
	return 0;
}

UINT CDrxDXGLShaderReflection::GetMovcInstructionCount()
{
	DXGL_NOT_IMPLEMENTED
	return 0;
}

UINT CDrxDXGLShaderReflection::GetConversionInstructionCount()
{
	DXGL_NOT_IMPLEMENTED
	return 0;
}

UINT CDrxDXGLShaderReflection::GetBitwiseInstructionCount()
{
	DXGL_NOT_IMPLEMENTED
	return 0;
}

D3D_PRIMITIVE CDrxDXGLShaderReflection::GetGSInputPrimitive()
{
	DXGL_NOT_IMPLEMENTED
	return D3D_PRIMITIVE_TRIANGLE;
}

BOOL CDrxDXGLShaderReflection::IsSampleFrequencyShader()
{
	DXGL_NOT_IMPLEMENTED
	return FALSE;
}

UINT CDrxDXGLShaderReflection::GetNumInterfaceSlots()
{
	DXGL_NOT_IMPLEMENTED
	return 0;
}

HRESULT CDrxDXGLShaderReflection::GetMinFeatureLevel(enum D3D_FEATURE_LEVEL* pLevel)
{
	DXGL_NOT_IMPLEMENTED
	return E_FAIL;
}

UINT CDrxDXGLShaderReflection::GetThreadGroupSize(UINT* pSizeX, UINT* pSizeY, UINT* pSizeZ)
{
	DXGL_NOT_IMPLEMENTED
	return 0;
}
