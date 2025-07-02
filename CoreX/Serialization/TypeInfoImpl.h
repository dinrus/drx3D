// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/DrxTypeInfo.h>
#include <drx3D/CoreX/Serialization/Decorators/Range.h>
#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/CoreX/Math/ISplines.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/CoreX/ParticleSys/ParticleParams.h>
#include <drx3D/CoreX/Serialization/Color.h>
#include <drx3D/CoreX/Math/Drx_Color.h>
#include "Decorators/Resources.h"

struct SPrivateTypeInfoInstanceLevel
{
	SPrivateTypeInfoInstanceLevel(const CTypeInfo* typeInfo, uk object, STypeInfoInstance* instance)
		: m_pTypeInfo(typeInfo)
		, m_pObject(object)
		, m_instance(instance)
	{
	}

	void Serialize(Serialization::IArchive& ar)
	{
		ForAllSubVars(pVar, *m_pTypeInfo)
		{
			string group;
			if (pVar->GetAttr("Group", group))
			{
				if (!m_sCurrentGroup.empty())
				{
					ar.closeBlock();
				}
				tukk name = m_instance->m_persistentStrings.insert(group).first->c_str();
				ar.openBlock(name, name);
				m_sCurrentGroup = name;
			}
			else
			{
				tukk name = pVar->GetName();
				if (!*name)
				{
					name = pVar->Type.Name;
				}

				tukk label = pVar->GetName();
				if (!*label)
				{
					string n = "^";
					n += pVar->GetName();
					label = m_instance->m_persistentStrings.insert(n).first->c_str();
				}

				SerializeVariable(pVar, m_pObject, ar, name, label);
			}
		}

		if (!m_sCurrentGroup.empty())
		{
			ar.closeBlock();
			m_sCurrentGroup.clear();
		}
	}

	template<class T>
	void SerializeT(const CTypeInfo::CVarInfo* pVar, uk pParentAddress, Serialization::IArchive& ar, tukk name, tukk label)
	{
		T value;
		const CTypeInfo& type = pVar->Type;
		type.ToValue(pVar->GetAddress(pParentAddress), value);
		ar(value, name, label);
		if (ar.isInput())
		{
			type.FromValue(pVar->GetAddress(pParentAddress), value);
		}
	}

	template<class T>
	void SerializeNumericalT(const CTypeInfo::CVarInfo* pVar, uk pParentAddress, Serialization::IArchive& ar, tukk name, tukk label)
	{
		T value;
		const CTypeInfo& type = pVar->Type;
		type.ToValue(pVar->GetAddress(pParentAddress), value);

		float limMin, limMax;
		if (pVar->GetLimit(eLimit_Min, limMin) && pVar->GetLimit(eLimit_Max, limMax))
		{
			ar(Serialization::Range<T>(value, limMin, limMax), name, label);
		}
		else
		{
			ar(value, name, label);
		}
		if (ar.isInput())
		{
			type.FromValue(pVar->GetAddress(pParentAddress), value);
		}
	}

	void SerializeVariable(const CTypeInfo::CVarInfo* pVar, uk pParentAddress, Serialization::IArchive& ar, tukk name, tukk label)
	{
		const CTypeInfo& type = pVar->Type;

		if (type.HasSubVars())
		{
			if (strcmp(name, "Color") == 0)
			{
				Color3F value;
				const CTypeInfo& type = pVar->Type;
				type.ToValue(pVar->GetAddress(pParentAddress), value);
				ColorF colour = value;
				ar(colour, name, label);
				if (ar.isInput())
				{
					value = Color3F(colour.r, colour.g, colour.b);
					type.FromValue(pVar->GetAddress(pParentAddress), value);
				}
			}
			else
			{
				// load params of sub-variables (variable is a struct or vector)
				SPrivateTypeInfoInstanceLevel instance(&type, pVar->GetAddress(pParentAddress), m_instance);
				ar(instance, name, label);
			}
		}
		else
		{
			if (type.IsType<bool>())
			{
				SerializeT<bool>(pVar, pParentAddress, ar, name, label);
			}
			else if (type.IsType<u8>())
			{
				SerializeNumericalT<u8>(pVar, pParentAddress, ar, name, label);
			}
			else if (type.IsType<char>())
			{
				SerializeNumericalT<char>(pVar, pParentAddress, ar, name, label);
			}
			else if (type.IsType<i32>())
			{
				SerializeNumericalT<i32>(pVar, pParentAddress, ar, name, label);
			}
			else if (type.IsType<uint>())
			{
				SerializeNumericalT<uint>(pVar, pParentAddress, ar, name, label);
			}
			else if (type.IsType<float>())
			{
				SerializeNumericalT<float>(pVar, pParentAddress, ar, name, label);
			}
			else if (type.EnumElem(0))
			{
				Serialization::StringList stringList;
				tukk enumType = type.EnumElem(0);
				for (i32 i = 1; enumType; ++i)
				{
					stringList.push_back(enumType);
					enumType = type.EnumElem(i);
				}

				string enumValue = pVar->ToString(pParentAddress);
				i32 index = std::max(stringList.find(enumValue.c_str()), 0);

				Serialization::StringListValue stringListValue(stringList, index);
				ar(stringListValue, name, label);
				if (ar.isInput())
				{
					pVar->FromString(pParentAddress, stringListValue.c_str());
				}
			}
			else
			{
				ISplineInterpolator* pSpline = 0;
				if (type.ToValue(pVar->GetAddress(pParentAddress), pSpline))
				{
					// TODO: Curve field
				}
				else
				{
					string value;
					value = pVar->ToString(pParentAddress);

					if (strcmp(name, "Texture") == 0)
					{
						ar(Serialization::TextureFilename(value), name, label);
					}
					else if (strcmp(name, "Material") == 0)
					{
						ar(Serialization::MaterialPicker(value), name, label);
					}
					else if (strcmp(name, "Geometry") == 0)
					{
						ar(Serialization::ModelFilename(value), name, label);
					}
					else if (strcmp(name, "Sound") == 0)
					{
						ar(Serialization::SoundName(value), name, label);
					}
					else if (strcmp(name, "GeomCache") == 0)
					{
						ar(Serialization::GeomCachePicker(value), name, label);
					}
					else
					{
						ar(value, name, label);
					}
					if (ar.isInput())
					{
						pVar->FromString(pParentAddress, value.c_str());
					}
				}
			}
		}
	}

private:
	const CTypeInfo*   m_pTypeInfo;
	uk              m_pObject;
	string             m_sCurrentGroup;
	STypeInfoInstance* m_instance;
};

inline void STypeInfoInstance::Serialize(Serialization::IArchive& ar)
{
	SPrivateTypeInfoInstanceLevel instance(m_pTypeInfo, m_pObject, this);
	instance.Serialize(ar);
}
