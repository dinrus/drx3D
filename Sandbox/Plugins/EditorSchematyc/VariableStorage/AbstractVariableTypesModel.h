// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QString>
#include <QColor>

// Note: This is just a temporary solution for now!

class CDataTypeItem
{
public:
	static const CDataTypeItem& Empty();

	CDataTypeItem(QString name, const QColor& color, const DrxGUID& guid);
	virtual ~CDataTypeItem();

	const QString&          GetName() const  { return m_name; }
	const QColor&           GetColor() const { return m_color; }

	const DrxGUID& GetGUID() const  { return m_guid; }

	bool                    operator==(const CDataTypeItem& other) const;
	bool                    operator!=(const CDataTypeItem& other) const;

protected:
	QString          m_name;
	QColor           m_color;

	DrxGUID m_guid;
};

inline bool CDataTypeItem::operator==(const CDataTypeItem& other) const
{
	if (this != &other || m_guid != other.m_guid)
	{
		return false;
	}
	return true;
}

inline bool CDataTypeItem::operator!=(const CDataTypeItem& other) const
{
	return !(*this == other);
}

class CDataTypesModel
{
	typedef std::unordered_map<DrxGUID, CDataTypeItem*> TypesByGuid;
	typedef std::vector<CDataTypeItem*>                          TypesByIndex;

public:
	static CDataTypesModel& GetInstance();

	u32                  GetTypeItemsCount() const { return m_typesByIndex.size(); }
	CDataTypeItem*          GetTypeItemByIndex(u32 index) const;
	CDataTypeItem*          GetTypeItemByGuid(const DrxGUID& guid) const;

private:
	CDataTypesModel();
	~CDataTypesModel();

	void GenerateTypeInfo();

private:
	TypesByGuid  m_typesByGuid;
	TypesByIndex m_typesByIndex;
};

