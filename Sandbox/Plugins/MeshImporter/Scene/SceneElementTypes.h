// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

enum class ESceneElementType : i32
{
	SourceNode,  // Element is of type CSceneElementSourceNode.

	// Created by proxy generation.
	PhysProxy,  // Element is of type CSceneElementPhysProxy.
	ProxyGeom,  // Element is of type CSceneElementProxyGeom.

	Skin, // Each node of type SourceNode that has a mesh with a skin has a child node of type Skin.
};

