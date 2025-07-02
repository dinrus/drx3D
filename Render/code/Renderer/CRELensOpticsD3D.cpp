// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/RendElements/OpticsElement.h>
#include <drx3D/Render/RendElements/RootOpticsElement.h>
#include <drx3D/Render/RendElements/FlareSoftOcclusionQuery.h>
#include <drx3D/Render/RendElements/OpticsPredef.hpp>
#include <drx3D/Render/DriverD3D.h>
#include <drx3D/Eng3D/I3DEngine.h>

#include <drx3D/Render/D3D/GraphicsPipeline/LensOptics.h>

CRELensOptics::CRELensOptics(void)
{
	mfSetType(eDATA_LensOptics);
	mfUpdateFlags(FCEF_TRANSFORM);
}
