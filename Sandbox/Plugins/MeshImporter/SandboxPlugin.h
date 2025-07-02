// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
// Sandbox plugin wrapper.
#pragma once
#include "IEditor.h"
#include "IPlugin.h"

// Base class for plugin
class CFbxToolPlugin : public IPlugin
{
public:
	static CFbxToolPlugin* GetInstance();

	explicit CFbxToolPlugin();
	~CFbxToolPlugin();

	void SetPersonalizationProperty(const QString& propName, const QVariant& value);
	const QVariant& GetPersonalizationProperty(const QString& propName);

	virtual tukk GetPluginName() override;
	virtual tukk GetPluginDescription() override;
	virtual i32       GetPluginVersion() override;

private:
};

