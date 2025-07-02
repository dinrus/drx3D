// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>
#include <drx3D/CoreX/Math/Drx_Camera.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/CoreX/Serialization/Forward.h>

struct SRenderContext;
struct AnimEventInstance;

namespace CharacterTool
{
class CharacterDocument;
using Serialization::IArchive;

struct IFeatureTest : public _i_reference_target_t
{
	virtual void Update(CharacterDocument* document)                                    {}
	// Tell if regular Character Tool update should be skipped
	virtual bool OverrideUpdate() const                                                 { return false; }
	virtual bool OverrideCameraPosition(CCamera* camera)                                { return false; }

	virtual void Render(const SRenderContext& x, CharacterDocument* document)           {}
	virtual bool AnimEvent(const AnimEventInstance& event, CharacterDocument* document) { return false; }
	virtual void Serialize(IArchive& ar)                                                {}
};

}

