// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/BaseTypes.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/CoreX/Platform/platform.h>
#include <drx3D/CoreX/DrxEnumMacro.h>

#define AUDIO_SYSTEM_DATA_ROOT "audio"

namespace DrxAudio
{
using IdType = u32;
using ControlId = IdType;
using SwitchStateId = IdType;
using EnvironmentId = IdType;
using PreloadRequestId = IdType;
using FileEntryId = IdType;
using TriggerImplId = IdType;
using TriggerInstanceId = IdType;
using EnumFlagsType = IdType;
using AuxObjectId = IdType;
using LibraryId = IdType;

static constexpr ControlId InvalidControlId = 0;
static constexpr SwitchStateId InvalidSwitchStateId = 0;
static constexpr EnvironmentId InvalidEnvironmentId = 0;
static constexpr PreloadRequestId InvalidPreloadRequestId = 0;
static constexpr FileEntryId InvalidFileEntryId = 0;
static constexpr TriggerImplId InvalidTriggerImplId = 0;
static constexpr TriggerInstanceId InvalidTriggerInstanceId = 0;
static constexpr EnumFlagsType InvalidEnumFlagType = 0;
static constexpr AuxObjectId InvalidAuxObjectId = 0;
static constexpr AuxObjectId DefaultAuxObjectId = 1;
static constexpr u8 MaxInfoStringLength = 128;
static constexpr u8 MaxControlNameLength = 128;
static constexpr u8 MaxFileNameLength = 128;
static constexpr u16 MaxFilePathLength = 256;
static constexpr u16 MaxObjectNameLength = 256;
static constexpr u16 MaxMiscStringLength = 512;
static constexpr u32 InvalidCRC32 = 0xFFFFffff;
static constexpr float FloatEpsilon = 1.0e-3f;

// Forward declarations.
struct IObject;
class CATLEvent;
class CATLStandaloneFile;

/**
 * @enum DrxAudio::ERequestFlags
 * @brief A strongly typed enum class representing flags that can be passed into methods via the SRequestUserData parameter that control how an internally generated request behaves or what to do along with it.
 * @var DrxAudio::ERequestFlags::None
 * @var DrxAudio::ERequestFlags::ExecuteBlocking
 * @var DrxAudio::ERequestFlags::CallbackOnExternalOrCallingThread
 * @var DrxAudio::ERequestFlags::CallbackOnAudioThread
 * @var DrxAudio::ERequestFlags::DoneCallbackOnExternalThread
 * @var DrxAudio::ERequestFlags::DoneCallbackOnAudioThread
 */
enum class ERequestFlags : EnumFlagsType
{
	None,                                       /**< Used to initialize variables of this type. */
	ExecuteBlocking                   = BIT(0), /**< Blocks the calling thread until the request has been processed. */
	CallbackOnExternalOrCallingThread = BIT(1), /**< Invokes a callback on the calling thread for blocking requests or invokes a callback on the external thread for non-blocking requests. */
	CallbackOnAudioThread             = BIT(2), /**< Invokes a callback on the audio thread informing of the outcome of the request. */
	DoneCallbackOnExternalThread      = BIT(3), /**< Invokes a callback on the external thread once a trigger instance finished playback of all its events. */
	DoneCallbackOnAudioThread         = BIT(4), /**< Invokes a callback on the audio thread once a trigger instance finished playback of all its events. */
};
DRX_CREATE_ENUM_FLAG_OPERATORS(ERequestFlags);

/**
 * @enum DrxAudio::ERequestStatus
 * @brief A strongly typed enum class representing a list of possible statuses of an internally generated audio request. Used as a return type for many methods used by the AudioSystem internally and also for most of the DrxAudio::Impl::IImpl calls.
 * @var DrxAudio::ERequestStatus::None
 * @var DrxAudio::ERequestStatus::ExecuteBlocking
 * @var DrxAudio::ERequestStatus::CallbackOnExternalOrCallingThread
 * @var DrxAudio::ERequestStatus::CallbackOnAudioThread
 * @var DrxAudio::ERequestStatus::DoneCallbackOnExternalThread
 * @var DrxAudio::ERequestStatus::DoneCallbackOnAudioThread
 */
enum class ERequestStatus : EnumFlagsType
{
	None,                    /**< Used to initialize variables of this type and to determine whether the variable was properly handled. */
	Success,                 /**< Returned if the request processed successfully. */
	SuccessDoNotTrack,       /**< Audio middleware implementations return this if during ExecuteTrigger an event was actually stopped so that internal data can be immediately freed. */
	SuccessNeedsRefresh,     /**< Audio middleware implementations return this if after an action they require to be refreshed. */
	PartialSuccess,          /**< Returned if the outcome of the request wasn't a complete success but also not complete failure. */
	Failure,                 /**< Returned if the request failed to process. */
	Pending,                 /**< Returned if the request was delivered but final execution is pending. It's then kept in the system until its status changed. */
	FailureInvalidControlId, /**< Returned if the request referenced a non-existing audio control. */
	FailureInvalidRequest,   /**< Returned if the request type is unknown/unhandled. */
};

/**
 * @enum DrxAudio::ERequestResult
 * @brief A strongly typed enum class representing a list of possible outcomes of a request which gets communicated via the callbacks if the user decided to be informed of the outcome of a particular request.
 * @var DrxAudio::ERequestResult::None
 * @var DrxAudio::ERequestResult::Success
 * @var DrxAudio::ERequestResult::Failure
 */
enum class ERequestResult : EnumFlagsType
{
	None,    /**< Used to initialize variables of this type and to determine whether the variable was properly handled. */
	Success, /**< Set if the request processed successfully. */
	Failure, /**< Set if the request failed to process. */
};

class CObjectTransformation
{
public:

	CObjectTransformation()
		: m_position(Vec3Constants<float>::fVec3_Zero)
		, m_forward(Vec3Constants<float>::fVec3_OneY)
		, m_up(Vec3Constants<float>::fVec3_OneZ)
	{}

	CObjectTransformation(Vec3 const& position)
		: m_position(position)
		, m_forward(Vec3Constants<float>::fVec3_OneY)
		, m_up(Vec3Constants<float>::fVec3_OneZ)
	{}

	CObjectTransformation(Matrix34 const& transformation)
		: m_position(transformation.GetColumn3())
		, m_forward(transformation.GetColumn1()) //!< Assuming forward vector = (0,1,0), also assuming unscaled.
		, m_up(transformation.GetColumn2())      //!< Assuming up vector = (0,0,1).
	{
		m_forward.NormalizeFast();
		m_up.NormalizeFast();
	}

	bool IsEquivalent(CObjectTransformation const& transformation, float const epsilon = VEC_EPSILON) const
	{
		return m_position.IsEquivalent(transformation.GetPosition(), epsilon) &&
		       m_forward.IsEquivalent(transformation.GetForward(), epsilon) &&
		       m_up.IsEquivalent(transformation.GetUp(), epsilon);
	}

	bool IsEquivalent(Matrix34 const& transformation, float const epsilon = VEC_EPSILON) const
	{
		return m_position.IsEquivalent(transformation.GetColumn3(), epsilon) &&
		       m_forward.IsEquivalent(transformation.GetColumn1(), epsilon) &&
		       m_up.IsEquivalent(transformation.GetColumn2(), epsilon);
	}

	void                                SetPosition(Vec3 const& position) { m_position = position; }
	ILINE Vec3 const&                   GetPosition() const               { return m_position; }
	ILINE Vec3 const&                   GetForward() const                { return m_forward; }
	ILINE Vec3 const&                   GetUp() const                     { return m_up; }

	static CObjectTransformation const& GetEmptyObject()                  { static CObjectTransformation const emptyInstance; return emptyInstance; }

private:

	Vec3 m_position;
	Vec3 m_forward;
	Vec3 m_up;
};

struct SRequestUserData
{
	explicit SRequestUserData(
	  ERequestFlags const flags_ = ERequestFlags::None,
	  uk const pOwner_ = nullptr,
	  uk const pUserData_ = nullptr,
	  uk const pUserDataOwner_ = nullptr)
		: flags(flags_)
		, pOwner(pOwner_)
		, pUserData(pUserData_)
		, pUserDataOwner(pUserDataOwner_)
	{}

	static SRequestUserData const& GetEmptyObject() { static SRequestUserData const emptyInstance; return emptyInstance; }

	ERequestFlags const            flags;
	uk const                    pOwner;
	uk const                    pUserData;
	uk const                    pUserDataOwner;
};

struct SFileData
{
	SFileData() = default;
	SFileData(SFileData const&) = delete;
	SFileData(SFileData&&) = delete;
	SFileData& operator=(SFileData const&) = delete;
	SFileData& operator=(SFileData&&) = delete;

	float duration = 0.0f;
};

struct STriggerData
{
	STriggerData() = default;
	STriggerData(STriggerData const&) = delete;
	STriggerData(STriggerData&&) = delete;
	STriggerData& operator=(STriggerData const&) = delete;
	STriggerData& operator=(STriggerData&&) = delete;

	float radius = 0.0f;
};

struct SPlayFileInfo
{
	explicit SPlayFileInfo(
	  char const* const _szFile,
	  bool const _bLocalized = true,
	  ControlId const _usedPlaybackTrigger = InvalidControlId)
		: szFile(_szFile)
		, bLocalized(_bLocalized)
		, usedTriggerForPlayback(_usedPlaybackTrigger)
	{}

	char const* const szFile;
	bool const        bLocalized;
	ControlId const   usedTriggerForPlayback;
};

struct SImplInfo
{
	DrxFixedStringT<MaxInfoStringLength> name;
	DrxFixedStringT<MaxInfoStringLength> folderName;
};
} //endns DrxAudio