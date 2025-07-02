// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "PropertyTreeModel.h"
#include <drx3D/CoreX/Serialization/yasli/Archive.h>

namespace PropertyTree2
{
	class PropertyTreeOArchive : public yasli::Archive
	{
	public:
		PropertyTreeOArchive(CRowModel& root);
		virtual ~PropertyTreeOArchive();

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
		//Requires the VALIDATION flag
		//void validatorMessage(bool error, ukk handle, const yasli::TypeID& type, tukk message) override;
		void documentLastField(tukk docString) override;

	private:

		void EnterScope(CRowModel* scope);
		void ExitScope();

		template<typename DataType>
		CRowModel* FindOrCreateRowInScope(tukk name, tukk label)
		{
			return FindOrCreateRowInScope(name, label, yasli::TypeID::get<DataType>());
		}

		//If type is invalid, only name will be matched
		CRowModel* FindOrCreateRowInScope(tukk name, tukk label, const yasli::TypeID& type);
		CRowModel* FindRowInScope(tukk name);
		CRowModel* GetLastVisitedRow();

		template<typename ValueType>
		void ProcessSimpleRow(const ValueType& value, tukk name, tukk label);

		struct Scope
		{
			Scope(CRowModel* row) 
				: m_scopeRow(row)
				, m_lastVisitedIndex(-1) 
			{}

			CRowModel* m_scopeRow;
			i32 m_lastVisitedIndex;
		};

		std::vector<Scope> m_scopeStack;
		Scope m_currentScope;
		bool m_firstPopulate;
	};
}
