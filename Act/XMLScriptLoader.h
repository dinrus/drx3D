// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __XMLSCRIPTLOADER_H__
#define __XMLSCRIPTLOADER_H__

#pragma once

SmartScriptTable XmlScriptLoad(tukk definitionFile, tukk dataFile);
SmartScriptTable XmlScriptLoad(tukk definitionFile, XmlNodeRef data);
XmlNodeRef       XmlScriptSave(tukk definitionFile, SmartScriptTable scriptTable);
bool             XmlScriptSave(tukk definitionFile, tukk dataFile, SmartScriptTable scriptTable);

#endif
