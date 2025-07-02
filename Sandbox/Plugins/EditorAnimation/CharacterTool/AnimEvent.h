// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Math.h>

class XmlNodeRef;
class CAnimEventData;
struct AnimEventInstance;

#include <drx3D/CoreX/Serialization/Forward.h>
#include <DrxAnimation/IAnimEventPlayer.h>

namespace CharacterTool
{

struct AnimEvent
{
	float  startTime;
	float  endTime;
	string type;
	string parameter;
	string boneName;
	string model;
	Vec3   offset;
	Vec3   direction;

	AnimEvent()
		: startTime(0.0f)
		, endTime(0.0f)
		, type("audio_trigger")
		, offset(ZERO)
		, direction(ZERO)
	{
	}

	bool operator<(const AnimEvent& animEvent) const { return startTime < animEvent.startTime; }

	void Serialize(Serialization::IArchive& ar);
	void FromData(const CAnimEventData& eventData);
	void ToData(CAnimEventData* eventData) const;
	void ToInstance(AnimEventInstance* instance) const;
	bool LoadFromXMLNode(const XmlNodeRef& node);
};
typedef std::vector<AnimEvent> AnimEvents;

struct AnimEventPreset
{
	string    name;
	float     colorHue;
	AnimEvent event;

	AnimEventPreset() : name("Preset"), colorHue(0.66f) {}

	void Serialize(Serialization::IArchive& ar);
};
typedef std::vector<AnimEventPreset> AnimEventPresets;

bool IsAudioEventType(tukk audioEventType);

class AnimEventPlayer_CharacterTool : public IAnimEventPlayer
{
public:
	DRXINTERFACE_BEGIN()
		DRXINTERFACE_ADD(IAnimEventPlayer)
	DRXINTERFACE_END()
	CRYGENERATE_CLASS_GUID(AnimEventPlayer_CharacterTool, "AnimEventPlayer_CharacterTool", "a5fefb2d-fe05-dec4-a816-9d6e3ac635b0"_drx_guid)

		AnimEventPlayer_CharacterTool();
	virtual ~AnimEventPlayer_CharacterTool() {}

	const SCustomAnimEventType* GetCustomType(i32 customTypeIndex) const override
	{
		i32 listSize = m_list.size();
		i32 lowerLimit = 0;
		for (i32 i = 0; i < listSize; ++i)
		{
			if (!m_list[i])
				continue;
			i32 typeCount = m_list[i]->GetCustomTypeCount();
			if (customTypeIndex >= lowerLimit && customTypeIndex < lowerLimit + typeCount)
				return m_list[i]->GetCustomType(customTypeIndex - lowerLimit);
			lowerLimit += typeCount;
		}
		return 0;
	}

	void Initialize() override
	{
		size_t listSize = m_list.size();
		for (size_t i = 0; i < listSize; ++i)
		{
			if (!m_list[i].get())
				continue;
			m_list[i]->Initialize();
		}
	}

	i32 GetCustomTypeCount() const override
	{
		i32 typeCount = 0;

		size_t listSize = m_list.size();
		for (size_t i = 0; i < listSize; ++i)
		{
			if (!m_list[i].get())
				continue;
			typeCount += m_list[i]->GetCustomTypeCount();
		}

		return typeCount;
	}

	void Serialize(Serialization::IArchive& ar) override
	{
		// m_list could be serialized directly to unlock full customization of the list:
		//   ar(m_list, "list", "^");

		size_t num = m_list.size();
		for (size_t i = 0; i < num; ++i)
			ar(*m_list[i], m_list[i]->GetFactory()->GetName(), m_names[i].c_str());
	}

	tukk SerializeCustomParameter(tukk parameterValue, Serialization::IArchive& ar, i32 customTypeIndex) override
	{
		i32 listSize = m_list.size();
		i32 lowerLimit = 0;
		for (i32 i = 0; i < listSize; ++i)
		{
			if (!m_list[i])
				continue;
			i32 typeCount = m_list[i]->GetCustomTypeCount();
			if (customTypeIndex >= lowerLimit && customTypeIndex < lowerLimit + typeCount)
				return m_list[i]->SerializeCustomParameter(parameterValue, ar, customTypeIndex - lowerLimit);
			lowerLimit += typeCount;
		}

		return "";
	}

	void Reset() override
	{
		i32 listSize = m_list.size();
		for (i32 i = 0; i < listSize; ++i)
		{
			if (!m_list[i])
				continue;
			m_list[i]->Reset();
		}
	}

	void Render(const QuatT& playerPosition, SRendParams& params, const SRenderingPassInfo& passInfo) override
	{
		i32 listSize = m_list.size();
		for (i32 i = 0; i < listSize; ++i)
		{
			if (!m_list[i])
				continue;
			m_list[i]->Render(playerPosition, params, passInfo);
		}
	}

	void Update(ICharacterInstance* character, const QuatT& playerLocation, const Matrix34& cameraMatrix) override
	{
		i32 listSize = m_list.size();
		for (i32 i = 0; i < listSize; ++i)
		{
			if (!m_list[i])
				continue;
			m_list[i]->Update(character, playerLocation, cameraMatrix);
		}
	}

	void EnableAudio(bool enableAudio) override
	{
		i32 listSize = m_list.size();
		for (i32 i = 0; i < listSize; ++i)
		{
			if (!m_list[i])
				continue;
			m_list[i]->EnableAudio(enableAudio);
		}
	}

	bool Play(ICharacterInstance* character, const AnimEventInstance& event) override
	{
		i32 listSize = m_list.size();
		for (i32 i = 0; i < listSize; ++i)
		{
			if (!m_list[i])
				continue;
			if (m_list[i]->Play(character, event))
				return true;
		}
		return false;
	}

private:
	std::vector<IAnimEventPlayerPtr> m_list;
	std::vector<string>              m_names;
	string                           m_defaultGamePlayerName;
};
}

