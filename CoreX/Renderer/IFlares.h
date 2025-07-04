// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
// -------------------------------------------------------------------------
//  Имя файла:   IFlares.h
//  Created:     8/12/2011
////////////////////////////////////////////////////////////////////////////
#include <drx3D/CoreX/Sandbox/IFuncVariable.h> // <> required for Interfuscator
#include <drx3D/Sys/IXml.h>       // <> required for Interfuscator
#include <drx3D/CoreX/smartptr.h>

struct IShader;
class CCamera;
struct SRenderingPassInfo;

class __MFPA
{
};
class __MFPB
{
};

enum EFlareType
{
	eFT__Base__,
	eFT_Root,
	eFT_Group,
	eFT_Ghost,
	eFT_MultiGhosts,
	eFT_Glow,
	eFT_ChromaticRing,
	eFT_IrisShafts,
	eFT_CameraOrbs,
	eFT_ImageSpaceShafts,
	eFT_Streaks,
	eFT_Reference,
	eFT_Proxy,
	eFT_Max
};

#define FLARE_LIBS_PATH           "Libs/Flares/"
#define FLARE_EXPORT_FILE         "LensFlareList.xml"
#define FLARE_EXPORT_FILE_VERSION "1"

struct FlareInfo
{
	EFlareType  type;
	tukk name;
#if defined(FLARES_SUPPORT_EDITING)
	tukk imagename;
#endif
};

#if defined(FLARES_SUPPORT_EDITING)
	#define ADD_FLARE_INFO(type, name, imagename) { type, name, imagename }
#else
	#define ADD_FLARE_INFO(type, name, imagename) { type, name }
#endif

class FlareInfoArray
{
public:
	struct Props
	{
		const FlareInfo* p;
		size_t           size;
	};

	static const Props Get()
	{
		static const FlareInfo flareInfoArray[] =
		{
			ADD_FLARE_INFO(eFT__Base__,          "__Base__",      NULL),
			ADD_FLARE_INFO(eFT_Root,             "Root",          NULL),
			ADD_FLARE_INFO(eFT_Group,            "Group",         NULL),
			ADD_FLARE_INFO(eFT_Ghost,            "Ghost",         "%ENGINE%/EngineAssets/Textures/flares/icons/ghost.dds"),
			ADD_FLARE_INFO(eFT_MultiGhosts,      "Multi Ghost",   "%ENGINE%/EngineAssets/Textures/flares/icons/multi_ghost.dds"),
			ADD_FLARE_INFO(eFT_Glow,             "Glow",          "%ENGINE%/EngineAssets/Textures/flares/icons/glow.dds"),
			ADD_FLARE_INFO(eFT_ChromaticRing,    "ChromaticRing", "%ENGINE%/EngineAssets/Textures/flares/icons/ring.dds"),
			ADD_FLARE_INFO(eFT_IrisShafts,       "IrisShafts",    "%ENGINE%/EngineAssets/Textures/flares/icons/iris_shafts.dds"),
			ADD_FLARE_INFO(eFT_CameraOrbs,       "CameraOrbs",    "%ENGINE%/EngineAssets/Textures/flares/icons/orbs.dds"),
			ADD_FLARE_INFO(eFT_ImageSpaceShafts, "Vol Shafts",    "%ENGINE%/EngineAssets/Textures/flares/icons/vol_shafts.dds"),
			ADD_FLARE_INFO(eFT_Streaks,          "Streaks",       "%ENGINE%/EngineAssets/Textures/flares/icons/iris_shafts.dds")
		};

		Props ret;
		ret.p = flareInfoArray;
		ret.size = DRX_ARRAY_COUNT(flareInfoArray);
		return ret;
	}

private:
	FlareInfoArray();
	~FlareInfoArray();
};

struct SLensFlareRenderParam
{
	SLensFlareRenderParam(CCamera* pCamera, IShader* pShader, const SRenderingPassInfo& passInfo) :
		pCamera(pCamera),
		pShader(pShader),
		passInfo(passInfo)
	{
	}
	~SLensFlareRenderParam(){}
	bool IsValid() const
	{
		return pCamera && pShader;
	}
	CCamera* pCamera;
	IShader* pShader;
	const SRenderingPassInfo& passInfo;
};

class ISoftOcclusionQuery
{
public:
	// <interfuscator:shuffle>
	virtual ~ISoftOcclusionQuery() {}

	virtual void AddRef() = 0;
	virtual void Release() = 0;
	// </interfuscator:shuffle>
};

class IOpticsElementBase : public __MFPA, public __MFPB
{
public:

	IOpticsElementBase() : m_nRefCount(0)
	{
	}
	void AddRef()
	{
		DrxInterlockedIncrement(&m_nRefCount);
	}
	void Release()
	{
		if (DrxInterlockedDecrement(&m_nRefCount) <= 0)
			DeleteThis();
	}

	// <interfuscator:shuffle>
	virtual EFlareType          GetType() = 0;
	virtual bool                IsGroup() const = 0;
	virtual tukk         GetName() const = 0;
	virtual void                SetName(tukk ch_name) = 0;
	virtual void                Load(IXmlNode* pNode) = 0;

	virtual IOpticsElementBase* GetParent() const = 0;
	virtual ~IOpticsElementBase(){}

	virtual bool                IsEnabled() const = 0;
	virtual void                SetEnabled(bool b) = 0;

	virtual void                AddElement(IOpticsElementBase* pElement) = 0;
	virtual void                InsertElement(i32 nPos, IOpticsElementBase* pElement) = 0;
	virtual void                Remove(i32 i) = 0;
	virtual void                RemoveAll() = 0;
	virtual i32                 GetElementCount() const = 0;
	virtual IOpticsElementBase* GetElementAt(i32 i) const = 0;

	virtual void                GetMemoryUsage(IDrxSizer* pSizer) const = 0;
	virtual void                Invalidate() = 0;

	virtual void                RenderPreview(const SLensFlareRenderParam* pParam, const Vec3& vPos) = 0;

	virtual void                SetOpticsReference(IOpticsElementBase* pReference) {}
	virtual IOpticsElementBase* GetOpticsReference() const                         { return NULL; }
	// </interfuscator:shuffle>

#if defined(FLARES_SUPPORT_EDITING)
	virtual DynArray<FuncVariableGroup> GetEditorParamGroups() = 0;
#endif

protected:
	virtual void                DeleteThis() = 0;

private:
	 i32 m_nRefCount;
};

class IOpticsUpr
{
public:
	// <interfuscator:shuffle>
	virtual ~IOpticsUpr(){}
	virtual void                Reset() = 0;
	virtual IOpticsElementBase* Create(EFlareType type) const = 0;
	virtual bool                Load(tukk fullFlareName, i32& nOutIndex) = 0;
	virtual bool                Load(XmlNodeRef& rootNode, i32& nOutIndex) = 0;
	virtual IOpticsElementBase* GetOptics(i32 nIndex) = 0;
	virtual bool                AddOptics(IOpticsElementBase* pOptics, tukk name, i32& nOutNewIndex) = 0;
	virtual bool                Rename(tukk fullFlareName, tukk newFullFlareName) = 0;
	virtual void                GetMemoryUsage(IDrxSizer* pSizer) const = 0;
	virtual void                Invalidate() = 0;
	// </interfuscator:shuffle>
};

typedef _smart_ptr<IOpticsElementBase> IOpticsElementBasePtr;
