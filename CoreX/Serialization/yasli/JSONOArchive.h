/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <memory>
#include <drx3D/CoreX/Serialization/yasli/Archive.h>
#include "Pointers.h"

namespace yasli{

class MemoryWriter;

class JSONOArchive : public Archive{
public:
	// header = 0 - default header, use "" to omit
	YASLI_INLINE JSONOArchive(i32 textWidth = 80, tukk header = 0);
	YASLI_INLINE ~JSONOArchive();

	YASLI_INLINE bool save(tukk fileName);

	YASLI_INLINE tukk c_str() const;
	tukk buffer() const { return c_str(); }
	YASLI_INLINE size_t length() const;

	YASLI_INLINE bool operator()(bool& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(char& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(float& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(double& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(i8& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(u8& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(i16& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(u16& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(i32& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(u32& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(i64& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(u64& value, tukk name = "", tukk label = 0) override;

	YASLI_INLINE bool operator()(StringInterface& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(WStringInterface& value, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(const Serializer& ser, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(ContainerInterface& ser, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(KeyValueInterface& keyValue, tukk name = "", tukk label = 0) override;
	YASLI_INLINE bool operator()(PointerInterface& ser, tukk name = "", tukk label = 0) override;

	using Archive::operator();
private:
	YASLI_INLINE void openBracket();
	YASLI_INLINE void closeBracket();
	YASLI_INLINE void openContainerBracket();
	YASLI_INLINE void closeContainerBracket();
	YASLI_INLINE void placeName(tukk name);
	YASLI_INLINE void placeIndent(bool putComma = true);
	YASLI_INLINE void placeIndentCompact(bool putComma = true);

	YASLI_INLINE bool joinLinesIfPossible();

	struct Level{
		Level(bool _isContainer, std::size_t position, i32 column)
		: isContainer(_isContainer)
		, isKeyValue(false)
		, isDictionary(false)
		, startPosition(position)
		, indentCount(-column)
		, elementIndex(0)
		, nameIndex(0)
		{}
		bool isKeyValue;
		bool isContainer;
		bool isDictionary;
		std::size_t startPosition;
		i32 nameIndex;
		i32 elementIndex;
		i32 indentCount;
	};

	typedef std::vector<Level> Stack;
	Stack stack_;
	std::unique_ptr<MemoryWriter> buffer_;
	tukk header_;
	i32 textWidth_;
	string fileName_;
	i32 compactOffset_;
	bool isKeyValue_;
};

}


#if YASLI_INLINE_IMPLEMENTATION
#include "JSONOArchiveImpl.h"
#endif