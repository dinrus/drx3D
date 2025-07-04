// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.
#pragma once

<drx3D/CoreX/Lobby/IDrxStats.h>

struct IDrxVoice
{
	// <interfuscator:shuffle>
	virtual ~IDrxVoice(){}

	//! Turn voice on/off between a local and remote user. When mute is on no voice will be sent to or received from the remote user.
	//! \param localUser   The pad number of the local user.
	//! \param remoteUser   The DrxUserID of the remote user.
	//! \param mute			   Mute on/off
	virtual void Mute(u32 localUser, DrxUserID remoteUser, bool mute) = 0;

	//! Turn voice on/off for a local user. When mute is on no voice will be sent to or received to any remote user.
	//! \param localUser   The pad number of the local user.
	//! \param mute		   Mute on/off
	virtual void MuteLocalPlayer(u32 localUser, bool mute) = 0;

	//! Has voice between a local and remote user been muted by game.
	//! \param localUser   The pad number of the local user.
	//! \param remoteUser   The DrxUserID of the remote user.
	//! \return		   Will return true if the game has set mute to true.
	virtual bool IsMuted(u32 localUser, DrxUserID remoteUser) = 0;

	//! Add remote user to the local user's global mute list via the external SDK. Users added to the global mute list.
	//! will persist between games/cross titles/power cycles.
	//! \param localUser   The pad number of the local user.
	//! \param remoteUser   The DrxUserID of the remote user.
	//! \param mute			   Mute on/off
	virtual void MuteExternally(u32 localUser, DrxUserID remoteUser, bool mute) = 0;

	//! Has voice between a local and remote user muted via the external SDK's GUI.
	//! Microsoft TCR's say that a user must have no indication that they have been muted with the Xbox GUI
	//! so this function must not be used for that purpose.
	//! \param localUser   The pad number of the local user.
	//! \param remoteUser   The DrxUserID of the remote user.
	//! \return		   Will return true if the user has set mute to true via the external SDK's GUI.
	virtual bool IsMutedExternally(u32 localUser, DrxUserID remoteUser) = 0;

	//! Is the remote user currently speaking to the local user.
	//! \param localUser   The pad number of the local user.
	//! \param remoteUser   The DrxUserID of the remote user.
	//! \return		   true if voice data is currently being received from the remote user to the local user.
	virtual bool IsSpeaking(u32 localUser, DrxUserID remoteUser) = 0;

	//! Does the user have a microphone connected.
	//! \param userID		   The DrxUserID of the user
	//! \return		   true if a microphone is connected
	virtual bool IsMicrophoneConnected(DrxUserID userID) = 0;

	//! Enables Push-to-talk functionality for microphone. Currently only supported on PC.
	//! \param localUser   The pad number of the local user.
	//! \param bSet			   Enabled true or false.
	virtual void EnablePushToTalk(u32 localUser, bool bSet) = 0;

	//! Is Push-to-talk mode enabled?
	//! \return		   true or false
	virtual bool IsPushToTalkEnabled(u32 localUser) = 0;

	// Mark the push-to-talk button as depressed.
	//! \param localUser   The pad number of the local user.
	virtual void PushToTalkButtonDown(u32 localUser) = 0;

	//! Mark the push-to-talk button as released.
	//! \param localUser   The pad number of the local user.
	virtual void PushToTalkButtonUp(u32 localUser) = 0;

	//! Set the playback volume for voice communications.
	//! \param localUser   The pad number of the local user.
	//! \param volume		   The volume. 0.0 to 1.0
	virtual void SetVoicePlaybackVolume(u32 localUser, float volume) = 0;

	//! Get the playback volume for voice communications.
	//! \param localUser   The pad number of the local user.
	//! \return		   volume, 0.0 to 1.0
	virtual float GetVoicePlaybackVolume(u32 localUser) = 0;
	// </interfuscator:shuffle>
};
