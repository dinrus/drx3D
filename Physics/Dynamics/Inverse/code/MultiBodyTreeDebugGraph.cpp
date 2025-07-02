#include "../MultiBodyTreeDebugGraph.h"
#include <drx/Core/Core.h>
#include <cstdio>

namespace drx3d_inverse
{
i32 writeGraphvizDotFile(const MultiBodyTree* tree, const MultiBodyNameMap* map,
						 tukk filename)
{
	if (0x0 == tree)
	{
		throw drx::Exc("tree pointer is null\n");
		return -1;
	}
	if (0x0 == filename)
	{
		throw drx::Exc("filename is null\n");
		return -1;
	}

	FILE* fp = fopen(filename, "w");
	if (NULL == fp)
	{
		throw drx::Exc(drx::Format("cannot open file %s for writing\n", filename));
		return -1;
	}
	fprintf(fp,
			"// to generate postscript file, run dot -Tps %s -o %s.ps\n"
			"// details see graphviz documentation at http://graphviz.org\n"
			"digraph tree {\n",
			filename, filename);

	for (i32 body = 0; body < tree->numBodies(); body++)
	{
		STxt name;
		if (0x0 != map)
		{
			if (-1 == map->getBodyName(body, &name))
			{
				throw drx::Exc(drx::Format("can't get name of body %d\n", body));
				return -1;
			}
			fprintf(fp, "              %d [label=\"%d/%s\"];\n", body, body, name.c_str());
		}
	}
	for (i32 body = 0; body < tree->numBodies(); body++)
	{
		i32 parent;
		tukk joint_type;
		i32 qi;
		if (-1 == tree->getParentIndex(body, &parent))
		{
			throw drx::Exc("indexing error\n");
			return -1;
		}
		if (-1 == tree->getJointTypeStr(body, &joint_type))
		{
			throw drx::Exc("indexing error\n");
			return -1;
		}
		if (-1 == tree->getDoFOffset(body, &qi))
		{
			throw drx::Exc("indexing error\n");
			return -1;
		}
		if (-1 != parent)
		{
			fprintf(fp, "              %d -> %d [label= \"type:%s, q=%d\"];\n", parent, body,
					joint_type, qi);
		}
	}

	fprintf(fp, "}\n");
	fclose(fp);
	return 0;
}
}  // namespace drx3d_inverse
