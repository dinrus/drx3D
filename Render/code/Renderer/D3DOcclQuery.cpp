// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*=============================================================================
   D3DOcclQuery.cpp : Occlusion queries unified interface implementation

   Revision история:
* Created by Tiago Sousa

   =============================================================================*/

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DriverD3D.h>

void COcclusionQuery::Create()
{
	Release();

	// Create visibility queries

	D3DOcclusionQuery* pVizQuery = GetDeviceObjectFactory().CreateOcclusionQuery();
	assert(pVizQuery);

	m_nOcclusionID = (UINT_PTR) pVizQuery;
}

void COcclusionQuery::Release()
{
	D3DOcclusionQuery* pVizQuery = (D3DOcclusionQuery*)m_nOcclusionID;
	SAFE_RELEASE(pVizQuery);

	m_nOcclusionID = 0;
	m_nDrawFrame = 0;
	m_nCheckFrame = 0;
	m_nVisSamples = ~0;
}

void COcclusionQuery::BeginQuery()
{
	if (!m_nOcclusionID)
		return;

	D3DOcclusionQuery* pVizQuery = (D3DOcclusionQuery*)m_nOcclusionID;
	CDeviceCommandListRef commandList = GetDeviceObjectFactory().GetCoreCommandList();
	commandList.GetGraphicsInterface()->BeginOcclusionQuery(pVizQuery);
}

void COcclusionQuery::EndQuery()
{
	if (!m_nOcclusionID)
		return;

	CD3D9Renderer* rd = gcpRendD3D;
	m_nDrawFrame = gRenDev->GetRenderFrameID();

	D3DOcclusionQuery* pVizQuery = (D3DOcclusionQuery*)m_nOcclusionID;
	CDeviceCommandListRef commandList = GetDeviceObjectFactory().GetCoreCommandList();
	commandList.GetGraphicsInterface()->EndOcclusionQuery(pVizQuery);
}

bool COcclusionQuery::IsReady()
{
	CD3D9Renderer* rd = gcpRendD3D;
	i32 nFrame = gRenDev->GetRenderFrameID();
	return (m_nCheckFrame == nFrame);
}

u32 COcclusionQuery::GetVisibleSamples(bool bAsynchronous)
{
	if (!m_nOcclusionID)
		return ~0;

	CD3D9Renderer* rd = gcpRendD3D;
	i32 nFrame = gRenDev->GetRenderFrameID();

	if (m_nCheckFrame == nFrame)
		return m_nVisSamples;

	uint64 samplesPassed;
	HRESULT hRes;
	D3DOcclusionQuery* const pVizQuery = (D3DOcclusionQuery*)m_nOcclusionID;

	if (!bAsynchronous)
	{
		PROFILE_FRAME(COcclusionQuery::GetVisibleSamples);

		hRes = S_FALSE;
		while (hRes == S_FALSE)
		{
			hRes = GetDeviceObjectFactory().GetOcclusionQueryResults(pVizQuery, samplesPassed) ? S_OK : S_FALSE;
		}
	}
	else
	{
		PROFILE_FRAME(COcclusionQuery::GetVisibleSamplesAsync);

		hRes = GetDeviceObjectFactory().GetOcclusionQueryResults(pVizQuery, samplesPassed) ? S_OK : S_FALSE;
	}
	
	if (hRes == S_OK)
	{
		m_nCheckFrame = nFrame;
		m_nVisSamples = static_cast<i32>(samplesPassed);
	}

	return m_nVisSamples;
}
