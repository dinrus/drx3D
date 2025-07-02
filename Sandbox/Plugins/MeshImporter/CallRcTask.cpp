// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "CallRcTask.h"
#include "RcCaller.h"
#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.h>
#include <drx3D/CoreX/ToolsHelpers/ResourceCompilerHelper.inl>
#include <drx3D/CoreX/ToolsHelpers/SettingsManagerHelpers.inl>
#include <drx3D/CoreX/ToolsHelpers/EngineSettingsManager.inl>
#include "IEditor.h"
#include <QFileInfo>

void LogPrintf(tukk szFormat, ...);

CCallRcTask::CCallRcTask()
	: m_metaDataFilename()
	, m_pUserData(nullptr)
	, m_bSucceeded(false)
{}

void CCallRcTask::SetMetaDataFilename(const QString& szFilename)
{
	m_metaDataFilename = szFilename;
}

void CCallRcTask::SetCallback(const Callback& callback)
{
	m_callback = callback;
}

void CCallRcTask::SetUserData(uk pUserData)
{
	m_pUserData = pUserData;
}

void CCallRcTask::SetOptions(const QString& options)
{
	m_options = options;
}

void CCallRcTask::SetMessage(const QString& message)
{
	m_message = message;
}

uk CCallRcTask::GetUserData()
{
	return m_pUserData;
}

bool CCallRcTask::Succeeded() const
{
	return m_bSucceeded;
}

bool CCallRcTask::InitializeTask()
{
	return true;
}

namespace
{

class CRcListener : public CRcCaller::IListener
{
public:
	explicit CRcListener(CAsyncTaskBase& task)
		: m_task(task)
	{
	}

	virtual void ShowWarning(const string& message) override
	{
		m_task.ShowWarning(QtUtil::ToQString(message));
	}

	virtual void ShowError(const string& message) override
	{
		m_task.ShowError(QtUtil::ToQString(message));
	}
private:
	CAsyncTaskBase& m_task;
};

} //endns

bool CCallRcTask::PerformTask()
{
	this->UpdateProgress(m_message.isEmpty() ? QStringLiteral("Compiling ...") : m_message, -1);

	const QString options = m_options + " /refresh";

	CRcCaller caller;
	CRcListener listener(*this);
	caller.SetListener(&listener);
	caller.SetAdditionalOptions(QtUtil::ToString(options));
	if (!caller.Call(QtUtil::ToString(m_metaDataFilename)))
	{
		LogPrintf("CallResourceCompiler failed\n");
		return false;
	}
	return true;
}

void CCallRcTask::FinishTask(bool bTaskSucceeded)
{
	m_bSucceeded = bTaskSucceeded;
	if (m_callback)
	{
		m_callback(this);
	}
	delete this;
}

void CCallRcTask::Launch(const QString& metaDataFilename, uk pUserData, const CCallRcTask::Callback& callback, ITaskHost* pTaskHost, const QString& options, const QString& message)
{
	CCallRcTask* pTask = new CCallRcTask();
	pTask->SetMetaDataFilename(metaDataFilename);
	pTask->SetCallback(callback);
	pTask->SetUserData(pUserData);
	pTask->SetOptions(options);
	pTask->SetMessage(message);
	pTaskHost->RunTask(pTask);
}

