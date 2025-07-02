// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/DriverD3D.h>
#include <drx3D/Eng3D/I3DEngine.h>
#include <drx3D/Render/D3DPostProcess.h>
#include <drx3D/Render/Textures/TextureHelpers.h>

// TODO: Unify with other deferred primitive implementation
const t_arrDeferredMeshVertBuff& CD3D9Renderer::GetDeferredUnitBoxVertexBuffer() const
{
	return m_arrDeferredVerts;
}

const t_arrDeferredMeshIndBuff& CD3D9Renderer::GetDeferredUnitBoxIndexBuffer() const
{
	return m_arrDeferredInds;
}

void CD3D9Renderer::CreateDeferredUnitBox(t_arrDeferredMeshIndBuff& indBuff, t_arrDeferredMeshVertBuff& vertBuff)
{
	SVF_P3F_C4B_T2F vert;
	Vec3 vNDC;

	indBuff.clear();
	indBuff.reserve(36);

	vertBuff.clear();
	vertBuff.reserve(8);

	//Create frustum
	for (i32 i = 0; i < 8; i++)
	{
		//Generate screen space frustum (CCW faces)
		vNDC = Vec3((i == 0 || i == 1 || i == 4 || i == 5) ? 0.0f : 1.0f,
		            (i == 0 || i == 3 || i == 4 || i == 7) ? 0.0f : 1.0f,
		            (i == 0 || i == 1 || i == 2 || i == 3) ? 0.0f : 1.0f
		            );
		vert.xyz = vNDC;
		vert.st = Vec2(0.0f, 0.0f);
		vert.color.dcolor = -1;
		vertBuff.push_back(vert);
	}

	//CCW faces
	u16 nFaces[6][4] = {
		{ 0, 1, 2, 3 },
		{ 4, 7, 6, 5 },
		{ 0, 3, 7, 4 },
		{ 1, 5, 6, 2 },
		{ 0, 4, 5, 1 },
		{ 3, 2, 6, 7 }
	};

	//init indices for triangles drawing
	for (i32 i = 0; i < 6; i++)
	{
		indBuff.push_back((u16)  nFaces[i][0]);
		indBuff.push_back((u16)  nFaces[i][1]);
		indBuff.push_back((u16)  nFaces[i][2]);

		indBuff.push_back((u16)  nFaces[i][0]);
		indBuff.push_back((u16)  nFaces[i][2]);
		indBuff.push_back((u16)  nFaces[i][3]);
	}
}
