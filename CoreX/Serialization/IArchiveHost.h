// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Containers/DrxArray.h>
#include <drx3D/CoreX/Serialization/Forward.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <drx3D/Sys/IXml.h>

class XmlNodeRef;

namespace Serialization
{
//! Specifies the version of xml archive implementation used by the IArchiveHost
enum class EDrxXmlVersion : u32
{
	Auto = 0,            //!< Automatically select version to use
	Version1 = 1,        //!< Use xml archive version 1
	Version2 = 2,        //!< Use xml archive version 2
	Last                 //!< Keep this value as the last version
};


//! IArchiveHost serves a purpose of sharing IArchive implementations among diffferent modules.
//! Example of usage:
//! struct SType
//! {
//!    void Serialize(Serialization::IArchive& ar);
//! };
//!
//! SType instanceToSave;
//! bool saved = Serialization::SaveJsonFile("Scripts/instance.json", instanceToSave);
//!
//! SType instanceToLoad;
//! bool loaded = Serialization::LoadJsonFile(instanceToLoad, "Scripts/instance.json");
struct IArchiveHost
{
	virtual ~IArchiveHost() {}
	//! Parses JSON from file into the specified object
	//! \par Example
	//! \include DinrusXSys/Examples/JsonSerialization.cpp
	virtual bool LoadJsonFile(const SStruct& outObj, tukk filename, bool bCanBeOnDisk) = 0;
	//! Saves JSON into a file, reading data from the specified object
	//! \par Example
	//! \include DinrusXSys/Examples/JsonSerialization.cpp
	virtual bool SaveJsonFile(tukk filename, const SStruct& obj) = 0;
	//! Parses JSON from buffer into the specified object
	//! \par Example
	//! \include DinrusXSys/Examples/JsonSerialization.cpp
	virtual bool LoadJsonBuffer(const SStruct& outObj, tukk buffer, size_t bufferLength) = 0;
	//! Saves JSON into a buffer, reading data from the specified object
	virtual bool SaveJsonBuffer(DynArray<char>& outBuffer, const SStruct& obj) = 0;

	virtual bool LoadBinaryFile(const SStruct& outObj, tukk filename) = 0;
	virtual bool SaveBinaryFile(tukk filename, const SStruct& obj) = 0;
	virtual bool LoadBinaryBuffer(const SStruct& outObj, tukk buffer, size_t bufferLength) = 0;
	virtual bool SaveBinaryBuffer(DynArray<char>& outBuffer, const SStruct& obj) = 0;
	virtual bool CloneBinary(const SStruct& dest, const SStruct& source) = 0;

	//! Compares two instances in serialized form through binary archive
	virtual bool       CompareBinary(const SStruct& lhs, const SStruct& rhs) = 0;

	virtual bool       LoadXmlFile(const SStruct& outObj, tukk filename, EDrxXmlVersion forceVersion = EDrxXmlVersion::Auto) = 0;
	virtual bool       SaveXmlFile(tukk filename, const SStruct& obj, tukk rootNodeName, EDrxXmlVersion forceVersion = EDrxXmlVersion::Auto) = 0;
	virtual bool       LoadXmlNode(const SStruct& outObj, const XmlNodeRef& node, EDrxXmlVersion forceVersion = EDrxXmlVersion::Auto) = 0;
	virtual XmlNodeRef SaveXmlNode(const SStruct& obj, tukk nodeName, EDrxXmlVersion forceVersion = EDrxXmlVersion::Auto) = 0;
	virtual bool       SaveXmlNode(XmlNodeRef& node, const SStruct& obj, EDrxXmlVersion forceVersion = EDrxXmlVersion::Auto) = 0;

	virtual bool       LoadBlackBox(const SStruct& outObj, SBlackBox& box) = 0;
};
} // namespace Serialization

#include <drx3D/Sys/ISystem.h> // gEnv

namespace Serialization
{
//! Syntactic sugar.
template<class T> bool LoadJsonFile(T& instance, tukk filename)
{
	return gEnv->pSystem->GetArchiveHost()->LoadJsonFile(Serialization::SStruct(instance), filename, false);
}

template<class T> bool SaveJsonFile(tukk filename, const T& instance)
{
	return gEnv->pSystem->GetArchiveHost()->SaveJsonFile(filename, Serialization::SStruct(instance));
}

template<class T> bool LoadJsonBuffer(T& instance, tukk buffer, size_t bufferLength)
{
	return gEnv->pSystem->GetArchiveHost()->LoadJsonBuffer(Serialization::SStruct(instance), buffer, bufferLength);
}

template<class T> bool SaveJsonBuffer(DynArray<char>& outBuffer, const T& instance)
{
	return gEnv->pSystem->GetArchiveHost()->SaveJsonBuffer(outBuffer, Serialization::SStruct(instance));
}

// ---------------------------------------------------------------------------

template<class T> bool LoadBinaryFile(T& outInstance, tukk filename)
{
	return gEnv->pSystem->GetArchiveHost()->LoadBinaryFile(Serialization::SStruct(outInstance), filename);
}

template<class T> bool SaveBinaryFile(tukk filename, const T& instance)
{
	return gEnv->pSystem->GetArchiveHost()->SaveBinaryFile(filename, Serialization::SStruct(instance));
}

template<class T> bool LoadBinaryBuffer(T& outInstance, tukk buffer, size_t bufferLength)
{
	return gEnv->pSystem->GetArchiveHost()->LoadBinaryBuffer(Serialization::SStruct(outInstance), buffer, bufferLength);
}

template<class T> bool SaveBinaryBuffer(DynArray<char>& outBuffer, const T& instance)
{
	return gEnv->pSystem->GetArchiveHost()->SaveBinaryBuffer(outBuffer, Serialization::SStruct(instance));
}

template<class T> bool CloneBinary(T& outInstance, const T& inInstance)
{
	return gEnv->pSystem->GetArchiveHost()->CloneBinary(Serialization::SStruct(outInstance), Serialization::SStruct(inInstance));
}

template<class T> bool CompareBinary(const T& lhs, const T& rhs)
{
	return gEnv->pSystem->GetArchiveHost()->CompareBinary(Serialization::SStruct(lhs), Serialization::SStruct(rhs));
}

// ---------------------------------------------------------------------------

template<class T> bool LoadXmlFile(T& outInstance, tukk filename, EDrxXmlVersion forceVersion = EDrxXmlVersion::Auto)
{
	return gEnv->pSystem->GetArchiveHost()->LoadXmlFile(Serialization::SStruct(outInstance), filename, forceVersion);
}

template<class T> bool SaveXmlFile(tukk filename, const T& instance, tukk rootNodeName, EDrxXmlVersion forceVersion = EDrxXmlVersion::Auto)
{
	return gEnv->pSystem->GetArchiveHost()->SaveXmlFile(filename, Serialization::SStruct(instance), rootNodeName, forceVersion);
}

template<class T> bool LoadXmlNode(T& outInstance, const XmlNodeRef& node, EDrxXmlVersion forceVersion = EDrxXmlVersion::Auto)
{
	return gEnv->pSystem->GetArchiveHost()->LoadXmlNode(Serialization::SStruct(outInstance), node, forceVersion);
}

template<class T> XmlNodeRef SaveXmlNode(const T& instance, tukk nodeName, EDrxXmlVersion forceVersion = EDrxXmlVersion::Auto)
{
	return gEnv->pSystem->GetArchiveHost()->SaveXmlNode(Serialization::SStruct(instance), nodeName, forceVersion);
}

template<class T> bool SaveXmlNode(XmlNodeRef& node, const T& instance, EDrxXmlVersion forceVersion = EDrxXmlVersion::Auto)
{
	return gEnv->pSystem->GetArchiveHost()->SaveXmlNode(node, Serialization::SStruct(instance), forceVersion);
}

// ---------------------------------------------------------------------------

template<class T> bool LoadBlackBox(T& outInstance, SBlackBox& box)
{
	return gEnv->pSystem->GetArchiveHost()->LoadBlackBox(Serialization::SStruct(outInstance), box);
}

}
