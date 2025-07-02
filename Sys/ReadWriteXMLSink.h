// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __READWRITEXMLSINK_H__
#define __READWRITEXMLSINK_H__

#pragma once

#include <drx3D/Sys/ISystem.h>
#include <drx3D/Sys/IReadWriteXMLSink.h>

class CReadWriteXMLSink : public IReadWriteXMLSink
{
public:
	bool       ReadXML(tukk definitionFile, tukk dataFile, IReadXMLSink* pSink);
	bool       ReadXML(tukk definitionFile, XmlNodeRef node, IReadXMLSink* pSink);
	bool       ReadXML(XmlNodeRef definition, tukk dataFile, IReadXMLSink* pSink);
	bool       ReadXML(XmlNodeRef definition, XmlNodeRef node, IReadXMLSink* pSink);

	XmlNodeRef CreateXMLFromSource(tukk definitionFile, IWriteXMLSource* pSource);
	bool       WriteXML(tukk definitionFile, tukk dataFile, IWriteXMLSource* pSource);
};

// helper to define the if/else chain that we need in a few locations...
// types must match IReadXMLSink::TValueTypes
#define XML_SET_PROPERTY_HELPER(ELSE_LOAD_PROPERTY) \
  if (false);                                       \
  ELSE_LOAD_PROPERTY(Vec3);                         \
  ELSE_LOAD_PROPERTY(i32);                          \
  ELSE_LOAD_PROPERTY(float);                        \
  ELSE_LOAD_PROPERTY(string);                       \
  ELSE_LOAD_PROPERTY(bool);

#endif // __READWRITEXMLSINK_H__
