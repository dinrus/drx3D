// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Move everything out of Serialization namespace?
// #SchematycTODO : Move to BaseEnv!!!

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/CoreX/Serialization/Color.h>
#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/CoreX/Serialization/Decorators/ResourcesAudio.h>
#include <drx3D/CoreX/Serialization/StringList.h>

#include <drx3D/Schema2/IFramework.h>
#include <drx3D/Schema2/IQuickSearchOptions.h>
#include <drx3D/Schema2/ILog.h>
#include <drx3D/Schema2/GameResources.h>

namespace GameSerialization
{
template<typename TYPE> class CEnumBitFlagsDecorator
{
public:

	inline CEnumBitFlagsDecorator(TYPE& value, TYPE base, TYPE filter)
		: m_value(value)
		, m_base(base)
		, m_filter(filter)
	{}

	void Serialize(Serialization::IArchive& archive)
	{
		const Serialization::EnumDescription& enumDescription = Serialization::getEnumDescription<TYPE>();
		if (archive.isInput())
		{
			m_value = m_base;
			for (i32 index = 0, count = enumDescription.count(); index < count; ++index)
			{
				i32k flag = enumDescription.valueByIndex(index);
				if ((static_cast<const UnderlyingType>(m_filter) & flag) != 0)
				{
					bool bFlagValue = false;
					archive(bFlagValue, enumDescription.nameByIndex(index), enumDescription.labelByIndex(index));
					if (bFlagValue)
					{
						m_value = static_cast<TYPE>(static_cast<const UnderlyingType>(m_value) | enumDescription.valueByIndex(index));
					}
				}
			}
		}
		else if (archive.isOutput())
		{
			for (i32 index = 0, count = enumDescription.count(); index < count; ++index)
			{
				i32k flag = enumDescription.valueByIndex(index);
				if ((static_cast<const UnderlyingType>(m_filter) & flag) != 0)
				{
					bool bFlagValue = (static_cast<const UnderlyingType>(m_value) & enumDescription.valueByIndex(index)) != 0;
					archive(bFlagValue, enumDescription.nameByIndex(index), enumDescription.labelByIndex(index));
				}
			}
		}
	}

private:

	typedef typename std::underlying_type<TYPE>::type UnderlyingType;

	TYPE&      m_value;
	const TYPE m_base;
	const TYPE m_filter;
};

template<typename TYPE> CEnumBitFlagsDecorator<TYPE> EnumBitFlags(TYPE& value, TYPE base = TYPE(0), TYPE filter = TYPE(~0))
{
	return CEnumBitFlagsDecorator<TYPE>(value, base, filter);
}

template<i32 TRangeMin_X,
         i32 TRangeMax_X,
         i32 TRangeMin_Y = TRangeMin_X,
         i32 TRangeMax_Y = TRangeMax_X,
         i32 TRangeMin_Z = TRangeMin_Y,
         i32 TRangeMax_Z = TRangeMax_Y>
class CRadianAngleDecorator
{
public:
	CRadianAngleDecorator(Ang3& _angles)
		: m_angles(_angles)
	{}

	void Serialize(Serialization::IArchive& archive)
	{
		m_angles.x = RAD2DEG(m_angles.x);
		m_angles.y = RAD2DEG(m_angles.y);
		m_angles.z = RAD2DEG(m_angles.z);

		archive(Serialization::Range(m_angles.x, float(TRangeMin_X), float(TRangeMax_X)), "degX", "^");
		archive.doc("Rotation around the X axis in degree.");
		archive(Serialization::Range(m_angles.y, float(TRangeMin_Y), float(TRangeMax_Y)), "degY", "^");
		archive.doc("Rotation around the Y axis in degree.");
		archive(Serialization::Range(m_angles.z, float(TRangeMin_Z), float(TRangeMax_Z)), "degZ", "^");
		archive.doc("Rotation around the Z axis in degree.");

		m_angles.x = DEG2RAD(m_angles.x);
		m_angles.y = DEG2RAD(m_angles.y);
		m_angles.z = DEG2RAD(m_angles.z);
	}

private:
	Ang3& m_angles;
};

template<i32 TRangeMin_X,
         i32 TRangeMax_X,
         i32 TRangeMin_Y,
         i32 TRangeMax_Y,
         i32 TRangeMin_Z,
         i32 TRangeMax_Z>
inline CRadianAngleDecorator<TRangeMin_X, TRangeMax_X, TRangeMin_Y, TRangeMax_Y, TRangeMin_Z, TRangeMax_Z> RadianAngleDecorator(Ang3& value)
{
	return CRadianAngleDecorator<TRangeMin_X, TRangeMax_X, TRangeMin_Y, TRangeMax_Y, TRangeMin_Z, TRangeMax_Z>(value);
}

template<i32 TRangeMin_X,
         i32 TRangeMax_X,
         i32 TRangeMin_Y,
         i32 TRangeMax_Y>
inline CRadianAngleDecorator<TRangeMin_X, TRangeMax_X, TRangeMin_Y, TRangeMax_Y> RadianAngleDecorator(Ang3& value)
{
	return CRadianAngleDecorator<TRangeMin_X, TRangeMax_X, TRangeMin_Y, TRangeMax_Y>(value);
}

template<i32 TRangeMin_X,
         i32 TRangeMax_X>
inline CRadianAngleDecorator<TRangeMin_X, TRangeMax_X> RadianAngleDecorator(Ang3& value)
{
	return CRadianAngleDecorator<TRangeMin_X, TRangeMax_X>(value);
}

inline SGameResourceFile GeometryPath(string& value)
{
	return SGameResourceFile(value, "Geometry (cgf)|*.cgf");
}

inline SGameResourceFile GeometryPath(string& value, tukk szFilter, tukk szStartFolder)
{
	return SGameResourceFile(value, szFilter, szStartFolder);
}

inline SGameResourceFile MannequinAnimationDatabasePath(string& value)
{
	return SGameResourceFile(value, "Animation Database (adb)|*.adb");
}

inline SGameResourceFileWithType MannequinControllerDefinitionPath(string& value)
{
	return SGameResourceFileWithType(value, "MannequinControllerDefinition", "Controller Definition (xml)|*.xml");
}

inline SGameResourceFileWithType Material(string& value)
{
	return SGameResourceFileWithType(value, "Material", "Material (mtl)|*.mtl");
}

inline SGameResourceFileWithType Skin(string& value)
{
	return SGameResourceFileWithType(value, "Character", "Attachment Geometry (skin)|*.skin");
}

inline SGameResourceFileWithType Skin(string& value, tukk szFilter, tukk szStartFolder)
{
	return SGameResourceFileWithType(value, "Character", szFilter, szStartFolder);
}

template<class TYPE> SGameResourceSelector<TYPE> BehaviorTree(TYPE& value)
{
	return SGameResourceSelector<TYPE>(value, "BehaviorTree");
}

template<class TYPE> SGameResourceSelector<TYPE> LensFlare(TYPE& value)
{
	return SGameResourceSelector<TYPE>(value, "LensFlare");
}

template<class TYPE> inline SGameResourceSelector<TYPE> SpawnableEntityClassName(TYPE& value)
{
	return SGameResourceSelector<TYPE>(value, "SpawnableEntityClassName");
}

struct SQuickSearchResourceSelector
{
	inline SQuickSearchResourceSelector(sxema2::SQuickSearchOption& value)
		: resourceSelector(value, "QuickSearch")
	{}

	Serialization::ResourceSelector<sxema2::SQuickSearchOption> resourceSelector;
};

inline bool Serialize(Serialization::IArchive& archive, SQuickSearchResourceSelector& value, tukk szName, tukk szLabel)
{
	Serialization::SContext context(archive, static_cast<const sxema2::IQuickSearchOptions*>(&value.resourceSelector.value.options));
	archive(value.resourceSelector, szName, szLabel);
	return true;
}

inline SQuickSearchResourceSelector QuickSearch(sxema2::SQuickSearchOption& value)
{
	return SQuickSearchResourceSelector(value);
}

struct SStringListStaticQuickSearchResourceSelector
{
	inline SStringListStaticQuickSearchResourceSelector(Serialization::StringListStaticValue& value)
		: resourceSelector(value, "StringListStaticQuickSearch")
	{}

	Serialization::ResourceSelector<Serialization::StringListStaticValue> resourceSelector;
};

inline bool Serialize(Serialization::IArchive& archive, SStringListStaticQuickSearchResourceSelector& value, tukk szName, tukk szLabel)
{
	Serialization::SContext context(archive, static_cast<const Serialization::StringListStatic*>(&value.resourceSelector.value.stringList()));
	archive(value.resourceSelector, szName, szLabel);
	return true;
}

inline SStringListStaticQuickSearchResourceSelector QuickSearch(Serialization::StringListStaticValue& value)
{
	return SStringListStaticQuickSearchResourceSelector(value);
}

struct IContext
{
	virtual ~IContext() {}
};

template<class TYPE> class CContext : public Serialization::SContextLink, public IContext
{
public:

	CContext(Serialization::IArchive& archive, TYPE& context)
		: m_archive(archive)
	{
		Serialization::SContextLink::previousContext = m_archive.setLastContext(this);
		Serialization::SContextLink::type = Serialization::TypeID::get<TYPE>();
		Serialization::SContextLink::object = const_cast<TYPE*>(&context);
	}

	~CContext()
	{
		m_archive.setLastContext(Serialization::SContextLink::previousContext);
	}

private:

	Serialization::IArchive& m_archive;
};

template<class TYPE> class CContext<const TYPE> : public IContext
{
public:

	CContext(Serialization::IArchive& archive, const TYPE& context)
		: m_archive(archive)
	{
		m_contextLink.type = Serialization::TypeID::get<const TYPE>();
		m_contextLink.object = const_cast<TYPE*>(&context);
		m_contextLink.previousContext = m_archive.setLastContext(&m_contextLink);
	}

	~CContext()
	{
		m_archive.setLastContext(m_contextLink.previousContext);
	}

private:

	Serialization::IArchive&    m_archive;
	Serialization::SContextLink m_contextLink;
};

DECLARE_SHARED_POINTERS(IContext)
}

namespace sxema2
{
typedef std::vector<string> StringVector;

template<typename TYPE> struct SArchiveBlock
{
	inline SArchiveBlock(TYPE& _value, tukk _szName = "", tukk _szLabel = nullptr)
		: value(_value)
		, szName(_szName)
		, szLabel(_szLabel)
	{}

	void Serialize(Serialization::IArchive& archive)
	{
		archive(value, szName, szLabel);
	}

	TYPE&       value;
	tukk szName;
	tukk szLabel;
};

// Indent serialized value in a new block.
template<typename TYPE> inline SArchiveBlock<TYPE> ArchiveBlock(TYPE& value, tukk szName = "", tukk szLabel = nullptr)
{
	return SArchiveBlock<TYPE>(value, szName, szLabel);
}

struct SCharacterFileName
{
	inline SCharacterFileName() {}

	inline SCharacterFileName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SCharacterFileName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SCharacterFileName& value, tukk szName, tukk szLabel)
{
	auto selector = Serialization::CharacterPath(value.value);
	archive(GameSerialization::SGameResourceSelector<string>(selector), szName, szLabel);
	return true;
}

struct SGeometryFileName
{
	inline SGeometryFileName() {}

	inline SGeometryFileName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SGeometryFileName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SGeometryFileName& value, tukk szName, tukk szLabel)
{
	archive(GameSerialization::GeometryPath(value.value), szName, szLabel);
	return true;
}

struct SMaterialFileName
{
	inline SMaterialFileName() {}

	inline SMaterialFileName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SMaterialFileName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SMaterialFileName& value, tukk szName, tukk szLabel)
{
	auto selector = Serialization::MaterialPicker(value.value);
	archive(GameSerialization::SGameResourceSelector<string>(selector), szName, szLabel);
	return true;
}

struct SParticleEffectName
{
	inline SParticleEffectName() {}

	inline SParticleEffectName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SParticleEffectName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

// TODO pavloi 2016.08.11: remove this hack function, when Serialization::ParticleName() will strat supporting pfx2.
inline bool SerializeParticleName(Serialization::IArchive& archive, string& particleName, tukk szName, tukk szLabel)
{
	if (!archive.isEdit())
	{
		// Doesn't matter which ResourceSelector is used - they all will serialize plain string.
		auto selector = Serialization::ParticlePicker(particleName);
		return archive(GameSerialization::SGameResourceSelector<string>(selector), szName, szLabel);
	}
	else
	{
		tukk szNamePfx2 = "hack_ParticlePFX2";
		tukk szLabelPfx2 = "Particle PFX2";
		tukk szNamePfx1 = "hack_ParticlePFX1";
		tukk szLabelPfx1 = "Particle PFX1";

		const bool bIsPfx2 = particleName.find(".pfx") != string::npos;
		const bool bIsEmpty = particleName.empty();

		if (bIsEmpty)
		{
			string oldValue = particleName;
			const bool bResult = archive(GameSerialization::SGameResourceFile(particleName, "pfx2 particle|*.pfx"), szNamePfx2, szLabelPfx2);
			if (archive.isInput() && (oldValue != particleName))
			{
				return bResult;
			}

			return archive(Serialization::ResourceSelector<string>(particleName, "ParticlePfx1"), szNamePfx1, szLabelPfx1);
		}
		else if (bIsPfx2)
		{
			return archive(GameSerialization::SGameResourceFile(particleName, "pfx2 particle|*.pfx"), szNamePfx2, szLabelPfx2);
		}
		else
		{
			return archive(Serialization::ResourceSelector<string>(particleName, "ParticlePfx1"), szNamePfx1, szLabelPfx1);
		}
	}
}

inline bool Serialize(Serialization::IArchive& archive, SParticleEffectName& value, tukk szName, tukk szLabel)
{
	//archive(Serialization::ParticleName(value.value), szName, szLabel);
	//return true;
	return SerializeParticleName(archive, value.value, szName, szLabel);
}

struct SSkinName
{
	inline SSkinName() {}

	inline SSkinName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SSkinName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SSkinName& value, tukk szName, tukk szLabel)
{
	archive(GameSerialization::SGameResourceFile(value.value, "Attachment Geometry (skin)|*.skin", "Objects"), szName, szLabel);
	return true;
}

struct SSoundName
{
	inline SSoundName() {}

	inline SSoundName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SSoundName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SSoundName& value, tukk szName, tukk szLabel)
{
	// pauls - 1/4/2016 - Removed during integration from main. SSoundName should no longer be used.
	archive(/*Serialization::SoundName(*/ value.value /*)*/, szName, szLabel);
	return true;
}

struct SDialogName
{
	inline SDialogName() {}

	inline SDialogName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SDialogName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SDialogName& value, tukk szName, tukk szLabel)
{
	archive(Serialization::DialogName(value.value), szName, szLabel);
	return true;
}

struct SForceFeedbackId
{
	inline SForceFeedbackId() {}

	inline SForceFeedbackId(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SForceFeedbackId& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SForceFeedbackId& value, tukk szName, tukk szLabel)
{
	archive(Serialization::ForceFeedbackIdName(value.value), szName, szLabel);
	return true;
}

struct SEntityClassName
{
	inline SEntityClassName() {}

	inline SEntityClassName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SEntityClassName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SEntityClassName& value, tukk szName, tukk szLabel)
{
	archive(Serialization::EntityClassName(value.value), szName, szLabel);
	return true;
}

struct SSpawnableEntityClass
{
	inline SSpawnableEntityClass() {}

	inline SSpawnableEntityClass(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SSpawnableEntityClass& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SSpawnableEntityClass& value, tukk szName, tukk szLabel)
{
	archive(GameSerialization::SpawnableEntityClassName(value.value), szName, szLabel);
	return true;
}

struct SActionMapName
{
	inline SActionMapName() {}

	inline SActionMapName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SActionMapName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SActionMapName& value, tukk szName, tukk szLabel)
{
	archive(Serialization::ActionMapName(value.value), szName, szLabel);
	return true;
}

struct SActionMapActionName
{
	inline SActionMapActionName(){}

	inline SActionMapActionName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SActionMapActionName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SActionMapActionName& value, tukk szName, tukk szLabel)
{
	archive(Serialization::ActionMapActionName(value.value), szName, szLabel);
	return true;
}

struct SAudioSwitchName
{
	inline SAudioSwitchName() {}

	inline SAudioSwitchName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SAudioSwitchName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SAudioSwitchName& value, tukk szName, tukk szLabel)
{
	archive(Serialization::AudioSwitch(value.value), szName, szLabel);
	return true;
}

struct SAudioSwitchStateName
{
	inline SAudioSwitchStateName() {}

	inline SAudioSwitchStateName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SAudioSwitchStateName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SAudioSwitchStateName& value, tukk szName, tukk szLabel)
{
	archive(Serialization::AudioSwitchState(value.value), szName, szLabel);
	return true;
}

struct SRtpcName
{
	inline SRtpcName() {}

	inline SRtpcName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SRtpcName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SRtpcName& value, tukk szName, tukk szLabel)
{
	archive(Serialization::AudioRTPC(value.value), szName, szLabel);
	return true;
}

struct SLogStreamName
{
	inline SLogStreamName()
		: value(gEnv->pSchematyc2->GetLog().GetStreamName(LOG_STREAM_DEFAULT))
	{}

	inline SLogStreamName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SLogStreamName& rhs) const
	{
		return value == rhs.value;
	}

	string value;
};

namespace Private
{
struct SLogStreamVisitor
{
	EVisitStatus VisitLogStream(tukk szStreamName, const LogStreamId& streamId)
	{
		streamNames.push_back(szStreamName);
		return EVisitStatus::Continue;
	}

	Serialization::StringList streamNames;
};
}

inline bool Serialize(Serialization::IArchive& archive, SLogStreamName& value, tukk szName, tukk szLabel)
{
	Private::SLogStreamVisitor logStreamVisitor;
	gEnv->pSchematyc2->GetLog().VisitStreams(LogStreamVisitor::FromMemberFunction<Private::SLogStreamVisitor, & Private::SLogStreamVisitor::VisitLogStream>(logStreamVisitor));
	if (archive.isInput())
	{
		Serialization::StringListValue temp(logStreamVisitor.streamNames, 0);
		archive(temp, szName, szLabel);
		value.value = temp.c_str();
	}
	else if (archive.isOutput())
	{
		i32k pos = logStreamVisitor.streamNames.find(value.value.c_str());
		archive(Serialization::StringListValue(logStreamVisitor.streamNames, pos), szName, szLabel);
	}
	return true;
}

typedef TemplateUtils::CDelegate<void (Serialization::IArchive&, Serialization::StringList&, i32&)> DynamicStringListGenerator;

class CDynamicStringListValue
{
	friend bool Serialize(Serialization::IArchive& archive, CDynamicStringListValue& value, tukk szName, tukk szLabel);

public:

	inline CDynamicStringListValue(const DynamicStringListGenerator& stringListGenerator, tukk szValue = "")
		: m_stringListGenerator(stringListGenerator)
		, m_value(szValue)
	{}

	inline tukk c_str() const
	{
		return m_value.c_str();
	}

private:

	inline bool Serialize(Serialization::IArchive& archive, tukk szName, tukk szLabel)
	{
		if (archive.isEdit())
		{
			Serialization::StringList stringList;
			i32 index = Serialization::StringList::npos;
			if (m_stringListGenerator)
			{
				m_stringListGenerator(archive, stringList, index);
			}
			tukk szWarning = nullptr;
			if (!m_value.empty())
			{
				index = stringList.find(m_value.c_str());
				if (index == Serialization::StringList::npos)
				{
					index = stringList.size();
					stringList.push_back(m_value.c_str());
					szWarning = "Current value is out of range!";
				}
			}
			Serialization::StringListValue value(stringList, index);
			archive(value, szName, szLabel);
			if (archive.isInput())
			{
				m_value = value.index() != Serialization::StringList::npos ? value.c_str() : "";
			}
			if (szWarning)
			{
				archive.warning(value, szWarning);
			}
		}
		else
		{
			archive(m_value, szName);
		}
		return true;
	}

	DynamicStringListGenerator m_stringListGenerator;
	string                     m_value;
};

inline bool Serialize(Serialization::IArchive& archive, CDynamicStringListValue& value, tukk szName, tukk szLabel)
{
	return value.Serialize(archive, szName, szLabel);
}

class CDynamicStringListValueList
{
	friend bool Serialize(Serialization::IArchive& archive, CDynamicStringListValueList& value, tukk szName, tukk szLabel);

public:

	inline CDynamicStringListValueList(const DynamicStringListGenerator& stringListGenerator)
		: m_stringListGenerator(stringListGenerator)
	{}

	const StringVector& GetActiveValues() const
	{
		return m_activeValues;
	}

private:

	struct SValue
	{
		inline SValue()
			: bActive(false)
		{}

		SValue(bool _bActive, tukk _szName)
			: bActive(_bActive)
			, name(_szName)
		{}

		void Serialize(Serialization::IArchive& archive)
		{
			if (archive.openBlock("value", "^"))
			{
				archive(bActive, "active", "^");
				archive(name, "name", "!^");
				archive.closeBlock();
			}
		}

		bool   bActive;
		string name;
	};

	typedef std::vector<SValue> ValueVector;

	inline bool Serialize(Serialization::IArchive& archive, tukk szName, tukk szLabel)
	{
		if (archive.isEdit())
		{
			Serialization::StringList stringList;
			i32 index = Serialization::StringList::npos;
			if (m_stringListGenerator)
			{
				m_stringListGenerator(archive, stringList, index);
			}

			m_values.clear();
			m_values.reserve(stringList.size());
			for (Serialization::StringList::value_type& stringListValue : stringList)
			{
				m_values.push_back(SValue(false, stringListValue.c_str()));
			}

			StringVector missingValues;
			for (string& activeValue : m_activeValues)
			{
				bool bFoundValue = false;
				for (SValue& value : m_values)
				{
					if (activeValue == value.name)
					{
						value.bActive = true;
						bFoundValue = true;
						break;
					}
				}
				if (!bFoundValue)
				{
					missingValues.push_back(activeValue);
				}
			}

			for (string& missingValue : missingValues)
			{
				stl::push_back_unique(m_activeValues, missingValue);
				archive.warning(m_values, "Missing value: %s", missingValue.c_str());
				m_values.push_back(SValue(true, missingValue));
			}

			archive(m_values, szName, szLabel);

			if (archive.isInput())
			{
				m_activeValues.clear();
				for (SValue& value : m_values)
				{
					if (value.bActive)
					{
						m_activeValues.push_back(value.name);
					}
				}
			}
			else if (archive.isOutput())
			{
				archive(m_activeValues, "activeValues");
			}
		}
		else
		{
			archive(m_activeValues, "activeValues");
		}
		return true;
	}

	DynamicStringListGenerator m_stringListGenerator;
	ValueVector                m_values;
	StringVector               m_activeValues;
};

inline bool Serialize(Serialization::IArchive& archive, CDynamicStringListValueList& value, tukk szName, tukk szLabel)
{
	return value.Serialize(archive, szName, szLabel);
}

template<typename TYPE> inline bool SerializeWithNewName(Serialization::IArchive& archive, TYPE& value, tukk szOldName, tukk szNewName, tukk szLabel)
{
	if (archive.isInput())
	{
		if (archive(value, szNewName, szLabel))
		{
			return true;
		}
		else
		{
			return archive(value, szOldName, szLabel);
		}
	}
	else if (archive.isOutput())
	{
		return archive(value, szNewName, szLabel);
	}
	return true;
}
}
