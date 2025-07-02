// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "FileMonitorBase.h"

namespace ACE
{
class CFileMonitorSystem final : public CFileMonitorBase
{
	Q_OBJECT

public:

	explicit CFileMonitorSystem(i32 const delay, QObject* const pParent);

	CFileMonitorSystem() = delete;

	void Enable();
	void EnableDelayed();

private:

	string        m_monitorFolder;
	QTimer* const m_delayTimer;
};
} //endns ACE
