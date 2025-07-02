/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */

#pragma once

#include <drx3D/CoreX/Serialization/yasli/Config.h>
#include <drx3D/CoreX/Serialization/yasli/Archive.h>
#include <drx3D/CoreX/Serialization/yasli/Pointers.h>

class PropertyRow;
class PropertyTreeModel;
class ValidatorBlock;
using yasli::SharedPtr;

class PropertyOArchive : public yasli::Archive{
public:
	PropertyOArchive(PropertyTreeModel* model, PropertyRow* rootNode, ValidatorBlock* validator);
	~PropertyOArchive();

	void setOutlineMode(bool outlineMode);
protected:
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
	bool operator()(yasli::PointerInterface& ptr, tukk name, tukk label) override;
	bool operator()(yasli::ContainerInterface& ser, tukk name, tukk label) override;
	bool operator()(yasli::CallbackInterface& callback, tukk name, tukk label) override;
	bool operator()(yasli::Object& obj, tukk name, tukk label) override;
	using yasli::Archive::operator();

	bool openBlock(tukk name, tukk label, tukk icon=0) override;
	void closeBlock() override;
	void validatorMessage(bool error, ukk handle, const yasli::TypeID& type, tukk message) override;
	void documentLastField(tukk docString) override;

protected:
	PropertyOArchive(PropertyTreeModel* model, bool forDefaultType);

private:
	struct Level {
		std::vector<SharedPtr<PropertyRow> > oldRows;
		i32 rowIndex;
		Level() : rowIndex(0) {}
	};
	std::vector<Level> stack_;

	template<class RowType, class ValueType>
	PropertyRow* updateRowPrimitive(tukk name, tukk label, tukk typeName, const ValueType& value, ukk handle, const yasli::TypeID& typeId);

	template<class RowType, class ValueType>
	RowType* updateRow(tukk name, tukk label, tukk typeName, const ValueType& value);

	void enterNode(PropertyRow* row); // sets currentNode
	void closeStruct(tukk name);
	PropertyRow* defaultValueRootNode();

	bool updateMode_;
	bool defaultValueCreationMode_;
	PropertyTreeModel* model_;
	ValidatorBlock* validator_;
	SharedPtr<PropertyRow> currentNode_;
	SharedPtr<PropertyRow> lastNode_;

	// for defaultArchive
	SharedPtr<PropertyRow> rootNode_;
	yasli::string typeName_;
	tukk derivedTypeName_;
	yasli::string derivedTypeNameAlt_;
	bool outlineMode_;
};

// vim:ts=4 sw=4:

