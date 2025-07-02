/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <drx3D/CoreX/Serialization/yasli/Archive.h>

namespace yasli{
	class EnumDescription;
	class Object;
}

class PropertyRow;
class PropertyTreeModel;

class PropertyIArchive : public yasli::Archive{
public:
	PropertyIArchive(PropertyTreeModel* model, PropertyRow* root);

	bool operator()(yasli::StringInterface& value, tukk name, tukk label) override;
	bool operator()(yasli::WStringInterface& value, tukk name, tukk label) override;
	bool operator()(bool& value, tukk name, tukk label) override;
	bool operator()(char& value, tukk name, tukk label) override;

	bool operator()(yasli::i8& value, tukk name, tukk label) override;
	bool operator()(yasli::i16& value, tukk name, tukk label) override;
	bool operator()(yasli::i32& value, tukk name, tukk label) override;
	bool operator()(yasli::i64& value, tukk name, tukk label) override;
	bool operator()(yasli::u8& value, tukk name, tukk label) override;
	bool operator()(yasli::u16& value, tukk name, tukk label) override;
	bool operator()(yasli::u32& value, tukk name, tukk label) override;
	bool operator()(yasli::u64& value, tukk name, tukk label) override;

	bool operator()(float& value, tukk name, tukk label) override;
	bool operator()(double& value, tukk name, tukk label) override;

	bool operator()(const yasli::Serializer& ser, tukk name, tukk label) override;
	bool operator()(yasli::PointerInterface& ser, tukk name, tukk label) override;
	bool operator()(yasli::ContainerInterface& ser, tukk name, tukk label) override;
	bool operator()(yasli::Object& obj, tukk name, tukk label) override;
	bool operator()(yasli::CallbackInterface& callback, tukk name, tukk label) override;
	using yasli::Archive::operator();

	bool openBlock(tukk name, tukk label, tukk icon=0) override;
	void closeBlock() override;

protected:
	bool needDefaultArchive(tukk baseName) const { return false; }
private:
	bool openRow(tukk name, tukk label, tukk typeName);
	void closeRow(tukk name);

	struct Level {
		i32 rowIndex;
		Level() : rowIndex(0) {}
	};

	vector<Level> stack_;

	PropertyTreeModel* model_;
	PropertyRow* currentNode_;
	PropertyRow* lastNode_;
	PropertyRow* root_;
};


