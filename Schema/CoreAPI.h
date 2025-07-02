// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

// Prerequisite headers.

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/IXml.h>

// Core headers.

#include <drx3D/Schema/Action.h>
#include <drx3D/Schema/Component.h>
#include <drx3D/Schema/FundamentalTypes.h>
#include <drx3D/Schema/ICore.h>
#include <drx3D/Schema/IObject.h>
#include <drx3D/Schema/IObjectProperties.h>
#include <drx3D/Schema/ResourceTypes.h>
#include <drx3D/Schema/MathTypes.h>

#include <drx3D/Schema/CompilerContext.h>
#include <drx3D/Schema/ICompiler.h>
#include <drx3D/Schema/IGraphNodeCompiler.h>

#include <drx3D/Schema/IQuickSearchOptions.h>

#include <drx3D/Schema/EnvContext.h>
#include <drx3D/Schema/EnvElementBase.h>
#include <drx3D/Schema/EnvPackage.h>
#include <drx3D/Schema/EnvUtils.h>
#include <drx3D/Schema/IEnvContext.h>
#include <drx3D/Schema/IEnvElement.h>
#include <drx3D/Schema/IEnvPackage.h>
#include <drx3D/Schema/IEnvRegistrar.h>
#include <drx3D/Schema/IEnvRegistry.h>

#include <drx3D/Schema/EnvAction.h>
#include <drx3D/Schema/EnvClass.h>
#include <drx3D/Schema/EnvComponent.h>
#include <drx3D/Schema/EnvDataType.h>
#include <drx3D/Schema/EnvFunction.h>
#include <drx3D/Schema/EnvInterface.h>
#include <drx3D/Schema/EnvModule.h>
#include <drx3D/Schema/EnvSignal.h>
#include <drx3D/Schema/IEnvAction.h>
#include <drx3D/Schema/IEnvClass.h>
#include <drx3D/Schema/IEnvComponent.h>
#include <drx3D/Schema/IEnvDataType.h>
#include <drx3D/Schema/IEnvFunction.h>
#include <drx3D/Schema/IEnvInterface.h>
#include <drx3D/Schema/IEnvModule.h>
#include <drx3D/Schema/IEnvSignal.h>

#include <drx3D/Schema/INetworkObject.h>
#include <drx3D/Schema/INetworkSpawnParams.h>

#include <drx3D/Schema/ActionDesc.h>
#include <drx3D/Schema/ComponentDesc.h>
#include <drx3D/Schema/FunctionDesc.h>
#include <drx3D/Schema/ReflectionUtils.h>
#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/TypeOperators.h>

#include <drx3D/Schema/IRuntimeClass.h>
#include <drx3D/Schema/IRuntimeRegistry.h>
#include <drx3D/Schema/RuntimeGraph.h>
#include <drx3D/Schema/RuntimeParamMap.h>
#include <drx3D/Schema/RuntimeParams.h>

#include <drx3D/Schema/IScript.h>
#include <drx3D/Schema/IScriptElement.h>
#include <drx3D/Schema/IScriptExtension.h>
#include <drx3D/Schema/IScriptGraph.h>
#include <drx3D/Schema/IScriptRegistry.h>
#include <drx3D/Schema/IScriptView.h>
#include <drx3D/Schema/ScriptDependencyEnumerator.h>
#include <drx3D/Schema/ScriptUtils.h>

#include <drx3D/Schema/IScriptActionInstance.h>
#include <drx3D/Schema/IScriptBase.h>
#include <drx3D/Schema/IScriptClass.h>
#include <drx3D/Schema/IScriptComponentInstance.h>
#include <drx3D/Schema/IScriptConstructor.h>
#include <drx3D/Schema/IScriptEnum.h>
#include <drx3D/Schema/IScriptFunction.h>
#include <drx3D/Schema/IScriptInterface.h>
#include <drx3D/Schema/IScriptInterfaceFunction.h>
#include <drx3D/Schema/IScriptInterfaceImpl.h>
#include <drx3D/Schema/IScriptInterfaceTask.h>
#include <drx3D/Schema/IScriptModule.h>
#include <drx3D/Schema/IScriptRoot.h>
#include <drx3D/Schema/IScriptSignal.h>
#include <drx3D/Schema/IScriptSignalReceiver.h>
#include <drx3D/Schema/IScriptState.h>
#include <drx3D/Schema/IScriptStateMachine.h>
#include <drx3D/Schema/IScriptStruct.h>
#include <drx3D/Schema/IScriptTimer.h>
#include <drx3D/Schema/IScriptVariable.h>

#include <drx3D/Schema/ContainerSerializationUtils.h>
#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/IValidatorArchive.h>
#include <drx3D/Schema/MultiPassSerializer.h>
#include <drx3D/Schema/SerializationQuickSearch.h>
#include <drx3D/Schema/SerializationToString.h>
#include <drx3D/Schema/SerializationUtils.h>

#include <drx3D/Schema/ILog.h>
#include <drx3D/Schema/ILogRecorder.h>
#include <drx3D/Schema/ISettingsUpr.h>
#include <drx3D/Schema/ITimerSystem.h>
#include <drx3D/Schema/IUpdateScheduler.h>
#include <drx3D/Schema/LogMetaData.h>
#include <drx3D/Schema/LogStreamName.h>

#include <drx3D/Schema/Any.h>
#include <drx3D/Schema/AnyArray.h>
#include <drx3D/Schema/Array.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/DrxLinkUtils.h>
#include <drx3D/Schema/Delegate.h>
#include <drx3D/Schema/EnumFlags.h>
#include <drx3D/Schema/GUID.h>
#include <drx3D/Schema/HybridArray.h>
#include <drx3D/Schema/IGUIDRemapper.h>
#include <drx3D/Schema/IInterfaceMap.h>
#include <drx3D/Schema/IString.h>
#include <drx3D/Schema/PreprocessorUtils.h>
#include <drx3D/Schema/RingBuffer.h>
#include <drx3D/Schema/Rotation.h>
#include <drx3D/Schema/ScopedConnection.h>
#include <drx3D/Schema/Scratchpad.h>
#include <drx3D/Schema/SharedString.h>
#include <drx3D/Schema/Signal.h>
#include <drx3D/Schema/StackString.h>
#include <drx3D/Schema/STLUtils.h>
#include <drx3D/Schema/StringHashWrapper.h>
#include <drx3D/Schema/StringUtils.h>
#include <drx3D/Schema/Transform.h>
#include <drx3D/Schema/TypeName.h>
#include <drx3D/Schema/TypeUtils.h>
#include <drx3D/Schema/UniqueId.h>
#include <drx3D/Schema/Validator.h>
