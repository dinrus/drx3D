// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
//
// Описание: EyeTracker for Windows (covering EyeX)
// - 20/04/2016 Created by Benjamin Peters
// Copyright 2001-2016 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Input/IInput.h>

#ifdef DRX_PLATFORM_WINDOWS
#ifdef USE_EYETRACKER

class CEyeTrackerInput : public IEyeTrackerInput
{
public:
	CEyeTrackerInput();
	~CEyeTrackerInput();

	virtual bool        Init();
	virtual void        Update();

	void                SetEyeProjection(i32 iX, i32 iY);

private:
	bool    m_bProjectionChanged;
	float   m_fEyeX;
	float   m_fEyeY;
};

#endif //USE_EYETRACKER
#endif //DRX_PLATFORM_WINDOWS
