// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#include <drx3D/Sandbox/Editor/StdAfx.h>
#include <limits.h>
#include "VariableOArchive.h"
#include <drx3D/CoreX/Serialization/Decorators/Resources.h>
#include <drx3D/CoreX/Serialization/Decorators/Range.h>
#include <drx3D/CoreX/Serialization/StringList.h>

#include <CryGame/IGameFramework.h>
#include "IForceFeedbackSystem.h"

//#pragma optimize("", off)
//#pragma inline_depth(0)

using Serialization::CVariableOArchive;

namespace VarUtil
{
template<typename T>
_smart_ptr<IVariable> AddChildVariable(const _smart_ptr<IVariable>& pVariableArray, const T& value, tukk const name, tukk label)
{
	DRX_ASSERT(pVariableArray);

	_smart_ptr<IVariable> pVariable = new CVariable<T>();
	pVariable->SetName(name);
	pVariable->SetHumanName(label);
	pVariable->Set(value);

	pVariableArray->AddVariable(pVariable);

	return pVariable;
}

template<typename TMin, typename TMax>
void SetLimits(const _smart_ptr<IVariable>& pVariable, const TMin minValue, const TMax maxValue)
{
	pVariable->SetLimits(static_cast<float>(minValue), static_cast<float>(maxValue));
}
}

namespace VariableOArchiveHelpers
{
IForceFeedbackSystem* GetForceFeedbackSystem()
{
	if (gEnv->pGameFramework)
	{
		return gEnv->pGameFramework->GetIForceFeedbackSystem();
	}
	return NULL;
}

ICharacterInstance* GetCharacterInstance(const EntityId entityId)
{
	IEntity* const pEntity = gEnv->pEntitySystem->GetEntity(entityId);
	if (pEntity)
	{
		return pEntity->GetCharacter(0);
	}
	return NULL;
}

IDefaultSkeleton* GetDefaultSkeleton(const EntityId entityId)
{
	ICharacterInstance* const pCharacter = GetCharacterInstance(entityId);
	if (pCharacter)
	{
		return &pCharacter->GetIDefaultSkeleton();
	}

	return NULL;
}

IAttachmentManager* GetAttachmentManager(const EntityId entityId)
{
	ICharacterInstance* const pCharacter = GetCharacterInstance(entityId);
	if (pCharacter)
	{
		return pCharacter->GetIAttachmentManager();
	}

	return NULL;
}
}

CVariableOArchive::CVariableOArchive()
	: IArchive(IArchive::OUTPUT | IArchive::EDIT | IArchive::NO_EMPTY_NAMES)
	, m_pVariable(new CVariableArray())
	, m_animationEntityId(0)
{
	m_resourceHandlers["CharacterAnimation"] = &CVariableOArchive::SerializeAnimationName;
	m_resourceHandlers["AudioTrigger"] = &CVariableOArchive::SerializeAudioTriggerName;
	m_resourceHandlers["AudioRTPC"] = &CVariableOArchive::SerializeAudioParameterName;
	m_resourceHandlers["Joint"] = &CVariableOArchive::SerializeJointName;
	m_resourceHandlers["ForceFeedbackId"] = &CVariableOArchive::SerializeForceFeedbackIdName;
	m_resourceHandlers["Attachment"] = &CVariableOArchive::SerializeAttachmentName;
	m_resourceHandlers["Model"] = &CVariableOArchive::SerializeObjectFilename;
	m_resourceHandlers["StaticModel"] = &CVariableOArchive::SerializeObjectFilename;
	m_resourceHandlers["Particle"] = &CVariableOArchive::SerializeParticleName;

	m_structHandlers[TypeID::get < Serialization::IResourceSelector > ().name()] = &CVariableOArchive::SerializeIResourceSelector;
	m_structHandlers[TypeID::get < Serialization::RangeDecorator < float >> ().name()] = &CVariableOArchive::SerializeRangeFloat;
	m_structHandlers[TypeID::get < Serialization::RangeDecorator < i32 >> ().name()] = &CVariableOArchive::SerializeRangeInt;
	m_structHandlers[TypeID::get < Serialization::RangeDecorator < u32 >> ().name()] = &CVariableOArchive::SerializeRangeUInt;
	m_structHandlers[TypeID::get < StringListStaticValue > ().name()] = &CVariableOArchive::SerializeStringListStaticValue;
}

CVariableOArchive::~CVariableOArchive()
{
}

_smart_ptr<IVariable> CVariableOArchive::GetIVariable() const
{
	return m_pVariable;
}

CVarBlockPtr CVariableOArchive::GetVarBlock() const
{
	CVarBlockPtr pVarBlock = new CVarBlock();
	pVarBlock->AddVariable(m_pVariable);
	return pVarBlock;
}

void CVariableOArchive::SetAnimationEntityId(const EntityId animationEntityId)
{
	m_animationEntityId = animationEntityId;
}

bool CVariableOArchive::operator()(bool& value, tukk name, tukk label)
{
	VarUtil::AddChildVariable<bool>(m_pVariable, value, name, label);
	return true;
}

bool CVariableOArchive::operator()(Serialization::IString& value, tukk name, tukk label)
{
	const string valueString = value.get();
	VarUtil::AddChildVariable<string>(m_pVariable, valueString, name, label);
	return true;
}

bool CVariableOArchive::operator()(Serialization::IWString& value, tukk name, tukk label)
{
	CryFatalError("CVarBlockOArchive::operator() with IWString is not implemented");
	return false;
}

bool CVariableOArchive::operator()(float& value, tukk name, tukk label)
{
	VarUtil::AddChildVariable<float>(m_pVariable, value, name, label);
	return true;
}

bool CVariableOArchive::operator()(double& value, tukk name, tukk label)
{
	VarUtil::AddChildVariable<float>(m_pVariable, value, name, label);
	return true;
}

bool CVariableOArchive::operator()(i16& value, tukk name, tukk label)
{
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<i32>(m_pVariable, value, name, label);
	VarUtil::SetLimits(pVariable, SHRT_MIN, SHRT_MAX);
	return true;
}

bool CVariableOArchive::operator()(u16& value, tukk name, tukk label)
{
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<i32>(m_pVariable, value, name, label);
	VarUtil::SetLimits(pVariable, 0, USHRT_MAX);
	return true;
}

bool CVariableOArchive::operator()(i32& value, tukk name, tukk label)
{
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<i32>(m_pVariable, value, name, label);
	VarUtil::SetLimits(pVariable, INT_MIN, INT_MAX);
	return true;
}

bool CVariableOArchive::operator()(u32& value, tukk name, tukk label)
{
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<i32>(m_pVariable, value, name, label);
	VarUtil::SetLimits(pVariable, 0, INT_MAX);
	return true;
}

bool CVariableOArchive::operator()(int64& value, tukk name, tukk label)
{
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<i32>(m_pVariable, value, name, label);
	VarUtil::SetLimits(pVariable, INT_MIN, INT_MAX);
	return true;
}

bool CVariableOArchive::operator()(uint64& value, tukk name, tukk label)
{
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<i32>(m_pVariable, value, name, label);
	VarUtil::SetLimits(pVariable, 0, INT_MAX);
	return true;
}

bool CVariableOArchive::operator()(int8& value, tukk name, tukk label)
{
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<i32>(m_pVariable, value, name, label);
	VarUtil::SetLimits(pVariable, SCHAR_MIN, SCHAR_MAX);
	return true;
}

bool CVariableOArchive::operator()(u8& value, tukk name, tukk label)
{
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<i32>(m_pVariable, value, name, label);
	VarUtil::SetLimits(pVariable, 0, UCHAR_MAX);
	return true;
}

bool CVariableOArchive::operator()(char& value, tukk name, tukk label)
{
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<i32>(m_pVariable, value, name, label);
	VarUtil::SetLimits(pVariable, CHAR_MIN, CHAR_MAX);
	return true;
}

bool CVariableOArchive::operator()(const Serialization::SStruct& ser, tukk name, tukk label)
{
	tukk const typeName = ser.type().name();
	HandlersMap::const_iterator it = m_structHandlers.find(typeName);
	const bool handlerFound = (it != m_structHandlers.end());
	if (handlerFound)
	{
		StructHandlerFunctionPtr pHandler = it->second;
		return (this->*pHandler)(ser, name, label);
	}

	return SerializeStruct(ser, name, label);
}

static tukk gVec4Names[] = { "X", "Y", "Z", "W" };
static tukk gEmptyNames[] = { "" };

bool CVariableOArchive::operator()(Serialization::IContainer& ser, tukk name, tukk label)
{
	CVariableOArchive childArchive;
	childArchive.setFilter(getFilter());
	childArchive.setLastContext(lastContext());

	_smart_ptr<IVariable> pChildVariable = childArchive.GetIVariable();
	pChildVariable->SetName(name);
	pChildVariable->SetHumanName(label);

	m_pVariable->AddVariable(pChildVariable);

	const size_t containerSize = ser.size();

	tukk* nameArray = gEmptyNames;
	size_t nameArraySize = 1;
	if (containerSize >= 2 && containerSize <= 4)
	{
		nameArray = gVec4Names;
		nameArraySize = containerSize;
	}

	size_t index = 0;
	if (0 < containerSize)
	{
		do
		{
			ser(childArchive, nameArray[index % nameArraySize], nameArray[index % nameArraySize]);
			++index;
		}
		while (ser.next());
	}

	return true;
}

bool CVariableOArchive::SerializeStruct(const Serialization::SStruct& ser, tukk name, tukk label)
{
	CVariableOArchive childArchive;
	childArchive.setFilter(getFilter());
	childArchive.setLastContext(lastContext());

	_smart_ptr<IVariable> pChildVariable = childArchive.GetIVariable();
	pChildVariable->SetName(name);
	pChildVariable->SetHumanName(label);

	m_pVariable->AddVariable(pChildVariable);

	const bool serializeSuccess = ser(childArchive);

	return serializeSuccess;
}

bool CVariableOArchive::SerializeAnimationName(const Serialization::IResourceSelector* pSelector, tukk name, tukk label)
{
	const string valueString = pSelector->GetValue();
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<string>(m_pVariable, valueString, name, label);
	pVariable->SetDataType(IVariable::DT_ANIMATION);
	pVariable->SetUserData(gEnv->pEntitySystem->GetEntity(m_animationEntityId));

	return true;
}

bool CVariableOArchive::SerializeAudioTriggerName(const Serialization::IResourceSelector* pSelector, tukk name, tukk label)
{
	const string valueString = pSelector->GetValue();
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<string>(m_pVariable, valueString, name, label);
	pVariable->SetDataType(IVariable::DT_AUDIO_TRIGGER);

	return true;
}

bool CVariableOArchive::SerializeJointName(const Serialization::IResourceSelector* pSelector, tukk name, tukk label)
{
	std::vector<string> jointNames;

	const IDefaultSkeleton* const pDefaultSkeleton = VariableOArchiveHelpers::GetDefaultSkeleton(m_animationEntityId);
	if (pDefaultSkeleton)
	{
		u32k jointCount = pDefaultSkeleton->GetJointCount();
		for (u32 i = 0; i < jointCount; ++i)
		{
			tukk const jointName = pDefaultSkeleton->GetJointNameByID(i);
			jointNames.push_back(jointName);
		}
	}

	const string valueString = pSelector->GetValue();
	CreateChildEnumVariable(jointNames, valueString, name, label);

	return true;
}

bool CVariableOArchive::SerializeParticleName(const Serialization::IResourceSelector* pSelector, tukk name, tukk label)
{
	const string valueString = pSelector->GetValue();
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<string>(m_pVariable, valueString, name, label);
	pVariable->SetDataType(IVariable::DT_PARTICLE_EFFECT);

	return true;
}

void CVariableOArchive::CreateChildEnumVariable(const std::vector<string>& enumValues, const string& value, tukk name, tukk label)
{
	if (enumValues.empty())
	{
		VarUtil::AddChildVariable<string>(m_pVariable, value, name, label);
	}
	else
	{
		_smart_ptr<CVariableEnum<string>> pVariable = new CVariableEnum<string>();
		pVariable->SetName(name);
		pVariable->SetHumanName(label);

		pVariable->AddEnumItem("", "");

		const size_t enumValuesCount = enumValues.size();
		for (size_t i = 0; i < enumValuesCount; ++i)
		{
			pVariable->AddEnumItem(enumValues[i], enumValues[i]);
		}

		pVariable->Set(value);

		m_pVariable->AddVariable(pVariable);
	}
}

bool CVariableOArchive::SerializeForceFeedbackIdName(const Serialization::IResourceSelector* pSelector, tukk name, tukk label)
{
	struct SEffectList
		: public IFFSPopulateCallBack
	{
		virtual void AddFFSEffectName(tukk const pName)
		{
			names.push_back(pName);
		}

		std::vector<string> names;
	} effectList;

	IForceFeedbackSystem* const pForceFeedbackSystem = VariableOArchiveHelpers::GetForceFeedbackSystem();
	if (pForceFeedbackSystem)
	{
		pForceFeedbackSystem->EnumerateEffects(&effectList);
	}

	const string valueString = pSelector->GetValue();
	CreateChildEnumVariable(effectList.names, valueString, name, label);

	return true;
}

bool CVariableOArchive::SerializeAttachmentName(const Serialization::IResourceSelector* pSelector, tukk name, tukk label)
{
	std::vector<string> attachmentNames;

	const IAttachmentManager* const pAttachmentManager = VariableOArchiveHelpers::GetAttachmentManager(m_animationEntityId);
	if (pAttachmentManager)
	{
		i32k attachmentCount = pAttachmentManager->GetAttachmentCount();
		for (i32 i = 0; i < attachmentCount; ++i)
		{
			const IAttachment* const pAttachment = pAttachmentManager->GetInterfaceByIndex(i);
			DRX_ASSERT(pAttachment);
			tukk const attachmentName = pAttachment->GetName();
			attachmentNames.push_back(attachmentName);
		}
	}

	const string valueString = pSelector->GetValue();
	CreateChildEnumVariable(attachmentNames, valueString, name, label);

	return true;
}

bool CVariableOArchive::SerializeObjectFilename(const Serialization::IResourceSelector* pSelector, tukk name, tukk label)
{
	const string valueString = pSelector->GetValue();
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<string>(m_pVariable, valueString, name, label);
	pVariable->SetDataType(IVariable::DT_OBJECT);

	return true;
}

bool CVariableOArchive::SerializeStringListStaticValue(const Serialization::SStruct& ser, tukk name, tukk label)
{
	const StringListStaticValue* const pStringListStaticValue = reinterpret_cast<StringListStaticValue*>(ser.pointer());
	const StringListStatic& stringListStatic = pStringListStaticValue->stringList();
	i32k index = pStringListStaticValue->index();

	_smart_ptr<CVariableEnum<i32>> pVariable = new CVariableEnum<i32>();
	pVariable->SetName(name);
	pVariable->SetHumanName(label);

	const size_t stringListStaticSize = stringListStatic.size();
	for (size_t i = 0; i < stringListStaticSize; ++i)
	{
		pVariable->AddEnumItem(stringListStatic[i], static_cast<i32>(i));
	}

	if (0 <= index)
	{
		DRX_ASSERT(index < stringListStaticSize);
		pVariable->Set(static_cast<i32>(index));
	}

	m_pVariable->AddVariable(pVariable);

	return true;
}

bool CVariableOArchive::SerializeIResourceSelector(const Serialization::SStruct& ser, tukk name, tukk label)
{
	const Serialization::IResourceSelector* pSelector = reinterpret_cast<Serialization::IResourceSelector*>(ser.pointer());

	ResourceHandlersMap::iterator it = m_resourceHandlers.find(pSelector->resourceType);
	if (it != m_resourceHandlers.end())
	{
		return (this->*(it->second))(pSelector, name, label);
	}
	return false;
}

template<class T>
static void SetLimits(IVariable* pVariable, const Serialization::RangeDecorator<T>* pRange, float stepValue)
{
	if (pRange->hardMin != std::numeric_limits<T>::lowest() || pRange->hardMax != std::numeric_limits<T>::max())
	{
		float minimal = (float)pRange->hardMin;
		float maximal = (float)pRange->hardMax;
		bool hardMin = false;
		bool hardMax = false;
		if (pRange->hardMin != std::numeric_limits<T>::lowest())
		{
			minimal = pRange->hardMin;
			hardMin = true;
		}
		if (pRange->hardMax != std::numeric_limits<T>::max())
		{
			maximal = pRange->hardMax;
			hardMax = true;
		}
		pVariable->SetLimits(minimal, maximal, stepValue, hardMin, hardMax);
	}
	else
	{
		float minimal = 0.0f;
		float maximal = 0.0f;
		float oldStep = 0.0f;
		bool hardMin = false;
		bool hardMax = false;
		pVariable->GetLimits(minimal, maximal, oldStep, hardMin, hardMax);
		pVariable->SetLimits(minimal, maximal, stepValue, hardMin, hardMax);
	}
}

bool CVariableOArchive::SerializeRangeFloat(const Serialization::SStruct& ser, tukk name, tukk label)
{
	const Serialization::RangeDecorator<float>* const pRange = reinterpret_cast<Serialization::RangeDecorator<float>*>(ser.pointer());

	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<float>(m_pVariable, *pRange->value, name, label);

	SetLimits(pVariable.get(), pRange, 0.01f);
	return true;
}

bool CVariableOArchive::SerializeRangeInt(const Serialization::SStruct& ser, tukk name, tukk label)
{
	const Serialization::RangeDecorator<i32>* const pRange = reinterpret_cast<Serialization::RangeDecorator<i32>*>(ser.pointer());

	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<i32>(m_pVariable, *pRange->value, name, label);
	SetLimits(pVariable.get(), pRange, 1.0f);
	return true;
}

bool CVariableOArchive::SerializeRangeUInt(const Serialization::SStruct& ser, tukk name, tukk label)
{
	const Serialization::RangeDecorator<u32>* const pRange = reinterpret_cast<Serialization::RangeDecorator<u32>*>(ser.pointer());

	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<i32>(m_pVariable, *pRange->value, name, label);
	SetLimits(pVariable, pRange, 1.0f);
	return true;
}

bool Serialization::CVariableOArchive::SerializeAudioParameterName(const IResourceSelector* pSelector, tukk name, tukk label)
{
	const string valueString = pSelector->GetValue();
	_smart_ptr<IVariable> pVariable = VarUtil::AddChildVariable<string>(m_pVariable, valueString, name, label);
	pVariable->SetDataType(IVariable::DT_AUDIO_RTPC);

	return true;
}

