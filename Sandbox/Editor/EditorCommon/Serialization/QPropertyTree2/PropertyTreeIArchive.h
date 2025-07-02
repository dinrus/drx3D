// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "PropertyTreeModel.h"
#include <drx3D/CoreX/Serialization/yasli/Archive.h>

namespace PropertyTree2
{
	class PropertyTreeIArchive : public yasli::Archive
	{
	public:
		PropertyTreeIArchive(const CRowModel& root);
		virtual ~PropertyTreeIArchive();

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
		bool operator()(yasli::ContainerInterface& container, tukk name, tukk label) override;
		bool operator()(yasli::CallbackInterface& callback, tukk name, tukk label) override;
		bool operator()(yasli::Object& obj, tukk name, tukk label) override;
		using yasli::Archive::operator();

		bool openBlock(tukk name, tukk label, tukk icon = 0) override;
		void closeBlock() override;

	private:

		template<typename DataType>
		const CRowModel* FindRowInScope(tukk name)
		{
			return FindRowInScope(name, yasli::TypeID::get<DataType>());
		}

		//If type is invalid, only name will be matched
		const CRowModel* FindRowInScope(tukk name, const yasli::TypeID& type);

		template<typename ValueType>
		bool ProcessSimpleRow(tukk name, ValueType& value);

		const CRowModel* m_currentScope;
		i32 m_lastVisitedIndex; //assuming rows will be accessed sequentially, this helps with optimization
	};
}
