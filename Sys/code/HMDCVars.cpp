// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/HMDCVars.h>
#include<drx3D/Sys/IHMDDevice.h>

namespace DrxVR
{

i32 CVars::hmd_info = 0;
i32 CVars::hmd_social_screen = static_cast<i32>(EHmdSocialScreen::DistortedDualImage);
i32 CVars::hmd_social_screen_aspect_mode = static_cast<i32>(EHmdSocialScreenAspectMode::Fill);
i32 CVars::hmd_post_inject_camera = 1;
i32 CVars::hmd_tracking_origin = static_cast<i32>(EHmdTrackingOrigin::Seated);
float CVars::hmd_resolution_scale = 1.f;
ICVar* CVars::pSelectedHmdNameVar = nullptr;

}
