// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include "Util/Variable.h"
#include "Serialization.h"

namespace Serialization
{
struct IResourceSelector;

class CVariableOArchive
	: public IArchive
{
public:
	CVariableOArchive();
	virtual ~CVariableOArchive();

	_smart_ptr<IVariable> GetIVariable() const;
	CVarBlockPtr          GetVarBlock() const;

	void                  SetAnimationEntityId(const EntityId animationEntityId);

	// IArchive
	virtual bool operator()(bool& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(IString& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(IWString& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(float& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(double& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(i16& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(u16& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(i32& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(u32& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(int64& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(uint64& value, tukk name = "", tukk label = 0) override;

	virtual bool operator()(int8& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(u8& value, tukk name = "", tukk label = 0) override;
	virtual bool operator()(char& value, tukk name = "", tukk label = 0);

	virtual bool operator()(const SStruct& ser, tukk name = "", tukk label = 0) override;
	virtual bool operator()(IContainer& ser, tukk name = "", tukk label = 0) override;
	//virtual bool operator()( IPointer& ptr, tukk name = "", tukk label = 0 ) override;
	// ~IArchive

	using IArchive::operator();

private:
	bool SerializeStruct(const SStruct& ser, tukk name, tukk label);
	bool SerializeStringListStaticValue(const SStruct& ser, tukk name, tukk label);
	bool SerializeRangeFloat(const SStruct& ser, tukk name, tukk label);
	bool SerializeRangeInt(const SStruct& ser, tukk name, tukk label);
	bool SerializeRangeUInt(const SStruct& ser, tukk name, tukk label);
	bool SerializeIResourceSelector(const SStruct& ser, tukk name, tukk label);

	bool SerializeAnimationName(const IResourceSelector* pSelector, tukk name, tukk label);
	bool SerializeAudioTriggerName(const IResourceSelector* pSelector, tukk name, tukk label);
	bool SerializeAudioParameterName(const IResourceSelector* pSelector, tukk name, tukk label);
	bool SerializeJointName(const IResourceSelector* pSelector, tukk name, tukk label);
	bool SerializeForceFeedbackIdName(const IResourceSelector* pSelector, tukk name, tukk label);
	bool SerializeAttachmentName(const IResourceSelector* pSelector, tukk name, tukk label);
	bool SerializeObjectFilename(const IResourceSelector* pSelector, tukk name, tukk label);
	bool SerializeParticleName(const IResourceSelector* pSelector, tukk name, tukk label);

	void CreateChildEnumVariable(const std::vector<string>& enumValues, const string& value, tukk name, tukk label);

private:
	_smart_ptr<IVariable> m_pVariable;

	typedef bool (CVariableOArchive::*                 StructHandlerFunctionPtr)(const SStruct&, tukk , tukk );
	typedef std::map<string, StructHandlerFunctionPtr> HandlersMap;
	HandlersMap m_structHandlers;   // TODO: have only one of these.

	typedef bool (CVariableOArchive::*                   ResourceHandlerFunctionPtr)(const IResourceSelector*, tukk , tukk );
	typedef std::map<string, ResourceHandlerFunctionPtr> ResourceHandlersMap;
	ResourceHandlersMap m_resourceHandlers;

	EntityId            m_animationEntityId;
};
}

