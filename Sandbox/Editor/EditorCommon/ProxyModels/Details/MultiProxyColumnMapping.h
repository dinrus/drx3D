// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QVector>
#include <QMap>
#include <QVariant>
#include <QAbstractItemModel>

/**
 * \brief implements support for column mapping for multi model proxies
 */
class CMultiProxyColumnMapping
{
public:
	typedef i32                   SourceColumn;
	typedef i32                   ProxyColumn;

	typedef QVector<ProxyColumn>  SourceToProxy;
	typedef QVector<SourceColumn> ProxyToSource;

	struct MountData
	{
		friend class CMultiProxyColumnMapping;
	public:
		/// \return column in source model for the given proxyColumn
		/// \note returns -1 if column does not exist in source model
		inline SourceColumn GetColumnFromProxy(ProxyColumn proxyColumn) const
		{
			return m_proxyToSource.value(proxyColumn, -1);
		}

		/// \return column in proxy model for the given sourceColumn
		/// \note returns -1 if column is not mapped in the proxy model
		inline ProxyColumn GetColumnFromSource(SourceColumn sourceColumn) const
		{
			return m_sourceToProxy.value(sourceColumn, -1);
		}

	private:
		SourceToProxy m_sourceToProxy; // [source column] = proxy column
		ProxyToSource m_proxyToSource; // [proxy column] = source column (value -1 means no source)
	};

	typedef QVector<QVariant> ColumnValues;

public:
	CMultiProxyColumnMapping(const ColumnValues& = ColumnValues(), i32 valueRole = Qt::DisplayRole);

	inline i32 GetValueRole() const
	{
		return m_valueRole;
	}

	inline i32 GetColumnCount() const
	{
		return m_proxyColumns.count();
	}

	inline QVariant GetColumnValue(i32 proxyColumn) const
	{
		if (proxyColumn < 0 || proxyColumn >= m_proxyColumns.size())
		{
			return QVariant();
		}
		return m_proxyColumns[proxyColumn].key();
	}

	void UpdateMountData(const QAbstractItemModel* sourceModel, MountData& mountData) const;

	/// \note this invalidates all mountData
	void Reset(const ColumnValues&, i32 valueRole);

	/// \note this invalidates all mountData
	void InsertColumns(i32 firstProxyColumn, const ColumnValues&);

	/// \note this invalidates all mountData
	void RemoveColumns(i32 firstProxyColumn, i32 count);

private:
	typedef i32                      ValueRole;
	typedef QVariant                 Value;
	typedef QMap<Value, ProxyColumn> ValueColumn;
	typedef ValueColumn::iterator    ColumnIt;
	typedef QVector<ColumnIt>        ProxyColumns;

private:
	ValueRole    m_valueRole;
	ValueColumn  m_valueColumn;  // [value] = proxy column
	ProxyColumns m_proxyColumns; // [proxy column] = ColumnIt
};

