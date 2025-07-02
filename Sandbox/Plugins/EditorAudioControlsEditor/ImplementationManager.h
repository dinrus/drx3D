// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include <QObject>
#include <drx3D/CoreX/Sandbox/DrxSignal.h>

#include <IImpl.h>

namespace ACE
{
extern Impl::IImpl* g_pIImpl;

class CImplementationManager final : public QObject
{
	Q_OBJECT

public:

	CImplementationManager();
	virtual ~CImplementationManager() override;

	bool LoadImplementation();
	void Release();

	CDrxSignal<void()> SignalImplementationAboutToChange;
	CDrxSignal<void()> SignalImplementationChanged;

private:

	HMODULE m_hMiddlewarePlugin;
};
} //endns ACE

