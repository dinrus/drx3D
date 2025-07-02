// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_DynamicResponseSystem
#include <drx3D/CoreX/Platform/platform.h>

//forward declarations
namespace DRS
{
struct IVariable;
struct IVariableCollection;
struct IResponseActor;
struct IResponseAction;
struct IResponseActionInstance;
struct IResponseCondition;
}

#include <drx3D/CoreX/Project/ProjectDefines.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/CoreX/String/DrxString.h>

#include <drx3D/CoreX/Serialization/yasli/ConfigLocal.h>

#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/ClassFactory.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/CoreX/Serialization/STL.h>

//sxema includes
#include <drx3D/Schema/CoreAPI.h>

#include <drx3D/CoreX/String/HashedString.h>
#include <drx3D/DynRespSys/IDynamicResponseSystem.h>
#include <drx3D/DynRespSys/IDynamicResponseCondition.h>
#include <drx3D/DynRespSys/IDynamicResponseAction.h>

#define DrsLogError(x) DrxFatalError("%s", x);

#if !defined(_RELEASE)
	#define DRS_COLLECT_DEBUG_DATA
	#define DrsLogWarning(x) DrxWarning(VALIDATOR_MODULE_DRS, VALIDATOR_WARNING, "DRS Warning: %s: %s", __FUNCTION__, x);
//#define DrsLogInfo(x) DrxLog("DRS Info: %s: %s", __FUNCTION__, x);
	#define DrsLogInfo(x)
#else
	#define DrsLogWarning(x)
	#define DrsLogInfo(x)
#endif

#define REGISTER_DRS_ACTION(_action, _actionname, _color)                       \
  namespace DRS {                                                               \
  SERIALIZATION_CLASS_NAME(IResponseAction, _action, _actionname, _actionname); \
  SERIALIZATION_CLASS_ANNOTATION(IResponseAction, _action, "color", _color); }

#define REGISTER_DRS_CONDITION(_condition, _conditionname, _color)                          \
  namespace DRS {                                                                           \
  SERIALIZATION_CLASS_NAME(IResponseCondition, _condition, _conditionname, _conditionname); \
  SERIALIZATION_CLASS_ANNOTATION(IResponseCondition, _condition, "color", _color); }
