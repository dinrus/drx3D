// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace compute_skinning
{

#pragma pack(push)
#pragma pack(1)

struct SSkinVertexIn
{
	Vec3 pos;
	uint morphDeltaOffset;
	Quat qtangent;
	Vec2 uv;
	uint triOffset;
	uint triCount;
};

struct SActiveMorphs
{
	uint  morphIndex;
	float weight;
};

#pragma pack(pop)

}
