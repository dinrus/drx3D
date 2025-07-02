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
#include "Archive.h"
#include "Pointers.h"

namespace yasli{

class MemoryWriter;

class TextOArchive : public Archive{
public:
	// header = 0 - default header, use "" to omit
	TextOArchive(i32 textWidth = 80, tukk header = 0);
	~TextOArchive();

	bool save(tukk fileName);

	tukk c_str() const;
	size_t length() const;

	// from Archive:
	bool operator()(bool& value, tukk name = "", tukk label = 0) override;
	bool operator()(char& value, tukk name = "", tukk label = 0) override;
	bool operator()(StringInterface& value, tukk name = "", tukk label = 0) override;
	bool operator()(WStringInterface& value, tukk name = "", tukk label = 0) override;
	bool operator()(float& value, tukk name = "", tukk label = 0) override;
	bool operator()(double& value, tukk name = "", tukk label = 0) override;
	bool operator()(i8& value, tukk name = "", tukk label = 0) override;
	bool operator()(u8& value, tukk name = "", tukk label = 0) override;
	bool operator()(i16& value, tukk name = "", tukk label = 0) override;
	bool operator()(u16& value, tukk name = "", tukk label = 0) override;
	bool operator()(i32& value, tukk name = "", tukk label = 0) override;
	bool operator()(u32& value, tukk name = "", tukk label = 0) override;
	bool operator()(i64& value, tukk name = "", tukk label = 0) override;
	bool operator()(u64& value, tukk name = "", tukk label = 0) override;


	bool operator()(const Serializer& ser, tukk name = "", tukk label = 0) override;
	bool operator()(ContainerInterface& ser, tukk name = "", tukk label = 0) override;
	// ^^^

	using Archive::operator();
private:
	void openBracket();
	void closeBracket();
	void openContainerBracket();
	void closeContainerBracket();
	void placeName(tukk name);
	void placeIndent();
	void placeIndentCompact();

	bool joinLinesIfPossible();

	struct Level{
		Level(bool _isContainer, std::size_t position, i32 column)
		: isContainer(_isContainer)
		, startPosition(position)
		, indentCount(-column)
		{}
		bool isContainer;
		std::size_t startPosition;
		i32 indentCount;
	};

	typedef std::vector<Level> Stack;
	Stack stack_;
	std::unique_ptr<MemoryWriter> buffer_;
	tukk header_;
	i32 textWidth_;
	string fileName_;
	i32 compactOffset_;
};

}
