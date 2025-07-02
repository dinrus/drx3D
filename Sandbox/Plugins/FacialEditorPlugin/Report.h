// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __REPORT_H__
#define __REPORT_H__

class IReportField
{
public:
	virtual ~IReportField() {}

	virtual tukk GetDescription() const = 0;
	virtual tukk GetText() const = 0;
};

template<typename T, typename G> class CReportField : public IReportField
{
public:
	typedef T Object;
	typedef G TextGetter;

	CReportField(Object& object, tukk description, TextGetter& getter);
	virtual tukk GetDescription() const;
	virtual tukk GetText() const;

private:
	TextGetter m_getter;
	string     m_text;
	string     m_description;
};

class IReportRecord
{
public:
	virtual ~IReportRecord() {}
	virtual i32         GetFieldCount() const = 0;
	virtual tukk GetFieldDescription(i32 fieldIndex) const = 0;
	virtual tukk GetFieldText(i32 fieldIndex) const = 0;
};

template<typename T> class CReportRecord : public IReportRecord
{
public:
	typedef T Object;

	CReportRecord(Object& object);
	virtual ~CReportRecord();
	virtual i32                                   GetFieldCount() const;
	virtual tukk                           GetFieldDescription(i32 fieldIndex) const;
	virtual tukk                           GetFieldText(i32 fieldIndex) const;
	template<typename G> CReportField<Object, G>* AddField(tukk description, G& getter);

private:
	Object         m_object;
	typedef std::vector<IReportField*> FieldContainer;
	FieldContainer m_fields;
};

class CReport
{
public:
	~CReport();
	template<typename T> CReportRecord<T>* AddRecord(T& object);
	i32                                    GetRecordCount() const;
	IReportRecord*                         GetRecord(i32 recordIndex);
	void                                   Clear();

private:
	typedef std::vector<IReportRecord*> RecordContainer;
	RecordContainer m_records;
};

template<typename T, typename G> inline CReportField<T, G>::CReportField(Object& object, tukk description, TextGetter& getter)
	: m_getter(getter),
	m_description(description)
{
	m_text = m_getter(object);
}

template<typename T, typename G> inline tukk CReportField<T, G >::GetDescription() const
{
	return m_description.c_str();
}

template<typename T, typename G> inline tukk CReportField<T, G >::GetText() const
{
	return m_text.c_str();
}

template<typename T> inline CReportRecord<T>::CReportRecord(Object& object)
	: m_object(object)
{
}

template<typename T> inline CReportRecord<T>::~CReportRecord()
{
	for (FieldContainer::iterator it = m_fields.begin(); it != m_fields.end(); ++it)
		delete (*it);
}

template<typename T> inline i32 CReportRecord<T >::GetFieldCount() const
{
	return m_fields.size();
}

template<typename T> inline tukk CReportRecord<T >::GetFieldDescription(i32 fieldIndex) const
{
	return m_fields[fieldIndex]->GetDescription();
}

template<typename T> inline tukk CReportRecord<T >::GetFieldText(i32 fieldIndex) const
{
	return m_fields[fieldIndex]->GetText();
}

template<typename T> template<typename G> inline CReportField<T, G>* CReportRecord<T >::AddField(tukk description, G& getter)
{
	CReportField<Object, G>* field = new CReportField<Object, G>(m_object, description, getter);
	m_fields.push_back(field);
	return field;
}

inline CReport::~CReport()
{
	Clear();
}

template<typename T> inline CReportRecord<T>* CReport::AddRecord(T& object)
{
	CReportRecord<T>* record = new CReportRecord<T>(object);
	m_records.push_back(record);
	return record;
}

inline i32 CReport::GetRecordCount() const
{
	return m_records.size();
}

inline IReportRecord* CReport::GetRecord(i32 recordIndex)
{
	return m_records[recordIndex];
}

inline void CReport::Clear()
{
	for (RecordContainer::iterator it = m_records.begin(); it != m_records.end(); ++it)
		delete (*it);
	m_records.clear();
}

#endif //__REPORT_H__

