// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Sys/IEngineModule.h>
#include <memory>

class IDrxSizer;
struct IFlashPlayer;
struct IFlashPlayerBootStrapper;
struct IFlashLoadMovieHandler;

struct IScaleformHelperEngineModule : public Drx::IDefaultModule
{
	DRXINTERFACE_DECLARE_GUID(IScaleformHelperEngineModule, "3b0e8940-4ac6-4cbb-aa3f-7e13ecb9f871"_drx_guid);
};

//! Helper for Scaleform-specific function access
struct IScaleformHelper
{
public:
	//! Initialize helper object
	virtual bool Init() = 0;

	//! Destroy helper object
	virtual void Destroy() = 0;

	//! Enables or disables AMP support.
	//! Note: This function does nothing if AMP feature is not compiled in.
	virtual void SetAmpEnabled(bool bEnabled) = 0;

	//! Advance AMP by one frame.
	//! Note: This function does nothing if AMP feature is not compiled in.
	virtual void AmpAdvanceFrame() = 0;

	//! Sets the word-wrapping mode for the specified language.
	virtual void SetTranslatorWordWrappingMode(tukk szLanguage) = 0;

	//! Mark translator as dirty, causing a refresh of localized strings.
	virtual void SetTranslatorDirty(bool bDirty) = 0;

	//! Reset all cached meshes
	virtual void ResetMeshCache() = 0;

	//! Create new instance of flash player
	virtual std::shared_ptr<IFlashPlayer> CreateFlashPlayerInstance() = 0;

	//! Create new instance of flash player bootstrapper
	virtual IFlashPlayerBootStrapper* CreateFlashPlayerBootStrapper() = 0;

	//! Displays flash information usign AUX renderer
	virtual void RenderFlashInfo() = 0;

	//! Query memory usage of all flash player instances
	virtual void GetFlashMemoryUsage(IDrxSizer* pSizer) = 0;

	//! Set handler for call-backs of flash loading
	virtual void SetFlashLoadMovieHandler(IFlashLoadMovieHandler* pHandler) = 0;

	//! Query the total time spent in flash updating
	virtual float GetFlashProfileResults() = 0;

	//! Query the number of draws and triangles used to render flash content this frame
	virtual void GetFlashRenderStats(unsigned& numDPs, u32& numTris) = 0;

	//! Set thread ID values for Scaleform
	virtual void SetRenderThreadIDs(threadID main, threadID render) = 0;
};
