// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _INTROMOVIERENDERER_H_
#define _INTROMOVIERENDERER_H_

#include <drx3D/Sys/IFlashPlayer.h>

class CIntroMovieRenderer : public ILoadtimeCallback, public IFSCommandHandler
{
protected:

	enum EVideoStatus
	{
		eVideoStatus_PrePlaying = 0,
		eVideoStatus_Playing    = 1,
		eVideoStatus_Stopped    = 2,
		eVideoStatus_Finished   = 3,
		eVideoStatus_Error      = 4,
	};

public:

	CIntroMovieRenderer() = default;
	virtual ~CIntroMovieRenderer() = default;

	bool Initialize();
	void WaitForCompletion();

	// ILoadtimeCallback
	virtual void LoadtimeUpdate(float deltaTime);
	virtual void LoadtimeRender();
	// ~ILoadtimeCallback

	// IFSCommandHandler
	virtual void HandleFSCommand(tukk pCommand, tukk pArgs, uk pUserData = 0) {}
	// ~IFSCommandHandler

protected:

	void         UpdateViewport();
	void         SetViewportIfChanged(i32k x, i32k y, i32k width, i32k height, const float pixelAR);
	i32          GetSubtitleChannelForSystemLanguage();

	EVideoStatus GetCurrentStatus();

	//////////////////////////////////////////////////////////////////////////

	std::shared_ptr<IFlashPlayer> m_pFlashPlayer;

};

#endif // _INTROMOVIERENDERER_H_
