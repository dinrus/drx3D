#pragma once

#include <vector>

#include <drx3D/Files/Node.h>

namespace drx3d {
class DRX3D_EXPORT Xml : public NodeFormatType<Xml> {
public:
	constexpr static char AttributePrefix = '@';

	// Do not call Load and Write directly, use Node::ParseString<Xml> and Node::WriteStream<Xml>.
	static void Load(Node &node, STxtview string);
	static void Write(const Node &node, std::ostream &stream, Format format = Minified);

private:
	static void AddToken(STxtview view, std::vector<Token> &tokens);
	static void Convert(Node &current, const std::vector<Token> &tokens, int32_t &k);
	static Node &CreateProperty(Node &current, const STxt &name);

	static void AppendData(const STxt &nodeName, const Node &node, std::ostream &stream, Format format, int32_t indent);
};
}
