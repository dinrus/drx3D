// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Render/StdAfx.h>
#include <drx3D/Render/OpticsFactory.h>

#include <drx3D/Render/RendElements/RootOpticsElement.h>
#include <drx3D/Render/RendElements/OpticsElement.h>
#include <drx3D/Render/Ghost.h>
#include <drx3D/Render/Glow.h>
#include <drx3D/Render/ChromaticRing.h>
#include <drx3D/Render/RendElements/CameraOrbs.h>
#include <drx3D/Render/IrisShafts.h>
#include <drx3D/Render/Streaks.h>
#include <drx3D/Render/ImageSpaceShafts.h>
#include <drx3D/Render/OpticsReference.h>
#include <drx3D/Render/OpticsProxy.h>
#include <drx3D/Render/OpticsPredef.hpp>

IOpticsElementBase* COpticsFactory::Create(EFlareType type) const
{
	switch (type)
	{
	case eFT_Root:
		return new RootOpticsElement;
	case eFT_Group:
		return new COpticsGroup("[Group]");
	case eFT_Ghost:
		return new CLensGhost("Ghost");
	case eFT_MultiGhosts:
		return new CMultipleGhost("Multi Ghost");
	case eFT_Glow:
		return new Glow("Glow");
	case eFT_IrisShafts:
		return new IrisShafts("Iris Shafts");
	case eFT_ChromaticRing:
		return new ChromaticRing("Chromatic Ring");
	case eFT_CameraOrbs:
		return new CameraOrbs("Orbs");
	case eFT_ImageSpaceShafts:
		return new ImageSpaceShafts("Vol Shafts");
	case eFT_Streaks:
		return new Streaks("Streaks");
	case eFT_Reference:
		return new COpticsReference("Reference");
	case eFT_Proxy:
		return new COpticsProxy("Proxy");
	default:
		return NULL;
	}
}
