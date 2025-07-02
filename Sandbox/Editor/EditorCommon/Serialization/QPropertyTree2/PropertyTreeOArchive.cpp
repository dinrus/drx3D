// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//#include "stdafx.h"
#include "PropertyTreeOArchive.h"

#include <drx3D/CoreX/Serialization/yasli/Callback.h>
#include "IPropertyTreeWidget.h"

namespace PropertyTree2
{
	template<typename ValueType>
	void PropertyTreeOArchive::ProcessSimpleRow(const ValueType& value, tukk name, tukk label)
	{
		CRowModel* row = FindOrCreateRowInScope<ValueType>(name, label);
		row->SetWidgetAndType<ValueType>();
		row->GetPropertyTreeWidget()->SetValue(value, *this);
	}

	//////////////////////////////////////////////////////////////////////////

	PropertyTreeOArchive::PropertyTreeOArchive(CRowModel& root)
		: yasli::Archive(OUTPUT | EDIT | DOCUMENTATION)
		, m_currentScope(&root)
	{
		//If the root object has no children we can optimize populating it
		m_firstPopulate = !root.HasChildren();

		root.MarkClean();
	}

	PropertyTreeOArchive::~PropertyTreeOArchive()
	{

	}

	bool PropertyTreeOArchive::operator()(yasli::StringInterface& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::WStringInterface& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(bool& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(char& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::i8& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::i16& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::i32& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::i64& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::u8& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::u16& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::u32& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::u64& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(float& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(double& value, tukk name, tukk label)
	{
		ProcessSimpleRow(value, name, label);
		return true;
	}

	bool PropertyTreeOArchive::operator()(const yasli::Serializer& ser, tukk name, tukk label)
	{
		CRowModel* row = FindOrCreateRowInScope(name, label, ser.type());

		if (!row->IsWidgetSet() && GetPropertyTreeWidgetFactory().IsRegistered(ser.type().name()))
		{
			row->SetWidgetAndType(ser.type());
		}

		bool serializeChildren = true;
		if (row->GetPropertyTreeWidget())
		{
			row->GetPropertyTreeWidget()->SetValue(ser, *this); 
			serializeChildren = row->GetPropertyTreeWidget()->AllowChildSerialization();
		}

		if (ser && serializeChildren)
		{
			EnterScope(row);
			ser(*this);
			ExitScope();
		}

		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::PointerInterface& ptr, tukk name, tukk label)
	{
		//using the fully qualified pointer type as row type id
		CRowModel* row = FindOrCreateRowInScope(name, label, ptr.pointerType());
		if (!row->IsWidgetSet())
		{
			row->SetWidgetAndType<yasli::PointerInterface>();
			row->SetType(ptr.pointerType());
		}

		row->GetPropertyTreeWidget()->SetValue(ptr, *this);

		if (yasli::Serializer ser = ptr.serializer())
		{
			EnterScope(row);
			ser(*this);
			ExitScope();
		}

		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::ContainerInterface& container, tukk name, tukk label)
	{
		CRowModel* row = FindOrCreateRowInScope(name, label, container.containerType());

		if (!row->IsWidgetSet())
		{
			row->SetWidgetAndType<yasli::ContainerInterface>();
			row->SetType(container.containerType());

			if (container.isFixedSize())
			{
				row->SetLabel(QString("%1 (%2 Items)").arg(label).arg(container.size()));
			}
		}

		if (!container.isFixedSize())
		{
			row->SetLabel(QString("%1 (%2 Items)").arg(label).arg(container.size()));
		}

		row->GetPropertyTreeWidget()->SetValue(container, *this);

		if (container.size() > 0)
		{
			EnterScope(row);

			i32 i = 1;
			container.begin();
			do
			{
				const auto itemString = string().Format("Item %d", i);
				container(*this, itemString, itemString);
				i++;
			} while (container.next());

			ExitScope();
		}

		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::CallbackInterface& callback, tukk name, tukk label)
	{
		CRowModel* row = FindRowInScope(name);
		bool shouldConnect = row ? !row->IsWidgetSet() : true;

		if (!callback.serializeValue(*this, name, label))
			return false;

		//attach the callback otherwise
		row = GetLastVisitedRow();
		if (shouldConnect && row->GetPropertyTreeWidget())
		{
			auto clone = callback.clone();
			row->GetPropertyTreeWidget()->signalChanged.Connect([clone,row]() 
			{
				auto applyLambda = [row](uk arg, const yasli::TypeID& type)
				{
					row->GetPropertyTreeWidget()->GetValue(arg, type);
				};
				clone->call(applyLambda);
			});
		}
				
		return true;
	}

	bool PropertyTreeOArchive::operator()(yasli::Object& obj, tukk name, tukk label)
	{
		return (*this)(obj.serializer(), name, label);
	}

	bool PropertyTreeOArchive::openBlock(tukk name, tukk label, tukk icon /*= 0*/)
	{
		CRowModel* row = FindOrCreateRowInScope(name, label, yasli::TypeID());
		EnterScope(row);
		return true;
	}

	void PropertyTreeOArchive::closeBlock()
	{
		ExitScope();
	}

	void PropertyTreeOArchive::EnterScope(CRowModel* scope)
	{
		m_scopeStack.push_back(m_currentScope);
		m_currentScope = Scope(scope);
	}

	void PropertyTreeOArchive::ExitScope()
	{
		m_currentScope = m_scopeStack.back();
		m_scopeStack.pop_back();
	}

	PropertyTree2::CRowModel* PropertyTreeOArchive::FindOrCreateRowInScope(tukk name, tukk label, const yasli::TypeID& type)
	{
		DRX_ASSERT(m_currentScope.m_scopeRow);

		if(!m_firstPopulate) //First populate will always create a row for every row visited
		{
			if (m_currentScope.m_scopeRow->HasChildren())
			{
				auto& childArray = m_currentScope.m_scopeRow->GetChildren();
				i32k count = childArray.size();

				//first test index
				if (m_currentScope.m_lastVisitedIndex != -1)
				{
					m_currentScope.m_lastVisitedIndex++;

					if (m_currentScope.m_lastVisitedIndex >= 0 && m_currentScope.m_lastVisitedIndex < count)
					{
						CRowModel* row = childArray[m_currentScope.m_lastVisitedIndex];
						if (row->GetName() == name)
						{
							if (!type || row->GetType() == type)
							{
								row->MarkClean();
								return row;
							}
						}
					}
				}

				//otherwise search for it
				for (i32 i = 0; i < count; i++)
				{
					CRowModel* row = childArray[i];
					if (row->GetName() == name)
					{
						if (!type || row->GetType() == type)
						{
							m_currentScope.m_lastVisitedIndex = i;
							row->MarkClean();
							return row;
						}
					}
				}
			}
		}
		
		//create if not found
		m_currentScope.m_lastVisitedIndex = m_currentScope.m_scopeRow->GetChildren().size();
		return new CRowModel(name, label, m_currentScope.m_scopeRow, type);
	}

	PropertyTree2::CRowModel* PropertyTreeOArchive::FindRowInScope(tukk name)
	{
		DRX_ASSERT(m_currentScope.m_scopeRow);

		if (m_currentScope.m_scopeRow->HasChildren())
		{
			auto& childArray = m_currentScope.m_scopeRow->GetChildren();
			i32k count = childArray.size();

			for (i32 i = 0; i < count; i++)
			{
				CRowModel* row = childArray[i];
				if (row->GetName() == name)
				{
					return row;
				}
			}
		}

		return nullptr;
	}

	PropertyTree2::CRowModel* PropertyTreeOArchive::GetLastVisitedRow()
	{
		DRX_ASSERT(m_currentScope.m_scopeRow);
		if (m_currentScope.m_lastVisitedIndex >= 0 && m_currentScope.m_lastVisitedIndex < m_currentScope.m_scopeRow->GetChildren().size())
			return m_currentScope.m_scopeRow->GetChildren()[m_currentScope.m_lastVisitedIndex];
		else
			return nullptr;
	}
	
	void PropertyTreeOArchive::documentLastField(tukk docString)
	{
		auto row = GetLastVisitedRow();
		if (row)
			row->SetTooltip(docString);
		else
			DRX_ASSERT_MESSAGE(false, "Could not resolve last visited row");
	}

}
