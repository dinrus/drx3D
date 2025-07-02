// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 1999-2014.
// -------------------------------------------------------------------------
//  File name:   CVarWindow.h
//  Version:     v1.00
//  Created:     10/04/2014 by Matthijs vd Meide
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   Visual Studio 2010
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#include <vector>
#include "Messages.h"

#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

//auto-complete dialog for the console
class CCVarWindow : public QWidget
{
	//the model of a single auto-complete item
	struct SCVarItem
	{
		//typed value storage
		union
		{
			i32   intValue;
			float floatValue;
		}       value;
		QString stringValue;

		//name and type
		QString name;
		enum EType
		{
			eType_Int,
			eType_Float,
			eType_String,
			eType_Count
		} type;
	};

	//the model of all hints
	class CCVarModel : public QAbstractListModel
	{
		//the collection of auto-complete hints
		std::vector<SCVarItem> m_items;

		//icons for the hints
		QPixmap m_icons[SCVarItem::eType_Count];

		//the parent window
		CCVarWindow* const m_pParent;

	public:
		//constructor
		CCVarModel(CCVarWindow* pParent);

		//the number of rows in the model
		i32 rowCount(const QModelIndex& parent) const { return m_items.size(); };

		//the number of columns in the model
		i32 columnCount(const QModelIndex& parent) const { return 2; }

		//get flags for an item
		Qt::ItemFlags flags(const QModelIndex& index) const
		{
			Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled; //every column is selectable
			if (index.column() == 1) result |= Qt::ItemIsEditable;           //only the second column is editable
			return result;
		}

		//retrieve data for an item
		QVariant data(const QModelIndex& index, i32 role) const;

		//update data for an item
		bool setData(const QModelIndex& index, const QVariant& value, i32 role);

		//get header information
		QVariant headerData(i32 section, Qt::Orientation orientation, i32 role) const
		{
			if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
			{
				return QVariant(section ? tr("Value") : tr("CVar name"));
			}
			return QVariant();
		}

		//get item by index
		const SCVarItem* Get(size_t index) const { return index < m_items.size() ? &m_items.at(index) : NULL; }

		//add or update item
		void AddOrUpdate(const Messages::SCVarUpdate& update, bool bSuppressLayout);

		//destroy item
		void Destroy(const Messages::SCVarDestroyed& destroyed);

		//iterate over the known items
		//the specified functor should take one argument of type "SCVarItem &"
		template<typename Functor>
		void EnumerateItems(Functor& functor)
		{
			for (std::vector<SCVarItem>::iterator i = m_items.begin(); i != m_items.end(); ++i) functor(*i);
		}
	};

public:
	//constructor
	CCVarWindow();

	//destructor
	~CCVarWindow() {}

	//handle change of filter expression
	void HandleFilterChanged(const QString& filter);

	//send a request to update a CVar
	template<typename TMessage>
	bool SendCVarUpdate(const TMessage& message) const
	{
		return true;
	}

private:
	//handle CVar update
	template<typename T>
	void HandleCVarUpdate(const T& update) { m_model.AddOrUpdate(update, m_bInitializing); }

	//handle CVar destroyed
	void HandleCVarDestroyed(const Messages::SCVarDestroyed& message) { m_model.Destroy(message); }

	//setup this window
	void SetupUI();

	//get unique address
	const string m_address;

	//the model for the collection to display
	CCVarModel m_model;

	//set flag when initializing
	bool m_bInitializing;

	//the address to send CVar updates to
	uint        m_engineAddress;

	QTableView* m_pCVarList;
	QLineEdit*  m_pFilterInput;
};

