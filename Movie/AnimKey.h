// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/Sys/IConsole.h>     // <> required for Interfuscator
#include <drx3D/Sys/IDrxPak.h> // <> required for Interfuscator
#include <drx3D/Sys/ILocalizationUpr.h>
#include <drx3D/CoreX/Math/Bezier.h>
#include <drx3D/CoreX/Audio/IAudioSystem.h>
#include <drx3D/Movie/AnimTime.h>

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/IntrusiveFactory.h>
#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/CoreX/Serialization/Color.h>
#include <drx3D/CoreX/Serialization/Math.h>
#include <drx3D/CoreX/Serialization/Decorators/Resources.h>
#include <drx3D/CoreX/Serialization/Decorators/ResourceFolderPath.h>
#include <drx3D/CoreX/Serialization/Decorators/ResourceFilePath.h>
#include <drx3D/CoreX/Serialization/Decorators/ResourcesAudio.h>

#include <drx3D/CoreX/Extension/DrxGUID.h>

struct STrackKey
{
	SAnimTime          m_time;

	static tukk GetType()                 { return "Key"; }
	static bool        DerivesTrackDurationKey() { return false; }

	void               Serialize(Serialization::IArchive& ar)
	{
		ar(m_time, "time", "Time");
	}

	tukk GetDescription() const { return ""; }

	// compare keys.
	bool operator<(const STrackKey& key) const  { return m_time < key.m_time; }
	bool operator==(const STrackKey& key) const { return m_time == key.m_time; }
	bool operator>(const STrackKey& key) const  { return m_time > key.m_time; }
	bool operator<=(const STrackKey& key) const { return m_time <= key.m_time; }
	bool operator>=(const STrackKey& key) const { return m_time >= key.m_time; }
	bool operator!=(const STrackKey& key) const { return m_time != key.m_time; }
};

/** Base class for all keys that have a simple duration
 */
struct STrackDurationKey : public STrackKey
{
	STrackDurationKey() : m_duration(0.0f) {}

	static tukk GetType()                 { return "Duration"; }
	static bool        DerivesTrackDurationKey() { return true; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);
		ar(m_duration, "duration", "Duration");
	}

	SAnimTime m_duration;
};

/** Keys with a controllable range. Used for animations.
 */
struct STimeRangeKey : public STrackKey
{
	STimeRangeKey()
		: m_startTime(0.0f)
		, m_endTime(0.0f)
		, m_speed(1.0f)
		, m_bLoop(false)
	{}

	static tukk GetType() { return "TimeRange"; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);
		ar(m_startTime, "startTime", "Start Time");
		ar(m_endTime, "endTime", "End Time");
		ar(m_speed, "speed", "Speed");
		ar(m_bLoop, "loop", "Loop");
	}

	float m_startTime;  // Start time of this animation (Offset from beginning of animation).
	float m_endTime;    // End time of this animation (can be smaller than the duration).
	float m_speed;      // Speed multiplier for this key.
	bool  m_bLoop;      // True if time is looping
};

/** S2DBezierKey used in float spline tracks.
 */
struct S2DBezierKey : public STrackKey
{
	static tukk GetType() { return "2DBezier"; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);
		m_controlPoint.Serialize(ar);
	}

	SBezierControlPoint m_controlPoint;
};

/** SEventKey used in Entity track.
 */
struct SEventKey : public STrackDurationKey
{
	SEventKey() : m_value(0.0f), m_bNoTriggerInScrubbing(false) {}

	static tukk GetType()              { return "Event"; }
	tukk        GetDescription() const { return m_event.c_str(); }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackDurationKey::Serialize(ar);

		ar(Serialization::EntityEventPicker(m_event), "event", "Event");
		ar(m_eventValue, "eventValue", "Event Value");
		ar(m_animation, "animation", "Animation");
		ar(m_value, "value", "Value");
		ar(m_bNoTriggerInScrubbing, "noTriggerInScrubbing", "No Trigger in Scrubbing");
	}

	string m_event;
	string m_eventValue;
	string m_animation;
	float  m_value;
	bool   m_bNoTriggerInScrubbing;
};

struct STrackEventKey : public STrackKey
{
	STrackEventKey() : m_bNoTriggerInScrubbing(false) {}

	static tukk GetType()              { return "TrackEvent"; }
	tukk        GetDescription() const { return m_event.c_str(); }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);

		ar(Serialization::SequenceEventPicker(m_event), "event", "Event");
		ar(m_eventValue, "event", "Event Value");
		ar(m_bNoTriggerInScrubbing, "noTriggerInScrubbing", "No Trigger in Scrubbing");
	}

	string m_event;
	string m_eventValue;
	bool   m_bNoTriggerInScrubbing;
};

/** SCameraKey is used for camera tracks
 */
struct SCameraKey : public STrackKey
{
	SCameraKey() : m_blendTime(0.0f)
	{
		m_selection[0] = 0;
	}

	static tukk GetType()              { return "Camera"; }
	tukk        GetDescription() const { return m_selection; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);

		string selection(m_selection);
		ar(Serialization::SequenceCameraPicker(selection), "selection", "Selection");

		if (ar.isInput())
			drx_strcpy(m_selection, selection.c_str());

		ar(m_blendTime, "blendTime", "Blend Time");
	}

	float m_blendTime;
	char  m_selection[128]; // Node name.
};

struct SFaceSequenceKey : public STrackDurationKey
{
	SFaceSequenceKey() : m_blendTime(0.0f)
	{
		m_selection[0] = 0;
	}

	static tukk GetType()              { return "FaceSequence"; }
	tukk        GetDescription() const { return m_selection; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackDurationKey::Serialize(ar);

		if (ar.isInput())
		{
			string selection;
			ar(selection, "selection", "Selection");
			drx_strcpy(m_selection, selection.c_str());
		}
		else
		{
			string selection = m_selection;
			ar(selection, "selection", "Selection");
		}

		ar(m_blendTime, "blendTime", "Blend Time");
	}

	char  m_selection[128]; // Node name.
	float m_blendTime;
};

/** SSequenceKey used in sequence track.
 */
struct SSequenceKey : public STrackKey
{
	SSequenceKey()
		: m_sequenceGUID(DrxGUID::Null())
		, m_speed(1.0f)
		, m_boverrideTimes(false)
		, m_bDoNotStop(false) {}

	static tukk GetType() { return "Sequence"; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);
		ar(m_startTime, "startTime", "Start Time");
		ar(m_endTime, "endTime", "End Time");
		ar(m_speed, "speed", "Speed");
		ar(m_sequenceGUID, "sequenceGUID");
		ar(m_boverrideTimes, "overrideTimes", "override Times");
		ar(m_bDoNotStop, "doNotStop", "Do Not Stop");
	}

	DrxGUID   m_sequenceGUID;
	SAnimTime m_startTime;
	SAnimTime m_endTime;
	float     m_speed;
	bool      m_boverrideTimes;
	bool      m_bDoNotStop;
};

/** SAudioTriggerKey used in audio trigger track.
 */
struct SAudioTriggerKey : public STrackDurationKey
{
	SAudioTriggerKey()
		: STrackDurationKey()
	{}

	static tukk GetType()              { return "AudioTrigger"; }
	tukk        GetDescription() const { return m_keyDescription.c_str(); }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackDurationKey::Serialize(ar);

		ar(Serialization::AudioTrigger(m_startTriggerName), "startTrigger", "Start Trigger");

		if (m_duration > SAnimTime(0))
		{
			ar(Serialization::AudioTrigger(m_stopTriggerName), "stopTrigger", "Stop Trigger");
			m_keyDescription.Format("%s : %s", m_startTriggerName.c_str(), m_stopTriggerName.c_str());
		}
		else
		{
			m_keyDescription = m_startTriggerName;
		}

		if (ar.isInput())
		{
			m_startTriggerId = DrxAudio::StringToId(m_startTriggerName.c_str());
			m_stopTriggerId = DrxAudio::StringToId(m_stopTriggerName.c_str());
		}
	}

	string              m_startTriggerName;
	string              m_stopTriggerName;
	string              m_keyDescription;
	DrxAudio::ControlId m_startTriggerId;
	DrxAudio::ControlId m_stopTriggerId;
};

/** SAudioFileKey used in audio file track.
 */
struct SAudioFileKey : public STrackDurationKey
{
	SAudioFileKey()
		: STrackDurationKey()
		, m_bIsLocalized(false)
		, m_bNoTriggerInScrubbing(false)
	{}

	static tukk GetType()              { return "AudioFile"; }
	tukk        GetDescription() const { return m_audioFile; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);

		ar(Serialization::SoundFilename(m_audioFile), "file", "Audio File");
		ar(m_bIsLocalized, "isLocalized", "Localized");
		ar(m_bNoTriggerInScrubbing, "noTriggerInScrubbing", "No Trigger in Scrubbing");

		if (!PathUtil::GetFileName(m_audioFile).empty())
		{
			i32 pathLength = m_audioFile.find(PathUtil::GetGameFolder());
			const string tempFilePath = (pathLength == -1) ? PathUtil::GetGameFolder() + DRX_NATIVE_PATH_SEPSTR + m_audioFile : m_audioFile;

			DrxAudio::SFileData audioData;
			gEnv->pAudioSystem->GetFileData(tempFilePath.c_str(), audioData);
			m_duration = audioData.duration;
		}
		else
		{
			if (m_bIsLocalized)
			{
				tukk szLanguage = gEnv->pSystem->GetLocalizationUpr()->GetLanguage();
				m_audioFile = PathUtil::GetGameFolder() + DRX_NATIVE_PATH_SEPSTR + PathUtil::GetLocalizationFolder() + DRX_NATIVE_PATH_SEPSTR + szLanguage + DRX_NATIVE_PATH_SEPSTR;
			}
			else
			{
				m_audioFile = PathUtil::GetGameFolder() + DRX_NATIVE_PATH_SEPSTR;
			}
		}
	}

	string m_audioFile;
	bool   m_bIsLocalized;
	bool   m_bNoTriggerInScrubbing;
};

/** SAudioSwitchKey used in CAudioSwitchTrack
 */
struct SAudioSwitchKey : public STrackKey
{
	SAudioSwitchKey()
		: STrackKey()
		, m_audioSwitchId(DrxAudio::InvalidControlId)
		, m_audioSwitchStateId(DrxAudio::InvalidSwitchStateId)
	{}

	static tukk GetType()              { return "AudioSwitch"; }
	tukk        GetDescription() const { return m_keyDescription.c_str(); }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);

		ar(Serialization::AudioSwitch(m_audioSwitchName), "switchName", "Switch Name");
		ar(Serialization::AudioSwitchState(m_audioSwitchStateName), "switchState", "Switch State");

		if (ar.isInput())
		{
			m_audioSwitchId = DrxAudio::StringToId(m_audioSwitchName.c_str());
			m_audioSwitchStateId = DrxAudio::StringToId(m_audioSwitchStateName.c_str());
		}

		m_keyDescription.Format("%s : %s", m_audioSwitchName.c_str(), m_audioSwitchStateName.c_str());
	}

	string                  m_audioSwitchName;
	string                  m_audioSwitchStateName;
	string                  m_keyDescription;

	DrxAudio::ControlId     m_audioSwitchId;
	DrxAudio::SwitchStateId m_audioSwitchStateId;
};

struct SDynamicResponseSignalKey : public STrackKey
{
	SDynamicResponseSignalKey()
		: STrackKey()
		, m_bNoTriggerInScrubbing(false)
	{}

	static tukk GetType()              { return "DynamicResponseSignal"; }
	tukk        GetDescription() const { return m_signalName; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);

		ar(m_signalName, "signalName", "Signal Name");
		ar(m_contextVariableName, "contextVariableName", "Context Variable Name");
		ar(m_contextVariableValue, "contextVariableValue", "Context Variable Value");
		ar(m_bNoTriggerInScrubbing, "noTriggerInScrubbing", "No Trigger in Scrubbing");
	}

	string m_signalName;
	string m_contextVariableName;
	string m_contextVariableValue;
	bool   m_bNoTriggerInScrubbing;
};

/** SCharacterKey used in Character animation track.
 */
struct SCharacterKey : public STimeRangeKey
{
	SCharacterKey()
		: m_defaultAnimDuration(0.0f)
		, m_bBlendGap(false)
		, m_bUnload(false)
		, m_bInPlace(false)
	{
		m_animation[0] = 0;
	}

	static tukk GetType()              { return "Character"; }
	tukk        GetDescription() const { return m_animation; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STimeRangeKey::Serialize(ar);

		string animation(m_animation);
		ar(Serialization::TrackCharacterAnimationPicker(animation), "animation", "Animation");
		if (ar.isInput())
			drx_strcpy(m_animation, animation.c_str());

		ar(m_bBlendGap, "blendGap", "Blend Gap");
		ar(m_bUnload, "unload", "Unload");
		ar(m_bInPlace, "inPlace", "In Place");
	}

	float GetMaxEndTime() const
	{
		if (m_endTime == 0.0f || (!m_bLoop && m_endTime > GetAnimDuration()))
		{
			return GetAnimDuration();
		}

		return m_endTime;
	}

	float GetCroppedAnimDuration() const
	{
		if ((m_startTime > 0.0f) && (m_endTime > 0.0f))
		{
			return (m_startTime < m_endTime) ? m_endTime - m_startTime : 0.0f;
		}
		else if (m_startTime > 0.0f)
		{
			return max(0.0f, m_defaultAnimDuration - m_startTime);
		}
		else if (m_endTime > 0.0f)
		{
			return min(m_defaultAnimDuration, m_endTime);
		}

		return m_defaultAnimDuration;
	}

	float GetAnimDuration() const
	{
		return m_defaultAnimDuration;
	}

	char  m_animation[64]; // Name of character animation needed for animation system.
	float m_defaultAnimDuration;  // Caches the duration of the referenced animation
	bool  m_bBlendGap;     // True if gap to next animation should be blended
	bool  m_bUnload;       // Unload after sequence is finished
	bool  m_bInPlace;      // Play animation in place (Do not move root).
};

/** SMannequinKey used in Mannequin animation track.
 */
struct SMannequinKey : public STrackDurationKey
{
	SMannequinKey() : m_priority(0)
	{
		m_fragmentName[0] = 0;
		m_tags[0] = 0;
	}

	static tukk GetType()              { return "Mannequin"; }
	tukk        GetDescription() const { return m_fragmentName; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackDurationKey::Serialize(ar);

		if (ar.isInput())
		{
			string fragmentName;
			ar(fragmentName, "fragmentName", "Fragment Name");
			drx_strcpy(m_fragmentName, fragmentName.c_str());

			string tags;
			ar(tags, "tags", "Tags");
			drx_strcpy(m_tags, tags.c_str());
		}
		else
		{
			string fragmentName = m_fragmentName;
			ar(fragmentName, "fragmentName", "Fragment Name");

			string tags = m_tags;
			ar(tags, "tags", "Tags");
		}

		ar(m_priority, "priority", "Priority");
	}

	char m_fragmentName[64]; // Name of character animation.
	char m_tags[64];         // Name of character animation.
	i32  m_priority;
};

/** SExprKey used in expression animation track.
 */
struct SExprKey : public STrackKey
{
	SExprKey() : m_amp(1.0f), m_blendIn(0.5f),
		m_hold(1.0f), m_blendOut(0.5f)
	{
		m_name[0] = 0;
	}

	static tukk GetType()              { return "Expr"; }
	tukk        GetDescription() const { return m_name; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);

		if (ar.isInput())
		{
			string name;
			ar(name, "name", "Name");
			drx_strcpy(m_name, name.c_str());
		}
		else
		{
			string name = m_name;
			ar(name, "name", "Name");
		}

		ar(m_amp, "amp", "Amp");
		ar(m_blendIn, "blendIn", "Blend In");
		ar(m_blendOut, "blendOut", "Blend Out");
		ar(m_hold, "hold", "Hold");
	}

	char  m_name[128]; // Name of morph-target
	float m_amp;
	float m_blendIn;
	float m_hold;
	float m_blendOut;
};

/** SConsoleKey used in Console track, triggers console commands and variables.
 */
struct SConsoleKey : public STrackDurationKey
{
	SConsoleKey()
	{
		m_command[0] = 0;
	}

	static tukk GetType()              { return "Console"; }
	tukk        GetDescription() const { return m_command; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackDurationKey::Serialize(ar);

		if (ar.isInput())
		{
			string command;
			ar(command, "command", "Command");
			drx_strcpy(m_command, command.c_str());
		}
		else
		{
			string command = m_command;
			ar(command, "command", "Command");
		}
	}

	char m_command[64];
};

struct SMusicKey : public STrackKey
{
	enum EMusicKeyType
	{
		eMusicKeyType_SetMood    = 0,
		eMusicKeyType_VolumeRamp = 1,
	};

	SMusicKey() : m_type(eMusicKeyType_SetMood), m_volumeRampTime(0.0f)
	{
		m_mood[0] = 0;
		m_description[0] = 0;
	}

	static tukk GetType()              { return "Music"; }
	tukk        GetDescription() const { return m_description; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);
		ar(m_type, "type", "Type");

		if (ar.isInput())
		{
			string mood;
			ar(mood, "mood", "Mood");
			drx_strcpy(m_mood, mood.c_str());

			string description;
			ar(description, "description", "Description");
			drx_strcpy(m_description, description.c_str());
		}
		else
		{
			string mood = m_mood;
			ar(mood, "mood", "Mood");

			string description = m_description;
			ar(description, "description", "Description");
		}
	}

	EMusicKeyType m_type;
	float         m_volumeRampTime;
	char          m_mood[64];
	char          m_description[32];
};

struct SLookAtKey : public STrackDurationKey
{
	SLookAtKey()
	{
		m_selection[0] = 0;
		m_lookPose[0] = 0;
	}

	static tukk GetType()              { return "LookAt"; }
	tukk        GetDescription() const { return m_selection; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackDurationKey::Serialize(ar);

		if (ar.isInput())
		{
			string selection;
			ar(selection, "selection", "Selection");
			drx_strcpy(m_selection, selection.c_str());

			string lookPose;
			ar(lookPose, "lookPose", "Look Pose");
			drx_strcpy(m_lookPose, lookPose.c_str());
		}
		else
		{
			string selection = m_selection;
			ar(selection, "selection", "Selection");

			string lookPose = m_lookPose;
			ar(lookPose, "lookPose", "Look Pose");
		}

		ar(m_smoothTime, "smoothTime", "Smooth Time");
	}

	char  m_selection[128]; // Node name.
	char  m_lookPose[128];
	float m_smoothTime;
};

/* Discrete (non-interpolated) float key.
 */
struct SDiscreteFloatKey : public STrackKey
{
	SDiscreteFloatKey() : m_value(0.0f) {}

	static tukk GetType() { return "DiscreteFloat"; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);
		ar(m_value, "value", "Value");
	}

	float m_value;
};

struct SCaptureFormatInfo
{
	enum ECaptureFileFormat
	{
		eCaptureFormat_TGA,
		eCaptureFormat_JPEG,
		eCaptureFormat_PNG,
		eCaptureFormat_Num
	};

	static tukk GetCaptureFormatExtension(ECaptureFileFormat captureFormat)
	{
		tukk captureFormatNames[eCaptureFormat_Num] =
		{
			"tga",
			"jpg",
			"png"
		};
		return captureFormatNames[captureFormat];
	};

	static ECaptureFileFormat GetCaptureFormatByExtension(tukk szFormatName)
	{
		for (i32 i = 0; i < eCaptureFormat_Num; ++i)
		{
			ECaptureFileFormat captureFormat = static_cast<ECaptureFileFormat>(i);
			if (!stricmp(szFormatName, GetCaptureFormatExtension(captureFormat)))
			{
				return captureFormat;
			}
		}
		DrxLog("Can't find specified capture format: %s - reverting to %s", szFormatName, GetCaptureFormatExtension(eCaptureFormat_TGA));
		return eCaptureFormat_TGA;
	};

	enum ECaptureBuffer
	{
		eCaptureBuffer_Color,
		eCaptureBuffer_Num
	};

	static tukk GetCaptureBufferName(ECaptureBuffer captureBuffer)
	{
		tukk captureBufferNames[eCaptureBuffer_Num] =
		{
			"Color"
		};
		return captureBufferNames[captureBuffer];
	};

	static ECaptureBuffer GetCaptureBufferByName(tukk szBufferName)
	{
		for (i32 i = 0; i < eCaptureFormat_Num; ++i)
		{
			ECaptureBuffer captureBuffer = static_cast<ECaptureBuffer>(i);
			if (!stricmp(szBufferName, GetCaptureBufferName(captureBuffer)))
			{
				return captureBuffer;
			}
		}
		DrxLog("Can't find specified capture buffer type: %s - reverting to %s", szBufferName, GetCaptureBufferName(eCaptureBuffer_Color));
		return eCaptureBuffer_Color;
	};
};

struct SCaptureKey : public STrackDurationKey
{
	SCaptureKey()
		: m_bOnce(false)
		, m_timeStep(0.033f)
		, m_frameRate(30)
		, m_bufferToCapture(SCaptureFormatInfo::eCaptureBuffer_Color)
		, m_captureFormat(SCaptureFormatInfo::eCaptureFormat_TGA)
	{
		memset(m_folder, 0, sizeof(m_folder));
		memset(m_prefix, 0, sizeof(m_prefix));

		if (m_frameRate > 0)
		{
			m_timeStep = 1.0f / m_frameRate;
		}

		ICVar* pCaptureFolderCVar = gEnv->pConsole->GetCVar("capture_folder");
		if (pCaptureFolderCVar && pCaptureFolderCVar->GetString())
		{
			drx_strcpy(m_folder, pCaptureFolderCVar->GetString());
		}

		ICVar* pCaptureFilePrefixCVar = gEnv->pConsole->GetCVar("capture_file_prefix");
		if (pCaptureFilePrefixCVar && pCaptureFilePrefixCVar->GetString())
		{
			drx_strcpy(m_prefix, pCaptureFilePrefixCVar->GetString());
		}

		ICVar* pCaptureFileFormatCVar = gEnv->pConsole->GetCVar("capture_file_format");
		if (pCaptureFileFormatCVar)
		{
			m_captureFormat = SCaptureFormatInfo::GetCaptureFormatByExtension(pCaptureFileFormatCVar->GetString());
		}
	}

	SCaptureKey(const SCaptureKey& other)
		: STrackDurationKey(other)
		, m_bOnce(other.m_bOnce)
		, m_timeStep(other.m_timeStep)
		, m_frameRate(other.m_frameRate)
		, m_bufferToCapture(other.m_bufferToCapture)
		, m_captureFormat(other.m_captureFormat)
	{
		drx_strcpy(m_folder, other.m_folder);
		drx_strcpy(m_prefix, other.m_prefix);
	}

	i32                GetDurationInFrames() const  { return m_duration.GetTicks() / (SAnimTime::numTicksPerSecond / (m_frameRate > 0 ? m_frameRate : 1)); }
	i32                GetStartTimeInFrames() const { return m_time.GetTicks() / (SAnimTime::numTicksPerSecond / (m_frameRate > 0 ? m_frameRate : 1)); }
	i32                GetEndTimeInFrames() const   { return SAnimTime(m_time + m_duration).GetTicks() / (SAnimTime::numTicksPerSecond / (m_frameRate > 0 ? m_frameRate : 1)); }

	static tukk GetType()                    { return "Capture"; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackDurationKey::Serialize(ar);
		ar(m_frameRate, "frameRate", "Frame Rate");
		if (m_frameRate > 0)
		{
			m_timeStep = (1.0f / m_frameRate);
		}

		string tempFolder = m_folder;
		size_t pathLength = tempFolder.find(PathUtil::GetGameFolder());
		string captureFolder = (pathLength == string::npos) ? PathUtil::GetGameFolder() + DRX_NATIVE_PATH_SEPSTR + m_folder : m_folder;

		ar(Serialization::ResourceFolderPath(captureFolder, ""), "folder", "Folder");
		drx_strcpy(m_folder, captureFolder.c_str(), captureFolder.length());

		ar(m_captureFormat, "format", "Format");
		ar(m_bOnce, "once", "Once");

		string prefix(m_prefix);
		ar(prefix, "prefix", "Prefix");
		drx_strcpy(m_prefix, prefix.c_str(), prefix.length());

		ar(m_bufferToCapture, "bufferToCapture", "Buffer to Capture");
	}

	bool                                   m_bOnce;
	float                                  m_timeStep;
	uint                                   m_frameRate;
	char                                   m_folder[IDrxPak::g_nMaxPath];
	char                                   m_prefix[IDrxPak::g_nMaxPath / 4];
	SCaptureFormatInfo::ECaptureBuffer     m_bufferToCapture;
	SCaptureFormatInfo::ECaptureFileFormat m_captureFormat;
};

struct SBoolKey : public STrackKey
{
	static tukk GetType() { return "Bool"; }
	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);
	}

	SBoolKey() {};
};

struct SCommentKey : public STrackDurationKey
{
	enum ETextAlign
	{
		eTA_Left   = 0,
		eTA_Center = BIT(1),
		eTA_Right  = BIT(2)
	};

	SCommentKey() : m_size(1.f), m_align(eTA_Left), m_color(Vec3(1.0f, 1.0f, 1.0f))
	{
		drx_strcpy(m_font, "default");
	}

	static tukk GetType()              { return "Comment"; }
	tukk        GetDescription() const { return m_comment.c_str(); }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackDurationKey::Serialize(ar);

		ar(m_comment, "comment", "Comment");

		if (ar.isInput())
		{
			string font;
			ar(font, "font", "Font");
			drx_strcpy(m_font, font.c_str());
		}
		else
		{
			string font = m_font;
			ar(font, "font", "Font");
		}

		ar(Serialization::Vec3AsColor(m_color), "color", "Color");
		ar(m_size, "size", "Size");
		ar(m_align, "align", "Align");
	}

	string     m_comment;
	char       m_font[64];
	Vec3       m_color;
	float      m_size;
	ETextAlign m_align;
};

struct SScreenFaderKey : public STrackKey
{
	enum EFadeType
	{
		eFT_FadeIn  = 0,
		eFT_FadeOut = 1
	};

	enum EFadeChangeType
	{
		eFCT_Linear      = 0,
		eFCT_Square      = 1,
		eFCT_CubicSquare = 2,
		eFCT_SquareRoot  = 3,
		eFCT_Sin         = 4
	};

	SScreenFaderKey() : STrackKey(), m_fadeTime(2.f), m_bUseCurColor(true), m_fadeType(eFT_FadeOut),
		m_fadeChangeType(eFCT_Linear), m_fadeColor(Vec4(0.0f, 0.0f, 0.0f, 1.0f))
	{
		m_texture[0] = 0;
	}

	SScreenFaderKey(const SScreenFaderKey& other)
		: STrackKey(other), m_fadeTime(other.m_fadeTime), m_bUseCurColor(other.m_bUseCurColor),
		m_fadeType(other.m_fadeType), m_fadeChangeType(other.m_fadeChangeType),
		m_fadeColor(other.m_fadeColor)
	{
		drx_strcpy(m_texture, other.m_texture);
	}

	static tukk GetType() { return "ScreenFader"; }

	void               Serialize(Serialization::IArchive& ar)
	{
		STrackKey::Serialize(ar);
		ar(m_fadeTime, "fadeTime", "Fade Time");
		ar(m_fadeColor, "fadeColor", "Fade Color");

		string texturePath = m_texture;
		ar(Serialization::TextureFilename(texturePath), "texture", "Texture");
		drx_strcpy(m_texture, texturePath.c_str());

		ar(m_bUseCurColor, "useCurColor", "Use Current Color");
		ar(m_fadeType, "fadeType", "Fade Action");
		ar(m_fadeChangeType, "fadeChangeType", "Fade Type");
	}

	float           m_fadeTime;
	Vec4            m_fadeColor;
	char            m_texture[256];
	bool            m_bUseCurColor;
	EFadeType       m_fadeType;
	EFadeChangeType m_fadeChangeType;
};

// Wraps keys in a polymorphic and serializable interface. Need to register key types in AnimKey_impl.h for this to work.
struct IAnimKeyWrapper : public _i_reference_target_t
{
	IAnimKeyWrapper(tukk type = "") : m_pType(type) {}
	virtual ~IAnimKeyWrapper() {}

	virtual void             Serialize(Serialization::IArchive& ar) = 0;

	tukk              GetInstanceType() const { return m_pType; }

	virtual void             SetTime(const SAnimTime time) = 0;
	virtual SAnimTime        GetTime() const = 0;

	virtual tukk      GetDescription() const = 0;

	virtual const STrackKey* Key() const = 0;

	tukk m_pType;
};

template<class KeyType>
struct SAnimKeyWrapper : public IAnimKeyWrapper
{
	SAnimKeyWrapper() : IAnimKeyWrapper(KeyType::GetType()) {}

	static tukk GetType() { return KeyType::GetType(); }

	virtual void       Serialize(Serialization::IArchive& ar) override
	{
		m_key.Serialize(ar);
	}

	virtual void SetTime(const SAnimTime time) override
	{
		m_key.m_time = time;
	}

	virtual SAnimTime GetTime() const override
	{
		return m_key.m_time;
	}

	virtual tukk GetDescription() const override
	{
		return m_key.GetDescription();
	}

	virtual const STrackKey* Key() const override
	{
		return &m_key;
	}

	KeyType m_key;
};

inline bool Serialize(Serialization::IArchive& ar, _smart_ptr<IAnimKeyWrapper>& ptr, tukk name, tukk label)
{
	CIntrusiveFactory<IAnimKeyWrapper>::SSerializer serializer(ptr);
	return ar(serializer, name, label);
}

#define SERIALIZATION_ANIM_KEY(type)                     \
  typedef SAnimKeyWrapper<type> SAnimKeyWrapper ## type; \
  REGISTER_IN_INTRUSIVE_FACTORY(IAnimKeyWrapper, SAnimKeyWrapper ## type);

//! \endcond