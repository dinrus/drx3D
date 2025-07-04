// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   CDrxDXGLShaderReflection.hpp
//  Version:     v1.00
//  Created:     20/02/2013 by Valerio Guagliumi.
//  Описание: Declaration of the DXGL wrappers for D3D11 shader
//               reflection interfaces
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __DRXDXGLSHADERREFLECTION__
#define __DRXDXGLSHADERREFLECTION__

#include <drx3D/Render/D3D/DXGL/CDrxDXGLBase.hpp>

////////////////////////////////////////////////////////////////////////////////
// CDrxDXGLShaderReflectionVariable
////////////////////////////////////////////////////////////////////////////////

class CDrxDXGLShaderReflectionVariable : public CDrxDXGLBase
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLShaderReflectionVariable, D3D11ShaderReflectionVariable)
#if DXGL_FULL_EMULATION
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLShaderReflectionVariable, D3D11ShaderReflectionType)
#endif //DXGL_FULL_EMULATION

	CDrxDXGLShaderReflectionVariable();
	virtual ~CDrxDXGLShaderReflectionVariable();

	bool Initialize(uk pvData);

	// Implementation of ID3D11ShaderReflectionVariable
	HRESULT                               GetDesc(D3D11_SHADER_VARIABLE_DESC* pDesc);
	ID3D11ShaderReflectionType*           GetType();
	ID3D11ShaderReflectionConstantBuffer* GetBuffer();
	UINT                                  GetInterfaceSlot(UINT uArrayIndex);

	// Implementation of ID3D11ShaderReflectionType
	HRESULT                     GetDesc(D3D11_SHADER_TYPE_DESC* pDesc);
	ID3D11ShaderReflectionType* GetMemberTypeByIndex(UINT Index);
	ID3D11ShaderReflectionType* GetMemberTypeByName(LPCSTR Name);
	LPCSTR                      GetMemberTypeName(UINT Index);
	HRESULT                     IsEqual(ID3D11ShaderReflectionType* pType);
	ID3D11ShaderReflectionType* GetSubType();
	ID3D11ShaderReflectionType* GetBaseClass();
	UINT                        GetNumInterfaces();
	ID3D11ShaderReflectionType* GetInterfaceByIndex(UINT uIndex);
	HRESULT                     IsOfType(ID3D11ShaderReflectionType* pType);
	HRESULT                     ImplementsInterface(ID3D11ShaderReflectionType* pBase);

	struct Impl;
	Impl* m_pImpl;
};

////////////////////////////////////////////////////////////////////////////////
// CDrxDXGLShaderReflectionConstBuffer
////////////////////////////////////////////////////////////////////////////////

class CDrxDXGLShaderReflectionConstBuffer : public CDrxDXGLBase
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLShaderReflectionConstBuffer, D3D11ShaderReflectionConstantBuffer)

	CDrxDXGLShaderReflectionConstBuffer();
	virtual ~CDrxDXGLShaderReflectionConstBuffer();

	bool Initialize(uk pvData);

	// Implementation of ID3D11ShaderReflectionConstantBuffer
	HRESULT                         GetDesc(D3D11_SHADER_BUFFER_DESC* pDesc);
	ID3D11ShaderReflectionVariable* GetVariableByIndex(UINT Index);
	ID3D11ShaderReflectionVariable* GetVariableByName(LPCSTR Name);

	struct Impl;
	Impl* m_pImpl;
};

////////////////////////////////////////////////////////////////////////////////
// CDrxDXGLShaderReflection
////////////////////////////////////////////////////////////////////////////////

class CDrxDXGLShaderReflection : public CDrxDXGLBase
{
public:
	DXGL_IMPLEMENT_INTERFACE(CDrxDXGLShaderReflection, D3D11ShaderReflection)

	CDrxDXGLShaderReflection();
	virtual ~CDrxDXGLShaderReflection();

	bool Initialize(ukk pvData);

	// Implementation of ID3D11ShaderReflection
	HRESULT                               GetDesc(D3D11_SHADER_DESC* pDesc);
	ID3D11ShaderReflectionConstantBuffer* GetConstantBufferByIndex(UINT Index);
	ID3D11ShaderReflectionConstantBuffer* GetConstantBufferByName(LPCSTR Name);
	HRESULT                               GetResourceBindingDesc(UINT ResourceIndex, D3D11_SHADER_INPUT_BIND_DESC* pDesc);
	HRESULT                               GetInputParameterDesc(UINT ParameterIndex, D3D11_SIGNATURE_PARAMETER_DESC* pDesc);
	HRESULT                               GetOutputParameterDesc(UINT ParameterIndex, D3D11_SIGNATURE_PARAMETER_DESC* pDesc);
	HRESULT                               GetPatchConstantParameterDesc(UINT ParameterIndex, D3D11_SIGNATURE_PARAMETER_DESC* pDesc);
	ID3D11ShaderReflectionVariable*       GetVariableByName(LPCSTR Name);
	HRESULT                               GetResourceBindingDescByName(LPCSTR Name, D3D11_SHADER_INPUT_BIND_DESC* pDesc);
	UINT                                  GetMovInstructionCount();
	UINT                                  GetMovcInstructionCount();
	UINT                                  GetConversionInstructionCount();
	UINT                                  GetBitwiseInstructionCount();
	D3D_PRIMITIVE                         GetGSInputPrimitive();
	BOOL                                  IsSampleFrequencyShader();
	UINT                                  GetNumInterfaceSlots();
	HRESULT                               GetMinFeatureLevel(enum D3D_FEATURE_LEVEL* pLevel);
	UINT                                  GetThreadGroupSize(UINT* pSizeX, UINT* pSizeY, UINT* pSizeZ);

	struct Impl;
	Impl* m_pImpl;
};

#endif //__DRXDXGLSHADERREFLECTION__
