// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ScriptBind_System.h
//  Version:     v1.00
//  Created:     8/7/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ScriptBind_System_h__
#define __ScriptBind_System_h__

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/Script/IScriptSystem.h>

struct ISystem;
struct ILog;
struct IRenderer;
struct IConsole;
struct IInput;
struct ITimer;
struct IEntitySystem;
struct I3DEngine;
struct IPhysicalWorld;
struct ICVar;

/*!
 *	<description>This class implements script-functions for exposing the System functionalities.</description>
 *	<remarks>This object doesn't have a global mapping(is not present as global variable into the script state).</remarks>
 */
class CScriptBind_System : public CScriptableBase
{
public:
	CScriptBind_System(IScriptSystem* pScriptSystem, ISystem* pSystem);
	virtual ~CScriptBind_System();
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

public:
	//! <code>System.CreateDownload()</code>
	i32 CreateDownload(IFunctionHandler* pH);

	//! <code>System.LoadFont(pszName)</code>
	//!		<param name="pszName">Font name.</param>
	//! <description>Loads a font.</description>
	i32 LoadFont(IFunctionHandler* pH);

	//! <code>System.ExecuteCommand(szCmd)</code>
	//!		<param name="szCmd">Command string.</param>
	//! <description>Executes a command.</description>
	i32 ExecuteCommand(IFunctionHandler* pH);

	//! <code>System.LogToConsole(sText)</code>
	//!		<param name="sText">Text to be logged.</param>
	//! <description>Logs a message to the console.</description>
	i32 LogToConsole(IFunctionHandler* pH);

	//! <code>System.ClearConsole()</code>
	//! <description>Clears the console.</description>
	i32 ClearConsole(IFunctionHandler* pH);

	//! <code>System.Log(sText)</code>
	//!		<param name="sText">Text to be logged.</param>
	//! <description>Logs a message.</description>
	i32 Log(IFunctionHandler* pH);

	//! <code>System.LogAlways(sText)</code>
	//!		<param name="sText">Text to be logged.</param>
	//! <description>Logs important data that must be printed regardless verbosity.</description>
	i32 LogAlways(IFunctionHandler* pH);

	//! <code>System.Warning(sText)</code>
	//!		<param name="sText">Text to be logged.</param>
	//! <description>Shows a message text with the warning severity.</description>
	i32 Warning(IFunctionHandler* pH);

	//! <code>System.Error(sText)</code>
	//!		<param name="sText">Text to be logged.</param>
	//! <description>Shows a message text with the error severity.</description>
	i32 Error(IFunctionHandler* pH);

	//! <code>System.IsEditor()</code>
	//! <description>Checks if the system is the editor.</description>
	i32 IsEditor(IFunctionHandler* pH);

	//! <code>System.IsEditing()</code>
	//! <description>Checks if the system is in pure editor mode, i.e. not editor game mode.</description>
	i32 IsEditing(IFunctionHandler* pH);

	//! <code>System.GetCurrTime()</code>
	//! <description>Gets the current time.</description>
	i32 GetCurrTime(IFunctionHandler* pH);

	//! <code>System.GetCurrAsyncTime()</code>
	//! <description>Gets the current asynchronous time.</description>
	i32 GetCurrAsyncTime(IFunctionHandler* pH);

	//! <code>System.GetFrameTime()</code>
	//! <description>Gets the frame time.</description>
	i32 GetFrameTime(IFunctionHandler* pH);

	//! <code>System.GetLocalOSTime()</code>
	//! <description>Gets the local operating system time.</description>
	i32 GetLocalOSTime(IFunctionHandler* pH);

	//! <code>System.GetUserName()</code>
	//! <description>Gets the username on this machine.</description>
	i32 GetUserName(IFunctionHandler* pH);

	//! <code>System.ShowConsole(nParam)</code>
	//!		<param name="nParam">1 to show the console, 0 to hide.</param>
	//! <description>Shows/hides the console.</description>
	i32 ShowConsole(IFunctionHandler* pH);

	//! <code>System.CheckHeapValid(name)</code>
	//!		<param name="name">Name string.</param>
	//! <description>Checks the heap validity.</description>
	i32 CheckHeapValid(IFunctionHandler* pH);

	//! <code>System.GetConfigSpec()</code>
	//! <description>Gets the config specification.</description>
	i32 GetConfigSpec(IFunctionHandler* pH);

	//! <code>System.IsMultiplayer()</code>
	//! <description>Checks if the game is multiplayer.</description>
	i32 IsMultiplayer(IFunctionHandler* pH);

	//! <code>System.GetEntity(entityId)</code>
	//!		<param name="entityId">Entity identifier.</param>
	//! <description>Gets an entity from its ID.</description>
	i32 GetEntity(IFunctionHandler* pH);

	//! <code>System.GetEntityClass(entityId)</code>
	//!		<param name="entityId">Entity identifier.</param>
	//! <description>Gets an entity class from its ID.</description>
	i32 GetEntityClass(IFunctionHandler* pH);

	//! <code>System.GetEntities(center, radius)</code>
	//!		<param name="center">Center position vector for the area where to get entities.</param>
	//!		<param name="radius">Radius of the area.</param>
	//! <description>Gets all the entities contained in the specific area of the level.</description>
	i32 GetEntities(IFunctionHandler* pH);

	//! <code>System.GetEntitiesByClass(EntityClass)</code>
	//!		<param name="EntityClass">Entity class name.</param>
	//! <description>Gets all the entities of the specified class.</description>
	i32 GetEntitiesByClass(IFunctionHandler* pH, tukk EntityClass);

	//! <code>System.GetEntitiesInSphere( centre, radius )</code>
	//!		<param name="centre">Centre position vector for the sphere where to look at entities.</param>
	//!		<param name="radius">Radius of the sphere.</param>
	//! <description>Gets all the entities contained into the specified sphere.</description>
	i32 GetEntitiesInSphere(IFunctionHandler* pH, Vec3 center, float radius);

	//! <code>System.GetEntitiesInSphereByClass( centre, radius, EntityClass )</code>
	//!		<param name="centre">Centre position vector for the sphere where to look at entities.</param>
	//!		<param name="radius">Radius of the sphere.</param>
	//!		<param name="EntityClass">Entity class name.</param>
	//! <description>Gets all the entities contained into the specified sphere for the specific class name.</description>
	i32 GetEntitiesInSphereByClass(IFunctionHandler* pH, Vec3 center, float radius, tukk EntityClass);

	//! <code>System.GetPhysicalEntitiesInBox( centre, radius )</code>
	//!		<param name="centre">Centre position vector for the area where to look at entities.</param>
	//!		<param name="radius">Radius of the sphere.</param>
	//! <description>Gets all the entities contained into the specified area.</description>
	i32 GetPhysicalEntitiesInBox(IFunctionHandler* pH, Vec3 center, float radius);

	//! <code>System.GetPhysicalEntitiesInBoxByClass( centre, radius, className )</code>
	//!		<param name="centre">Centre position vector for the area where to look at entities.</param>
	//!		<param name="radius">Radius of the sphere.</param>
	//!		<param name="className">Entity class name.</param>
	//! <description>Gets all the entities contained into the specified area for the specific class name.</description>
	i32 GetPhysicalEntitiesInBoxByClass(IFunctionHandler* pH, Vec3 center, float radius, tukk className);

	//! <code>System.GetNearestEntityByClass( centre, radius, className )</code>
	//!		<param name="centre">Centre position vector for the area where to look at entities.</param>
	//!		<param name="radius">Radius of the sphere.</param>
	//!		<param name="className">Entity class name.</param>
	//! <description>Gets the nearest entity with the specified class.</description>
	i32 GetNearestEntityByClass(IFunctionHandler* pH, Vec3 center, float radius, tukk className);

	//! <code>System.GetEntityByName( sEntityName )</code>
	//! <description>
	//!		Retrieve entity table for the first entity with specified name.
	//!		If multiple entities with same name exist, first one found is returned.
	//! </description>
	//!		<param name="sEntityName">Name of the entity to search.</param>
	i32 GetEntityByName(IFunctionHandler* pH, tukk sEntityName);

	//! <code>System.GetEntityIdByName( sEntityName )</code>
	//! <description>
	//!		Retrieve entity Id for the first entity with specified name.
	//!		If multiple entities with same name exist, first one found is returned.
	//! </description>
	//!		<param name="sEntityName">Name of the entity to search.</param>
	i32 GetEntityIdByName(IFunctionHandler* pH, tukk sEntityName);

	//! <code>System.DrawLabel( vPos, fSize, text [, r [, g [, b [, alpha]]]] )</code>
	//!		<param name="vPos">Position vector.</param>
	//!		<param name="fSize">Size for the label.</param>
	//!		<param name="text">Text of the label.</param>
	//!		<param name="r">Red component for the label colour. Default is 1.</param>
	//!		<param name="g">Green component for the label colour. Default is 1.</param>
	//!		<param name="b">Blue component for the label colour. Default is 1.</param>
	//!		<param name="alpha">Alpha component for the label colour. Default is 1.</param>
	//! <description>Draws a label with the specified parameter.</description>
	i32 DrawLabel(IFunctionHandler* pH);

	//! <code>System.DeformTerrain()</code>
	//! <description>Deforms the terrain.</description>
	i32 DeformTerrain(IFunctionHandler* pH);

	//! <code>System.DeformTerrainUsingMat()</code>
	//! <description>Deforms the terrain using material.</description>
	i32 DeformTerrainUsingMat(IFunctionHandler* pH);

	//! <code>System.ApplyForceToEnvironment( pos, force, radius)</code>
	//!		<param name="pos">Position of the force.</param>
	//!		<param name="force">Strength of the force.</param>
	//!		<param name="radius">Area where the force has effects.</param>
	//! <description>Applies a force to the environment.</description>
	i32 ApplyForceToEnvironment(IFunctionHandler* pH);

	//! <code>System.ScreenToTexture()</code>
	i32 ScreenToTexture(IFunctionHandler* pH);

	//! <code>System.DrawTriStrip(handle, nMode, vtxs, r, g, b, alpha )</code>
	//!		<param name="handle">.</param>
	//!		<param name="nMode">.</param>
	//!		<param name="vtx">.</param>
	//!		<param name="r">Red component for the label color. Default is 1.</param>
	//!		<param name="g">Green component for the label color. Default is 1.</param>
	//!		<param name="b">Blue component for the label color. Default is 1.</param>
	//!		<param name="alpha">Alpha component for the label color. Default is 1.</param>
	//! <description>Draws a triangle strip.</description>
	i32 DrawTriStrip(IFunctionHandler* pH);

	//! <code>System.DrawLine( p1, p2, r, g, b, alpha )</code>
	//!		<param name="p1">Start position of the line.</param>
	//!		<param name="p2">End position of the line.</param>
	//!		<param name="r">Red component for the label color. Default is 1.</param>
	//!		<param name="g">Green component for the label color. Default is 1.</param>
	//!		<param name="b">Blue component for the label color. Default is 1.</param>
	//!		<param name="alpha">Alpha component for the label color. Default is 1.</param>
	//! <description>Draws a line.</description>
	i32 DrawLine(IFunctionHandler* pH);

	//! <code>System.Draw2DLine(p1x, p1y, p2x, p2y, r, g, b, alpha )</code>
	//!		<param name="p1x">X value of the start point of the line.</param>
	//!		<param name="p1y">Y value of the start point of the line.</param>
	//!		<param name="p2x">X value of the end point of the line.</param>
	//!		<param name="p2y">Y value of the end point of the line.</param>
	//!		<param name="r">Red component for the label color. Default is 1.</param>
	//!		<param name="g">Green component for the label color. Default is 1.</param>
	//!		<param name="b">Blue component for the label color. Default is 1.</param>
	//!		<param name="alpha">Alpha component for the label color. Default is 1.</param>
	//! <description>Draws a 2D line.</description>
	i32 Draw2DLine(IFunctionHandler* pH);

	//! <code>System.DrawText( x, y, text, font, size, p2y, r, g, b, alpha )</code>
	//!		<param name="x">X position for the text.</param>
	//!		<param name="y">Y position for the text.</param>
	//!		<param name="text">Text to be displayed.</param>
	//!		<param name="font">Font name.</param>
	//!		<param name="size">Text size.</param>
	//!		<param name="r">Red component for the label color. Default is 1.</param>
	//!		<param name="g">Green component for the label color. Default is 1.</param>
	//!		<param name="b">Blue component for the label color. Default is 1.</param>
	//!		<param name="alpha">Alpha component for the label color. Default is 1.</param>
	//! <description>Draws text.</description>
	i32 DrawText(IFunctionHandler* pH);

	//! <code>System.DrawSphere( x, y, z, radius, r, g, b, a )</code>
	//!		<param name="x">X position for the centre of the sphere.</param>
	//!		<param name="y">Y position for the centre of the sphere.</param>
	//!		<param name="z">Z position for the centre of the sphere.</param>
	//!		<param name="radius">Radius of the sphere.</param>
	//!		<param name="r">Red component for the sphere color. Default is 1.</param>
	//!		<param name="g">Green component for the sphere color. Default is 1.</param>
	//!		<param name="b">Blue component for the sphere color. Default is 1.</param>
	//!		<param name="a">Alpha component for the sphere color. Default is 1.</param>
	//! <description>Draws a wireframe sphere.</description>
	i32 DrawSphere(IFunctionHandler* pH, float x, float y, float z, float radius, i32 r, i32 g, i32 b, i32 a);

	//! <code>System.DrawAABB( x, y, z, x2, y2, z2, r, g, b, a )</code>
	//!		<param name="x">X position of first corner.</param>
	//!		<param name="y">Y position of first corner.</param>
	//!		<param name="z">Z position of first corner.</param>
	//!		<param name="x2">X position of second corner.</param>
	//!		<param name="y2">Y position of second corner.</param>
	//!		<param name="z2">Z position of second corner.</param>
	//!		<param name="r">Red component for the sphere color. Default is 1.</param>
	//!		<param name="g">Green component for the sphere color. Default is 1.</param>
	//!		<param name="b">Blue component for the sphere color. Default is 1.</param>
	//!		<param name="a">Alpha component for the sphere color. Default is 1.</param>
	//! <description>Draws a axis aligned box.</description>
	i32 DrawAABB(IFunctionHandler* pH, float x, float y, float z, float x2, float y2, float z2, i32 r, i32 g, i32 b, i32 a);

	//! <code>System.DrawAABB( x, y, z, x2, y2, z2, r, g, b, a )</code>
	//!		<param name="x">X component of the centre point of the box.</param>
	//!		<param name="y">Y component of the centre point of the box.</param>
	//!		<param name="z">Z component of the centre point of the box.</param>
	//!		<param name="w">Width of the box.</param>
	//!		<param name="h">Height of the box.</param>
	//!		<param name="d">Depth of the box.</param>
	//!		<param name="rx">Rotation in X of the box.</param>
	//!		<param name="ry">Rotation in X of the box.</param>
	//!		<param name="rz">Rotation in X of the box.</param>
	//! <description>Draws an object bounding box.</description>
	i32 DrawOBB(IFunctionHandler* pH, float x, float y, float z, float w, float h, float d, float rx, float ry, float rz);

	//! <code>System.SetGammaDelta( fDelta )</code>
	//!		<param name="fDelta">Delta value.</param>
	//! <description>Sets the gamma/delta value.</description>
	i32 SetGammaDelta(IFunctionHandler* pH);

	//! <code>System.SetPostProcessFxParam( pszEffectParam, value )</code>
	//!		<param name="pszEffectParam">Parameter for the post processing effect.</param>
	//!		<param name="value">Value for the parameter.</param>
	//! <description>Sets a post processing effect parameter value.</description>
	i32 SetPostProcessFxParam(IFunctionHandler* pH);

	//! <code>System.GetPostProcessFxParam( pszEffectParam, value )</code>
	//!		<param name="pszEffectParam">Parameter for the post processing effect.</param>
	//!		<param name="value">Value for the parameter.</param>
	//! <description>Gets a post processing effect parameter value.</description>
	i32 GetPostProcessFxParam(IFunctionHandler* pH);

	//! <code>System.SetScreenFx( pszEffectParam, value )</code>
	//!		<param name="pszEffectParam">Parameter for the post processing effect.</param>
	//!		<param name="value">Value for the parameter.</param>
	//! <description>Sets a post processing effect parameter value.</description>
	i32 SetScreenFx(IFunctionHandler* pH);

	//! <code>System.GetScreenFx( pszEffectParam, value )</code>
	//!		<param name="pszEffectParam">Parameter for the post processing effect.</param>
	//!		<param name="value">Value for the parameter.</param>
	//! <description>Gets a post processing effect parameter value.</description>
	i32 GetScreenFx(IFunctionHandler* pH);

	//! <code>System.SetCVar( sCVarName, value )</code>
	//!		<param name="sCVarName">Name of the variable.</param>
	//!		<param name="value">Value of the variable.</param>
	//! <description>Sets the value of a CVariable.</description>
	i32 SetCVar(IFunctionHandler* pH);

	//! <code>System.GetCVar( sCVarName)</code>
	//!		<param name="sCVarName">Name of the variable.</param>
	//! <description>Gets the value of a CVariable.</description>
	i32 GetCVar(IFunctionHandler* pH);

	//! <code>System.AddCCommand( sCCommandName, sCommand, sHelp)</code>
	//!		<param name="sCCommandName">C command name.</param>
	//!		<param name="sCommand">Command string.</param>
	//!		<param name="sHelp">Help for the command usage.</param>
	//! <description>Adds a C command to the system.</description>
	i32 AddCCommand(IFunctionHandler* pH);

	//! <code>System.SetScissor( x, y, w, h )</code>
	//!		<param name="x">X position.</param>
	//!		<param name="y -	Y position.</param>
	//!		<param name="w">Width size.</param>
	//!		<param name="h">Height size.</param>
	//! <description>Sets scissor info.</description>
	i32 SetScissor(IFunctionHandler* pH);

	//! <code>System.GetSystemMem()</code>
	//! <description>Gets the amount of the memory for the system.</description>
	i32 GetSystemMem(IFunctionHandler* pH);

	//! <code>System.IsPS20Supported()</code>
	//! <description>Checks if the PS20 is supported.</description>
	i32 IsPS20Supported(IFunctionHandler* pH);

	//! <code>System.IsHDRSupported()</code>
	//! <description>Checks if the HDR is supported.</description>
	i32 IsHDRSupported(IFunctionHandler* pH);

	//! <code>System.SetBudget(sysMemLimitInMB, videoMemLimitInMB, frameTimeLimitInMS, soundChannelsPlayingLimit, soundMemLimitInMB, numDrawCallsLimit )</code>
	//!		<param name="sysMemLimitInMB">Limit of the system memory in MB.</param>
	//!		<param name="videoMemLimitInMB">Limit of the video memory in MB.</param>
	//!		<param name="frameTimeLimitInMS">Limit in the frame time in MS.</param>
	//!		<param name="soundChannelsPlayingLimit">Limit of the sound channels playing.</param>
	//!		<param name="soundMemLimitInMB">Limit of the sound memory in MB.</param>
	//!		<param name="numDrawCallsLimit">Limit of the draw calls.</param>
	//! <description>Sets system budget.</description>
	i32 SetBudget(IFunctionHandler* pH);

	//! <code>System.SetVolumetricFogModifiers( gobalDensityModifier, atmosphereHeightModifier )</code>
	//!		<param name="gobalDensityModifier">Modifier for the global density.</param>
	//!		<param name="atmosphereHeightModifier">Modifier for the atmosphere height.</param>
	//! <description>Sets the volumetric fog modifiers.</description>
	i32 SetVolumetricFogModifiers(IFunctionHandler* pH);

	//! <code>System.SetWind( vWind )</code>
	//!		<param name="vWind">Wind direction.</param>
	//! <description>Sets the wind direction.</description>
	i32 SetWind(IFunctionHandler* pH);

	//! <code>System.SetWind()</code>
	//! <description>Gets the wind direction.</description>
	i32 GetWind(IFunctionHandler* pH);

	//! <code>System.GetSurfaceTypeIdByName( surfaceName )</code>
	//!		<param name="surfaceName">Surface name.</param>
	//! <description>Gets the surface type identifier by its name.</description>
	i32 GetSurfaceTypeIdByName(IFunctionHandler* pH, tukk surfaceName);

	//! <code>System.GetSurfaceTypeNameById( surfaceId )</code>
	//!		<param name="surfaceId">Surface identifier.</param>
	//! <description>Gets the surface type name by its identifier.</description>
	i32 GetSurfaceTypeNameById(IFunctionHandler* pH, i32 surfaceId);

	//! <code>System.RemoveEntity( entityId )</code>
	//!		<param name="entityId">Entity identifier.</param>
	//! <description>Removes the specified entity.</description>
	i32 RemoveEntity(IFunctionHandler* pH, ScriptHandle entityId);

	//! <code>System.SpawnEntity( params )</code>
	//!		<param name="params">Entity parameters.</param>
	//! <description>Spawns an entity.</description>
	i32 SpawnEntity(IFunctionHandler* pH, SmartScriptTable params);

	//DOC-IGNORE-BEGIN
	//! <code>System.ActivateLight(name, activate)</code>
	//! <description>NOT SUPPORTED ANYMORE.</description>
	i32 ActivateLight(IFunctionHandler* pH);
	//DOC-IGNORE-END

	//	i32 ActivateMainLight(IFunctionHandler *pH); //pos, activate
	//	i32 SetSkyBox(IFunctionHandler *pH); //szShaderName, fBlendTime, bUseWorldBrAndColor

	//! <code>System.SetWaterVolumeOffset()</code>
	//! <description>SetWaterLevel is not supported by 3dengine for now.</description>
	i32 SetWaterVolumeOffset(IFunctionHandler* pH);

	//! <code>System.IsValidMapPos( v )</code>
	//!		<param name="v">Position vector.</param>
	//! <description>Checks if the position is a valid map position.</description>
	i32 IsValidMapPos(IFunctionHandler* pH);

	//DOC-IGNORE-BEGIN
	//! <code>System.EnableMainView()</code>
	//! <description>Feature unimplemented.</description>
	i32 EnableMainView(IFunctionHandler* pH);
	//DOC-IGNORE-END

	//! <code>System.EnableOceanRendering()</code>
	//!		<param name="bOcean">True to activate the ocean rendering, false to deactivate it.</param>
	//! <description>Enables/disables ocean rendering.</description>
	i32 EnableOceanRendering(IFunctionHandler* pH);

	//! <code>System.ScanDirectory( pszFolderName, nScanMode )</code>
	//!		<param name="pszFolderName">Folder name.</param>
	//!		<param name="nScanMode">Scan mode for the folder. Can be:
	//!				SCANDIR_ALL
	//!				SCANDIR_FILES
	//!				SCANDIR_SUBDIRS</param>
	//! <description>Scans a directory.</description>
	i32 ScanDirectory(IFunctionHandler* pH);

	//! <code>System.DebugStats( cp )</code>
	i32 DebugStats(IFunctionHandler* pH);

	//! <code>System.ViewDistanceSet( fViewDist )</code>
	//!		<param name="fViewDist">View distance.</param>
	//! <description>Sets the view distance.</description>
	i32 ViewDistanceSet(IFunctionHandler* pH);

	//! <code>System.ViewDistanceSet()</code>
	//! <description>Gets the view distance.</description>
	i32 ViewDistanceGet(IFunctionHandler* pH);

	//! <code>System.GetOutdoorAmbientColor()</code>
	//! <description>Gets the outdoor ambient color.</description>
	i32 GetOutdoorAmbientColor(IFunctionHandler* pH);

	//! <code>System.GetOutdoorAmbientColor( v3Color )</code>
	//!		<param name="v3Color">Outdoor ambient color value.</param>
	//! <description>Sets the outdoor ambient color.</description>
	i32 SetOutdoorAmbientColor(IFunctionHandler* pH);

	//! <code>System.GetTerrainElevation( v3Pos )</code>
	//!		<param name="v3Pos">Position of the terraint to be checked.</param>
	//! <description>Gets the terrain elevation of the specified position.</description>
	i32 GetTerrainElevation(IFunctionHandler* pH);
	//	i32 SetIndoorColor(IFunctionHandler *pH);

	//! <code>System.ActivatePortal( vPos, bActivate, nID )</code>
	//!		<param name="vPos">Position vector.</param>
	//!		<param name="bActivate">True to activate the portal, false to deactivate.</param>
	//!		<param name="nID">Entity identifier.</param>
	//! <description>Activates/deactivates a portal.</description>
	i32 ActivatePortal(IFunctionHandler* pH);

	//! <code>System.DumpMMStats()</code>
	//! <description>Dumps the MM statistics.</description>
	i32 DumpMMStats(IFunctionHandler* pH);

	//! <code>System.EnumAAFormats( m_Width, m_Height, m_BPP )</code>
	//! <description>Enumerates the AA formats.</description>
	i32 EnumAAFormats(IFunctionHandler* pH);

	//! <code>System.EnumDisplayFormats()</code>
	//! <description>Enumerates display formats.</description>
	i32 EnumDisplayFormats(IFunctionHandler* pH);

	//! <code>System.IsPointIndoors( vPos )</code>
	//!		<param name="vPos">Position vector.</param>
	//! <description>Checks if a point is indoors.</description>
	i32 IsPointIndoors(IFunctionHandler* pH);

	//! <code>System.SetConsoleImage( pszName, bRemoveCurrent )</code>
	//!		<param name="pszName">Texture image.</param>
	//!		<param name="bRemoveCurrent">True to remove the current image, false otherwise.</param>
	//! <description>Sets the console image.</description>
	i32 SetConsoleImage(IFunctionHandler* pH);

	//! <code>System.ProjectToScreen( vec )</code>
	//!		<param name="vec">Position vector.</param>
	//! <description>Projects to the screen (not guaranteed to work if used outside Renderer).</description>
	i32 ProjectToScreen(IFunctionHandler* pH, Vec3 vec);

	//DOC-IGNORE-BEGIN
	//! <code>System.EnableHeatVision()</code>
	//! <description>Is not supported anymore.</description>
	i32 EnableHeatVision(IFunctionHandler* pH);
	//DOC-IGNORE-END

	//! <code>System.ShowDebugger()</code>
	//! <description>Shows the debugger.</description>
	i32 ShowDebugger(IFunctionHandler* pH);

	//! <code>System.DumpMemStats( bUseKB )</code>
	//!		<param name="bUseKB">True to use KB, false otherwise.</param>
	//! <description>Dumps memory statistics.</description>
	i32 DumpMemStats(IFunctionHandler* pH);

	//! <code>System.DumpMemoryCoverage( bUseKB )</code>
	//! <description>Dumps memory coverage.</description>
	i32 DumpMemoryCoverage(IFunctionHandler* pH);

	//! <code>System.ApplicationTest( pszParam )</code>
	//!		<param name="pszParam">Parameters.</param>
	//! <description>Test the application with the specified parameters.</description>
	i32 ApplicationTest(IFunctionHandler* pH);

	//! <code>System.QuitInNSeconds( fInNSeconds )</code>
	//!		<param name="fInNSeconds">Number of seconds before quitting.</param>
	//! <description>Quits the application in the specified number of seconds.</description>
	i32 QuitInNSeconds(IFunctionHandler* pH);

	//! <code>System.DumpWinHeaps()</code>
	//! <description>Dumps windows heaps.</description>
	i32 DumpWinHeaps(IFunctionHandler* pH);

	//! <code>System.Break()</code>
	//! <description>Breaks the application with a fatal error message.</description>
	i32 Break(IFunctionHandler* pH);

	//! <code>System.SetViewCameraFov( fov )</code>
	//! <description>Sets the view camera fov.</description>
	i32 SetViewCameraFov(IFunctionHandler* pH, float fov);

	//! <code>System.GetViewCameraFov()</code>
	//! <description>Gets the view camera fov.</description>
	i32 GetViewCameraFov(IFunctionHandler* pH);

	//! <code>System.IsPointVisible( point )</code>
	//!		<param name="point">Point vector.</param>
	//! <description>Checks if the specified point is visible.</description>
	i32 IsPointVisible(IFunctionHandler* pH, Vec3 point);

	//! <code>System.GetViewCameraPos()</code>
	//! <description>Gets the view camera position.</description>
	i32 GetViewCameraPos(IFunctionHandler* pH);

	//! <code>System.GetViewCameraDir()</code>
	//! <description>Gets the view camera direction.</description>
	i32 GetViewCameraDir(IFunctionHandler* pH);

	//! <code>System.GetViewCameraUpDir()</code>
	//! <description>Gets the view camera up-direction.</description>
	i32 GetViewCameraUpDir(IFunctionHandler* pH);

	//! <code>System.GetViewCameraAngles()</code>
	//! <description>Gets the view camera angles.</description>
	i32 GetViewCameraAngles(IFunctionHandler* pH);

	//! <code>System.RayWorldIntersection(vPos, vDir, nMaxHits, iEntTypes)</code>
	//!		<param name="vPos">Position vector.</param>
	//!		<param name="vDir">Direction vector.</param>
	//!		<param name="nMaxHits">Maximum number of hits.</param>
	//!		<param name="iEntTypes">.</param>
	//! <description>Shots rays into the world.</description>
	i32 RayWorldIntersection(IFunctionHandler* pH);

	//! <code>System.RayTraceCheck(src, dst, skipId1, skipId2)</code>
	i32 RayTraceCheck(IFunctionHandler* pH);

	//! <code>System.BrowseURL(szURL)</code>
	//!		<param name="szURL">URL string.</param>
	//! <description>Browses a URL address.</description>
	i32 BrowseURL(IFunctionHandler* pH);

	//! <code>System.IsDevModeEnable()</code>
	//! <description>
	//!		Checks if game is running in dev mode (cheat mode)
	//!		to check if we are allowed to enable certain scripts
	//!		function facilities (god mode, fly mode etc.).
	//! </description>
	i32 IsDevModeEnable(IFunctionHandler* pH);

	//! <code>System.SaveConfiguration()</code>
	//! <description>Saves the configuration.</description>
	i32 SaveConfiguration(IFunctionHandler* pH);

	//! <code>System.Quit()</code>
	//! <description>Quits the program.</description>
	i32 Quit(IFunctionHandler* pH);

	//! <code>System.GetHDRDynamicMultiplier()</code>
	//! <description>Gets the HDR dynamic multiplier.</description>
	i32 GetHDRDynamicMultiplier(IFunctionHandler* pH);

	//! <code>System.SetHDRDynamicMultiplier( fMul )</code>
	//!		<param name="fMul">Dynamic multiplier value.</param>
	//! <description>Sets the HDR dynamic multiplier.</description>
	i32 SetHDRDynamicMultiplier(IFunctionHandler* pH);

	//! <code>System.GetFrameID()</code>
	//! <description>Gets the frame identifier.</description>
	i32 GetFrameID(IFunctionHandler* pH);

	//! <code>System.ClearKeyState()</code>
	//! <description>Clear the key state.</description>
	i32 ClearKeyState(IFunctionHandler* pH);

	//! <code>System.SetSunColor( vColor )</code>
	//! <description>Set color of the sun, only relevant outdoors.</description>
	//!		<param name="vColor">Sun Color as an {x,y,z} vector (x=r,y=g,z=b).</param>
	i32 SetSunColor(IFunctionHandler* pH, Vec3 vColor);

	//! <code>Vec3 System.GetSunColor()</code>
	//! <description>Retrieve color of the sun outdoors.</description>
	//! <returns>Sun Color as an {x,y,z} vector (x=r,y=g,z=b).</returns>
	i32 GetSunColor(IFunctionHandler* pH);

	//! <code>System.SetSkyColor( vColor )</code>
	//! <description>Set color of the sky (outdoors ambient color).</description>
	//!		<param name="vColor">Sky Color as an {x,y,z} vector (x=r,y=g,z=b).</param>
	i32 SetSkyColor(IFunctionHandler* pH, Vec3 vColor);

	//! <code>Vec3 System.GetSkyColor()</code>
	//! <description>Retrieve color of the sky (outdoor ambient color).</description>
	//! <returns>Sky Color as an {x,y,z} vector (x=r,y=g,z=b).</returns>
	i32 GetSkyColor(IFunctionHandler* pH);

	//! <code>System.SetSkyHighlight( params )</code>
	//! <description>Set Sky highlighing parameters.</description>
	//! <seealso cref="GetSkyHighlight">
	//!		<param name="params">Table with Sky highlighing parameters.
	//!			<para>
	//!				Highligh Params     Meaning
	//!				-------------       -----------
	//!				size                Sky highlight scale.
	//!				color               Sky highlight color.
	//!				direction           Direction of the sky highlight in world space.
	//!				pod                 Position of the sky highlight in world space.
	//!			 </para></param>
	i32 SetSkyHighlight(IFunctionHandler* pH, SmartScriptTable params);

	//! <code>System.SetSkyHighlight( params )</code>
	//! <description>
	//!		Retrieves Sky highlighing parameters.
	//!		see SetSkyHighlight for parameters description.
	//! </description>
	//! <seealso cref="SetSkyHighlight">
	i32 GetSkyHighlight(IFunctionHandler* pH, SmartScriptTable params);

	//! <code>System.LoadLocalizationXml( filename )</code>
	//! <description>Loads Excel exported xml file with text and dialog localization data.</description>
	i32 LoadLocalizationXml(IFunctionHandler* pH, tukk filename);

private:
	void MergeTable(IScriptTable* pDest, IScriptTable* pSrc);
	//log a string to console and/or to disk with support for different languages
	void LogString(IFunctionHandler* pH, bool bToConsoleOnly);
	i32  DeformTerrainInternal(IFunctionHandler* pH, bool nameIsMaterial);

private:
	ISystem*   m_pSystem;
	ILog*      m_pLog;
	IRenderer* m_pRenderer;
	IConsole*  m_pConsole;
	ITimer*    m_pTimer;
	I3DEngine* m_p3DEngine;

	//Vlad is too lazy to add this to 3DEngine">so I have to put it here. It should
	//not be here, but I have to put it somewhere.....
	float            m_SkyFadeStart; // when fogEnd less">start to fade sky to fog
	float            m_SkyFadeEnd;   // when fogEnd less">no sky, just fog

	SmartScriptTable m_pScriptTimeTable;
	SmartScriptTable m_pGetEntitiesCacheTable;
};

#endif // __ScriptBind_System_h__
