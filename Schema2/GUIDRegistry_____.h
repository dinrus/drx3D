// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Extension/ClassWeaver.h>
#include <drx3D/Schema2/IFramework.h>

#include <drx3D/Schema2/Compiler.h>
//#include <drx3D/Schema2/DocUpr.h>
#include <drx3D/Schema2/EnvRegistry.h>
#include <drx3D/Schema2/LibRegistry.h>
#include <drx3D/Schema2/Log.h>
#include <drx3D/Schema2/ObjectUpr.h>
#include <drx3D/Schema2/TimerSystem.h>
//#include <drx3D/Schema2/StringPool.h>
#include <drx3D/Schema2/UpdateScheduler.h>

#define SXEMA_FRAMEWORK_CLASS_NAME "GameExtension_SxemaFramework"

namespace sxema
{
	// Framework.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CFramework : public sxema2::IFramework
	{
		DRXINTERFACE_SIMPLE(IFramework)
		DRXGENERATE_CLASS_GUID(CFramework, SXEMA_FRAMEWORK_CLASS_NAME, "96d98d98-35aa-4fb6-830b-53dbfe71908d"_drx_guid)

	public:

		void Init();

		// IFramework
		virtual void SetGUIDGenerator(const TGUIDGenerator& guidGenerator) OVERRIDE;
		virtual sxema2::SGUID CreateGUID() const OVERRIDE;
		virtual IStringPool& GetStringPool() OVERRIDE;
		virtual IEnvRegistry& GetEnvRegistry() OVERRIDE;
		virtual ILibRegistry& GetLibRegistry() OVERRIDE;
		virtual tukk GetRootFolder() const OVERRIDE;
		virtual tukk GetDocFolder() const OVERRIDE;
		virtual tukk GetDocExtension() const OVERRIDE;
		virtual tukk GetSettingsFolder() const OVERRIDE;
		virtual tukk GetSettingsExtension() const OVERRIDE;
		virtual IDocUpr& GetDocUpr() OVERRIDE;
		virtual ICompiler& GetCompiler() OVERRIDE;
		virtual IObjectUpr& GetObjectUpr() OVERRIDE;
		virtual ILog& GetLog() OVERRIDE;
		virtual IUpdateScheduler& GetUpdateScheduler() OVERRIDE;
		virtual ITimerSystem& GetTimerSystem() OVERRIDE;
		virtual IRefreshEnvSignal& GetRefreshEnvSignal() OVERRIDE;
		virtual void RefreshEnv() OVERRIDE;
		// ~IFramework

	private:

		typedef TemplateUtils::CSignal<void ()> TRefreshEnvSignal;

		TGUIDGenerator		m_guidGenerator;
		CStringPool				m_stringPool;
		CEnvRegistry			m_envRegistry;
		CLibRegistry			m_libRegistry;
		CDocUpr				m_docUpr;
		mutable string		m_docFolder;
		mutable string		m_settingsFolder;
		CCompiler					m_compiler;
		CTimerSystem			m_timerSystem;	// TODO : Shutdown is reliant on order of destruction so we should formalize this rather than relying on placement within CFramework class.
		CObjectUpr		m_objectUpr;
		CLog							m_log;
		CUpdateScheduler	m_updateScheduler;
		TRefreshEnvSignal	m_refreshEnvSignal;
	};
}
