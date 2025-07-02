#pragma once

#include <vector>

#include <drx3D/Files/Node.h>

namespace drx3d {
class DRX3D_EXPORT Json : public NodeFormatType<Json> {
public:
	// Do not call Load and Write directly, use Node::ParseString<Json> and Node::WriteStream<Json>.
	static void Load(Node &node, STxtview string);
	static void Write(const Node &node, std::ostream &stream, Format format = Minified);

private:
	static void AddToken(STxtview view, std::vector<Token> &tokens);
	static void Convert(Node &current, const std::vector<Token> &tokens, int32_t &k);

	static void AppendData(const Node &node, std::ostream &stream, Format format, int32_t indent);
};
}
