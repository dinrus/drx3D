// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Movie/StdAfx.h>
#include <drx3D/Movie/AudioNode.h>
#include <drx3D/Movie/AnimTrack.h>
#include <drx3D/Movie/Tracks.h>
#include <algorithm>

namespace AudioNode
{
bool s_audioParamsInit = false;
std::vector<CAnimNode::SParamInfo> s_audioParams;

void AddSupportedParam(tukk sName, i32 paramId, EAnimValue valueType)
{
	CAnimNode::SParamInfo param;
	param.name = sName;
	param.paramType = paramId;
	param.valueType = valueType;
	param.flags = IAnimNode::eSupportedParamFlags_MultipleTracks;
	s_audioParams.push_back(param);
}
};

CAudioNode::CAudioNode(i32k id)
	: CAnimNode(id)
{
	CAudioNode::Initialize();
}

void CAudioNode::Initialize()
{
	if (!AudioNode::s_audioParamsInit)
	{
		AudioNode::s_audioParamsInit = true;
		AudioNode::s_audioParams.reserve(4);

		AudioNode::AddSupportedParam("Trigger", eAnimParamType_AudioTrigger, eAnimValue_Unknown);
		AudioNode::AddSupportedParam("File", eAnimParamType_AudioFile, eAnimValue_Unknown);
		AudioNode::AddSupportedParam("Parameter", eAnimParamType_AudioParameter, eAnimValue_Float);
		AudioNode::AddSupportedParam("Switch", eAnimParamType_AudioSwitch, eAnimValue_Unknown);
	}
}

void CAudioNode::Animate(SAnimContext& animContext)
{
	size_t numAudioParameterTracks = 0;
	size_t numAudioTriggerTracks = 0;
	size_t numAudioSwitchTracks = 0;
	size_t numAudioFileTracks = 0;

	i32k trackCount = NumTracks();
	for (i32 paramIndex = 0; paramIndex < trackCount; ++paramIndex)
	{
		const CAnimParamType paramType = m_tracks[paramIndex]->GetParameterType();
		IAnimTrack* pTrack = m_tracks[paramIndex];

		if (!pTrack || pTrack->GetNumKeys() == 0 || pTrack->GetFlags() & IAnimTrack::eAnimTrackFlags_Disabled)
		{
			continue;
		}
		const bool bMuted = gEnv->IsEditor() && (pTrack->GetFlags() & IAnimTrack::eAnimTrackFlags_Muted);

		switch (paramType.GetType())
		{
		case eAnimParamType_AudioTrigger:
			{
				++numAudioTriggerTracks;

				if (numAudioTriggerTracks > m_audioTriggerTracks.size())
				{
					m_audioTriggerTracks.resize(numAudioTriggerTracks);
				}

				if (!animContext.bResetting && !bMuted && animContext.time > SAnimTime(0))
				{
					SAudioTriggerKey audioTriggerKey;
					SAudioInfo& audioTriggerInfo = m_audioTriggerTracks[numAudioTriggerTracks - 1];
					i32k audioTriggerKeyNum = static_cast<CAudioTriggerTrack*>(pTrack)->GetActiveKey(animContext.time, &audioTriggerKey);
					if (audioTriggerKeyNum >= 0)
					{
						const SAnimTime audioTriggerKeyTime = (animContext.time - audioTriggerKey.m_time);

						if (audioTriggerInfo.audioKeyStart < audioTriggerKeyNum)
						{
							ApplyAudioTriggerKey(audioTriggerKey.m_startTriggerId);
							m_activeAudioTriggers.emplace_back(audioTriggerKey);
						}

						if (audioTriggerInfo.audioKeyStart > audioTriggerKeyNum)
						{
							audioTriggerInfo.audioKeyStop = audioTriggerKeyNum;
						}

						audioTriggerInfo.audioKeyStart = audioTriggerKeyNum;

						if (audioTriggerKey.m_duration > SAnimTime(0) && audioTriggerKeyTime >= audioTriggerKey.m_duration)
						{
							if (audioTriggerInfo.audioKeyStop < audioTriggerKeyNum)
							{
								audioTriggerInfo.audioKeyStop = audioTriggerKeyNum;

								if (audioTriggerKey.m_stopTriggerId != DrxAudio::InvalidControlId)
								{
									ApplyAudioTriggerKey(audioTriggerKey.m_stopTriggerId);
								}
								else
								{
									ApplyAudioTriggerKey(audioTriggerKey.m_startTriggerId, false);
								}

								m_activeAudioTriggers.erase(std::remove(m_activeAudioTriggers.begin(), m_activeAudioTriggers.end(), audioTriggerKey), m_activeAudioTriggers.end());
							}
						}
						else
						{
							audioTriggerInfo.audioKeyStop = -1;
						}
					}
					else
					{
						audioTriggerInfo.audioKeyStart = -1;
						audioTriggerInfo.audioKeyStop = -1;
					}
				}
			}
			break;

		case eAnimParamType_AudioFile:
			{
				++numAudioFileTracks;

				if (numAudioFileTracks > m_audioFileTracks.size())
				{
					m_audioFileTracks.resize(numAudioFileTracks);
				}

				if (!animContext.bResetting && !bMuted)
				{
					SAudioFileKey audioFileKey;
					SAudioInfo& audioFileInfo = m_audioFileTracks[numAudioFileTracks - 1];
					i32k audioFileKeyNum = static_cast<CAudioFileTrack*>(pTrack)->GetActiveKey(animContext.time, &audioFileKey);
					if (audioFileKeyNum >= 0 && audioFileKey.m_duration > SAnimTime(0) && !(audioFileKey.m_bNoTriggerInScrubbing && animContext.bSingleFrame))
					{
						const SAnimTime audioKeyTime = (animContext.time - audioFileKey.m_time);
						if (animContext.time <= audioFileKey.m_time + audioFileKey.m_duration)
						{
							if (audioFileInfo.audioKeyStart < audioFileKeyNum)
							{
								ApplyAudioFileKey(audioFileKey.m_audioFile, audioFileKey.m_bIsLocalized);
							}

							audioFileInfo.audioKeyStart = audioFileKeyNum;
						}
						else if (audioKeyTime >= audioFileKey.m_duration)
						{
							audioFileInfo.audioKeyStart = -1;
						}
					}
					else
					{
						audioFileInfo.audioKeyStart = -1;
					}
				}
			}
			break;

		case eAnimParamType_AudioParameter:
			{
				++numAudioParameterTracks;

				if (numAudioParameterTracks > m_audioParameterTracks.size())
				{
					m_audioParameterTracks.resize(numAudioParameterTracks, 0.0f);
				}

				DrxAudio::ControlId audioParameterId = static_cast<CAudioParameterTrack*>(pTrack)->m_audioParameterId;
				if (audioParameterId != DrxAudio::InvalidControlId)
				{
					const float newAudioParameterValue = stl::get<float>(pTrack->GetValue(animContext.time));
					float& prevAudioParameterValue = m_audioParameterTracks[numAudioParameterTracks - 1];
					if (fabs(prevAudioParameterValue - newAudioParameterValue) > FLT_EPSILON)
					{
						ApplyAudioParameterValue(audioParameterId, newAudioParameterValue);
						prevAudioParameterValue = newAudioParameterValue;
					}
				}
			}
			break;

		case eAnimParamType_AudioSwitch:
			{
				++numAudioSwitchTracks;

				if (numAudioSwitchTracks > m_audioSwitchTracks.size())
				{
					m_audioSwitchTracks.resize(numAudioSwitchTracks, -1);
				}

				if (!animContext.bResetting && !bMuted)
				{
					SAudioSwitchKey audioSwitchKey;
					CAudioSwitchTrack* pAudioSwitchTrack = static_cast<CAudioSwitchTrack*>(pTrack);
					i32& prevAudioSwitchKeyNum = m_audioSwitchTracks[numAudioSwitchTracks - 1];
					i32k newAudioSwitchKeyNum = pAudioSwitchTrack->GetActiveKey(animContext.time, &audioSwitchKey);
					if (prevAudioSwitchKeyNum != newAudioSwitchKeyNum)
					{
						if (newAudioSwitchKeyNum >= 0)
						{
							ApplyAudioSwitchKey(audioSwitchKey.m_audioSwitchId, audioSwitchKey.m_audioSwitchStateId);
							prevAudioSwitchKeyNum = newAudioSwitchKeyNum;
						}
					}
				}
			}
			break;
		}
	}
}

void CAudioNode::OnStart()
{
	DRX_ASSERT_MESSAGE(m_activeAudioTriggers.empty(), "m_activeAudioTriggers is not empty during CAudioNode::OnStart");
}

void CAudioNode::OnReset()
{
	m_audioParameterTracks.clear();
	m_audioTriggerTracks.clear();
	m_audioSwitchTracks.clear();
	m_audioFileTracks.clear();
}

void CAudioNode::OnStop()
{
	for (auto const& audioTriggerKey : m_activeAudioTriggers)
	{
		if (audioTriggerKey.m_stopTriggerId != DrxAudio::InvalidControlId)
		{
			ApplyAudioTriggerKey(audioTriggerKey.m_stopTriggerId);
		}
		else
		{
			ApplyAudioTriggerKey(audioTriggerKey.m_startTriggerId, false);
		}
	}

	m_activeAudioTriggers.clear();
}

u32 CAudioNode::GetParamCount() const
{
	DRX_ASSERT(AudioNode::s_audioParams.size() <= std::numeric_limits<u32>::max());
	return static_cast<u32>(AudioNode::s_audioParams.size());
}

CAnimParamType CAudioNode::GetParamType(u32 index) const
{
	if (index < GetParamCount())
	{
		return AudioNode::s_audioParams[index].paramType;
	}

	return eAnimParamType_Invalid;
}

bool CAudioNode::GetParamInfoFromType(const CAnimParamType& paramId, SParamInfo& info) const
{
	const size_t paramCount = AudioNode::s_audioParams.size();
	for (size_t i = 0; i < paramCount; ++i)
	{
		if (AudioNode::s_audioParams[i].paramType == paramId)
		{
			info = AudioNode::s_audioParams[i];
			return true;
		}
	}

	return false;
}

void CAudioNode::ApplyAudioFileKey(const string& filePath, const bool bLocalized)
{
	if (!filePath.empty())
	{
		DrxAudio::SPlayFileInfo const info(filePath.c_str(), bLocalized);
		gEnv->pAudioSystem->PlayFile(info);
	}
}

void CAudioNode::ApplyAudioSwitchKey(DrxAudio::ControlId audioSwitchId, DrxAudio::SwitchStateId audioSwitchStateId)
{
	if (audioSwitchId != DrxAudio::InvalidControlId && audioSwitchStateId != DrxAudio::InvalidSwitchStateId)
	{
		gEnv->pAudioSystem->SetSwitchState(audioSwitchId, audioSwitchStateId);
	}
}

void CAudioNode::ApplyAudioTriggerKey(DrxAudio::ControlId audioTriggerId, const bool bPlay)
{
	if (audioTriggerId != DrxAudio::InvalidControlId)
	{
		if (bPlay)
		{
			gEnv->pAudioSystem->ExecuteTrigger(audioTriggerId);
		}
		else
		{
			gEnv->pAudioSystem->StopTrigger(audioTriggerId);
		}
	}
}

void CAudioNode::ApplyAudioParameterValue(DrxAudio::ControlId audioParameterId, const float value)
{
	if (audioParameterId != DrxAudio::InvalidControlId)
	{
		gEnv->pAudioSystem->SetParameter(audioParameterId, value);
	}
}
