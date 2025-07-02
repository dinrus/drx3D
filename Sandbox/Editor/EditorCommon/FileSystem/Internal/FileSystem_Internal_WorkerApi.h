// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

#include "FileSystem/FileSystem_SubTreeMonitor.h"
#include "FileSystem/FileSystem_FileMonitor.h"

#include <QObject>
#include <QThread>

namespace FileSystem
{

class CEnumerator;

namespace Internal
{

class CWorker;

/// \brief interface from any thread to worker thread
class CWorkerApi
	: public QObject
{
	Q_OBJECT

public:
	CWorkerApi(const QString& absoluteEnginePath, QObject* parent = nullptr);
	~CWorkerApi();

signals:
	void StartFileMonitor(i32 handle, const FileSystem::SFileFilter& filter, const FileSystem::FileMonitorPtr&);
	void StopFileMonitor(i32 handle);

	void StartSubTreeMonitor(i32 handle, const FileSystem::SFileFilter& filter, const FileSystem::SubTreeMonitorPtr&);
	void StopSubTreeMonitor(i32 handle);

	void RegisterFileTypes(const QVector<const SFileType*>& fileTypes);
	void IsScannerActiveChanged(bool);

private slots:
	void SetCurrentSnapshot(const SnapshotPtr&);
	void SetIsScannerActive(bool);

	void TriggerFileMonitorActivated(const FileMonitorPtr&, const SnapshotPtr&);
	void TriggerFileMonitorUpdate(const FileMonitorPtr&, const SFileMonitorUpdate&);

	void TriggerSubTreeMonitorActivated(const SubTreeMonitorPtr&, const SnapshotPtr&);
	void TriggerSubTreeMonitorUpdate(const SubTreeMonitorPtr&, const SSubTreeMonitorUpdate&);

private:
	friend class FileSystem::CEnumerator;
	const SnapshotPtr& GetCurrentSnapshot() const { return m_currentSnapshot; }
	bool               IsScannerActive() const;

private:
	bool        m_isScannerActive;
	SnapshotPtr m_currentSnapshot;
	QThread     m_thread;
	CWorker*    m_worker;
};

} //endns Internal
} //endns FileSystem

