// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <Export/ExportDataTypes.h>

struct IStatObj;

// IExporter: interface to present an exporter
// Exporter is responding to export data from object of IData type
// to file with specified format
// Exporter could be provided by user through plug-in system
struct IExporter
{
	// Get file extension of exporter type, f.i. "obj"
	virtual tukk GetExtension() const = 0;

	// Get short format description for showing it in FileSave dialog
	// Example: "Object format"
	virtual tukk GetShortDescription() const = 0;

	// Implementation of en exporting data to the file
	virtual bool ExportToFile(tukk filename, const SExportData* pData) = 0;
	virtual bool ImportFromFile(tukk filename, SExportData* pData) = 0;

	// Before Export Manager is destroyed Release will be called
	virtual void Release() = 0;
};

// IExportManager: interface to export manager
struct IExportManager
{
	//! Register exporter
	//! return true if succeed, otherwise false
	virtual bool       RegisterExporter(IExporter* pExporter) = 0;

	virtual IExporter* GetExporterForExtension(tukk szExtension) const = 0;

	virtual bool       ExportSingleStatObj(IStatObj* pStatObj, tukk filename) = 0;

	//! Export specified geometry
	//! return true if succeed, otherwise false
	virtual bool Export(tukk defaultName, tukk defaultExt = "", tukk defaultPath = "", bool isSelectedObjects = true,
		bool isSelectedRegionObjects = false, bool isTerrain = false, bool isOccluder = false, bool bAnimationExport = false) = 0;

};

