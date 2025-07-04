// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "SkyLightNishita.h"

STRUCT_INFO_BEGIN(CSkyLightNishita::SOpticalDepthLUTEntry)
STRUCT_VAR_INFO(mie, TYPE_INFO(f32))
STRUCT_VAR_INFO(rayleigh, TYPE_INFO(f32))
STRUCT_INFO_END(CSkyLightNishita::SOpticalDepthLUTEntry)

STRUCT_INFO_BEGIN(CSkyLightNishita::SOpticalScaleLUTEntry)
STRUCT_VAR_INFO(atmosphereLayerHeight, TYPE_INFO(f32))
STRUCT_VAR_INFO(mie, TYPE_INFO(f32))
STRUCT_VAR_INFO(rayleigh, TYPE_INFO(f32))
STRUCT_INFO_END(CSkyLightNishita::SOpticalScaleLUTEntry)
