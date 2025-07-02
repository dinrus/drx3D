// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <DrxSystem/File/IFileChangeMonitor.h>
#include <QTimer>

namespace ACE
{
class CFileMonitorBase : public QTimer, public IFileChangeListener
{
	Q_OBJECT

public:

	void Disable();

signals:

	void SignalReloadData();

protected:

	explicit CFileMonitorBase(i32 const delay, QObject* const pParent);
	virtual ~CFileMonitorBase() override;

	CFileMonitorBase() = delete;

	// IFileChangeListener
	virtual void OnFileChange(char const* szFileName, EChangeType type) override;
	// ~IFileChangeListener

	i32 const m_delay;
};
} //endns ACE
