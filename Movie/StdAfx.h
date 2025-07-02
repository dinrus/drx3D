// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__0B7BFEE0_95B3_4DD3_956A_33AD2ADB212D__INCLUDED_)
#define AFX_STDAFX_H__0B7BFEE0_95B3_4DD3_956A_33AD2ADB212D__INCLUDED_

#if _MSC_VER > 1000
	#pragma once
#endif // _MSC_VER > 1000

#include <drx3D/CoreX/Project/DrxModuleDefs.h>
#define eDrxModule eDrxM_Movie

#define DRXMOVIE_EXPORTS

#include <drx3D/CoreX/Platform/platform.h>

#include <vector>
#include <list>
#include <map>
#include <algorithm>

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/Sys/IXml.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Entity/IEntitySystem.h>
#include <drx3D/Movie/IMovieSystem.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>

#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/CoreX/Serialization/IntrusiveFactory.h>
#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/CoreX/Serialization/Color.h>
#include <drx3D/CoreX/Serialization/Math.h>
#include <drx3D/CoreX/Serialization/SmartPtr.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0B7BFEE0_95B3_4DD3_956A_33AD2ADB212D__INCLUDED_)
