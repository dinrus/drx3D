// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Render/RendElements/OpticsElement.h>
#include <drx3D/Render/RendElements/RootOpticsElement.h>
#include <drx3D/Render/Ghost.h>
#include <drx3D/Render/Glow.h>
#include <drx3D/Render/ChromaticRing.h>
#include <drx3D/Render/IrisShafts.h>
#include <drx3D/Render/Streaks.h>
#include <drx3D/Render/RendElements/CameraOrbs.h>

class OpticsPredef
{
public:
	COpticsGroup PREDEF_MULTIGLASS_GHOST;

private:
	void InitPredef()
	{
		static CTexture* s_pCenterFlare = CTexture::ForName("%ENGINE%/EngineAssets/Textures/flares/lens_flare1-wide.tif", FT_DONT_RELEASE | FT_DONT_STREAM, eTF_Unknown);

		PREDEF_MULTIGLASS_GHOST.SetName("[Multi-glass Reflection]");

		Glow* rotStreak = new Glow("RotatingStreak");
		rotStreak->SetSize(0.28f);
		rotStreak->SetAutoRotation(true);
		rotStreak->SetFocusFactor(-0.18f);
		rotStreak->SetBrightness(6.f);
		rotStreak->SetScale(Vec2(0.034f, 25.f));
		PREDEF_MULTIGLASS_GHOST.Add(rotStreak);

		CameraOrbs* orbs = new CameraOrbs("Orbs");
		orbs->SetIllumRange(1.2f);
		orbs->SetUseLensTex(true);
		PREDEF_MULTIGLASS_GHOST.Add(orbs);

		ChromaticRing* ring = new ChromaticRing("Forward Ring");
		PREDEF_MULTIGLASS_GHOST.Add(ring);

		ChromaticRing* backRing = new ChromaticRing("Backward Ring");
		backRing->SetCompletionFading(10.f);
		backRing->SetCompletionSpanAngle(25.f);
		PREDEF_MULTIGLASS_GHOST.Add(backRing);

		CLensGhost* centerCorona = new CLensGhost("Center Corona");
		centerCorona->SetTexture(s_pCenterFlare);
		centerCorona->SetSize(0.6f);
		PREDEF_MULTIGLASS_GHOST.Add(centerCorona);
	}
	OpticsPredef()
	{
		InitPredef();
	}

	OpticsPredef(OpticsPredef const& copy);
	void operator=(OpticsPredef const& copy);

public:
	static OpticsPredef* GetInstance()
	{
		static OpticsPredef instance;
		return &instance;
	}
};
