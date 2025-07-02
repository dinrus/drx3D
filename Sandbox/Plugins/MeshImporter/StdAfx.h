// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.
// Precompiled header.
#pragma once

#include <drx3D/CoreX/Platform/platform.h>

#define DRX_USE_MFC
#include <drx3D/CoreX/Platform/DrxAtlMfc.h> // Required to use QtViewPort

#include <drx3D/CoreX/Math/Drx_Math.h>
#include <drx3D/CoreX/Renderer/IRenderer.h>
#include <drx3D/CoreX/Renderer/IShader.h>

#include <IEditor.h>
#include "SandboxAPI.h"
#include <drx3D/Sandbox/Editor/Plugin/EditorCommonAPI.h>
#include "Material/Material.h"

// TODO: Make sure this project compiles without this include.
#include "FbxMetaData.h"
#include "FbxScene.h"

// don't allow implicit string conversions in this project
// NOTE: This does not play well with widgets from EditorCommon. So we disable this option for now.
// #define QT_NO_CAST_FROM_ASCII

// Qt common headers
#include <QObject>
#include <QString>
#include <QList>
#include "QtUtil.h"

// std::unique_ptr and _smart_ptr conversions
#include <drx3D/CoreX/smartptr.h>
#include <memory>

// Put these headers here, so we don't need to modify files from Drx3DEngine.
#include <drx3D/CoreX/Math/Drx_Geo.h>    // Used by MeshCompiler.cpp.
#include <drx3D/CoreX/Memory/DrxSizer.h> // Used by ChunkFile.h.

#define TSmartPtr _smart_ptr

#define MESH_IMPORTER_NAME "Mesh Importer"

// Global lock
namespace Detail
{
extern DrxCriticalSection g_lock;

inline bool AcquireGlobalLock()
{
	g_lock.Lock();
	return true;
}

inline bool TryAcquireGlobalLock()
{
	return g_lock.TryLock();
}

inline void ReleaseGlobalLock()
{
	g_lock.Unlock();
}
}

// Helper for accessing global state
class CScopedGlobalLock
{
public:
	CScopedGlobalLock()
		: m_bLocked(Detail::AcquireGlobalLock()) {}

	CScopedGlobalLock(const bool bTry)
		: m_bLocked(bTry && Detail::TryAcquireGlobalLock()) {}

	~CScopedGlobalLock()
	{
		if (m_bLocked)
		{
			Detail::ReleaseGlobalLock();
		}
	}

	CScopedGlobalLock(CScopedGlobalLock&& other)
		: m_bLocked(other.m_bLocked)
	{
		other.m_bLocked = false;
	}

	CScopedGlobalLock& operator=(CScopedGlobalLock&& other)
	{
		assert(&other != this);
		if (m_bLocked)
		{
			Detail::ReleaseGlobalLock();
		}
		m_bLocked = other.m_bLocked;
		other.m_bLocked = false;
		return *this;
	}

private:
	// no copy/assign
	CScopedGlobalLock(const CScopedGlobalLock&);
	CScopedGlobalLock& operator=(const CScopedGlobalLock&);

	// set if lock owned
	bool m_bLocked;
};

namespace Detail
{
struct DeleteUsingRelease
{
	template<typename T>
	void operator()(T* pObject) const
	{
		pObject->Release();
	}
};
}

// Helper to create std::unique_ptr that calls "Release" member instead of "delete".
// This way we can avoid _smart_ptr (which can be copied, but not moved) conflicts with std::unique_ptr (which can be moved, but not copied).
// By having a class/struct with only one of those allows us to avoid having to write copy or move constructors and/or assignment operators.
template<typename T>
inline std::unique_ptr<T, Detail::DeleteUsingRelease> MakeUniqueUsingRelease(T* pObject)
{
	return std::unique_ptr<T, Detail::DeleteUsingRelease>(pObject);
}

// Converts one "instance" of shared-ownership from _smart_ptr to an unique_ptr.
template<typename T>
inline std::unique_ptr<T, Detail::DeleteUsingRelease> MakeUniqueUsingRelease(_smart_ptr<T>&& pSmart)
{
	return MakeUniqueUsingRelease(ReleaseOwnership(pSmart));
}

// Converts an unique_ptr to a _smart_ptr (counting as one "instance" of shared-ownership)
template<typename T>
inline _smart_ptr<T> MakeSmartFromUnique(std::unique_ptr<T, Detail::DeleteUsingRelease>&& pUnique)
{
	// Note: There is no _smart_ptr API that allows us to construct without calling AddRef
	_smart_ptr<T> pSmart(pUnique.get()); // Calls AddRef()
	pUnique.reset();                     // Calls Release()
	return pSmart;
}

