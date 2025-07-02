// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ScriptElementFactory.h>

#include <drx3D/Schema/ScriptActionInstance.h>
#include <drx3D/Schema/ScriptBase.h>
#include <drx3D/Schema/ScriptClass.h>
#include <drx3D/Schema/ScriptComponentInstance.h>
#include <drx3D/Schema/ScriptConstructor.h>
#include <drx3D/Schema/ScriptEnum.h>
#include <drx3D/Schema/ScriptFunction.h>
#include <drx3D/Schema/ScriptInterface.h>
#include <drx3D/Schema/ScriptInterfaceFunction.h>
#include <drx3D/Schema/ScriptInterfaceImpl.h>
#include <drx3D/Schema/ScriptInterfaceTask.h>
#include <drx3D/Schema/ScriptModule.h>
#include <drx3D/Schema/ScriptSignal.h>
#include <drx3D/Schema/ScriptSignalReceiver.h>
#include <drx3D/Schema/ScriptState.h>
#include <drx3D/Schema/ScriptStateMachine.h>
#include <drx3D/Schema/ScriptStruct.h>
#include <drx3D/Schema/ScriptTimer.h>
#include <drx3D/Schema/ScriptVariable.h>

namespace sxema
{
IScriptElementPtr CScriptElementFactory::CreateElement(EScriptElementType elementType)
{
	switch (elementType)
	{
	case EScriptElementType::Module:
		{
			return std::make_shared<CScriptModule>();
		}
	case EScriptElementType::Enum:
		{
			return std::make_shared<CScriptEnum>();
		}
	case EScriptElementType::Struct:
		{
			return std::make_shared<CScriptStruct>();
		}
	case EScriptElementType::Signal:
		{
			return std::make_shared<CScriptSignal>();
		}
	case EScriptElementType::Constructor:
		{
			return std::make_shared<CScriptConstructor>();
		}
	case EScriptElementType::Function:
		{
			return std::make_shared<CScriptFunction>();
		}
	case EScriptElementType::Interface:
		{
			return std::make_shared<CScriptInterface>();
		}
	case EScriptElementType::InterfaceFunction:
		{
			return std::make_shared<CScriptInterfaceFunction>();
		}
	case EScriptElementType::InterfaceTask:
		{
			return std::make_shared<CScriptInterfaceTask>();
		}
	case EScriptElementType::Class:
		{
			return std::make_shared<CScriptClass>();
		}
	case EScriptElementType::Base:
		{
			return std::make_shared<CScriptBase>();
		}
	case EScriptElementType::StateMachine:
		{
			return std::make_shared<CScriptStateMachine>();
		}
	case EScriptElementType::State:
		{
			return std::make_shared<CScriptState>();
		}
	case EScriptElementType::Variable:
		{
			return std::make_shared<CScriptVariable>();
		}
	case EScriptElementType::Timer:
		{
			return std::make_shared<CScriptTimer>();
		}
	case EScriptElementType::SignalReceiver:
		{
			return std::make_shared<CScriptSignalReceiver>();
		}
	case EScriptElementType::InterfaceImpl:
		{
			return std::make_shared<CScriptInterfaceImpl>();
		}
	case EScriptElementType::ComponentInstance:
		{
			return std::make_shared<CScriptComponentInstance>();
		}
	case EScriptElementType::ActionInstance:
		{
			return std::make_shared<CScriptActionInstance>();
		}
	}
	return IScriptElementPtr();
}
} // sxema
