/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "Config.h"
#include <drx3D/CoreX/Serialization/yasli/Archive.h>
#include "Token.h"
#include <memory>
#include <backward/auto_ptr.h>

namespace yasli{

class MemoryReader;

class JSONIArchive : public Archive{
public:
	YASLI_INLINE JSONIArchive();
	YASLI_INLINE ~JSONIArchive();

	YASLI_INLINE bool load(tukk filename);
	YASLI_INLINE bool open(tukk buffer, size_t length, bool free = false);

	YASLI_INLINE bool operator()(bool& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(char& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(float& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(double& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(i8& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(i16& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(i32& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(i64& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(u8& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(u16& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(u32& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(u64& value, tukk name = "", tukk label = 0) override;

	YASLI_INLINE bool operator()(StringInterface& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(WStringInterface& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(const Serializer& ser, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(BlackBox& ser, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(ContainerInterface& ser, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(KeyValueInterface& ser, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(PointerInterface& ser, tukk name = "", tukk label = 0) override;

	using Archive::operator();
private:
	YASLI_INLINE bool findName(tukk name, Token* outName = 0);
	YASLI_INLINE bool openBracket();
	YASLI_INLINE bool closeBracket();

	YASLI_INLINE bool openContainerBracket();
	YASLI_INLINE bool closeContainerBracket();

	YASLI_INLINE void checkValueToken();
	YASLI_INLINE bool checkStringValueToken();
	YASLI_INLINE void readToken();
	YASLI_INLINE void putToken();
	YASLI_INLINE i32 line(tukk position) const;
	YASLI_INLINE bool isName(Token token) const;

	YASLI_INLINE bool expect(char token);
	YASLI_INLINE void skipBlock();

	struct Level{
		tukk start;
		tukk firstToken;
		bool isContainer;
		bool isKeyValue;
		Level() : isContainer(false), isKeyValue(false) {}
	};
	typedef std::vector<Level> Stack;
	Stack stack_;

	std::unique_ptr<MemoryReader> reader_;
	Token token_;
	std::vector<char> unescapeBuffer_;
	string filename_;
	uk buffer_;
};

YASLI_INLINE double parseFloat(tukk s);

}

#if YASLI_INLINE_IMPLEMENTATION
#include "JSONIArchiveImpl.h"
#endif