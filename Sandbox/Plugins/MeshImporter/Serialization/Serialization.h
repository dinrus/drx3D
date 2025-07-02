// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

namespace FbxTool
{
struct SNode;
struct SMesh;
}

struct SNodeInfo
{
	const FbxTool::SNode* const pNode;

	void Serialize(Serialization::IArchive& ar);
};

struct SMeshInfo
{
	const FbxTool::SMesh* const pMesh;

	void Serialize(Serialization::IArchive& ar);
};

