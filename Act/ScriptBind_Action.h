// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SCRIPTBIND_ACTION__
#define __SCRIPTBIND_ACTION__

#pragma once

#include <drx3D/Script/IScriptSystem.h>
#include <drx3D/Script/ScriptHelpers.h>
#include <drx3D/Act/IViewSystem.h>

// FIXME: Cell SDK GCC bug workaround.
#ifndef __IGAMEOBJECTSYSTEM_H__
	#include "IGameObjectSystem.h"
#endif

class CDrxAction;

class CScriptBind_Action :
	public CScriptableBase
{
public:
	CScriptBind_Action(CDrxAction* pDrxAction);
	virtual ~CScriptBind_Action();

	void         Release() { delete this; };

	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	//! <code>Action.LoadXML(definitionFile, dataFile)</code>
	//!		<param name="definitionFile">.</param>
	//!		<param name="dataFile">XML-lua data file name.</param>
	i32 LoadXML(IFunctionHandler* pH, tukk definitionFile, tukk dataFile);

	//! <code>Action.SaveXML(definitionFile, dataFile, dataTable)</code>
	//!		<param name="definitionFile">.</param>
	//!		<param name="dataFile">XML-lua data file name.</param>
	//!		<param name="dataTable">.</param>
	i32 SaveXML(IFunctionHandler* pH, tukk definitionFile, tukk dataFile, SmartScriptTable dataTable);

	//! <code>Action.IsServer()</code>
	//! <returns>true if the current script runs on a server.</returns>
	i32 IsServer(IFunctionHandler* pH);

	//! <code>Action.IsClient()</code>
	//! <returns>true if the current script runs on a client.</returns>
	i32 IsClient(IFunctionHandler* pH);

	//! <code>Action.IsGameStarted()</code>
	//! <description>true if the game has started.</description>
	i32 IsGameStarted(IFunctionHandler* pH);

	//! <code>Action.IsRMIServer()</code>
	//! <description>true if the current script is running on an RMI (Remote Method Invocation) server.</description>
	i32 IsRMIServer(IFunctionHandler* pH);

	//! <code>Action.GetPlayerList()</code>
	//! <description>Checks the current players list.</description>
	i32 GetPlayerList(IFunctionHandler* pH);

	//! <code>Action.IsGameObjectProbablyVisible( gameObject )</code>
	//!		<param name="gameObject">Object that we want to check.</param>
	//! <returns>true if an object is probably visible.</returns>
	i32 IsGameObjectProbablyVisible(IFunctionHandler* pH, ScriptHandle gameObject);

	//! <code>Action.ActivateEffect( name )</code>
	//!		<param name="name">Name of the effect.</param>
	//! <description>Activates the specified effect.</description>
	i32 ActivateEffect(IFunctionHandler* pH, tukk name);

	//! <code>Action.GetWaterInfo( pos )</code>
	//!		<param name="pos">Position to be checked.</param>
	//! <description>Gets information about the water at the position pos.</description>
	i32 GetWaterInfo(IFunctionHandler* pH, Vec3 pos);

	//! <code>Action.SetViewCamera()</code>
	//! <description>Saves the previous valid view and override it with the current camera settings.</description>
	i32 SetViewCamera(IFunctionHandler* pH);

	//! <code>Action.ResetToNormalCamera()</code>
	//! <description>Resets the camera to the last valid view stored.</description>
	i32 ResetToNormalCamera(IFunctionHandler* pH);

	//! <code>Action.GetServer( number )</code>
	//! <description>Gets the server.</description>
	i32 GetServer(IFunctionHandler* pFH, i32 number);

	//! <code>Action.RefreshPings()</code>
	//! <description>Refreshes pings for all the servers listed.</description>
	i32 RefreshPings(IFunctionHandler* pFH);

	//! <code>Action.ConnectToServer( server )</code>
	//!		<param name="server">String that specifies the server to be used for the connection.</param>
	//! <description>Connects to the specified server.</description>
	i32 ConnectToServer(IFunctionHandler* pFH, tuk server);

	//! <code>Action.GetServerTime()</code>
	//! <description>Gets the current time on the server.</description>
	i32 GetServerTime(IFunctionHandler* pFH);

	//! <code>Action.PauseGame( pause )</code>
	//!		<param name="pause">True to set the game into the pause mode, false to resume the game.</param>
	//! <description>Puts the game into pause mode.</description>
	i32 PauseGame(IFunctionHandler* pFH, bool pause);

	//! <code>Action.IsImmersivenessEnabled()</code>
	//! <returns>true if immersive multiplayer is enabled.</returns>
	i32 IsImmersivenessEnabled(IFunctionHandler* pH);

	//! <code>Action.IsChannelSpecial()</code>
	//! <returns>true if the channel is special.</returns>
	i32 IsChannelSpecial(IFunctionHandler* pH);

	//! <code>Action.ForceGameObjectUpdate( entityId, force )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="force">True to force the update, false otherwise.</param>
	//! <description>Forces the game object to be updated.</description>
	i32 ForceGameObjectUpdate(IFunctionHandler* pH, ScriptHandle entityId, bool force);

	//! <code>Action.CreateGameObjectForEntity( entityId )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//! <description>Creates a game object for the specified entity.</description>
	i32 CreateGameObjectForEntity(IFunctionHandler* pH, ScriptHandle entityId);

	//! <code>Action.BindGameObjectToNetwork( entityId )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//! <description>Binds game object to the network.</description>
	i32 BindGameObjectToNetwork(IFunctionHandler* pH, ScriptHandle entityId);

	//! <code>Action.ActivateExtensionForGameObject( entityId, extension, activate )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="extension">Extension name.</param>
	//!		<param name="activate">True to activate the extension, false to deactivate it.</param>
	//! <description>Activates a specified extension for a game object.</description>
	i32 ActivateExtensionForGameObject(IFunctionHandler* pH, ScriptHandle entityId, tukk extension, bool activate);

	//! <code>Action.SetNetworkParent( entityId, parentId )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="parentID">Identifier for the parent network.</param>
	//! <description>Sets the network parent.</description>
	i32 SetNetworkParent(IFunctionHandler* pH, ScriptHandle entityId, ScriptHandle parentId);

	//! <code>Action.IsChannelOnHold( channelId )</code>
	//!		<param name="channelId">Identifier for the channel.
	//! <description>Checks if the specified channel is on hold.</description>
	i32 IsChannelOnHold(IFunctionHandler* pH, i32 channelId);

	//! <code>Action.BanPlayer( entityId, message )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="message">Message for the ban.</param>
	//! <description>Bans a specified player.</description>
	i32 BanPlayer(IFunctionHandler* pH, ScriptHandle entityId, tukk message);

	//! <code>Action.PersistantSphere( pos, radius, color, name, timeout )</code>
	//!		<param name="pos">Position of the sphere.</param>
	//!		<param name="radius">Radius of the sphere.</param>
	//!		<param name="color">Color of the sphere.</param>
	//!		<param name="name">Name assigned to the sphere.</param>
	//!		<param name="timeout">Timeout for the sphere.</param>
	//! <description>Adds a persistent sphere to the world.</description>
	i32 PersistantSphere(IFunctionHandler* pH, Vec3 pos, float radius, Vec3 color, tukk name, float timeout);

	//! <code>Action.PersistantLine( start, end, color, name, timeout )	</code>
	//!		<param name="start">Starting position of the line.</param>
	//!		<param name="end">Ending position of the line.</param>
	//!		<param name="color">Color of the line.</param>
	//!		<param name="name">Name assigned to the line.</param>
	//!		<param name="timeout">Timeout for the line.</param>
	//! <description>Adds a persistent line to the world.</description>
	i32 PersistantLine(IFunctionHandler* pH, Vec3 start, Vec3 end, Vec3 color, tukk name, float timeout);

	//! <code>Action.PersistantArrow( pos, radius, dir, color, name, timeout )</code>
	//!		<param name="pos">Position of the arrow.</param>
	//!		<param name="radius">Radius of the arrow.</param>
	//!		<param name="dir">Direction of the arrow.</param>
	//!		<param name="color">Color of the arrow.</param>
	//!		<param name="name">Name assigned to the arrow.</param>
	//!		<param name="timeout">Timeout for the arrow.</param>
	//! <description>Adds a persistent arrow to the world.</description>
	i32 PersistantArrow(IFunctionHandler* pH, Vec3 pos, float radius, Vec3 dir, Vec3 color, tukk name, float timeout);

	//! <code>Action.Persistant2DText( text, size, color, name, timeout )</code>
	//!		<param name="text">Text that has to be displayed.</param>
	//!		<param name="size">Size of the 2D text.</param>
	//!		<param name="color">Color of the 2D text.</param>
	//!		<param name="name">Name assigned to the 2D text.</param>
	//!		<param name="timeout">Timeout for the 2D text.</param>
	//! <description>Adds a persistent 2D text.</description>
	i32 Persistant2DText(IFunctionHandler* pH, tukk text, float size, Vec3 color, tukk name, float timeout);

	//! <code>Action.PersistantEntityTag( entityId, text )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="text">Text for the entity tag.</param>
	//! <description>Adds a persistent entity tag.</description>
	i32 PersistantEntityTag(IFunctionHandler* pH, ScriptHandle entityId, tukk text);

	//! <code>Action.ClearEntityTags( entityId )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//! <description>Clears the tag for the specified entity.</description>
	i32 ClearEntityTags(IFunctionHandler* pH, ScriptHandle entityId);

	//! <code>Action.ClearStaticTag( entityId, staticId )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="staticId">Identifier for the static tag.</param>
	//! <description>Clears the specified static tag for the specified entity.</description>
	i32 ClearStaticTag(IFunctionHandler* pH, ScriptHandle entityId, tukk staticId);

	//! <code>Action.SendGameplayEvent( entityId, event )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="event">Integer for the event.</param>
	//! <description>Sends an event for the gameplay.</description>
	i32 SendGameplayEvent(IFunctionHandler* pH, ScriptHandle entityId, i32 event);

	//! <code>Action.CacheItemSound( itemName )</code>
	//!		<param name="itemName">Item name string.</param>
	//! <description>Caches an item sound.</description>
	i32 CacheItemSound(IFunctionHandler* pH, tukk itemName);

	//! <code>Action.CacheItemGeometry( itemName )</code>
	//!		<param name="itemName">Item name string.</param>
	//! <description>Caches an item geometry.</description>
	i32 CacheItemGeometry(IFunctionHandler* pH, tukk itemName);

	//! <code>Action.DontSyncPhysics( entityId )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//! <description>Doesn't sync physics for the specified entity.</description>
	i32 DontSyncPhysics(IFunctionHandler* pH, ScriptHandle entityId);

	//! <code>Action.EnableSignalTimer( entityId, sText )</code>
	//!		<param name="entityId">Identifier for the entity.
	//!		<param name="sText">Text for the signal.</param>
	//! <description>Enables the signal timer.</description>
	i32 EnableSignalTimer(IFunctionHandler* pH, ScriptHandle entityId, tukk sText);

	//! <code>Action.DisableSignalTimer( entityId, sText )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="sText">Text for the signal.</param>
	//! <description>Disables the signal timer.</description>
	i32 DisableSignalTimer(IFunctionHandler* pH, ScriptHandle entityId, tukk sText);

	//! <code>Action.SetSignalTimerRate( entityId, sText, fRateMin, fRateMax )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="sText">Text for the signal.</param>
	//!		<param name="fRateMin">Minimum rate for the signal timer.</param>
	//!		<param name="fRateMax">Maximum rate for the signal timer.</param>
	//! <description>Sets the rate for the signal timer.</description>
	i32 SetSignalTimerRate(IFunctionHandler* pH, ScriptHandle entityId, tukk sText, float fRateMin, float fRateMax);

	//! <code>Action.ResetSignalTimer( entityId, sText )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="sText">Text for the signal.</param>
	//! <description>Resets the rate for the signal timer.</description>
	i32 ResetSignalTimer(IFunctionHandler* pH, ScriptHandle entityId, tukk sText);

	//! <code>Action.EnableRangeSignaling( entityId, bEnable )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="bEnable">Enable/Disable range signalling.</param>
	//! <description>Enable/Disable range signalling for the specified entity.</description>
	i32 EnableRangeSignaling(IFunctionHandler* pH, ScriptHandle entityId, bool bEnable);

	//! <code>Action.DestroyRangeSignaling( entityId )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	i32 DestroyRangeSignaling(IFunctionHandler* pH, ScriptHandle entityId);

	//! <code>Action.ResetRangeSignaling( entityId )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	i32 ResetRangeSignaling(IFunctionHandler* pH, ScriptHandle entityId);

	//! <code>Action.AddRangeSignal( entityId, fRadius, fFlexibleBoundary, sSignal )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="fRadius">Radius of the range area.</param>
	//!		<param name="fFlexibleBoundary">Flexible boundary size.</param>
	//!		<param name="sSignal">String for signal.</param>
	//! <description>Adds a range for the signal.</description>
	i32 AddRangeSignal(IFunctionHandler* pH, ScriptHandle entityId, float fRadius, float fFlexibleBoundary, tukk sSignal);

	//! <code>Action.AddTargetRangeSignal( entityId, targetId, fRadius, fFlexibleBoundary, sSignal )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="targetId">Identifier for the target.</param>
	//!		<param name="fRadius">Radius of the range area.</param>
	//!		<param name="fFlexibleBoundary">Flexible boundary size.</param>
	//!		<param name="sSignal">String for signal.</param>
	i32 AddTargetRangeSignal(IFunctionHandler* pH, ScriptHandle entityId, ScriptHandle targetId, float fRadius, float fFlexibleBoundary, tukk sSignal);

	//! <code>Action.AddRangeSignal( entityId, fAngle, fFlexibleBoundary, sSignal )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="fAngle">Angle value.</param>
	//!		<param name="fFlexibleBoundary">Flexible boundary size.</param>
	//!		<param name="sSignal">String for signal.</param>
	//! <description>Adds an angle for the signal.</description>
	i32 AddAngleSignal(IFunctionHandler* pH, ScriptHandle entityId, float fAngle, float fFlexibleBoundary, tukk sSignal);

	//! <code>Action.RegisterWithAI()</code>
	//! <description>Registers the entity to AI System, creating an AI object associated to it.</description>
	i32 RegisterWithAI(IFunctionHandler* pH);

	//! <code>Action.HasAI( entityId )</code>
	//! <returns>true if the entity has an AI object associated to it, meaning it has been registered with the AI System</returns>
	i32 HasAI(IFunctionHandler* pH, ScriptHandle entityId);

	//! <code>Action.GetClassName( classId )</code>
	//! <returns>the matching class name if available for specified classId.</returns>
	i32 GetClassName(IFunctionHandler* pH, i32 classId);

	//! <code>Action.SetAimQueryMode( entityId, mode )</code>
	//!		<param name="entityId">Identifier for the entity.</param>
	//!		<param name="mode">QueryAimFromMovementController or OverriddenAndAiming or OverriddenAndNotAiming</param>
	//! <description>
	//!		Set the aim query mode for the ai proxy. Normally the ai proxy
	//!		asks the movement controller if the character is aiming.
	//!		You can override that and set your own 'isAiming' state.
	//! </description>
	i32 SetAimQueryMode(IFunctionHandler* pH, ScriptHandle entityId, i32 mode);

	//! <code>Action.PreLoadADB( adbFileName )</code>
	//!		<param name="adbFileName">The path and filename of the animation ADB file which is to be pre-loaded.</param>
	//! <description>Use this function to pre-cache ADB files.</description>
	i32 PreLoadADB(IFunctionHandler* pH, tukk adbFileName);

private:
	void RegisterGlobals();
	void RegisterMethods();

	CDrxAction* m_pDrxAction;
	IView*      m_pPreviousView;
};

#endif
