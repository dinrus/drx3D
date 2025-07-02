/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include "yasli/Pointers.h"
#include "yasli/Archive.h"
#include "yasli/Token.h"
#include <memory>
#include <vector>

namespace yasli{

class MemoryReader;

class TextIArchive : public Archive{
public:
	TextIArchive();
	~TextIArchive();

	bool load(tukk filename);
	bool open(tukk buffer, size_t length, bool free = false);

	// virtuals:
	bool operator()(bool& value, tukk name = "", tukk label = 0) override;
	bool operator()(char& value, tukk name = "", tukk label = 0) override;
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


	bool operator()(StringInterface& value, tukk name = "", tukk label = 0) override;
	bool operator()(WStringInterface& value, tukk name = "", tukk label = 0) override;
	bool operator()(const Serializer& ser, tukk name = "", tukk label = 0) override;
	bool operator()(ContainerInterface& ser, tukk name = "", tukk label = 0) override;

	using Archive::operator();
private:
	bool findName(tukk name);
	bool openBracket();
	bool closeBracket();

	bool openContainerBracket();
	bool closeContainerBracket();

	void checkValueToken();
	bool checkStringValueToken();
	void readToken();
	void putToken();
	i32 line(tukk position) const; 
	bool isName(Token token) const;

	void expect(char token);
	void skipBlock();

	struct Level{
		tukk start;
		tukk firstToken;
	};
	typedef std::vector<Level> Stack;
	Stack stack_;

	std::unique_ptr<MemoryReader> reader_;
	Token token_;
	STxt filename_;
};

}
