// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLEXMLSAVER_H__
#define __VEHICLEXMLSAVER_H__

#pragma once

struct IVehicleData;

XmlNodeRef VehicleDataSave(tukk definitionFile, IVehicleData* pData);
bool       VehicleDataSave(tukk definitionFile, tukk dataFile, IVehicleData* pData);

// This Save method merges the vehicle data using the original source xml
// without losing data that is unknown to the vehicle definition
XmlNodeRef VehicleDataMergeAndSave(tukk originalXml, XmlNodeRef definition, IVehicleData* pData);

#endif

