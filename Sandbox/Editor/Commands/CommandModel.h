// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QWidget>
#include <QAbstractItemModel>

class CUiCommand;
class CCommandModuleDescription;

Q_DECLARE_METATYPE(CCommand*);

class CommandModelFactory
{
public:
	template<typename T>
	static T* Create()
	{
		T* pModel = new T();
		pModel->Initialize();
		return pModel;
	}
};

class CommandModel : public QAbstractItemModel
{
	friend CommandModelFactory;
protected:
	struct CommandModule
	{
		QString                          m_name;
		std::vector<CCommand*>           m_commands;
		const CCommandModuleDescription* m_desc;
	};

public:

	enum class Roles : i32
	{
		SearchRole = Qt::UserRole, //QString
		CommandDescriptionRole,
		CommandPointerRole, //CCommand*
		Max,
	};

	virtual ~CommandModel();

	virtual void Initialize();

	//QAbstractItemModel implementation begin
	virtual i32           columnCount(const QModelIndex& parent /* = QModelIndex() */) const override { return s_ColumnCount; }
	virtual QVariant      data(const QModelIndex& index, i32 role /* = Qt::DisplayRole */) const override;
	virtual bool          hasChildren(const QModelIndex& parent /* = QModelIndex() */) const override;
	virtual QVariant      headerData(i32 section, Qt::Orientation orientation, i32 role /* = Qt::DisplayRole */) const override;
	virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
	virtual QModelIndex   index(i32 row, i32 column, const QModelIndex& parent = QModelIndex()) const override;
	virtual QModelIndex   parent(const QModelIndex& index) const override;
	virtual bool          setData(const QModelIndex& index, const QVariant& value, i32 role /*= Qt::EditRole*/) override;
	virtual i32           rowCount(const QModelIndex& parent /* = QModelIndex() */) const override;
	//QAbstractItemModel implementation end

protected:
	CommandModel();

	virtual void    Rebuild();

	QModelIndex     GetIndex(CCommand* command, uint column = 0) const;
	CCommand*       GetCommand(const QModelIndex& index) const;
	QCommandAction* GetAction(uint moduleIndex, uint commandIndex) const;
	CommandModule&  FindOrCreateModule(const QString& moduleName);

protected:
	static i32k           s_ColumnCount = 3;
	static tukk         s_ColumnNames[3];

	std::vector<CommandModule> m_modules;
};

