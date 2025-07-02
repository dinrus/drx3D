// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   drx3dengine.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:    Определяет точку входа в DLL, реализует доступ к другим модулям.
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#include <drx3D/Eng3D/StdAfx.h>

// Must be included only once in DLL module.
#include <drx3D/CoreX/Platform/platform_impl.inl>

#include <drx3D/Eng3D/3dEngine.h>
#include <drx3D/Eng3D/MatMan.h>

#include <drx3D/Sys/IEngineModule.h>
#include <drx3D/CoreX/Extension/IDrxFactory.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#define MAX_ERROR_STRING MAX_WARNING_LENGTH

// Disable printf argument verification since it is generated at runtime
#if defined(__GNUC__)
	#if __GNUC__ >= 4 && __GNUC__MINOR__ < 7
		#pragma GCC diagnostic ignored "-Wformat-security"
	#else
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wformat-security"
	#endif
#endif
//////////////////////////////////////////////////////////////////////

struct CSystemEventListener_3DEngine : public ISystemEventListener
{
public:
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
	{
		switch (event)
		{
		case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
			{
				if (DinrusX3dEngBase::Get3DEngine())
					DinrusX3dEngBase::Get3DEngine()->ClearDebugFPSInfo(true);
				break;
			}
		case ESYSTEM_EVENT_LEVEL_LOAD_END:
			{
				if (DinrusX3dEngBase::Get3DEngine())
					DinrusX3dEngBase::Get3DEngine()->ClearDebugFPSInfo();
				if (DinrusX3dEngBase::GetObjUpr())
					DinrusX3dEngBase::GetObjUpr()->FreeNotUsedCGFs();
				DinrusX3dEngBase::m_bLevelLoadingInProgress = false;
				break;
			}
		case ESYSTEM_EVENT_LEVEL_LOAD_START:
			{
				DinrusX3dEngBase::m_bLevelLoadingInProgress = true;
				break;
			}
		case ESYSTEM_EVENT_LEVEL_UNLOAD:
			{
				DinrusX3dEngBase::m_bLevelLoadingInProgress = true;
				break;
			}
		case ESYSTEM_EVENT_3D_POST_RENDERING_START:
			{
				DinrusX3dEngBase::GetMatMan()->DoLoadSurfaceTypesInInit(false);
				break;
			}
		case ESYSTEM_EVENT_3D_POST_RENDERING_END:
			{
				SAFE_DELETE(DinrusX3dEngBase::Get3DEngine()->m_pObjectsTree);

				// We have to unload physics data *before* shutting down the geom manager
				// Otherwise physical entities that are destroyed later will reference dangling geom pointers
				DinrusX3dEngBase::Get3DEngine()->UnloadPhysicsData();

				if (CObjUpr* pObjUpr = DinrusX3dEngBase::GetObjUpr())
				{
					pObjUpr->UnloadObjects(true);
				}

				if (IGeomUpr* pGeomUpr = DinrusX3dEngBase::GetPhysicalWorld()->GetGeomUpr())
				{
					pGeomUpr->ShutDownGeoman();
				}

				if (DinrusX3dEngBase::GetMatMan())
				{
					DinrusX3dEngBase::GetMatMan()->ShutDown();
					DinrusX3dEngBase::GetMatMan()->DoLoadSurfaceTypesInInit(true);
				}

				break;
			}
		}
	}
};
static CSystemEventListener_3DEngine g_system_event_listener_engine;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class CEngineModule_DinrusX3dEng : public I3DEngineModule
{
	DRXINTERFACE_BEGIN()
	DRXINTERFACE_ADD(Drx::IDefaultModule)
	DRXINTERFACE_ADD(I3DEngineModule)
	DRXINTERFACE_END()

	DRXGENERATE_SINGLETONCLASS_GUID(CEngineModule_DinrusX3dEng, "EngineModule_DinrusX3dEng", "2d38f12a-521d-43cf-ba18-fd1fa7ea5020"_drx_guid)

	virtual ~CEngineModule_DinrusX3dEng()
	{
		gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(&g_system_event_listener_engine);
		SAFE_RELEASE(gEnv->p3DEngine);
	}

	//////////////////////////////////////////////////////////////////////////
	virtual tukk GetName() const override     { return "DinrusX3dEng"; }
	virtual tukk GetCategory() const override { return "drx3D"; }

	//////////////////////////////////////////////////////////////////////////
	virtual bool Initialize(SSysGlobEnv& env, const SSysInitParams& initParams) override
	{
		ISystem* pSystem = env.pSystem;

		pSystem->GetISystemEventDispatcher()->RegisterListener(&g_system_event_listener_engine, "CSystemEventListener_3DEngine");

		C3DEngine* p3DEngine = DrxAlignedNew<C3DEngine>(pSystem);
		env.p3DEngine = p3DEngine;
		return true;
	}
};

DRXREGISTER_SINGLETON_CLASS(CEngineModule_DinrusX3dEng)

//////////////////////////////////////////////////////////////////////////
void DinrusX3dEngBase::PrintComment(tukk szText, ...)
{
	if (!szText)
		return;

	va_list args;
	va_start(args, szText);
	GetLog()->LogV(IMiniLog::eComment, szText, args);
	va_end(args);
}

void DinrusX3dEngBase::PrintMessage(tukk szText, ...)
{
	if (!szText)
		return;

	va_list args;
	va_start(args, szText);
	GetLog()->LogV(GetCVars()->e_3dEngineLogAlways ? IMiniLog::eAlways : IMiniLog::eMessage, szText, args);
	va_end(args);

	GetLog()->UpdateLoadingScreen(0);
}

void DinrusX3dEngBase::PrintMessagePlus(tukk szText, ...)
{
	if (!szText)
		return;

	va_list arglist;
	char buf[MAX_ERROR_STRING];
	va_start(arglist, szText);
	drx_vsprintf(buf, szText, arglist);
	va_end(arglist);
	GetLog()->LogPlus(buf);

	GetLog()->UpdateLoadingScreen(0);
}

float DinrusX3dEngBase::GetCurTimeSec()
{ return (gEnv->pTimer->GetCurrTime()); }

float DinrusX3dEngBase::GetCurAsyncTimeSec()
{ return (gEnv->pTimer->GetAsyncTime().GetSeconds()); }

//////////////////////////////////////////////////////////////////////////
void DinrusX3dEngBase::Warning(tukk format, ...)
{
	if (!format)
		return;

	va_list args;
	va_start(args, format);
	// Call to validating warning of system.
	m_pSystem->WarningV(VALIDATOR_MODULE_3DENGINE, VALIDATOR_WARNING, 0, 0, format, args);
	va_end(args);

	GetLog()->UpdateLoadingScreen(0);
}

//////////////////////////////////////////////////////////////////////////
void DinrusX3dEngBase::Error(tukk format, ...)
{
	//  assert(!"DinrusX3dEngBase::Error");

	va_list args;
	va_start(args, format);
	// Call to validating warning of system.
	m_pSystem->WarningV(VALIDATOR_MODULE_3DENGINE, VALIDATOR_ERROR, 0, 0, format, args);
	va_end(args);

	GetLog()->UpdateLoadingScreen(0);
}

//////////////////////////////////////////////////////////////////////////
void DinrusX3dEngBase::FileWarning(i32 flags, tukk file, tukk format, ...)
{
	va_list args;
	va_start(args, format);
	// Call to validating warning of system.
	m_pSystem->WarningV(VALIDATOR_MODULE_3DENGINE, VALIDATOR_WARNING, flags | VALIDATOR_FLAG_FILE, file, format, args);
	va_end(args);

	GetLog()->UpdateLoadingScreen(0);
}

IMaterial* DinrusX3dEngBase::MakeSystemMaterialFromShader(tukk sShaderName, SInputShaderResources* Res)
{
	IMaterial* pMat = Get3DEngine()->GetMaterialUpr()->CreateMaterial(sShaderName);
	//pMat->AddRef();

	if (GetRenderer())
	{
		SShaderItem si;
		si = GetRenderer()->EF_LoadShaderItem(sShaderName, true, 0, Res);
		pMat->AssignShaderItem(si);
	}

	return pMat;
}

//////////////////////////////////////////////////////////////////////////
bool DinrusX3dEngBase::IsValidFile(tukk sFilename)
{
	LOADING_TIME_PROFILE_SECTION;

	return gEnv->pDrxPak->IsFileExist(sFilename);
}

//////////////////////////////////////////////////////////////////////////
bool DinrusX3dEngBase::IsResourceLocked(tukk sFilename)
{
	IResourceList* pResList = GetPak()->GetResourceList(IDrxPak::RFOM_NextLevel);
	if (pResList)
	{
		return pResList->IsExist(sFilename);
	}
	return false;
}

void DinrusX3dEngBase::DrawBBoxLabeled(const AABB& aabb, const Matrix34& m34, const ColorB& col, tukk format, ...)
{
	va_list args;
	va_start(args, format);
	char szText[256];
	drx_vsprintf(szText, format, args);
	va_end(args);
	float fColor[4] = { col[0] / 255.f, col[1] / 255.f, col[2] / 255.f, col[3] / 255.f };
	GetRenderer()->GetIRenderAuxGeom()->SetRenderFlags(SAuxGeomRenderFlags());
	IRenderAuxText::DrawLabelEx(m34.TransformPoint(aabb.GetCenter()), 1.3f, fColor, true, true, szText);
	GetRenderer()->GetIRenderAuxGeom()->DrawAABB(aabb, m34, false, col, eBBD_Faceted);
}

//////////////////////////////////////////////////////////////////////////
void DinrusX3dEngBase::DrawBBox(const Vec3& vMin, const Vec3& vMax, ColorB col)
{
	GetRenderer()->GetIRenderAuxGeom()->SetRenderFlags(SAuxGeomRenderFlags());
	GetRenderer()->GetIRenderAuxGeom()->DrawAABB(AABB(vMin, vMax), false, col, eBBD_Faceted);
}

void DinrusX3dEngBase::DrawBBox(const AABB& box, ColorB col)
{
	GetRenderer()->GetIRenderAuxGeom()->SetRenderFlags(SAuxGeomRenderFlags());
	GetRenderer()->GetIRenderAuxGeom()->DrawAABB(box, false, col, eBBD_Faceted);
}

void DinrusX3dEngBase::DrawLine(const Vec3& vMin, const Vec3& vMax, ColorB col)
{
	GetRenderer()->GetIRenderAuxGeom()->SetRenderFlags(SAuxGeomRenderFlags());
	GetRenderer()->GetIRenderAuxGeom()->DrawLine(vMin, col, vMax, col);
}

void DinrusX3dEngBase::DrawSphere(const Vec3& vPos, float fRadius, ColorB color)
{
	GetRenderer()->GetIRenderAuxGeom()->SetRenderFlags(SAuxGeomRenderFlags());
	GetRenderer()->GetIRenderAuxGeom()->DrawSphere(vPos, fRadius, color);
}

void DinrusX3dEngBase::DrawQuad(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& v3, ColorB color)
{
	GetRenderer()->GetIRenderAuxGeom()->SetRenderFlags(SAuxGeomRenderFlags());
	GetRenderer()->GetIRenderAuxGeom()->DrawTriangle(v0, color, v2, color, v3, color);
	GetRenderer()->GetIRenderAuxGeom()->DrawTriangle(v0, color, v1, color, v2, color);
}

// Check if preloading is enabled.
bool DinrusX3dEngBase::IsPreloadEnabled()
{
	bool bPreload = false;
	ICVar* pSysPreload = GetConsole()->GetCVar("sys_preload");
	if (pSysPreload && pSysPreload->GetIVal() != 0)
		bPreload = true;

	return bPreload;
}

//////////////////////////////////////////////////////////////////////////
bool DinrusX3dEngBase::CheckMinSpec(u32 nMinSpec)
{
	if (nMinSpec == CONFIG_DETAIL_SPEC && GetCVars()->e_ViewDistRatioDetail == 0)
		return false;

	if ((i32)nMinSpec != 0 && GetCVars()->e_ObjQuality != 0 && (i32)nMinSpec > GetCVars()->e_ObjQuality)
		return false;

	return true;
}

bool DinrusX3dEngBase::IsEscapePressed()
{
#if DRX_PLATFORM_WINDOWS
	if (DinrusX3dEngBase::m_bEditor && (DrxGetAsyncKeyState(VK_CANCEL) & (1 << 15)) && (DrxGetAsyncKeyState(VK_ESCAPE) & (1 << 15)))
	{
		Get3DEngine()->PrintMessage("*** ESC key was pressed - operation aborted ***");
		return true;
	}
#endif
	return false;
}

#if defined(__GNUC__)
	#if __GNUC__ >= 4 && __GNUC__MINOR__ < 7
		#pragma GCC diagnostic error "-Wformat-security"
	#else
		#pragma GCC diagnostic pop
	#endif
#endif

#include <drx3D/CoreX/CrtDebugStats.h>

// TypeInfo implementations for DinrusX3dEng
#ifndef _LIB
	#include <drx3D/CoreX/Common_TypeInfo.h>
#endif

#include <drx3D/CoreX/TypeInfo_impl.h>

// Common types
#include <drx3D/Eng3D/IIndexedMesh_info.h>
#include <drx3D/Eng3D/IRenderNode_info.h>
#include <drx3D/Eng3D/CGFContent_info.h>

// 3DEngine types
#include <drx3D/Eng3D/SkyLightNishita_info.h>
#include <drx3D/Eng3D/terrain_sector_info.h>

STRUCT_INFO_BEGIN(SImageSubInfo)
VAR_INFO(nDummy)
VAR_INFO(nDim)
VAR_INFO(fTilingIn)
VAR_INFO(fTiling)
VAR_INFO(fSpecularAmount)
VAR_INFO(nSortOrder)
STRUCT_INFO_END(SImageSubInfo)

STRUCT_INFO_BEGIN(SImageInfo)
VAR_INFO(baseInfo)
VAR_INFO(detailInfo)
VAR_INFO(szDetMatName)
VAR_INFO(arrTextureId)
VAR_INFO(nPhysSurfaceType)
VAR_INFO(szBaseTexName)
VAR_INFO(fUseRemeshing)
VAR_INFO(layerFilterColor)
VAR_INFO(nLayerId)
VAR_INFO(fBr)
STRUCT_INFO_END(SImageInfo)
