// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/ISettingsUpr.h>

namespace sxema
{
	class CSettingsUpr : public ISettingsUpr
	{
	private:

		typedef std::map<string, ISettingsPtr> Settings;

	public:

		// ISettingsUpr
		virtual bool RegisterSettings(tukk szName, const ISettingsPtr& pSettings) override;
		virtual ISettings* GetSettings(tukk szName) const override;
		virtual void VisitSettings(const SettingsVisitor& visitor) const override;
		virtual void LoadAllSettings() override;
		virtual void SaveAllSettings() override;
		// ~ISettingsUpr

	private:

		Settings m_settings;
	};
}
