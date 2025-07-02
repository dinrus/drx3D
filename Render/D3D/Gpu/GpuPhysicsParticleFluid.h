// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef GPU_FLUID_SIM_H
#define GPU_FLUID_SIM_H

#include <drx3D/CoreX/Renderer/IGpuPhysics.h>
#include <drx3D/Render/D3D/Gpu/GpuComputeBackend.h>
#include <drx3D/Render/D3D/GraphicsPipeline/Common/ComputeRenderPass.h>

namespace gpu_physics
{
i32k kThreadsInBlock = 1024u;

struct SGridCell
{
	uint count;
	uint sum;
	uint blockSum;
};

struct SParticleFluidParametersInternal : SParticleFluidParameters
{
	i32 numberOfBodies;
	i32 numberBodiesInject;
	i32 pad2[2];
};

struct SSimulationData
{
	SSimulationData(i32k numBodies, i32k totalGridSize)
		: bodies(numBodies), bodiesTemp(numBodies), bodiesOffsets(numBodies),
		grid(totalGridSize), bodiesInject(numBodies), adjacencyList(27)
	{
		bodies.CreateDeviceBuffer();
		bodiesTemp.CreateDeviceBuffer();
		bodiesOffsets.CreateDeviceBuffer();
		grid.CreateDeviceBuffer();
		bodiesInject.CreateDeviceBuffer();
		adjacencyList.CreateDeviceBuffer();
	};

	gpu::CStructuredResource<SFluidBody, gpu::BufferFlagsReadWriteAppend>   bodies;
	gpu::CStructuredResource<SFluidBody, gpu::BufferFlagsReadWriteReadback> bodiesTemp;
	gpu::CStructuredResource<uint, gpu::BufferFlagsReadWrite>               bodiesOffsets;
	gpu::CStructuredResource<SGridCell, gpu::BufferFlagsReadWrite>          grid;
	gpu::CStructuredResource<SFluidBody, gpu::BufferFlagsDynamic>           bodiesInject;
	gpu::CStructuredResource<i32, gpu::BufferFlagsDynamic>                  adjacencyList;
};

// fwd
class CParticleEffector;

class CParticleFluidSimulation : public ISimulationInstance
{
public:
	enum { simulationType = eSimulationType_ParticleFluid };

	CParticleFluidSimulation(i32k maxBodies);
	~CParticleFluidSimulation();

	// most of the simulation runs in the render thread
	void RenderThreadUpdate(CDeviceCommandListRef RESTRICT_REFERENCE commandList);

	void CreateResources();
	void EvolveParticles(CDeviceCommandListRef RESTRICT_REFERENCE commandList, CGpuBuffer& defaultParticleBuffer, i32 numParticles);
	void FluidCollisions(CDeviceCommandListRef RESTRICT_REFERENCE commandList, CConstantBufferPtr parameterBuffer, i32 constantBufferSlot);
protected:
	void InternalInjectBodies(const EBodyType type, const SBodyBase* b, i32k numBodies);
	void InternalSetParameters(const EParameterType type, const SParameterBase* p);
	void SetUAVsAndConstants();

	void RetrieveCounter();
	void UpdateDynamicBuffers();
	void UpdateSimulationBuffers(CDeviceCommandListRef RESTRICT_REFERENCE commandList, i32k cellBlock);
	void SortSimulationBodies(CDeviceCommandListRef RESTRICT_REFERENCE commandList, i32k gridCells, i32k cellBlock, i32k blocks);

	void IntegrateBodies(CDeviceCommandListRef RESTRICT_REFERENCE commandList, i32k blocks);

	void DebugDraw();

	i32k m_maxBodies;

	gpu::CTypedConstantBuffer<SParticleFluidParametersInternal> m_params;

	// resources
	std::unique_ptr<SSimulationData> m_pData;
	std::vector<SFluidBody>          m_bodiesInject;

	CComputeRenderPass               m_passCalcLambda;
	CComputeRenderPass               m_passPredictDensity;
	CComputeRenderPass               m_passCorrectDensityError;
	CComputeRenderPass               m_passCorrectDivergenceError;
	CComputeRenderPass               m_passPositionUpdate;
	CComputeRenderPass               m_passBodiesInject;
	CComputeRenderPass               m_passClearGrid;
	CComputeRenderPass               m_passAssignAndCount;
	CComputeRenderPass               m_passPrefixSumBlocks;
	CComputeRenderPass               m_passBuildGridIndices;
	CComputeRenderPass               m_passRearrangeParticles;
	CComputeRenderPass               m_passEvolveExternalParticles;
	CComputeRenderPass               m_passCollisionsScreenSpace;

	// only for debug
	std::vector<Vec3> m_points;
};
}

#endif
