// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __VEHICLEXMLLOADER_H__
#define __VEHICLEXMLLOADER_H__

#pragma once

struct IVehicleData;

IVehicleData* VehicleDataLoad(tukk definitionFile, tukk dataFile, bool bFillDefaults = true);
IVehicleData* VehicleDataLoad(const XmlNodeRef& definition, tukk dataFile, bool bFillDefaults = true);
IVehicleData* VehicleDataLoad(const XmlNodeRef& definition, const XmlNodeRef& data, bool bFillDefaults = true);

#endif

