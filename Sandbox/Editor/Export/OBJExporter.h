// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __COBJExporter_h__
#define __COBJExporter_h__
#pragma once

#include <IExportManager.h>

class COBJExporter : public IExporter
{
public:
	virtual tukk GetExtension() const;
	virtual tukk GetShortDescription() const;
	virtual bool        ExportToFile(tukk filename, const SExportData* pExportData);
	virtual bool        ImportFromFile(tukk filename, SExportData* pData) { return false; };
	virtual void        Release();

private:
	tukk TrimFloat(float fValue) const;
	string     MakeRelativePath(tukk pMainFileName, tukk pFileName) const;
};

#endif // __COBJExporter_h__

