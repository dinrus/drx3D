// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Serialization/Forward.h>

namespace SourceAsset
{
using std::vector;
using Serialization::IArchive;

struct Node
{
	string      name;
	i32         mesh;
	vector<i32> children;

	Node()
		: mesh(-1)
	{
	}

	void Serialize(IArchive& ar)
	{
		ar(name, "name", "^");
		ar(mesh, "mesh", "Mesh");
		ar(children, "children", "Children");
	}
};
typedef vector<Node> Nodes;

struct Mesh
{
	string name;

	void   Serialize(IArchive& ar)
	{
		ar(name, "name", "^");
	}
};
typedef vector<Mesh> Meshes;

struct Scene
{
	Nodes  nodes;
	Meshes meshes;

	void   Serialize(IArchive& ar)
	{
		ar(nodes, "nodes", "Nodes");
		ar(meshes, "meshes", "Meshes");
	}
};

}

