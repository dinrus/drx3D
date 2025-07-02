// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "AsyncHelper.h"
#include "FbxMetaData.h"

class CCallRcTask : public CAsyncTaskBase
{
public:
	typedef std::function<void (CCallRcTask*)> Callback;

	CCallRcTask();

	void  SetMetaDataFilename(const QString& filename);
	void  SetCallback(const Callback& callback);
	void  SetUserData(uk pUserData);
	void  SetOptions(const QString& options);
	void  SetMessage(const QString& message);

	uk GetUserData();

	bool  Succeeded() const;

	// CAsyncTaskBase implementation.

	virtual bool InitializeTask() override;
	virtual bool PerformTask() override;
	virtual void FinishTask(bool bTaskSucceeded) override;

	// Convenience function that creates and launches a task.
	static void Launch(const QString& metaDataFilename, uk pUserData, const CCallRcTask::Callback& callback, ITaskHost* pTaskHost, const QString& options = QString(), const QString& message = QString());
private:
	QString  m_metaDataFilename;
	Callback m_callback;
	uk    m_pUserData;
	QString  m_options;
	QString  m_message;
	bool     m_bSucceeded;
};

