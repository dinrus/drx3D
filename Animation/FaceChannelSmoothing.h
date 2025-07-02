// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CFacialAnimChannelInterpolator;

namespace FaceChannel
{
void GaussianBlurKeys(CFacialAnimChannelInterpolator* pSpline, float sigma);
void RemoveNoise(CFacialAnimChannelInterpolator* pSpline, float sigma, float threshold);
}
