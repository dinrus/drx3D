// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Audio/SharedAudioData.h>
#include <drx3D/Audio/ATLEntityData.h>
#include <drx3D/Sys/IXml.h>

/**
 * @namespace DrxAudio
 * @brief Это пространство имён применяется по всему движку.
 */
namespace DrxAudio
{
using DeviceId = u8;

/**
 * @namespace DrxAudio::Impl
 * @brief Подпространство имён, используемое в реализациях middleware.
 */
namespace Impl
{
/**
 * @struct IImpl
 * @brief Интерфейс, предоставляющий функционал аудиосистемы реализациям middleware.
 */
struct IImpl
{
	/** @cond */
	virtual ~IImpl() = default;
	/** @endcond */

	/**
	 * Вызывается с такой же частотой, что и внешняя нить, и с минимальной частотой,
	 * если внешняя нить падает ниже указанного порога.
	 * @return void
	 */
	virtual void Update() = 0;

	/**
	 * Инициализует все внутренние компоненты и аудио middleware.
	 * @param objectPoolSize - Число объектов, которым предварительно выделяется память.
	 * @param eventPoolSize - Число событий, которым предварительно выделяется память.
	 * @return ERequestStatus::Success if the initialization was successful, ERequestStatus::Failure otherwise.
	 * @see ShutDown
	 */
	virtual ERequestStatus Init(u32 const objectPoolSize, u32 const eventPoolSize) = 0;

	/**
	 * Shuts down all of the internal components and the audio middleware.
	 * Note: After a call to ShutDown(), the system can still be reinitialized by calling Init().
	 * @return ERequestStatus::Success if the shutdown was successful, ERequestStatus::Failure otherwise.
	 * @see Release, Init
	 */
	virtual ERequestStatus ShutDown() = 0;

	/**
	 * Called before the middleware is shutdown to give it time to clean up. After this call all the ATL audio objects and events will be released.
	 * @return ERequestStatus::Success if the action was successful, ERequestStatus::Failure otherwise.
	 * @see ShutDown, Release, Init
	 */
	virtual ERequestStatus OnBeforeShutDown() = 0;

	/**
	 * Frees all of the resources used by the class and destroys the instance. This action is not reversible.
	 * @return ERequestStatus::Success if the action was successful, ERequestStatus::Failure otherwise.
	 * @see ShutDown, Init
	 */
	virtual ERequestStatus Release() = 0;

	/**
	 * Perform a "hot restart" of the audio middleware. Reset all of the internal data.
	 * @see Release, Init
	 * @return void
	 */
	virtual void OnRefresh() = 0;

	/**
	 * This method is called every time the main Game (or Editor) window loses focus.
	 * @return ERequestStatus::Success if the action was successful, ERequestStatus::Failure otherwise.
	 * @see OnGetFocus
	 */
	virtual ERequestStatus OnLoseFocus() = 0;

	/**
	 * This method is called every time the main Game (or Editor) window gets focus.
	 * @return ERequestStatus::Success if the action was successful, ERequestStatus::Failure otherwise.
	 * @see OnLoseFocus
	 */
	virtual ERequestStatus OnGetFocus() = 0;

	/**
	 * Mute all sounds, after this call there should be no audio coming from the audio middleware.
	 * @return ERequestStatus::Success if the action was successful, ERequestStatus::Failure otherwise.
	 * @see UnmuteAll, StopAllSounds
	 */
	virtual ERequestStatus MuteAll() = 0;

	/**
	 * Restore the audio output of the audio middleware after a call to MuteAll().
	 * @return ERequestStatus::Success if the action was successful, ERequestStatus::Failure otherwise.
	 * @see MuteAll
	 */
	virtual ERequestStatus UnmuteAll() = 0;

	/**
	 * Pauses playback of all audio events.
	 * @return ERequestStatus::Success if the action was successful, ERequestStatus::Failure otherwise.
	 * @see ResumeAll
	 */
	virtual ERequestStatus PauseAll() = 0;

	/**
	 * Resumes playback of all audio events.
	 * @return ERequestStatus::Success if the action was successful, ERequestStatus::Failure otherwise.
	 * @see PauseAll
	 */
	virtual ERequestStatus ResumeAll() = 0;

	/**
	 * Stop all currently playing sounds. Has no effect on anything triggered after this method is called.
	 * @return ERequestStatus::Success if the action was successful, ERequestStatus::Failure otherwise.
	 * @see MuteAll
	 */
	virtual ERequestStatus StopAllSounds() = 0;

	/**
	 * Inform the audio middleware about the memory location of a preloaded audio-data file
	 * @param pFileInfo - ATL-specific information describing the resources used by the preloaded file being reported
	 * @return ERequestStatus::Success if the audio middleware is able to use the preloaded file, ERequestStatus::Failure otherwise
	 * @see UnregisterInMemoryFile
	 */
	virtual ERequestStatus RegisterInMemoryFile(SFileInfo* const pFileInfo) = 0;

	/**
	 * Inform the audio middleware that the memory containing the preloaded audio-data file should no longer be used
	 * @param pFileInfo - ATL-specific information describing the resources used by the preloaded file being invalidated
	 * @return ERequestStatus::Success if the audio middleware was able to unregister the preloaded file supplied, ERequestStatus::Failure otherwise
	 * @see RegisterInMemoryFile
	 */
	virtual ERequestStatus UnregisterInMemoryFile(SFileInfo* const pFileInfo) = 0;

	/**
	 * Parse the implementation-specific XML node that represents an audio file, fill the fields of the struct
	 * referenced by pFileInfo with the data necessary to correctly access and store the file's contents in memory.
	 * Create an object implementing IFile to hold implementation-specific data about the file and store a pointer to it in a member of pFileInfo
	 * @param pRootNode - an XML node containing the necessary information about the file
	 * @param pFileInfo - a pointer to the struct containing the data used by the ATL to load the file into memory
	 * @return ERequestStatus::Success if the XML node was parsed successfully, ERequestStatus::Failure otherwise
	 * @see DestructFile
	 */
	virtual ERequestStatus ConstructFile(XmlNodeRef const pRootNode, SFileInfo* const pFileInfo) = 0;

	/**
	 * Free the memory and potentially other resources used by the supplied IFile instance.
	 * Normally, an IFile instance is created by ConstructFile() and a pointer is stored in a member of SFileInfo.
	 * @param pIFile - pointer to the object implementing IFile to be discarded
	 * @return void
	 * @see ConstructFile
	 */
	virtual void DestructFile(IFile* const pIFile) = 0;

	/**
	 * Get the full path to the folder containing the file described by the pFileInfo
	 * @param pFileInfo - ATL-specific information describing the file whose location is being queried
	 * @return A C-string containing the path to the folder where the file corresponding to the pFileInfo is stored
	 */
	virtual char const* const GetFileLocation(SFileInfo* const pFileInfo) = 0;

	/**
	 * Parse the implementation-specific XML node that represents an ATLTriggerImpl, return a pointer to the data needed for identifying
	 * and using this ATLTriggerImpl instance inside the AudioImplementation
	 * @param pRootNode - an XML node corresponding to the new ATLTriggerImpl to be created
	 * @return ITrigger pointer to the audio implementation-specific data needed by the audio middleware and the
	 * @return AudioImplementation code to use the corresponding ATLTriggerImpl; nullptr if the new AudioTriggerImplData instance was not created
	 * @see DestructTrigger
	 */
	virtual ITrigger const* ConstructTrigger(XmlNodeRef const pRootNode) = 0;

	/**
	 * Free the memory and potentially other resources used by the supplied ITrigger instance
	 * @param pITrigger - pointer to the object implementing ITrigger to be discarded
	 * @return void
	 * @see ConstructTrigger
	 */
	virtual void DestructTrigger(ITrigger const* const pITrigger) = 0;

	/**
	 * Create an object implementing ConstructStandaloneFile that stores all of the data needed by the AudioImplementation
	 * to identify and use an audio standalone file. Return a pointer to that object.
	 * @param standaloneFile - reference to the CATLStandaloneFile associated with the IStandaloneFile object we want to construct. It's used as an ID to link the two objects.
	 * @param szFile - full path to the file that wants to be played
	 * @param bLocalized - is the file specified in szFile localized or not
	 * @param pITrigger - if set, routes the playing of the audio file through the specified implementation trigger
	 * @return IStandaloneFile pointer to the audio implementation-specific data needed by the audio middleware and the
	 * @return AudioImplementation code to use the corresponding audio standalone file.
	 * @return nullptr if the new IStandaloneFile instance was not created.
	 * @see DestructStandaloneFile
	 */
	virtual IStandaloneFile* ConstructStandaloneFile(CATLStandaloneFile& standaloneFile, char const* const szFile, bool const bLocalized, ITrigger const* pITrigger = nullptr) = 0;

	/**
	 * Free the memory and potentially other resources used by the supplied IStandaloneFile instance
	 * @param pIStandaloneFile - pointer to the object implementing IStandaloneFile to be discarded
	 * @return void
	 * @see ConstructStandaloneFile
	 */
	virtual void DestructStandaloneFile(IStandaloneFile const* const pIStandaloneFile) = 0;

	/**
	 * Parse the implementation-specific XML node that represents an ATLRtpcImpl, return a pointer to the data needed for identifying
	 * and using this ATLRtpcImpl instance inside the AudioImplementation
	 * @param pRootNode - an XML node corresponding to the new ATLRtpcImpl to be created
	 * @return IParameter pointer to the audio implementation-specific data needed by the audio middleware and the
	 * @return AudioImplementation code to use the corresponding ATLRtpcImpl; nullptr if the new AudioTrigger instance was not created
	 * @see DestructParameter
	 */
	virtual IParameter const* ConstructParameter(XmlNodeRef const pRootNode) = 0;

	/**
	 * Free the memory and potentially other resources used by the supplied IAudioParameter instance
	 * @param pIParameter - pointer to the object implementing IAudioParameter to be discarded
	 * @return void
	 * @see ConstructParameter
	 */
	virtual void DestructParameter(IParameter const* const pIParameter) = 0;

	/**
	 * Parse the implementation-specific XML node that represents an ATLSwitchStateImpl, return a pointer to the data needed for identifying
	 * and using this ATLSwitchStateImpl instance inside the AudioImplementation
	 * @param pRootNode - an XML node corresponding to the new ATLSwitchStateImpl to be created
	 * @return ISwitchState pointer to the audio implementation-specific data needed by the audio middleware and the
	 * @return AudioImplementation code to use the corresponding ATLSwitchStateImpl; nullptr if the new AudioTriggerImplData instance was not created
	 * @see DestructSwitchState
	 */
	virtual ISwitchState const* ConstructSwitchState(XmlNodeRef const pRootNode) = 0;

	/**
	 * Free the memory and potentially other resources used by the supplied ISwitchState instance
	 * @param pISwitchState - pointer to the object implementing ISwitchState to be discarded
	 * @return void
	 * @see ConstructSwitchState
	 */
	virtual void DestructSwitchState(ISwitchState const* const pISwitchState) = 0;

	/**
	 * Parse the implementation-specific XML node that represents an ATLEnvironmentImpl, return a pointer to the data needed for identifying
	 * and using this ATLEnvironmentImpl instance inside the AudioImplementation
	 * @param pRootNode - an XML node corresponding to the new ATLEnvironmentImpl to be created
	 * @return IEnvironment pointer to the audio implementation-specific data needed by the audio middleware and the
	 * @return AudioImplementation code to use the corresponding ATLEnvironmentImpl; nullptr if the new IEnvironment instance was not created
	 * @see DestructEnvironment
	 */
	virtual IEnvironment const* ConstructEnvironment(XmlNodeRef const pRootNode) = 0;

	/**
	 * Free the memory and potentially other resources used by the supplied IEnvironment instance
	 * @param pIEnvironment - pointer to the object implementing IEnvironment to be discarded
	 * @return void
	 * @see ConstructEnvironment
	 */
	virtual void DestructEnvironment(IEnvironment const* const pIEnvironment) = 0;

	/**
	 * Create an object implementing IObject that stores all of the data needed by the AudioImplementation
	 * to identify and use the GlobalAudioObject.
	 * @return IObject pointer to the audio implementation-specific data needed by the audio middleware and the
	 * @return AudioImplementation code to use the corresponding GlobalAudioObject; nullptr if the new IObject instance was not created
	 * @see DestructObject
	 */
	virtual IObject* ConstructGlobalObject() = 0;

	/**
	 * Create an object implementing IObject that stores all of the data needed by the AudioImplementation
	 * to identify and use the AudioObject. Return a pointer to that object.
	 * @param szName - optional name of the object to construct (not used in release builds)
	 * @return IObject pointer to the audio implementation-specific data needed by the audio middleware and the
	 * @return AudioImplementation code to use the corresponding GlobalAudioObject; nullptr if the new IObject instance was not created
	 * @see DestructObject
	 */
	virtual IObject* ConstructObject(char const* const szName = nullptr) = 0;

	/**
	 * Free the memory and potentially other resources used by the supplied IObject instance
	 * @param pIObject - pointer to the object implementing IObject to be discarded
	 * @return void
	 * @see ConstructObject, ConstructGlobalObject
	 */
	virtual void DestructObject(IObject const* const pIObject) = 0;

	/**
	 * Construct an object implementing IListener that stores all of the data needed by the AudioImplementation
	 * to identify and use an AudioListener. Return a pointer to that object.
	 * @param szName - optional name of the listener to construct (not used in release builds)
	 * @return DrxAudio::Impl::IListener pointer to the audio implementation-specific data needed by the audio middleware and the
	 * @return AudioImplementation code to use the corresponding AudioListener; nullptr if the new DrxAudio::Impl::IListener instance was not created.
	 * @see DestructListener
	 */
	virtual IListener* ConstructListener(char const* const szName = nullptr) = 0;

	/**
	 * Destruct the supplied DrxAudio::Impl::IListener instance.
	 * @param pIListener - pointer to the object implementing DrxAudio::Impl::IListener to be discarded
	 * @return void
	 * @see ConstructListener
	 */
	virtual void DestructListener(IListener* const pIListener) = 0;

	/**
	 * Create an object implementing IEvent that stores all of the data needed by the AudioImplementation
	 * to identify and use an AudioEvent. Return a pointer to that object.
	 * @param event - ATL Audio-Event associated with the newly created AudioEvent
	 * @return IEvent pointer to the audio implementation-specific data needed by the audio middleware and the
	 * @return AudioImplementation code to use the corresponding AudioEvent; nullptr if the new IEvent instance was not created
	 * @see DestructEvent
	 */
	virtual IEvent* ConstructEvent(CATLEvent& event) = 0;

	/**
	 * Free the memory and potentially other resources used by the supplied IEvent instance
	 * @param pIEvent - pointer to the object implementing IEvent to be discarded
	 * @return void
	 * @see ConstructEvent
	 */
	virtual void DestructEvent(IEvent const* const pIEvent) = 0;

	/**
	 * Called whenever a Gamepad gets connected.
	 * This is used by audio middleware that supports controller effects such as rumble.
	 * @param deviceUniqueID - unique device identifier
	 * @return void
	 * @see GamepadDisconnected
	 */
	virtual void GamepadConnected(DeviceId const deviceUniqueID) = 0;

	/**
	 * Called whenever a Gamepad gets disconnected.
	 * This is used by audio middleware that supports controller effects such as rumble.
	 * @param deviceUniqueID - unique device identifier
	 * @return void
	 * @see GamepadConnected
	 */
	virtual void GamepadDisconnected(DeviceId const deviceUniqueID) = 0;

	/**
	 * Informs the audio middleware that the localized sound banks and streamed files need to use
	 * a different language. NOTE: this function DOES NOT unload or reload the currently loaded audio files
	 * @param szLanguage - a C-string representing the drx3D language
	 * @return void
	 * @see GetFileLocation
	 */
	virtual void SetLanguage(char const* const szLanguage) = 0;

	/**
	 * Return a string of the audio middeware folder name plus a separator. This string is used for building the path
	 * to audio files and audio controls editor data.
	 * @param[out] implInfo - a reference to an instance of SImplInfo
	 * @return void
	 */
	virtual void GetInfo(SImplInfo& implInfo) const = 0;

	//////////////////////////////////////////////////////////////////////////
	// NOTE: The methods below are ONLY USED when INCLUDE_AUDIO_PRODUCTION_CODE is defined!
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Fill in the memoryInfo describing the current memory usage of this AudioImplementation.
	 * This data gets displayed in the AudioDebug header shown on the screen whenever s_DrawAudioDebug is not 0
	 * @param[out] memoryInfo - a reference to an instance of SMemoryInfo
	 * @return void
	 */
	virtual void GetMemoryInfo(SMemoryInfo& memoryInfo) const = 0;

	/**
	 * Asks the audio implementation to fill the fileData structure with data (e.g. duration of track) relating to the
	 * standalone file referenced in szName.
	 * @param[in] szName - filepath to the standalone file
	 * @param[out] fileData - a reference to an instance of SAudioFileData
	 * @return void
	 */
	virtual void GetFileData(char const* const szName, SFileData& fileData) const = 0;
};
} //endns Impl
} //endns DrxAudio
