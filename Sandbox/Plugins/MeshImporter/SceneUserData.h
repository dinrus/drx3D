// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "FbxScene.h"

bool HasDefaultUserData(const FbxTool::CScene* pScene, const FbxTool::SNode* pNode);

FbxTool::ENodeExportSetting GetDefaultNodeExportSetting(const FbxTool::SNode* pNode);

