// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
#include "IPlugin.h"

class CDependencyGraph : public IPlugin
{
public:
	CDependencyGraph();
	~CDependencyGraph();

	i32       GetPluginVersion()     { return 1; };
	tukk GetPluginName()        { return "Dependency Graph Plugin"; };
	tukk GetPluginDescription() { return "Plugin provides a graph view of asset dependencies"; };

	void SetPersonalizationProperty(const QString& propName, const QVariant& value);
	const QVariant& GetPersonalizationProperty(const QString& propName);

	static CDependencyGraph* GetInstance();
private:
};

