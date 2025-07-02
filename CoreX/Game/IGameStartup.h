// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/CoreX/Game/IGame.h> // <> required for Interfuscator
#include "IGameRef.h"

struct ILog;
struct ILogCallback;
struct IValidator;
struct ISystemUserCallback;

//! Interfaces used to initialize a legacy game using DRXENGINE.
struct DRX_DEPRECATED_GAME_DLL IGameStartup
{
	//! Entry function used to create a new instance of the game.
	//! This is considered deprecated, in favor of Drx::IEnginePlugin, and will be removed in the future
	//! \return A new instance of the game startup.
	typedef IGameStartup*(* TEntryFunction)();

	// <interfuscator:shuffle>
	DRX_DEPRECATED_GAME_DLL IGameStartup() = default;
	virtual ~IGameStartup(){}

	//! Initialize the game and/or any MOD, and get the IGameMod interface.
	//! The shutdown method, must be called independent of this method's return value.
	//! \param startupParams Pointer to SSysInitParams structure containing system initialization setup!
	//! \return Pointer to a IGameMod interface, or 0 if something went wrong with initialization.
	virtual IGameRef Init(SSysInitParams& startupParams) = 0;

	//! Shuts down the game and any loaded MOD and delete itself.
	virtual void Shutdown() = 0;

	//! Returns the RSA Public Key used by the engine to decrypt pak files which are encrypted by an offline tool.
	//! Part of the tools package includes a key generator, which will generate a header file with the public key which can be easily included in code.
	//! This *should* ideally be in IGame being game specific info, but paks are loaded very early on during boot and this data is needed to init DrxPak.
	virtual u8k* GetRSAKey(u32* pKeySize) const {* pKeySize = 0; return NULL; }
	// </interfuscator:shuffle>
};

//! \endcond