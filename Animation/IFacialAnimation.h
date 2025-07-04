// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/CoreX/Math/ISplines.h> // <> required for Interfuscator
#include <drx3D/CoreX/Math/Range.h>
#include <drx3D/CoreX/functor.h>

struct IFacialEffCtrl;
struct IFacialAnimSequence;

class IJoystickSet;

#define FACE_STORE_ASSET_VALUES (1)

//! This class is used all over the place in the Face Animation System to replace Strings by CRC32s on consoles.
//! Internally, CFaceIdentifierStorage is used to store the string value on PC for the Editor.
//! Note that an (nonempty) handle can not explicitly be created, it has to be created
//! by the CFacialAnimation class (CreateIdentifier) or from a CFaceIdentifierStorage Object.
class CFaceIdentifierHandle
{
	friend class CFaceIdentifierStorage;
	friend class CFacialAnimation;

public:
	tukk GetString() const
	{
#ifdef FACE_STORE_ASSET_VALUES
		return m_str;
#endif
		return NULL;
	}
	u32 GetCRC32() const
	{
		return m_crc32;
	}
	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	//! Allow for empty handles.
	CFaceIdentifierHandle()
	{
#ifdef FACE_STORE_ASSET_VALUES
		m_str = NULL;
#endif
		m_crc32 = 0;
	}
protected:
#ifdef FACE_STORE_ASSET_VALUES
	tukk m_str;
#endif
	u32 m_crc32;

	CFaceIdentifierHandle(tukk str, u32 crc32)
	{
#ifdef FACE_STORE_ASSET_VALUES
		m_str = str;
#endif
		m_crc32 = crc32;
	}
};

enum EFacialSequenceLayer
{
	eFacialSequenceLayer_Preview,               //!< Used in facial editor.
	eFacialSequenceLayer_Dialogue,              //!< One shot with sound, requested together with wave file.
	eFacialSequenceLayer_Trackview,             //!< Triggered via trackview sequence.
	eFacialSequenceLayer_AGStateAndAIAlertness, //!< Per animation-state and per ai-alertness (looping, requested/cleared by AG state node enter/leave).
	eFacialSequenceLayer_Mannequin,             //!< Sequence requested through mannequin.
	eFacialSequenceLayer_AIExpression,          //!< Looping, requested/cleared by goalpipe/goalop.
	eFacialSequenceLayer_FlowGraph,             //!< Just in case we will need it later =).

	eFacialSequenceLayer_COUNT,
};

//! Types of face Effectors.
enum EFacialEffectorType
{
	EFE_TYPE_GROUP        = 0x00, //!< This effector is a group of sub effectors.
	EFE_TYPE_EXPRESSION   = 0x01, //!< This effector contain specific state of effectors.
	EFE_TYPE_MORPH_TARGET = 0x02, //!< This Effector is a single morph target.
	EFE_TYPE_BONE         = 0x03, //!< This Effector controls a bone.
	EFE_TYPE_MATERIAL     = 0x04, //!< This Effector is a material parameter.
	EFE_TYPE_ATTACHMENT   = 0x05  //!< This Effector controls attachment.
};

enum EFacialEffectorFlags
{
	EFE_FLAG_ROOT        = 0x00001, //!< This is a root effector in the library.
	EFE_FLAG_UI_EXTENDED = 0x01000, //!< When this flag is set this effector must be extended in ui.
	EFE_FLAG_UI_MODIFIED = 0x02000, //!< When this flag is set this effector was modified.
	EFE_FLAG_UI_PREVIEW  = 0x04000, //!< This flag indicate preview only effector, it is not saved in library.
};

enum EFacialEffectorParam
{
	EFE_PARAM_BONE_NAME,     //!< String representing the name of the bone/attachment object in character, only for Bone/Attachment Effectors.
	EFE_PARAM_BONE_ROT_AXIS, //!< Vec3 representing the angles in radians as multipliers of effector weight,for Bone/Attachment Effector rotation.
	EFE_PARAM_BONE_POS_AXIS, //!< Vec3 representing the  offset multiplied with effector weight,for Bone/Attachment Effector offset.
};

enum MergeCollisionAction
{
	MergeCollisionActionNoOverwrite,
	MergeCollisionActionOverwrite
};

class CFacialAnimForcedRotationEntry
{
public:
	i32  jointId;
	Quat rotation;
};

//! General Effector that can be a muscle bone or some effect.
struct IFacialEffector
{
	// <interfuscator:shuffle>
	virtual void                  SetIdentifier(CFaceIdentifierHandle ident) = 0;
	virtual CFaceIdentifierHandle GetIdentifier() = 0;

	virtual EFacialEffectorType   GetType() = 0;

	//! \see EFacialEffectorFlags.
	virtual void   SetFlags(u32 nFlags) = 0;
	virtual u32 GetFlags() = 0;

	//! Get index of the effector in Facial State array, only valid for end effectors.
	virtual i32 GetIndexInState() = 0;

	//! Set facial effector parameters, mostly used for editing.
	virtual void        SetParamString(EFacialEffectorParam param, tukk str) = 0;
	virtual tukk GetParamString(EFacialEffectorParam param) = 0;
	virtual void        SetParamVec3(EFacialEffectorParam param, Vec3 vValue) = 0;
	virtual Vec3        GetParamVec3(EFacialEffectorParam param) = 0;
	virtual void        SetParamInt(EFacialEffectorParam param, i32 nValue) = 0;
	virtual i32         GetParamInt(EFacialEffectorParam param) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Sub Effectors.
	//////////////////////////////////////////////////////////////////////////
	virtual i32              GetSubEffectorCount() = 0;
	virtual IFacialEffector* GetSubEffector(i32 nIndex) = 0;
	virtual IFacialEffCtrl*  GetSubEffCtrl(i32 nIndex) = 0;
	virtual IFacialEffCtrl*  GetSubEffCtrlByName(tukk effectorName) = 0;
	virtual IFacialEffCtrl*  AddSubEffector(IFacialEffector* pEffector) = 0;
	virtual void             RemoveSubEffector(IFacialEffector* pEffector) = 0;
	virtual void             RemoveAllSubEffectors() = 0;
	virtual ~IFacialEffector(){}
	// </interfuscator:shuffle>

#ifdef FACE_STORE_ASSET_VALUES
	//! Functions should only be used in the editor.
	virtual void        SetName(tukk sNewName) = 0;
	virtual tukk GetName() const = 0;
#endif
};

//! Sub Effector controller.
struct IFacialEffCtrl
{
	enum ControlType
	{
		CTRL_LINEAR,
		CTRL_SPLINE
	};
	enum ControlFlags
	{
		CTRL_FLAG_UI_EXPENDED = 0x01000, //!< Controller is expanded.
	};
	// <interfuscator:shuffle>
	virtual ~IFacialEffCtrl(){}
	virtual IFacialEffCtrl::ControlType GetType() = 0;
	virtual void                        SetType(ControlType t) = 0;
	virtual IFacialEffector*            GetEffector() = 0;
	virtual float                       GetConstantWeight() = 0;
	virtual void                        SetConstantWeight(float fWeight) = 0;
	virtual float                       GetConstantBalance() = 0;
	virtual void                        SetConstantBalance(float fBalance) = 0;

	//! Get access to control spline.
	virtual ISplineInterpolator* GetSpline() = 0;
	virtual float                Evaluate(float fInput) = 0;

	//! \see ControlFlags.
	virtual i32 GetFlags() = 0;

	//! \see ControlFlags.
	virtual void SetFlags(i32 nFlags) = 0;
	// </interfuscator:shuffle>
};

//! Contains face Effectors.
struct IFacialEffectorsLibraryEffectorVisitor
{
	// <interfuscator:shuffle>
	virtual ~IFacialEffectorsLibraryEffectorVisitor(){}
	virtual void VisitEffector(IFacialEffector* pEffector) = 0;
	// </interfuscator:shuffle>
};
struct IFacialEffectorsLibrary
{
	// <interfuscator:shuffle>
	virtual ~IFacialEffectorsLibrary(){}
	virtual void AddRef() = 0;
	virtual void Release() = 0;

	//! Assign filename of the effectors library.
	virtual void SetName(tukk name) = 0;

	//! Retrieve filename of the effectors library.
	virtual tukk GetName() = 0;

	//! Find Effector by Identifier.
	//! Every Effector must have unique name.
	virtual IFacialEffector* Find(CFaceIdentifierHandle ident) = 0;

	//! Find Effector by Name.
	//! \note Do not use this at runtime - only in the editor.
	virtual IFacialEffector* Find(tukk identStr) = 0;

	//! Retrieve the root effector, all effectors contained in the library are direct or inderect siblings of this root.
	virtual IFacialEffector* GetRoot() = 0;

	//! Direct access to effectors.
	virtual void VisitEffectors(IFacialEffectorsLibraryEffectorVisitor* pVisitor) = 0;

	//! Creates a custom effector in the library.
	virtual IFacialEffector* CreateEffector(EFacialEffectorType nType, CFaceIdentifierHandle ident) = 0;

	//! Create a custom effector by supplying the name directly - ONLY USE IN EDITOR.
	virtual IFacialEffector* CreateEffector(EFacialEffectorType nType, tukk identStr) = 0;

	virtual void             RemoveEffector(IFacialEffector* pEffector) = 0;

	//! Merges in effectors from another library - callback specifies what to do in case of name collision.
	virtual void MergeLibrary(IFacialEffectorsLibrary* pMergeLibrary, const Functor1wRet<tukk , MergeCollisionAction>& collisionStrategy) = 0;

	//! Serialized library.
	virtual void Serialize(XmlNodeRef& node, bool bLoading) = 0;
	// </interfuscator:shuffle>
};

//! IFacialModel interface.
struct IFacialModel
{
	// <interfuscator:shuffle>
	virtual ~IFacialModel(){}
	virtual i32              GetEffectorCount() const = 0;
	virtual IFacialEffector* GetEffector(i32 nIndex) const = 0;

	//! Assign facial effectors library to this model.
	virtual void                     AssignLibrary(IFacialEffectorsLibrary* pLibrary) = 0;
	virtual IFacialEffectorsLibrary* GetLibrary() = 0;

	virtual i32                      GetMorphTargetCount() const = 0;
	virtual tukk              GetMorphTargetName(i32 morphTargetIndex) const = 0;
	// </interfuscator:shuffle>
};

//////////////////////////////////////////////////////////////////////////
struct IFaceState
{
	// <interfuscator:shuffle>
	virtual ~IFaceState(){}
	virtual float GetEffectorWeight(i32 nIndex) = 0;
	virtual void  SetEffectorWeight(i32 nIndex, float fWeight) = 0;
	// </interfuscator:shuffle>
};

//! IFacialInstance interface.
struct IFacialInstance
{
	// <interfuscator:shuffle>
	virtual ~IFacialInstance(){}
	virtual IFacialModel*        GetFacialModel() = 0;
	virtual IFaceState*          GetFaceState() = 0;

	virtual u32               StartEffectorChannel(IFacialEffector* pEffector, float fWeight, float fFadeTime, float fLifeTime = 0, i32 nRepeatCount = 0) = 0;
	virtual void                 StopEffectorChannel(u32 nChannelID, float fFadeOutTime) = 0;
	virtual void                 PreviewEffector(IFacialEffector* pEffector, float fWeight, float fBalance = 0.0f) = 0;
	virtual void                 PreviewEffectors(IFacialEffector** pEffectors, float* fWeights, float* fBalances, i32 nEffectorsCount) = 0;
	virtual IFacialAnimSequence* LoadSequence(tukk sSequenceName, bool addToCache = true) = 0;
	virtual void                 PrecacheFacialExpression(tukk sSequenceName) = 0;
	virtual void                 PlaySequence(IFacialAnimSequence* pSequence, EFacialSequenceLayer layer, bool bExclusive = false, bool bLooping = false) = 0;
	virtual void                 StopSequence(EFacialSequenceLayer layer) = 0;
	virtual bool                 IsPlaySequence(IFacialAnimSequence* pSequence, EFacialSequenceLayer layer) = 0;
	virtual void                 PauseSequence(EFacialSequenceLayer layer, bool bPaused) = 0;

	//! Seek sequence current time to the specified time offset from the begining of sequence playback time.
	virtual void SeekSequence(EFacialSequenceLayer layer, float fTime) = 0;

	//! Start/Stop Lip syncing with the sound.
	virtual void LipSyncWithSound(u32 nSoundId, bool bStop = false) = 0;
	virtual void OnExpressionLibraryLoad() = 0;

	virtual void EnableProceduralFacialAnimation(bool bEnable) = 0;
	virtual bool IsProceduralFacialAnimationEnabled() const = 0;
	virtual void SetForcedRotations(i32 numForcedRotations, CFacialAnimForcedRotationEntry* forcedRotations) = 0;

	virtual void SetMasterCharacter(ICharacterInstance* pMasterInstance) = 0;

	virtual void TemporarilyEnableBoneRotationSmoothing() = 0;

	virtual void StopAllSequencesAndChannels() = 0;

	virtual void SetUseFrameRateLimiting(bool useFrameRateLimiting) = 0;
	// </interfuscator:shuffle>
};

//! LipSyncing.
struct SPhonemeInfo
{
	wchar_t     codeIPA;      //!< IPA (International Phonetic Alphabet) code.
	char        ASCII[4];     //!< ASCII name for this phoneme.
	tukk description;  //!< Phoneme textual description.
};

//////////////////////////////////////////////////////////////////////////
struct IPhonemeLibrary
{
	// <interfuscator:shuffle>
	virtual ~IPhonemeLibrary(){}
	virtual i32  GetPhonemeCount() const = 0;
	virtual bool GetPhonemeInfo(i32 nIndex, SPhonemeInfo& phoneme) = 0;
	virtual i32  FindPhonemeByName(tukk sPhonemeName) = 0;
	// </interfuscator:shuffle>
};

//! Main interface to the facial animation system.
class IJoystickContext;
class IJoystickChannel;
struct IFacialAnimChannel;
struct IFacialAnimation
{
	// <interfuscator:shuffle>
	virtual ~IFacialAnimation(){}
	virtual IPhonemeLibrary* GetPhonemeLibrary() = 0;

	virtual void             ClearAllCaches() = 0;

	//! Creates a new effectors library.
	virtual IFacialEffectorsLibrary* CreateEffectorsLibrary() = 0;

	//! Loads effectors library from the file.
	virtual void                     ClearEffectorsLibraryFromCache(tukk filename) = 0;
	virtual IFacialEffectorsLibrary* LoadEffectorsLibrary(tukk filename) = 0;

	//! Creates a new animation sequence.
	virtual IFacialAnimSequence* CreateSequence() = 0;
	virtual void                 ClearSequenceFromCache(tukk filename) = 0;

	//! Synchronous loading of sequence -> do not use in the game!
	virtual IFacialAnimSequence* LoadSequence(tukk filename, bool bNoWarnings = false, bool addToCache = true) = 0;

	//! Starts to stream in a Sequence. IFacialAnimSequence* will be filled after StreamOnComplete.
	virtual IFacialAnimSequence* StartStreamingSequence(tukk filename) = 0;

	//! Looks for a sequence amongst the currently loaded sequences.
	virtual IFacialAnimSequence* FindLoadedSequence(tukk filename) const = 0;

	//! Access the joystick functionality.
	virtual IJoystickContext* GetJoystickContext() = 0;
	virtual IJoystickChannel* CreateJoystickChannel(IFacialAnimChannel* pEffector) = 0;
	virtual void              BindJoystickSetToSequence(IJoystickSet* pJoystickSet, IFacialAnimSequence* pSequence) = 0;

	//! Create an Identifier Handle to address objects in the Face Animation system by Handle instead of by strings.
	virtual CFaceIdentifierHandle CreateIdentifierHandle(tukk identifierString) = 0;
	// </interfuscator:shuffle>
};

//! A sentence is a collection of phonemes and other lip syncing related data.
struct IFacialSentence
{
	struct Phoneme
	{
		char  phoneme[4];   //!< Phoneme name.
		i32   time;         //!< Start time of the phoneme in milliseconds.
		i32   endtime;      //!< End time the phoneme in milliseconds.
		float intensity;    //!< Phoneme intensity.
	};
	struct Word
	{
		tukk sWord;     //!< Word text.
		i32         startTime; //!< Start time of the word in milliseconds.
		i32         endTime;   //!< End time of the word in milliseconds.
	};

	struct ChannelSample
	{
		tukk phoneme;
		float       strength;
	};

	// <interfuscator:shuffle>
	virtual ~IFacialSentence(){}
	virtual void        SetText(tukk text) = 0;
	virtual tukk GetText() = 0;

	//! Easy access to global phoneme library.
	virtual struct IPhonemeLibrary* GetPhonemeLib() = 0;

	virtual void                    ClearAllPhonemes() = 0;
	virtual i32                     GetPhonemeCount() = 0;
	virtual bool                    GetPhoneme(i32 index, Phoneme& ph) = 0;
	virtual i32                     AddPhoneme(const Phoneme& ph) = 0;

	//! Delete all words in the sentence.
	virtual void ClearAllWords() = 0;

	//! Return number of words in the sentence.
	virtual i32 GetWordCount() = 0;

	//! Retrieve word by index, 0 <= index < GetWordCount().
	virtual bool GetWord(i32 index, Word& wrd) = 0;

	//! Add a new word into the sentence.
	virtual void AddWord(const Word& wrd) = 0;
	virtual i32  Evaluate(float fTime, float fInputPhonemeStrength, i32 maxSamples, ChannelSample* samples) = 0;
	// </interfuscator:shuffle>
};

//! Facial animation channel used in facial sequence.
//! Each channel controls the weight of the single facial expression over time.
struct IFacialAnimChannel
{
	enum EFlags
	{
		FLAG_GROUP                     = 0x00001, //!< If set this channel is a group.
		FLAG_PHONEME_STRENGTH          = 0x00002, //!< This channel is the current phoneme strength.
		FLAG_VERTEX_DRAG               = 0x00004, //!< This channel is the vertex drag coefficient.
		FLAG_BALANCE                   = 0x00008, //!< This channel controls the balance of the expressions.
		FLAG_CATEGORY_BALANCE          = 0x00010, //!< This channel controls the balance of the expressions in a certain folder.
		FLAG_PROCEDURAL_STRENGTH       = 0x00020, //!< This channel controls the strength of procedural animation.
		FLAG_LIPSYNC_CATEGORY_STRENGTH = 0x00040, //!< This channel dampens all expressions in a folder during lipsyncing.
		FLAG_BAKED_LIPSYNC_GROUP       = 0x00080, //!< The contents of this folder override the auto-lipsynch.
		FLAG_UI_SELECTED               = 0x01000,
		FLAG_UI_EXTENDED               = 0x02000,
	};
	// <interfuscator:shuffle>
	virtual ~IFacialAnimChannel(){}
	virtual void                        SetIdentifier(CFaceIdentifierHandle ident) = 0;
	virtual const CFaceIdentifierHandle GetIdentifier() = 0;

	virtual void                        SetEffectorIdentifier(CFaceIdentifierHandle ident) = 0;
	virtual const CFaceIdentifierHandle GetEffectorIdentifier() = 0;

	//! Associate animation channel with the channel group.
	virtual void SetParent(IFacialAnimChannel* pParent) = 0;

	//! Get group of this animation channel.
	virtual IFacialAnimChannel* GetParent() = 0;

	//! Change channel flags.
	virtual void             SetFlags(u32 nFlags) = 0;
	virtual u32           GetFlags() = 0;

	virtual void             SetEffector(IFacialEffector* pEffector) = 0;
	virtual IFacialEffector* GetEffector() = 0;

	//! Retrieve interpolator spline used to animated channel value.
	virtual ISplineInterpolator* GetInterpolator(i32 i) = 0;
	virtual ISplineInterpolator* GetLastInterpolator() = 0;
	virtual void                 AddInterpolator() = 0;
	virtual void                 DeleteInterpolator(i32 i) = 0;
	virtual i32                  GetInterpolatorCount() const = 0;

	virtual void                 CleanupKeys(float fErrorMax) = 0;
	virtual void                 SmoothKeys(float sigma) = 0;
	virtual void                 RemoveNoise(float sigma, float threshold) = 0;
	// </interfuscator:shuffle>

#ifdef FACE_STORE_ASSET_VALUES
	//! Only for the editor.
	virtual void        SetName(tukk name) = 0;
	virtual tukk GetName() const = 0;
	virtual tukk GetEffectorName() = 0;
#endif

	bool IsGroup() { return (GetFlags() & FLAG_GROUP) != 0; }
};

//////////////////////////////////////////////////////////////////////////
struct IFacialAnimSoundEntry
{
	// <interfuscator:shuffle>
	virtual ~IFacialAnimSoundEntry() {}

	//! Set filename of the sound associated with this facial sequence.
	virtual void SetSoundFile(tukk sSoundFile) = 0;

	//! Retrieve`s filename of the sound associated with this facial sequence.
	virtual tukk GetSoundFile() = 0;

	//! Retrieve facial animation sentence interface.
	virtual IFacialSentence* GetSentence() = 0;

	virtual float            GetStartTime() = 0;
	virtual void             SetStartTime(float time) = 0;
	// </interfuscator:shuffle>
};

//////////////////////////////////////////////////////////////////////////
struct IFacialAnimSkeletonAnimationEntry
{
	// <interfuscator:shuffle>
	virtual ~IFacialAnimSkeletonAnimationEntry(){}

	//! Set the name of the animation associated with this entry.
	virtual void        SetName(tukk skeletonAnimationFile) = 0;
	virtual tukk GetName() const = 0;

	//! Set the starting time of the animation.
	virtual void  SetStartTime(float time) = 0;
	virtual float GetStartTime() const = 0;
	virtual void  SetEndTime(float time) = 0;
	virtual float GetEndTime() const = 0;
	// </interfuscator:shuffle>
};

//////////////////////////////////////////////////////////////////////////
struct IFacialAnimSequence
{
	enum EFlags
	{
		FLAG_RANGE_FROM_SOUND = 0x00001,  //!< Take time range from the sound length.
	};

	enum ESerializationFlags : u32
	{
		SFLAG_SOUND_ENTRIES = 0x00000001,
		SFLAG_CAMERA_PATH   = 0x00000002,
		SFLAG_ANIMATION     = 0x00000004,
		SFLAG_ALL           = 0xFFFFFFFF
	};

	// <interfuscator:shuffle>
	virtual ~IFacialAnimSequence(){}
	virtual void AddRef() = 0;
	virtual void Release() = 0;

	virtual bool StartStreaming(tukk sFilename) = 0;

	//! Change sequence name.
	virtual void SetName(tukk sNewName) = 0;

	//! Retrieve sequence name.
	virtual tukk GetName() = 0;

	virtual void        SetFlags(i32 nFlags) = 0;
	virtual i32         GetFlags() = 0;

	virtual Range       GetTimeRange() = 0;
	virtual void        SetTimeRange(Range range) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Access to channels.
	virtual i32                 GetChannelCount() = 0;
	virtual IFacialAnimChannel* GetChannel(i32 nIndex) = 0;
	virtual IFacialAnimChannel* CreateChannel() = 0;
	virtual IFacialAnimChannel* CreateChannelGroup() = 0;
	virtual void                RemoveChannel(IFacialAnimChannel* pChannel) = 0;
	//////////////////////////////////////////////////////////////////////////

	//! Access to sound entries.
	virtual i32                    GetSoundEntryCount() = 0;
	virtual void                   InsertSoundEntry(i32 index) = 0;
	virtual void                   DeleteSoundEntry(i32 index) = 0;
	virtual IFacialAnimSoundEntry* GetSoundEntry(i32 index) = 0;

	//! Access the skeleton animations associated with this facial sequence.
	virtual i32                                GetSkeletonAnimationEntryCount() = 0;
	virtual void                               InsertSkeletonAnimationEntry(i32 index) = 0;
	virtual void                               DeleteSkeletonAnimationEntry(i32 index) = 0;
	virtual IFacialAnimSkeletonAnimationEntry* GetSkeletonAnimationEntry(i32 index) = 0;

	//! Set the name of the joysticks associated with this sequence.
	virtual void                 SetJoystickFile(tukk joystickFile) = 0;
	virtual tukk          GetJoystickFile() const = 0;

	virtual void                 Serialize(XmlNodeRef& xmlNode, bool bLoading, ESerializationFlags flags = SFLAG_ALL) = 0;

	virtual void                 MergeSequence(IFacialAnimSequence* pMergeSequence, const Functor1wRet<tukk , MergeCollisionAction>& collisionStrategy) = 0;

	virtual ISplineInterpolator* GetCameraPathPosition() = 0;
	virtual ISplineInterpolator* GetCameraPathOrientation() = 0;
	virtual ISplineInterpolator* GetCameraPathFOV() = 0;

	//! Streaming related - reports if the sequence has been streamed into memory and is available for animation.
	virtual bool IsInMemory() const = 0;
	virtual void SetInMemory(bool bInMemory) = 0;

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const = 0;
	// </interfuscator:shuffle>
};

static const float FACIAL_EDITOR_FPS = 30.0f;
inline float FacialEditorSnapTimeToFrame(float time) { return i32((time * FACIAL_EDITOR_FPS) + 0.5f) * (1.0f / FACIAL_EDITOR_FPS); }

//! \endcond