// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <QtCore/QObject>
#include <vector>
#include "AnimationReference.h"
#include "PlaybackLayers.h"
#include <drx3D/CoreX/String/DrxFixedString.h>

struct IBackgroundTask;
struct ICharacterInstance;
struct IAnimationSet;
struct SAnimSettings;

namespace CharacterTool
{
using std::vector;

struct AnimationCompressionInfo
{
	uint64 uncompressedSize;
	uint64 compressedPreviewSize;
	uint64 compressedCafSize;

	AnimationCompressionInfo()
		: uncompressedSize(0)
		, compressedPreviewSize(0)
		, compressedCafSize(0)
	{
	}
};

class InterlockedReferenceTarget
{
public:
	InterlockedReferenceTarget() : m_cnt(0) {}
	virtual ~InterlockedReferenceTarget() {}
	i32 AddRef() { return DrxInterlockedIncrement(&m_cnt); }
	i32 Release()
	{
		i32k nCount = DrxInterlockedDecrement(&m_cnt);
		if (nCount == 0)
			delete this;
		else if (nCount < 0)
			assert(0);
		return nCount;
	}

	i32 GetRefCount() const { return m_cnt; }
private:
	volatile i32 m_cnt;
};

struct StateText
{
	enum EType
	{
		FAIL,
		WARNING,
		PROGRESS,
		INFO
	};
	EType                      type;
	DrxStackStringT<char, 128> animation;
	DrxStackStringT<char, 128> text;
};

class CompressionMachine : public QObject, public InterlockedReferenceTarget
{
	Q_OBJECT
public:
	CompressionMachine();

	void        SetCharacters(ICharacterInstance* uncompressedCharacter, ICharacterInstance* compressedCharacter);
	void        SetLoop(bool loop);
	void        SetPlaybackSpeed(float speed);
	void        PreviewAnimation(const PlaybackLayers& layers, const vector<bool>& isModified, bool showOriginalAnimation, const vector<SAnimSettings>& animSettings, float normalizedTime, bool forceRecompile, bool expectToReloadChrparams);
	void        Play(float normalizedTime);
	tukk AnimationPathConsideringPreview(tukk inputCaf) const;

	void        Reset();

	bool        IsPlaying() const;

	void        GetAnimationStateText(vector<StateText>* lines, bool compressedCharacter);

	bool        ShowOriginalAnimation() const { return m_showOriginalAnimation; }

signals:
	void SignalAnimationStarted();

private:
	enum State
	{
		eState_Idle,
		eState_Waiting,
		eState_PreviewAnimation,
		eState_PreviewCompressedOnly,
		eState_Failed
	};

	enum AnimationState
	{
		eAnimationState_NotSet,
		eAnimationState_Canceled,
		eAnimationState_Compression,
		eAnimationState_Ready,
		eAnimationState_Failed,
		eAnimationState_WaitingToReloadChrparams,
	};

	struct Animation
	{
		enum Type
		{
			UNKNOWN,
			CAF,
			AIMPOSE,
			BLEND_SPACE,
			ANM
		};

		bool                     enabled;
		AnimationState           state;
		Type                     type;
		string                   path;
		string                   name;
		string                   compressedReferencePath;
		bool                     hasSourceFile;
		bool                     hasReferenceCompressed;
		bool                     hasPreviewCompressed;
		IBackgroundTask*         previewTask;
		IBackgroundTask*         referenceTask;
		i32                      compressionSessionIndex;
		string                   failMessage;
		AnimationCompressionInfo compressionInfo;
		SAnimationReference      uncompressedCaf;
		SAnimationReference      compressedCaf;

		string                   compressedAnimationName;
		string                   compressedAnimationPath;
		string                   uncompressedAnimationName;
		string                   uncompressedAnimationPath;

		Animation()
			: state(eAnimationState_NotSet)
			, enabled(true)
			, hasSourceFile(true)
			, hasReferenceCompressed(false)
			, hasPreviewCompressed(false)
			, type(UNKNOWN)
		{}

	};

	class BackgroundTaskCompressPreview;
	class BackgroundTaskCompressReference;
	struct AnimationSetExtender;

	void StartPreview(bool forceRecompile, bool expectToReloadChrParams);
	void OnCompressed(BackgroundTaskCompressPreview* pTask);
	void AnimationStateChanged();
	void SetState(State state);

	vector<Animation>                     m_animations;
	PlaybackLayers                        m_playbackLayers;
	vector<bool>                          m_layerAnimationsModified;
	vector<SAnimSettings>                 m_animSettings;

	float                                 m_normalizedStartTime;
	bool                                  m_showOriginalAnimation;
	bool                                  m_loop;
	float                                 m_playbackSpeed;

	ICharacterInstance*                   m_uncompressedCharacter;
	ICharacterInstance*                   m_compressedCharacter;
	std::unique_ptr<AnimationSetExtender> m_previewReloadListener;
	std::unique_ptr<AnimationSetExtender> m_referenceReloadListener;

	State                                 m_state;

	i32 m_compressionSessionIndex;
};

}

