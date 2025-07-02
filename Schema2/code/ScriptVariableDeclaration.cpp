// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ScriptVariableDeclaration.h>


#include <drx3D/Sys/ArchiveHost.h>
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/Schema2/GUIDRemapper.h>
#include <drx3D/Schema2/IEnvTypeDesc.h>
#include <drx3D/Schema2/ISerializationContext.h>

#include <drx3D/Schema2/AggregateTypeIdSerialize.h>

namespace sxema2
{
	namespace
	{
		typedef std::vector<CAggregateTypeId> TypeIds; // Move inside class/namespace?

		class ScriptVariableTypeCollector
		{
		public:

			inline ScriptVariableTypeCollector(Serialization::StringList& _typeNames, TypeIds& _typeIds)
				: typeNames(_typeNames)
				, typeIds(_typeIds)
			{}

			inline EVisitStatus VisitEnvTypeDesc(const IEnvTypeDesc& typeDesc)
			{
				typeNames.push_back(typeDesc.GetName());
				typeIds.push_back(typeDesc.GetTypeInfo().GetTypeId());
				return EVisitStatus::Continue;
			}

			inline void VisitScriptEnumeration(const IScriptFile& enumerationFile, const IScriptEnumeration& enumeration)
			{
				stack_string fullName;
				DocUtils::GetFullElementName(enumerationFile, enumeration.GetGUID(), fullName);
				typeNames.push_back(fullName.c_str());
				typeIds.push_back(enumeration.GetTypeId());
			}

			inline void VisitScriptStructure(const IScriptFile& structureFile, const IScriptStructure& structure)
			{
				stack_string fullName;
				DocUtils::GetFullElementName(structureFile, structure.GetGUID(), fullName);
				typeNames.push_back(fullName.c_str());
				typeIds.push_back(structure.GetTypeId());
			}

		private:

			Serialization::StringList& typeNames;
			TypeIds&                   typeIds;
		};
	}

	//////////////////////////////////////////////////////////////////////////
	CScriptEnumerationValue::CScriptEnumerationValue(const IScriptEnumeration* pEnumeration)
		: m_pEnumeration(pEnumeration)
		, m_value(0)
	{}

	//////////////////////////////////////////////////////////////////////////
	CScriptEnumerationValue::CScriptEnumerationValue(const CScriptEnumerationValue& rhs)
		: m_pEnumeration(rhs.m_pEnumeration)
		, m_value(rhs.m_value)
	{}

	//////////////////////////////////////////////////////////////////////////	
	u32 CScriptEnumerationValue::GetSize() const
	{
		return sizeof(CScriptStructureValue);
	}

	//////////////////////////////////////////////////////////////////////////
	CTypeInfo CScriptEnumerationValue::GetTypeInfo() const
	{
		return CTypeInfo(m_pEnumeration ? m_pEnumeration->GetGUID() : SGUID(), ETypeFlags::Enum);
	}

	//////////////////////////////////////////////////////////////////////////
	IAny* CScriptEnumerationValue::Clone(uk pPlacement) const
	{
		return new (pPlacement) CScriptEnumerationValue(*this);
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyPtr CScriptEnumerationValue::Clone() const
	{
		return IAnyPtr(new CScriptEnumerationValue(*this));
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptEnumerationValue::ToString(const CharArrayView& str) const
	{
		if(m_pEnumeration)
		{
			StringUtils::Copy(m_pEnumeration->GetConstant(m_value), str);
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	GameSerialization::IContextPtr CScriptEnumerationValue::BindSerializationContext(Serialization::IArchive& archive) const
	{
		return GameSerialization::IContextPtr(new GameSerialization::CContext<const CScriptEnumerationValue>(archive, *this));
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptEnumerationValue::Serialize(Serialization::IArchive& archive, tukk name, tukk label)
	{
		LOADING_TIME_PROFILE_SECTION;
		if(m_pEnumeration)
		{
			if(archive.caps(archive.INPLACE))
			{
				archive(m_value, name);
			}
			else if(archive.isEdit())
			{
				Serialization::StringList constants;
				for(size_t iConstant = 0, constantCount = m_pEnumeration->GetConstantCount(); iConstant < constantCount; ++ iConstant)
				{
					constants.push_back(m_pEnumeration->GetConstant(iConstant));
				}
				if(archive.isInput())
				{
					Serialization::StringListValue constant(constants, 0);
					archive(constant, name, label);
					m_value = constant.index();
				}
				else if(archive.isOutput())
				{
					Serialization::StringListValue constant(constants, m_value);
					archive(constant, name, label);
				}
			}
			else
			{
				if(archive.isInput())
				{
					string constant;
					archive(constant, name);
					m_value = m_pEnumeration->FindConstant(constant.c_str());
				}
				else if(archive.isOutput())
				{
					string constant = m_pEnumeration->GetConstant(m_value);
					archive(constant, name);
				}
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyExtension* CScriptEnumerationValue::QueryExtension(EAnyExtension extension)
	{
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////
	const IAnyExtension* CScriptEnumerationValue::QueryExtension(EAnyExtension extension) const
	{
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////
	uk CScriptEnumerationValue::ToVoidPtr()
	{
		return this;
	}

	//////////////////////////////////////////////////////////////////////////
	ukk CScriptEnumerationValue::ToVoidPtr() const
	{
		return this;
	}

	//////////////////////////////////////////////////////////////////////////
	IAny& CScriptEnumerationValue::operator = (const IAny& rhs)
	{
		if(GetTypeInfo().GetTypeId() == rhs.GetTypeInfo().GetTypeId())
		{
			m_value = static_cast<const CScriptEnumerationValue*>(rhs.ToVoidPtr())->m_value;
		}
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptEnumerationValue::operator == (const IAny& rhs) const
	{
		if(GetTypeInfo().GetTypeId() == rhs.GetTypeInfo().GetTypeId())
		{
			return m_value == static_cast<const CScriptEnumerationValue*>(rhs.ToVoidPtr())->m_value;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	CScriptStructureValue::CScriptStructureValue(const IScriptStructure* pStructure)
		: m_pStructure(pStructure)
	{
		Refresh();
	}

	//////////////////////////////////////////////////////////////////////////
	CScriptStructureValue::CScriptStructureValue(const CScriptStructureValue& rhs)
		: m_pStructure(rhs.m_pStructure)
		, m_fields(rhs.m_fields)
	{
		Refresh();
	}

	//////////////////////////////////////////////////////////////////////////	
	u32 CScriptStructureValue::GetSize() const
	{
		return sizeof(CScriptStructureValue);
	}

	//////////////////////////////////////////////////////////////////////////	
	CTypeInfo CScriptStructureValue::GetTypeInfo() const
	{
		return CTypeInfo(m_pStructure ? m_pStructure->GetGUID() : SGUID(), ETypeFlags::Class);
	}

	//////////////////////////////////////////////////////////////////////////
	IAny* CScriptStructureValue::Clone(uk pPlacement) const
	{
		return new (pPlacement) CScriptStructureValue(*this);
	}

	//////////////////////////////////////////////////////////////////////////	
	IAnyPtr CScriptStructureValue::Clone() const
	{
		return IAnyPtr(new CScriptStructureValue(*this));
	}

	//////////////////////////////////////////////////////////////////////////	
	bool CScriptStructureValue::ToString(const CharArrayView& str) const
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////	
	GameSerialization::IContextPtr CScriptStructureValue::BindSerializationContext(Serialization::IArchive& archive) const
	{
		return GameSerialization::IContextPtr(new GameSerialization::CContext<const CScriptStructureValue>(archive, *this));
	}

	//////////////////////////////////////////////////////////////////////////	
	bool CScriptStructureValue::Serialize(Serialization::IArchive& archive, tukk szName, tukk szLabel)
	{
		archive(*this, szName, szLabel);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////	
	uk CScriptStructureValue::ToVoidPtr()
	{
		return this;
	}

	//////////////////////////////////////////////////////////////////////////	
	ukk CScriptStructureValue::ToVoidPtr() const
	{
		return this;
	}

	//////////////////////////////////////////////////////////////////////////
	IAny& CScriptStructureValue::operator = (const IAny& rhs)
	{
		SXEMA2_SYSTEM_ASSERT("Implement me!!!");
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	bool CScriptStructureValue::operator == (const IAny& rhs) const
	{
		SXEMA2_SYSTEM_ASSERT("Implement me!!!");
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyExtension* CScriptStructureValue::QueryExtension(EAnyExtension extension)
	{
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////
	const IAnyExtension* CScriptStructureValue::QueryExtension(EAnyExtension extension) const
	{
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////	
	void CScriptStructureValue::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;
		if(archive.isOutput()) // Ensure we don't refresh immediately after output and invalidate strings.
		{
			Refresh();
		}
		for(FieldMap::iterator iField = m_fields.begin(), iEndField = m_fields.end(); iField != iEndField; ++ iField)
		{
			if(iField->second)
			{
				tukk 	fieldName = iField->first.c_str();
				archive(*iField->second, fieldName, fieldName);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////	
	void CScriptStructureValue::Refresh()
	{
		if(m_pStructure)
		{
			// #SchematycTODO : How do we know when we really need to refresh?
			FieldMap	newFields;
			for(size_t iField = 0, fieldCount = m_pStructure->GetFieldCount(); iField < fieldCount; ++ iField)
			{
				tukk 	fieldName = m_pStructure->GetFieldName(iField);
				IAnyPtr			pNewValue;
				if(IAnyConstPtr pDefaultValue = m_pStructure->GetFieldValue(iField))
				{
					FieldMap::iterator	iPrevField = m_fields.find(fieldName);
					if((iPrevField != m_fields.end()) && iPrevField->second && (iPrevField->second->GetTypeInfo().GetTypeId() == pDefaultValue->GetTypeInfo().GetTypeId()))
					{
						pNewValue = iPrevField->second;
					}
					else
					{
						pNewValue = pDefaultValue->Clone();
					}
				}
				newFields.insert(FieldMap::value_type(fieldName, pNewValue));
			}
			std::swap(m_fields, newFields);
		}
		else
		{
			m_fields.clear();
		}
	}

	//////////////////////////////////////////////////////////////////////////
	bool ValidateScriptVariableTypeInfo(const IScriptFile& file, const CAggregateTypeId& typeId)
	{
		switch(typeId.GetDomain())
		{
		case EDomain::Env:
			{
				const IEnvTypeDesc* pTypeDesc = gEnv->pSchematyc2->GetEnvRegistry().GetTypeDesc(typeId.AsEnvTypeId());
				if(pTypeDesc)
				{
					return true;
				}
				break;
			}
		case EDomain::Script:
			{
				const IScriptElement* pElement = ScriptIncludeRecursionUtils::GetElement(file, typeId.AsScriptTypeGUID()).second;
				if(pElement)
				{
					const EScriptElementType elementType = pElement->GetElementType();
					if((elementType == EScriptElementType::Enumeration) || (elementType == EScriptElementType::Structure))
					{
						return true;
					}
				}
				break;
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk GetScriptVariableTypeName(const IScriptFile& file, const CAggregateTypeId& typeId)
	{
		switch(typeId.GetDomain())
		{
		case EDomain::Env:
			{
				const IEnvTypeDesc* pTypeDesc = gEnv->pSchematyc2->GetEnvRegistry().GetTypeDesc(typeId.AsEnvTypeId());
				if(pTypeDesc)
				{
					return pTypeDesc->GetName();
				}
				break;
			}
		case EDomain::Script:
			{
				const IScriptElement* pElement = ScriptIncludeRecursionUtils::GetEnumeration(file, typeId.AsScriptTypeGUID()).second;
				if(pElement)
				{
					return pElement->GetName();
				}
				break;
			}
		}
		return "";
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyPtr MakeScriptVariableValueShared(const IScriptFile& file, const CAggregateTypeId& typeId)
	{
		LOADING_TIME_PROFILE_SECTION

		switch(typeId.GetDomain())
		{
		case EDomain::Env:
			{
				const IEnvTypeDesc* pTypeDesc = gEnv->pSchematyc2->GetEnvRegistry().GetTypeDesc(typeId.AsEnvTypeId());
				if(pTypeDesc)
				{
					return pTypeDesc->Create();
				}
				break;
			}
		case EDomain::Script:
			{
				const IScriptElement* pElement = ScriptIncludeRecursionUtils::GetElement(file, typeId.AsScriptTypeGUID()).second;
				if(pElement)
				{
					switch(pElement->GetElementType())
					{
					case EScriptElementType::Enumeration:
						{
							return IAnyPtr(new CScriptEnumerationValue(static_cast<const IScriptEnumeration*>(pElement)));
						}
					case EScriptElementType::Structure:
						{
							return IAnyPtr(new CScriptStructureValue(static_cast<const IScriptStructure*>(pElement)));
						}
					}
				}
				break;
			}
		}
		return IAnyPtr();
	}

	//////////////////////////////////////////////////////////////////////////
	CScriptVariableDeclaration::CScriptVariableDeclaration()
		: m_name("Default")
	{
		m_pValue = IAnyPtr(std::make_shared<CAny<i32>>(0));
		m_typeId = m_pValue->GetTypeInfo().GetTypeId();
	}

	//////////////////////////////////////////////////////////////////////////
	CScriptVariableDeclaration::CScriptVariableDeclaration(tukk szName, const CAggregateTypeId& typeId, const IAnyPtr& pValue)
		: m_name(szName)
		, m_typeId(typeId)
		, m_pValue(pValue)
	{}

	//////////////////////////////////////////////////////////////////////////
	CScriptVariableDeclaration::CScriptVariableDeclaration(tukk szName, const IAnyPtr& pValue)
		: m_name(szName)
		, m_typeId(pValue->GetTypeInfo().GetTypeId())
		, m_pValue(pValue)
	{}

	//////////////////////////////////////////////////////////////////////////
	void CScriptVariableDeclaration::SetName(tukk szName)
	{
		m_name = szName;
	}

	//////////////////////////////////////////////////////////////////////////
	tukk CScriptVariableDeclaration::GetName() const
	{
		return m_name.c_str();
	}

	//////////////////////////////////////////////////////////////////////////
	CAggregateTypeId CScriptVariableDeclaration::GetTypeId() const
	{
		return m_typeId;
	}

	//////////////////////////////////////////////////////////////////////////
	IAnyConstPtr CScriptVariableDeclaration::GetValue() const
	{
		return m_pValue;
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptVariableDeclaration::EnumerateDependencies(const ScriptDependancyEnumerator& enumerator) const
	{
		SXEMA2_SYSTEM_ASSERT(enumerator);
		if(enumerator)
		{
			if(m_typeId.GetDomain() == EDomain::Script)
			{
				enumerator(m_typeId.AsScriptTypeGUID());
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptVariableDeclaration::Serialize(Serialization::IArchive& archive)
	{
		LOADING_TIME_PROFILE_SECTION;
		switch(SerializationContext::GetPass(archive))
		{
		case ESerializationPass::PreLoad:
			{
				SerializeInfo(archive);
				break;
			}
		case ESerializationPass::Load:
			{
				SerializeValue(archive);
				break;
			}
		case ESerializationPass::Save:
		case ESerializationPass::Edit:
			{
				SerializeInfo(archive);
				SerializeValue(archive);
				break;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptVariableDeclaration::SerializeInfo(Serialization::IArchive& archive)
	{
		archive(m_name, "name", "^");
		if(archive.isEdit())
		{
			IScriptFile* pFile = SerializationContext::GetScriptFile(archive);
			SXEMA2_SYSTEM_ASSERT(pFile);
			if(pFile)
			{
				Serialization::StringList typeNames;
				TypeIds                   typeIds;
				if(!m_typeId)
				{
					typeNames.push_back("None");
					typeIds.push_back(CAggregateTypeId());
				}
				ScriptVariableTypeCollector typeCollector(typeNames, typeIds);
				gEnv->pSchematyc2->GetEnvRegistry().VisitTypeDescs(EnvTypeDescVisitor::FromMemberFunction<ScriptVariableTypeCollector, &ScriptVariableTypeCollector::VisitEnvTypeDesc>(typeCollector));
				ScriptIncludeRecursionUtils::VisitEnumerations(*pFile, ScriptIncludeRecursionUtils::EnumerationVisitor::FromMemberFunction<ScriptVariableTypeCollector, &ScriptVariableTypeCollector::VisitScriptEnumeration>(typeCollector), SGUID(), true);
				if(archive.isInput())
				{
					Serialization::StringListValue typeName(typeNames, 0);
					archive(typeName, "type", "Type");
					m_typeId = typeIds[typeName.index()];
				}
				else if(archive.isOutput())
				{
					TypeIds::const_iterator        itTypeId = std::find(typeIds.begin(), typeIds.end(), m_typeId);
					Serialization::StringListValue typeName(typeNames, static_cast<i32>(itTypeId != typeIds.end() ? itTypeId - typeIds.begin() : 0));
					archive(typeName, "type", "Type");
				}
			}
		}
		else
		{
			if(!archive(m_typeId, "typeId"))
			{
				PatchAggregateTypeIdFromDocVariableTypeInfo(archive, m_typeId, "typeInfo");
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptVariableDeclaration::SerializeValue(Serialization::IArchive& archive)
	{
		if((archive.isInput()) && (!m_pValue || (m_typeId != m_pValue->GetTypeInfo().GetTypeId())))
		{
			IScriptFile* pFile = SerializationContext::GetScriptFile(archive);
			SXEMA2_SYSTEM_ASSERT(pFile);
			if(pFile)
			{
				m_pValue = MakeScriptVariableValueShared(*pFile, m_typeId);
			}
		}
		if(m_pValue)
		{
			archive(*m_pValue, "value", "Value");
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void CScriptVariableDeclaration::RemapGUIDs(IGUIDRemapper& guidRemapper)
	{
		if(m_typeId.GetDomain() == EDomain::Script)
		{
			m_typeId = CAggregateTypeId::FromScriptTypeGUID(guidRemapper.Remap(m_typeId.AsScriptTypeGUID()));
		}
	}
}
