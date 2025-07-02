// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/StdAfx.h>
#include <drx3D/CoreX/DrxCustomTypes.h>
#include <drx3D/Render/D3D/DeviceUpr/DeviceObjects.h>
#include <drx3D/Render/DriverD3D.h>
#include <drx3D/Render/DX12/API/DX12DescriptorHeap.hpp>
#include <drx3D/Render/D3D/DeviceUpr/DeviceResourceSet.h>

using namespace NDrxDX12;

class CDeviceResourceSet_DX12 : public CDeviceResourceSet
{
public:
	friend CDeviceGraphicsCommandInterfaceImpl;
	friend CDeviceComputeCommandInterfaceImpl;

	CDeviceResourceSet_DX12(CDeviceResourceSet::EFlags flags)
		: CDeviceResourceSet(flags)
		, m_pDescriptors(nullptr)
	{}

	~CDeviceResourceSet_DX12()
	{
		ReleaseDescriptors();
	}

	virtual bool UpdateImpl(const CDeviceResourceSetDesc& desc, CDeviceResourceSetDesc::EDirtyFlags dirtyFlags);

	const CDescriptorBlock& GetDescriptorBlock() const { return m_DescriptorBlockDX12; }

protected:
	// Workaround for: "template<template<typename, class> class C, typename T> using CContainer = C<T, CSingleBlockAllocator<T>>;"
	typedef std::vector<D3D12_CPU_DESCRIPTOR_HANDLE, DrxStack::CSingleBlockAllocator<D3D12_CPU_DESCRIPTOR_HANDLE>> StackDescriptorVector;
	const inline size_t NoAlign(size_t nSize) { return nSize; }

	void ReleaseDescriptors();

	bool GatherDescriptors(
		const CDeviceResourceSetDesc& resourceDesc,
		StackDescriptorVector& descriptors,
		CDescriptorBlock& descriptorScratchSpace,
		std::vector< std::pair<SResourceBindPoint, CDrxDX12Buffer*> >& constantBuffersInUse,
		std::vector< std::pair<SResourceBindPoint, CDrxDX12ShaderResourceView*> >& shaderResourceViewsInUse,
		std::vector< std::pair<SResourceBindPoint, CDrxDX12UnorderedAccessView*> >& unorderedAccessViewsInUse) const;

	void FillDescriptorBlock(
		StackDescriptorVector& descriptors,
		CDescriptorBlock& descriptorBlock) const;

	CDescriptorBlock  m_DescriptorBlockDX12;
	SDescriptorBlock* m_pDescriptors;

	std::vector< std::pair<SResourceBindPoint, CDrxDX12Buffer*> >              m_ConstantBuffersInUse;
	std::vector< std::pair<SResourceBindPoint, CDrxDX12ShaderResourceView*> >  m_ShaderResourceViewsInUse;
	std::vector< std::pair<SResourceBindPoint, CDrxDX12UnorderedAccessView*> > m_UnorderedAccessViewsInUse;
};

class CDeviceResourceLayout_DX12 : public CDeviceResourceLayout
{
public:
	CDeviceResourceLayout_DX12(CDevice* pDevice, UsedBindSlotSet requiredResourceBindings)
		: CDeviceResourceLayout(requiredResourceBindings)
		, m_pDevice(pDevice)
	{}

	bool            Init(const SDeviceResourceLayoutDesc& desc);
	CRootSignature* GetRootSignature() const { return m_pRootSignature.get(); }

protected:
	DX12_PTR(CDevice) m_pDevice;
	DX12_PTR(CRootSignature) m_pRootSignature;
};
