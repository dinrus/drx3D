// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "PropertyTreeIArchive.h"

#include <DrxSandbox/ScopedVariableSetter.h>
#include <drx3D/CoreX/Serialization/yasli/Callback.h>
#include "IPropertyTreeWidget.h"

namespace PropertyTree2
{

	template<typename ValueType>
	bool PropertyTreeIArchive::ProcessSimpleRow(tukk name, ValueType& value)
	{
		const CRowModel* row = FindRowInScope<ValueType>(name);
		if (row)
		{
			row->GetPropertyTreeWidget()->GetValue(value);
			return true;
		}
		else
		{
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	PropertyTreeIArchive::PropertyTreeIArchive(const CRowModel& root)
		: yasli::Archive(INPUT | EDIT)
		, m_currentScope(&root)
		, m_lastVisitedIndex(-1)
	{
		root.MarkClean();
	}

	PropertyTreeIArchive::~PropertyTreeIArchive()
	{

	}

	bool PropertyTreeIArchive::operator()(yasli::StringInterface& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(yasli::WStringInterface& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(bool& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(char& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(yasli::i8& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(yasli::i16& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(yasli::i32& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(yasli::i64& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(yasli::u8& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(yasli::u16& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(yasli::u32& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(yasli::u64& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(float& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(double& value, tukk name, tukk label)
	{
		return ProcessSimpleRow(name, value);
	}

	bool PropertyTreeIArchive::operator()(const yasli::Serializer& ser, tukk name, tukk label)
	{
		const CRowModel* row = FindRowInScope(name, ser.type());
		if (row)
		{
			bool serializeChildren = true;
			if (row->GetPropertyTreeWidget())
			{
				row->GetPropertyTreeWidget()->GetValue(ser);
				serializeChildren = row->GetPropertyTreeWidget()->AllowChildSerialization();
			}

			if (ser && serializeChildren)
			{
				CScopedVariableSetter<const CRowModel*> resetScope(m_currentScope, row);
				CScopedVariableSetter<i32> resetIndex(m_lastVisitedIndex, -1);
				ser(*this);
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	bool PropertyTreeIArchive::operator()(yasli::PointerInterface& ptr, tukk name, tukk label)
	{
		//using the fully qualified pointer type as row type id
		const CRowModel* row = FindRowInScope(name, ptr.pointerType());
		if (row)
		{
			row->GetPropertyTreeWidget()->GetValue(ptr);

			if (yasli::Serializer ser = ptr.serializer())
			{
				CScopedVariableSetter<const CRowModel*> resetScope(m_currentScope, row);
				CScopedVariableSetter<i32> resetIndex(m_lastVisitedIndex, -1);

				ser(*this);
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	bool PropertyTreeIArchive::operator()(yasli::ContainerInterface& container, tukk name, tukk label)
	{
		const CRowModel* row = FindRowInScope(name, container.containerType());
		if (row)
		{
			if (container.size() > 0)
			{
				CScopedVariableSetter<const CRowModel*> resetScope(m_currentScope, row);
				CScopedVariableSetter<i32> resetIndex(m_lastVisitedIndex, -1);

				i32 i = 1;
				container.begin();
				do
				{
					const auto itemString = string().Format("Item %d", i);
					container(*this, itemString, itemString);
					i++;
				} while (container.next());
			}

			//Action is applied on the array after its children have been serialized, otherwise the behavior would not be what we expect
			container.begin();
			row->GetPropertyTreeWidget()->GetValue(container);

			return true;
		}
		else
		{
			return false;
		}
	}

	bool PropertyTreeIArchive::operator()(yasli::CallbackInterface& callback, tukk name, tukk label)
	{
		return callback.serializeValue(*this, name, label);
	}

	bool PropertyTreeIArchive::operator()(yasli::Object& obj, tukk name, tukk label)
	{
		return (*this)(obj.serializer(), name, label);
	}

	bool PropertyTreeIArchive::openBlock(tukk name, tukk label, tukk icon /*= 0*/)
	{
		const CRowModel* row = FindRowInScope(name, yasli::TypeID() /*intentional*/);
		if (row && row->HasChildren())
		{
			m_currentScope = row;
			m_lastVisitedIndex = -1;
			return true;
		}
		else
		{
			return false;
		}
	}

	void PropertyTreeIArchive::closeBlock()
	{
		m_currentScope = m_currentScope->GetParent();
		DRX_ASSERT_MESSAGE(m_currentScope, "Cannot close a block at root scope");
		m_lastVisitedIndex = -1;
	}

	const PropertyTree2::CRowModel* PropertyTreeIArchive::FindRowInScope(tukk name, const yasli::TypeID& type)
	{
		if (!m_currentScope || !m_currentScope->HasChildren())
			return nullptr;

		using namespace PropertyTree2;

		const auto& childArray = m_currentScope->GetChildren();
		i32k count = childArray.size();

		//First test the next index
		if(m_lastVisitedIndex != -1)
		{
			m_lastVisitedIndex++;

			if (m_lastVisitedIndex >= 0 && m_lastVisitedIndex < count)
			{
				const CRowModel* row = childArray[m_lastVisitedIndex];
				if (row->GetName() == name)
				{
					if (!type || row->GetType() == type)
						return row;
				}
			}
		}

		//Otherwise do a full search
		for (i32 i = 0; i < count; i++)
		{
			const CRowModel* row = childArray[i];
			if (row->GetName() == name)
			{
				if(!type || row->GetType() == type)
				{
					m_lastVisitedIndex = i;
					return row;
				}
			}
		}

		return nullptr;
	}
}
