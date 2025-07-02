#ifndef MULTIBODYTREEDEBUGGRAPH_H_
#define MULTIBODYTREEDEBUGGRAPH_H_
#include <drx3D/Physics/Dynamics/Inverse/IDConfig.h>
#include <drx3D/Physics/Dynamics/Inverse/MultiBodyTree.h>
#include "MultiBodyNameMap.h"

namespace drx3d_inverse
{
/// generate a dot-file of the multibody tree for generating a graph using graphviz' dot tool
/// @param tree the multibody tree
/// @param map to add names of links (if 0x0, no names will be added)
/// @param filename name for the output file
/// @return 0 on success, -1 on error
i32 writeGraphvizDotFile(const MultiBodyTree* tree, const MultiBodyNameMap* map,
						 tukk filename);
}  // namespace drx3d_inverse

#endif  // MULTIBODYTREEDEBUGGRAPH_HPP
