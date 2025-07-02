// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct ITaskHost;

class CRcInOrderCaller
{
private:
	typedef std::function<void (CRcInOrderCaller* caller, const QString& filePath, uk pUserData)> Callback;

	struct SPayload
	{
		QString m_filePath;
		uint64  m_timestamp;
		uk   m_pUserData;
	};
public:
	CRcInOrderCaller();
	virtual ~CRcInOrderCaller();

	void    SetAssignCallback(const Callback& callback);
	void    SetDiscardCallback(const Callback& callback);

	QString GetFilePath() const;
	uk   GetUserData() const;

	void    CallRc(const QString& filePath, uk pUserData, ITaskHost* pTaskHost, const QString& options = QString(), const QString& message = QString());

	uint64 GetNextTimestamp() const;
	uint64 GetTimpestamp() const;
private:
	Callback m_onAssign;
	Callback m_onDiscard;
	uint64   m_nextTimestamp;

	QString  m_filePath;
	uk    m_pUserData;
	i32      m_timestamp;
};

// Convenience function that just deletes the payload.
template<typename TYPE>
void DeletePayload(CRcInOrderCaller* caller, const QString& filePath, uk pUserData)
{
	TYPE* pPayload = (TYPE*)pUserData;
	delete pPayload;
}

