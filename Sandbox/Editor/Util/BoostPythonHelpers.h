// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __BOOSTPYTHONHELPER_H__
#define __BOOSTPYTHONHELPER_H__

#pragma once

#ifdef USE_PYTHON_SCRIPTING
// Include BoostHelpers before other boost headers, so BOOST_ASSERT works correctly.
	#include <drx3D/CoreX/BoostHelpers.h>
	#include <boost/python.hpp>
	#include "BoostPythonMacros.h"
#endif

#include <drx3D/Sandbox/Editor/EditorCommon/ICommandManager.h>
#include "Util/Variable.h"
#include <drx3D/CoreX/Math/Drx_Geo.h>

// boost/python.hpp and/or pyconfig.h will include <assert.h>
// Here we go back to DRX_ASSERT instead
#include <drx3D/CoreX/Assert/DrxAssert.h>
#include <drx3D/CoreX/Math/Drx_Geo.h>

#include <drx3D/CoreX/Extension/DrxGUID.h>

// NOTE: Do not use std::shared_ptr in this file - Boost.Python is unable to perform conversion if not using boost::shared_ptr
// https://github.com/boostorg/python/issues/29

// Forward Declarations
class CBaseObject;
class CObjectLayer;
class CMaterial;
class CVegetationObject;
struct SEfResTexture;

struct SPyWrappedProperty
{
	void SetProperties(IVariable* pVar);

	struct SColor
	{
		i32 r, g, b;
	};

	struct SVec
	{
		float x, y, z, w;
	};

	struct STime
	{
		i32 hour, min;
	};

	enum EType
	{
		eType_Bool,
		eType_Int,
		eType_Float,
		eType_String,
		eType_Vec3,
		eType_Vec4,
		eType_Color,
		eType_Time
	};

	union UProperty
	{
		bool   boolValue;
		i32    intValue;
		float  floatValue;
		SColor colorValue;
		SVec   vecValue;
		STime  timeValue;
	};

	EType     type;
	UProperty property;
	string   stringValue;
};

typedef boost::shared_ptr<SPyWrappedProperty> pSPyWrappedProperty;

// Dynamic Class types to emulate key engine objects.
struct SPyWrappedClass
{
	enum EType
	{
		eType_ActorEntity,
		eType_Area,
		eType_Brush,
		eType_Camera,
		eType_Decal,
		eType_Entity,
		eType_Particle,
		eType_Prefab,
		eType_Group,
		eType_None,
	}                       type;

	boost::shared_ptr<void> ptr;
};
typedef boost::shared_ptr<SPyWrappedClass> pSPyWrappedClass;

// Engine Classes Wrapped for easy get \ set \ update functionality to Python.
//
// PyBaseObject
//		-PyGameLayer
//		-PyGameMaterial
//			-PyGameSubMaterial Array
//				-PyGameTexture Array
//		-PyGameClass
//			-PyGameBrush
//			-PyGameEntity
//			-PyGameSolid
//			-PyGameGroup
//			-PyGamePrefab
//			-PyGameCamera
//		-PyGameVegetation
//			-PyGameVegetationInstance

class PyGameLayer
{
public:
	PyGameLayer(uk layerPtr);
	~PyGameLayer();
	uk                                       GetPtr() const           { return m_layerPtr; }
	string                                     GetName() const          { return m_layerName; }
	string                                     GetPath() const          { return m_layerPath; }
	DrxGUID                                     GetGUID() const          { return m_layerGUID; }
	bool                                        IsVisible() const        { return m_layerVisible; }
	bool                                        IsFrozen() const         { return m_layerFrozen; }
	bool                                        IsExportable() const     { return m_layerExportable; };
	bool                                        IsExportLayerPak() const { return m_layerExportLayerPak; };
	bool                                        IsDefaultLoaded() const  { return m_layerDefaultLoaded; };
	bool                                        IsPhysics() const        { return m_layerPhysics; }
	std::vector<boost::shared_ptr<PyGameLayer>> GetChildren() const
	{
		return m_layerChildren;
	}

	// Many setters ignored here as they can better be handled in other areas.
	void SetName(string name)                  { m_layerName = name; };
	void SetVisible(bool visible)               { m_layerVisible = visible; };
	void SetFrozen(bool frozen)                 { m_layerFrozen = frozen; };
	void SetExportable(bool exportable)         { m_layerExportable = exportable; };
	void SetExportLayerPak(bool exportLayerPak) { m_layerExportLayerPak = exportLayerPak; };
	void SetDefaultLoaded(bool loaded)          { m_layerDefaultLoaded = loaded; };
	void SetHavePhysics(bool physics)           { m_layerPhysics = physics; }

	void UpdateLayer();

private:
	uk   m_layerPtr;
	string m_layerName;
	string m_layerPath;
	DrxGUID m_layerGUID;
	std::vector<boost::shared_ptr<PyGameLayer>> m_layerChildren;
	bool    m_layerVisible;
	bool    m_layerFrozen;
	bool    m_layerExportable;
	bool    m_layerExportLayerPak;
	bool    m_layerDefaultLoaded;
	bool    m_layerPhysics;
};
typedef boost::shared_ptr<PyGameLayer> pPyGameLayer;

class PyGameTexture
{
public:
	PyGameTexture(uk pTexture);
	~PyGameTexture();
	uk   GetPtr() const        { return m_texPtr; }
	string GetName() const       { return m_texName; }

	void    SetName(string name) { m_texName = name; }

	void    UpdateTexture();

private:
	uk   m_texPtr;
	string m_texName;
};
typedef boost::shared_ptr<PyGameTexture> pPyGameTexture;

class PyGameSubMaterial
{
public:
	PyGameSubMaterial(uk pMat, i32 id);
	~PyGameSubMaterial();
	uk                                  GetPtr() const         { return m_matPtr; }
	string                                GetName() const        { return m_matName; }
	string                                GetShader() const      { return m_matShader; }
	string                                GetSurfaceType() const { return m_matSurfaceType; }
	std::map<string, pSPyWrappedProperty> GetMatParams() const
	{
		return m_matParams;
	}
	std::vector<pPyGameTexture> GetTextures() const
	{
		return m_matTextures;
	}

	void SetName(string name)                                       { m_matName = name; }
	void SetShader(string shader)                                   { m_matShader = shader; }
	void SetSurfaceType(string surface)                             { m_matSurfaceType = surface; }
	void SetTextures(std::vector<pPyGameTexture> textures)           { m_matTextures = textures; }
	void SetMatParams(std::map<string, pSPyWrappedProperty> params) { m_matParams = params; }

	void UpdateSubMaterial();

private:
	uk                                  m_matPtr;
	i32                                    m_matId;
	string                                m_matName;
	string                                m_matPath;
	string                                m_matShader;
	string                                m_matSurfaceType;
	std::vector<pPyGameTexture>            m_matTextures;
	std::map<string, pSPyWrappedProperty> m_matParams;

};
typedef boost::shared_ptr<PyGameSubMaterial> pPyGameSubMaterial;

class PyGameMaterial
{
public:
	PyGameMaterial(uk pMat);
	~PyGameMaterial();

	uk                           GetPtr() const  { return m_matPtr; }
	string                         GetName() const { return m_matName; }
	string                         GetPath() const { return m_matPath; }
	std::vector<pPyGameSubMaterial> GetSubMaterials() const
	{
		return m_matSubMaterials;
	}

	void SetName(string name) { m_matName = name; };
	//void SetPath(string path) { m_matPath = path; };
	// No setters for this class as the engine handles this way better.

	void UpdateMaterial();

private:
	uk                           m_matPtr;
	string                         m_matName;
	string                         m_matPath;
	std::vector<pPyGameSubMaterial> m_matSubMaterials;
};
typedef boost::shared_ptr<PyGameMaterial> pPyGameMaterial;

class PyGameObject
{
public:
	PyGameObject(uk objPtr);
	//~PyGameObject();

	typedef boost::shared_ptr<PyGameObject> pPyGameObject;

	pSPyWrappedClass GetClassObject()                  { return m_objClass; }
	uk            GetPtr() const                    { return m_objPtr; }
	string          GetName() const                   { return m_objName; }
	string          GetType() const                   { return m_objType; }
	DrxGUID          GetGUID() const                   { return m_objGUID; }
	pPyGameLayer     GetLayer() const                  { return m_objLayer; }
	pPyGameMaterial  GetMaterial() const               { return m_objMaterial; }
	pPyGameObject    GetParent();
	Vec3             GetPosition() const               { return m_objPosition; }
	Vec3             GetRotation() const               { return m_objRotation; }
	Vec3             GetScale() const                  { return m_objScale; }
	AABB             GetBounds() const                 { return m_objBounds; }
	bool             IsInGroup() const                 { return m_objInGroup; }
	bool             IsVisible() const                 { return m_objVisible; }
	bool             IsFrozen() const                  { return m_objFrozen; }
	bool             IsSelected() const                { return m_objSelected; }

	void             SetName(string name)             { m_objName = name; };
	void             SetLayer(pPyGameLayer pLayer)     { m_objLayer = pLayer; };
	void             SetMaterial(pPyGameMaterial pMat) { m_objMaterial = pMat; };
	void             SetParent(pPyGameObject pParent);
	void             SetPosition(Vec3 position)        { m_objPosition = position; };
	void             SetRotation(Vec3 rotation)        { m_objRotation = rotation; };
	void             SetScale(Vec3 scale)              { m_objScale = scale; };
	void             SetVisible(bool visible)          { m_objVisible = visible; };
	void             SetFrozen(bool frozen)            { m_objFrozen = frozen; };
	void             SetSelected(bool selected)        { m_objSelected = selected; };

	void             UpdateObject(); // store the class ref as a void pointer. in update reinterpret cast the object back to its baseclass, in this case CBaseObject.

private:
	uk            m_objPtr;
	pSPyWrappedClass m_objClass;
	string          m_objName;
	string          m_objType;
	DrxGUID          m_objGUID;
	Vec3             m_objPosition;
	Vec3             m_objRotation;
	Vec3             m_objScale;
	AABB             m_objBounds;
	bool             m_objInGroup;
	bool             m_objVisible;
	bool             m_objFrozen;
	bool             m_objSelected;
	pPyGameLayer     m_objLayer;
	pPyGameMaterial  m_objMaterial;
	pPyGameObject    m_objParent;
};
typedef boost::shared_ptr<PyGameObject> pPyGameObject;

class PyGameBrush
{
public:
	PyGameBrush(uk brushPtr, pSPyWrappedClass sharedPtr);
	~PyGameBrush();
	string GetModel() const            { return m_brushModel; }
	i32     GetRatioLod() const         { return m_brushRatioLod; }
	i32     GetRatioViewDist() const    { return m_brushRatioviewDist; }
	i32     GetLodCount() const         { return m_brushLodCount; }

	void    SetModel(string model)     { m_brushModel = model; }
	void    SetRatioLod(i32 ratio)      { m_brushRatioLod = ratio; }
	void    SetRatioViewDist(i32 ratio) { m_brushRatioviewDist = ratio; }

	void    Reload();
	void    UpdateBrush();

private:
	uk   m_brushPtr;
	string m_brushModel;
	i32     m_brushRatioLod;
	i32     m_brushRatioviewDist;
	i32     m_brushLodCount;
};

class PyGameEntity
{
public:
	PyGameEntity(uk entityPtr, pSPyWrappedClass sharedPtr);
	~PyGameEntity();
	string                                GetModel() const         { return m_entityModel; }
	string                                GetSubClass() const      { return m_entitySubClass; }
	i32                                    GetRatioLod() const      { return m_entityRatioLod; }
	i32                                    GetRatioViewDist() const { return m_entityRatioviewDist; }
	std::map<string, pSPyWrappedProperty> GetProperties()
	{
		return m_entityProps;
	}

	void SetModel(string model)                                     { m_entityModel = model; }
	void SetSubClass(string cls)                                    { m_entitySubClass = cls; }
	void SetRatioLod(i32 ratio)                                      { m_entityRatioLod = ratio; }
	void SetRatioViewDist(i32 ratio)                                 { m_entityRatioviewDist = ratio; }
	void SetProperties(std::map<string, pSPyWrappedProperty> props) { m_entityProps = props; }

	void Reload();
	void UpdateEntity();

private:
	uk   m_entityPtr;
	i32     m_entityId;
	string m_entitySubClass;
	string m_entityModel;
	float   m_entityRatioLod;
	float   m_entityRatioviewDist;
	std::map<string, pSPyWrappedProperty> m_entityProps;
};

class PyGamePrefab
{
public:
	PyGamePrefab(uk prefabPtr, pSPyWrappedClass sharedPtr);
	~PyGamePrefab();

	bool                       IsOpen();
	string                    GetName() const { return m_prefabName; }
	std::vector<pPyGameObject> GetChildren();

	void                       SetName(string name) { m_prefabName = name; }

	void                       AddChild(pPyGameObject pObj);
	void                       RemoveChild(pPyGameObject pObj);

	void                       Open();
	void                       Close();
	void                       ExtractAll();

	void                       UpdatePrefab();

private:
	uk                      m_prefabPtr;
	string                    m_prefabName;
	std::vector<pPyGameObject> m_prefabChildren;
};

class PyGameGroup
{
public:
	PyGameGroup(uk groupPtr, pSPyWrappedClass sharedPtr);
	~PyGameGroup();

	bool                       IsOpen();
	std::vector<pPyGameObject> GetChildren();    // const { return m_groupChildren; }

	void                       AddChild(pPyGameObject pObj);
	void                       RemoveChild(pPyGameObject pObj);

	void                       Open();
	void                       Close();

	void                       UpdateGroup();

private:
	uk                      m_groupPtr;
	std::vector<pPyGameObject> m_groupChildren;
};

class PyGameCamera
{
public:
	PyGameCamera(uk cameraPtr, pSPyWrappedClass sharedPtr);
	~PyGameCamera();

	void UpdateCamera();

private:
	uk m_cameraPtr;
};

class PyGameVegetationInstance
{
public:
	PyGameVegetationInstance(uk vegPtr);
	~PyGameVegetationInstance();

	Vec3  GetPosition() const             { return m_vegPosition; }
	float GetAngle() const                { return m_vegAngle; }
	float GetScale() const                { return m_vegScale; }
	float GetBrightness() const           { return m_vegBrightness; }

	void  SetPosition(Vec3 position)      { m_vegPosition = position; }
	void  SetAngle(float angle)           { m_vegAngle = angle; }
	void  SetScale(float scale)           { m_vegScale = scale; }
	void  SetBrightness(float brightness) { m_vegBrightness = brightness; }

	void  UpdateVegetationInstance();

private:
	uk m_vegPtr;
	Vec3  m_vegPosition;
	float m_vegAngle;
	float m_vegScale;
	float m_vegBrightness;
};
typedef boost::shared_ptr<PyGameVegetationInstance> pPyGameVegetationInstance;

class PyGameVegetation
{
public:
	PyGameVegetation(uk vegPtr);
	~PyGameVegetation();

	uk                                  GetPtr() const          { return m_vegPtr; }
	string                                GetName() const         { return m_vegName; }
	string                                GetGroup() const        { return m_vegGroup; }
	i32                                    GetID() const           { return m_vegID; }
	i32                                    GetNumInstances() const { return m_vegInstCount; }
	std::vector<pPyGameVegetationInstance> GetInstances() const
	{
		return m_vegInstances;
	}
	bool IsSelected() const                    { return m_vegSelected; }
	bool IsVisible() const                     { return m_vegVisible; }
	bool IsFrozen() const                      { return m_vegFrozen; }
	bool IsCastShadow() const                  { return m_vegCastShadows; }
	bool GetGIMode() const                     { return m_vegGIMode; }
	bool IsAutoMerged() const                  { return m_vegAutoMerged; }
	bool IsHideable() const                    { return m_vegHideable; }

	void SetName(string name)                 { m_vegName = name; }
	void SetGroup(string group)               { m_vegGroup = group; }
	void SetSelected(bool selected)            { m_vegSelected = selected; }
	void SetVisible(bool visible)              { m_vegVisible = visible; }
	void SetFrozen(bool frozen)                { m_vegFrozen = frozen; }
	void SetCastShadow(bool castShadows)       { m_vegCastShadows = castShadows; }
	void SetGIMode(bool mode)                  { m_vegGIMode = mode; }
	void SetHideable(bool hideable)            { m_vegHideable = hideable; }

	void Load();
	void Unload();

	void UpdateVegetation();

private:
	uk   m_vegPtr;
	string m_vegName;
	string m_vegGroup;
	i32     m_vegID;
	i32     m_vegInstCount;
	bool    m_vegSelected;
	bool    m_vegVisible;
	bool    m_vegFrozen;
	bool    m_vegCastShadows;
	bool    m_vegGIMode;
	bool    m_vegAutoMerged;
	bool    m_vegHideable;
	std::vector<pPyGameVegetationInstance> m_vegInstances;

};
typedef boost::shared_ptr<PyGameVegetation> pPyGameVegetation;

// Python Engine Objects Cache Template.
template<class SHAREDPTR, class PTR>
struct SPyWrapperCache
{
public:
	~SPyWrapperCache()
	{
		ClearCache();
	}
	bool IsCached(PTR index)
	{
		for (std::vector<SHAREDPTR>::iterator iter = m_Cache.begin(); iter != m_Cache.end(); iter++)
		{
			if (iter->get()->GetPtr() == index)
				return true;
		}
		return false;
	}
	SHAREDPTR GetCachedSharedPtr(PTR index)
	{
		SHAREDPTR result;
		for (std::vector<SHAREDPTR>::iterator iter = m_Cache.begin(); iter != m_Cache.end(); iter++)
		{
			if (iter->get()->GetPtr() == index)
				return *iter;
		}
		return result;
	}
	void AddToCache(SHAREDPTR sharedPtr) { m_Cache.push_back(sharedPtr); }
	void RemoveFromCache(SHAREDPTR sharedPtr)
	{
		for (std::vector<SHAREDPTR>::iterator iter = m_Cache.begin(); iter != m_Cache.end(); iter++)
		{
			if (iter->get()->GetPtr() == index)
				m_Cache.resize(std::remove(m_Cache.begin(), m_Cache.end(), iter) - m_Cache.begin());
		}
	}
	void ClearCache() { m_Cache.clear(); }

private:
	std::vector<SHAREDPTR> m_Cache;
};

// Creating necessary Caches for all Editor <-> Python Objects.
typedef SPyWrapperCache<pPyGameObject, CBaseObject*>           PyObjCache;
typedef SPyWrapperCache<pPyGameLayer, CObjectLayer*>           PyLyrCache;
typedef SPyWrapperCache<pPyGameMaterial, CMaterial*>           PyMatCache;
typedef SPyWrapperCache<pPyGameSubMaterial, CMaterial*>        PySubMatCache;
typedef SPyWrapperCache<pPyGameTexture, SEfResTexture*>        PyTextureCache;
typedef SPyWrapperCache<pPyGameVegetation, CVegetationObject*> PyVegCache;

struct SPyCache
{
public:
	SPyCache();
	~SPyCache();
	PyObjCache*     GetObjectCache() const      { return m_ObjCache; }
	PyLyrCache*     GetLayerCache() const       { return m_LyrCache; }
	PyMatCache*     GetMaterialCache() const    { return m_MatCache; }
	PySubMatCache*  GetSubMaterialCache() const { return m_SubMatCache; }
	PyTextureCache* GetTextureCache() const     { return m_TextureCache; }
	PyVegCache*     GetVegetationCache() const  { return m_VegCache; }

private:
	PyObjCache*     m_ObjCache;
	PyLyrCache*     m_LyrCache;
	PyMatCache*     m_MatCache;
	PySubMatCache*  m_SubMatCache;
	PyTextureCache* m_TextureCache;
	PyVegCache*     m_VegCache;
};

/////////////////////////////////////////////////////////////////////////
// Python List and Dictionary Wrapper to avoid issues inherent in using the map indexing suite.
template<class KEY, class VAL>
struct map_item
{
	typedef std::map<KEY, VAL> Map;

	static VAL get(Map& self, const KEY idx)
	{
		if (self.find(idx) == self.end())
		{
			PyErr_SetString(PyExc_KeyError, "Map key not found");
			boost::python::throw_error_already_set();
		}
		return self[idx];
	}

	static void                set(Map& self, const KEY idx, const VAL val) { self[idx] = val; }
	static void                del(Map& self, const KEY n)                  { self.erase(n); }
	static bool                in(Map const& self, const KEY n)             { return self.find(n) != self.end(); }

	static boost::python::list keys(Map const& self)
	{
		boost::python::list t;
		for (typename Map::const_iterator it = self.begin(); it != self.end(); ++it)
			t.append(it->first);
		return t;
	}

	static boost::python::list values(Map const& self)
	{
		boost::python::list t;
		for (typename Map::const_iterator it = self.begin(); it != self.end(); ++it)
			t.append(it->second);
		return t;
	}

	static boost::python::list items(Map const& self)
	{
		boost::python::list t;
		for (typename Map::const_iterator it = self.begin(); it != self.end(); ++it)
			t.append(boost::python::make_tuple(it->first, it->second));
		return t;
	}
};

#define STL_MAP_WRAPPING_PTR(KEY_TYPE, VALUE_TYPE, PYTHON_TYPE_NAME)                                                          \
  boost::python::class_<std::pair<const KEY_TYPE, VALUE_TYPE>>((STxt(PYTHON_TYPE_NAME) + STxt("DATA")).c_str()) \
  .def_readonly("key", &std::pair<const KEY_TYPE, VALUE_TYPE>::first)                                                         \
  .def_readwrite("value", &std::pair<const KEY_TYPE, VALUE_TYPE>::second)                                                     \
  ;                                                                                                                           \
  boost::python::class_<std::map<KEY_TYPE, VALUE_TYPE>>(PYTHON_TYPE_NAME)                                                     \
  .def("__len__", &std::map<KEY_TYPE, VALUE_TYPE>::size)                                                                      \
  .def("__iter__", boost::python::iterator<std::map<KEY_TYPE, VALUE_TYPE>, boost::python::return_internal_reference<>>())     \
  .def("__getitem__", &map_item<KEY_TYPE, VALUE_TYPE>().get, boost::python::return_internal_reference<>())                    \
  .def("__setitem__", &map_item<KEY_TYPE, VALUE_TYPE>().set)                                                                  \
  .def("__delitem__", &map_item<KEY_TYPE, VALUE_TYPE>().del)                                                                  \
  .def("__contains__", &map_item<KEY_TYPE, VALUE_TYPE>().in)                                                                  \
  .def("clear", &std::map<KEY_TYPE, VALUE_TYPE>::clear)                                                                       \
  .def("has_key", &map_item<KEY_TYPE, VALUE_TYPE>().in)                                                                       \
  .def("keys", &map_item<KEY_TYPE, VALUE_TYPE>().keys)                                                                        \
  .def("values", &map_item<KEY_TYPE, VALUE_TYPE>().values)                                                                    \
  .def("items", &map_item<KEY_TYPE, VALUE_TYPE>().items)                                                                      \
  ;

#define STL_MAP_WRAPPING(KEY_TYPE, VALUE_TYPE, PYTHON_TYPE_NAME)                                                              \
  boost::python::class_<std::pair<const KEY_TYPE, VALUE_TYPE>>((STxt(PYTHON_TYPE_NAME) + STxt("DATA")).c_str()) \
  .def_readonly("key", &std::pair<const KEY_TYPE, VALUE_TYPE>::first)                                                         \
  .def_readwrite("value", &std::pair<const KEY_TYPE, VALUE_TYPE>::second)                                                     \
  ;                                                                                                                           \
  boost::python::class_<std::map<KEY_TYPE, VALUE_TYPE>>(PYTHON_TYPE_NAME)                                                     \
  .def("__len__", &std::map<KEY_TYPE, VALUE_TYPE>::size)                                                                      \
  .def("__iter__", boost::python::iterator<std::map<KEY_TYPE, VALUE_TYPE>, boost::python::return_internal_reference<>>())     \
  .def("__getitem__", &map_item<KEY_TYPE, VALUE_TYPE>().get)                                                                  \
  .def("__setitem__", &map_item<KEY_TYPE, VALUE_TYPE>().set)                                                                  \
  .def("__delitem__", &map_item<KEY_TYPE, VALUE_TYPE>().del)                                                                  \
  .def("__contains__", &map_item<KEY_TYPE, VALUE_TYPE>().in)                                                                  \
  .def("clear", &std::map<KEY_TYPE, VALUE_TYPE>::clear)                                                                       \
  .def("has_key", &map_item<KEY_TYPE, VALUE_TYPE>().in)                                                                       \
  .def("keys", &map_item<KEY_TYPE, VALUE_TYPE>().keys)                                                                        \
  .def("values", &map_item<KEY_TYPE, VALUE_TYPE>().values)                                                                    \
  .def("items", &map_item<KEY_TYPE, VALUE_TYPE>().items)                                                                      \
  ;

// Key Functions for use in Editor <-> Python Interface.
namespace PyScript
{
struct IPyScriptListener
{
	virtual void OnStdOut(tukk pString) = 0;
	virtual void OnStdErr(tukk pString) = 0;
};

// Initialize python for use in sandbox
void InitializePython();

// Shutdown python
void ShutdownPython();

// Registers a IPyScriptListener listener
void RegisterListener(IPyScriptListener* pListener);

// Removes a IPyScriptListener listener
void RemoveListener(IPyScriptListener* pListener);

// Executes a given python string with a special logging.
void Execute(tukk pString, ...);

// Prints a message to the script terminal, if it's opened
void PrintMessage(tukk pString);

// Prints an error to the script terminal, if it's opened
void PrintError(tukk pString);

// Load python Plugins from Editor/Python/Plugins
void LoadPythonPlugins();

// Gets the value of a python variable as string/i32/float/bool
tukk GetAsString(tukk varName);
i32         GetAsInt(tukk varName);
float       GetAsFloat(tukk varName);
bool        GetAsBool(tukk varName);

// Registers all necessary python classes into the DrxClasses module.
void InitCppClasses();

// Import all python exception types for later error handling.
void InitCppExceptions();

// Object Creation functions for External Use Only.
pPyGameObject      CreatePyGameObject(CBaseObject* pObj);
pPyGameLayer       CreatePyGameLayer(CObjectLayer* pLayer);
pPyGameMaterial    CreatePyGameMaterial(CMaterial* pMat);
pPyGameSubMaterial CreatePyGameSubMaterial(CMaterial* pSubMat, i32 id);
pPyGameTexture     CreatePyGameTexture(SEfResTexture* pTex);
pPyGameVegetation  CreatePyGameVegetation(CVegetationObject* pVeg);
}

// Used as helper in the source code ex: PyCommand("general.test()")
inline string PyCommand(tukk commandString, ...)
{
	char buffer[1024];
	va_list args;
	va_start(args, commandString);
	drx_vsprintf(buffer, commandString, args);
	va_end(args);
	return buffer;
}

#endif // __BOOSTPYTHONHELPER_H__

