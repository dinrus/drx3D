// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/ReadWriteXMLSink.h>

#include <drx3D/Sys/ISystem.h>
#include <stack>

typedef std::map<string, XmlNodeRef> IdTable;
struct SParseParams
{
	IdTable    idTable;
	XmlNodeRef useAlways;
	bool       strict;

	SParseParams()
	{
		strict = true;
	}
};

static XmlNodeRef Clone(XmlNodeRef source);
static void       CopyAttributes(const XmlNodeRef& source, XmlNodeRef& dest);
static bool       IsOptionalReadXML(const SParseParams& parseParams, XmlNodeRef& definition);
static bool       CheckEnum(const SParseParams& parseParams, tukk name, XmlNodeRef& definition, XmlNodeRef& data);

static bool       LoadTableInner(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink);
static bool       LoadArray(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink);
static bool       LoadProperty(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink);
static bool       LoadTable(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink);
static bool       LoadReferencedId(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink);
static bool       LoadSomething(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink);
static bool       LoadArraySetValueTable(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink, i32 elem);

typedef bool (* LoadArraySetValue)(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink, i32 elem);
typedef bool (* LoadDefinitionFunction)(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink);

template<class T> struct ReadPropertyTyped;

template<class T>
struct ReadPropertyTyped
{
	static bool Load(const SParseParams& parseParams, tukk name, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink)
	{
		T value;
		memset(&value, 0, sizeof(T));

		if (!pSink->IsCreationMode())
		{
			if (!data->haveAttr(name))
				return false;
			if (!data->getAttr(name, value))
				return false;
			if (!CheckEnum(parseParams, name, definition, data))
				return false;
		}

		IReadXMLSink::TValue vvalue(value);
		pSink->SetValue(name, vvalue, definition);
		return true;
	}
	static bool LoadArray(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink, i32 elem)
	{
		T value;
		memset(&value, 0, sizeof(T));

		if (!pSink->IsCreationMode())
		{
			if (!data->haveAttr("value"))
				return false;
			if (!data->getAttr("value", value))
				return false;
		}

		IReadXMLSink::TValue vvalue(value);
		pSink->SetAt(elem, vvalue, definition);
		return true;
	}
};

template<>
struct ReadPropertyTyped<string>
{
	static bool Load(const SParseParams& parseParams, tukk name, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink)
	{
		tukk value = 0;

		if (!pSink->IsCreationMode())
		{
			if (!data->haveAttr(name))
				return false;
			if (!CheckEnum(parseParams, name, definition, data))
				return false;

			value = data->getAttr(name);
		}

		IReadXMLSink::TValue vvalue(value);
		pSink->SetValue(name, vvalue, definition);
		return true;
	}
	static bool LoadArray(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink, i32 elem)
	{
		tukk value = 0;

		if (!pSink->IsCreationMode())
		{
			if (!data->haveAttr("value"))
				return false;

			value = data->getAttr("value");
		}

		IReadXMLSink::TValue vvalue(value);
		pSink->SetAt(elem, vvalue, definition);
		return true;
	}
};

XmlNodeRef Clone(XmlNodeRef source)
{
	assert(source != (IXmlNode*)NULL);

	// Can't use clone() on XmlNodeRef objects since they can contain a CXMLBinaryNode, which doesn't support
	// clone(). Instead we just create a regular xml node and manually copy content, tag, attributes and children
	XmlNodeRef cloned = GetISystem()->CreateXmlNode(source->getTag());
	cloned->setContent(source->getContent());
	CopyAttributes(source, cloned);
	i32k iChildCount = source->getChildCount();
	for (i32 i = 0; i < iChildCount; ++i)
	{
		cloned->addChild(Clone(source->getChild(i)));
	}

	return cloned;
}

void CopyAttributes(const XmlNodeRef& source, XmlNodeRef& dest)
{
	// Not as fast as CXmlNode::copyAttributes(), but that method will have undefined behavior if the XmlNodeRef contains
	// a CBinaryXmlNode object
	i32 nNumAttributes = source->getNumAttributes();
	for (i32 i = 0; i < nNumAttributes; ++i)
	{
		tukk key = NULL;
		tukk value = NULL;
		if (source->getAttributeByIndex(i, &key, &value))
			dest->setAttr(key, value);
	}
}

bool IsOptionalReadXML(const SParseParams& parseParams, XmlNodeRef& definition)
{
	// If strict mode is off, then everything is optional
	if (parseParams.strict == false)
		return true;

	bool optional = false;
	definition->getAttr("optional", optional);
	return optional;
}

bool CheckEnum(const SParseParams& parseParams, tukk name, XmlNodeRef& definition, XmlNodeRef& data)
{
	if (XmlNodeRef enumNode = definition->findChild("Enum"))
	{
		// If strict mode is off, then no need to check the enum value
		return true;

		// if restrictive attribute set to false, check always succeeds
		if (enumNode->haveAttr("restrictive"))
		{
			bool res = true;
			enumNode->getAttr("restrictive", res);
			if (!res)
				return true;
		}

		// else check enum values
		tukk val = data->getAttr(name);
		for (i32 i = 0; i < enumNode->getChildCount(); ++i)
		{
			if (0 == strcmp(enumNode->getChild(i)->getContent(), val))
				return true;
		}
		return false;
	}
	return true;
}

bool LoadProperty(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink)
{
	if (strlen(definition->getAttr("deprecated")))
	{
		// skip deprecated properties by not loading them and returning success
		return true;
	}

	tukk name = definition->getAttr("name");
	if (0 == strlen(name))
	{
		DrxLog("Property has no name");
		return false;
	}

	tukk type = definition->getAttr("type");
	if (0 == strlen(type))
	{
		DrxLog("Property '%s' has no type", type);
		return false;
	}

	XmlNodeRef dataToRead = data;
	if (!pSink->IsCreationMode())
	{
		// This check is done so the data xml can specify child elements instead of attributes if desired,
		// since the rest of the code assume only attributes
		if (XmlNodeRef childRef = data->findChild(name))
		{
			if (data->haveAttr(name))
			{
				DrxLog("Duplicate definition (attribute and element) for %s", name);
				return false;
			}
			if (childRef->getChildCount())
			{
				DrxLog("Property-style elements can not have children (property was %s)", name);
				return false;
			}

			dataToRead = GetISystem()->CreateXmlNode(data->getTag());

			string content = childRef->getContent();
			dataToRead->setAttr(name, content.Trim().c_str());
		}

		if (!dataToRead->haveAttr(name))
		{
			if (!IsOptionalReadXML(parseParams, definition))
			{
				DrxLog("Failed to load property %s", name);
				return false;
			}
			return true;
		}
	}

	bool ok = false;
#define LOAD_PROPERTY(whichType) else if (0 == strcmp(type, # whichType)) \
  ok = ReadPropertyTyped<whichType>::Load(parseParams, name, definition, dataToRead, pSink)
	XML_SET_PROPERTY_HELPER(LOAD_PROPERTY);
#undef LOAD_PROPERTY

	if (!ok)
		DrxLog("Failed loading attribute %s of type %s", name, type);

	return ok;
}

bool LoadArraySetValueTable(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink, i32 elem)
{
	IReadXMLSinkPtr pChildSink = pSink->BeginTableAt(elem, definition);

	if (pSink->IsCreationMode() && definition->haveAttr("type"))
	{
		if (!LoadSomething(parseParams, definition, data, &*pChildSink))
			return false;
	}
	else
	{
		if (!LoadTableInner(parseParams, definition, data, &*pChildSink))
			return false;
	}

	if (!pChildSink->EndTableAt(elem))
	{
		DrxLog("Failed to finish table at element %d", elem);
		return false;
	}

	return true;
}

bool LoadArray(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink)
{
	if (strlen(definition->getAttr("deprecated")))
	{
		// skip deprecated properties by not loading them and returning success
		return true;
	}

	tukk name = definition->getAttr("name");
	if (0 == strlen(name))
	{
		DrxLog("Array has no name");
		return false;
	}

	bool validateArray = true;
	tukk elementName = definition->getAttr("elementName");
	if (0 == strlen(elementName))
	{
		elementName = "element";
		validateArray = false;
	}
	else
	{
		definition->getAttr("validate", validateArray);
	}

	XmlNodeRef childData;

	if (!pSink->IsCreationMode())
	{
		childData = data->findChild(name);
		if (!childData)
		{
			bool ok = IsOptionalReadXML(parseParams, definition);
			if (!ok)
				DrxLog("Failed to load child table %s", name);
			return ok;
		}
	}

	IReadXMLSinkPtr childSink = pSink->BeginArray(name, definition);
	if (!childSink)
	{
		DrxLog("Failed to begin array named %s", name);
		return false;
	}

	LoadArraySetValue setter = NULL;
	if (definition->haveAttr("type"))
	{
		setter = NULL;
		tukk type = definition->getAttr("type");
#define SETTER_PROPERTY(whichType) else if (0 == strcmp(type, # whichType)) \
  setter = ReadPropertyTyped<whichType>::LoadArray
		XML_SET_PROPERTY_HELPER(SETTER_PROPERTY);
#undef SETTER_PROPERTY
		if (!setter)
		{
			DrxLog("Unknown type %s in array %s", type, name);
			return false;
		}
	}
	else
	{
		setter = LoadArraySetValueTable;
	}

	if (!pSink->IsCreationMode())
	{
		i32 numElems = childData->getChildCount();
		i32 elem = 1;
		for (i32 i = 0; i < numElems; i++)
		{
			XmlNodeRef elemData = childData->getChild(i);
			if (0 == strcmp(elemData->getTag(), elementName))
			{
				i32 increment = 1;
				if (elemData->haveAttr("_index"))
				{
					if (!elemData->getAttr("_index", elem))
					{
						DrxLog("_index is not an integer in array %s (pos hint=%d)", name, elem);
						return false;
					}
				}
				if (!setter(parseParams, definition, elemData, &*childSink, elem))
				{
					DrxLog("Failed loading element %d of array %s", elem, name);
					return false;
				}
				elem += increment;
			}
			else if (validateArray)
			{
				DrxLog("Invalid node %s in array %s", elemData->getTag(), name);
				return false;
			}
		}
	}
	else
	{
		// only process array content for the array being created
		if (0 == strcmp(name, pSink->GetCreationNode()->getAttr("name")))
		{
			if (!setter(parseParams, definition, data, &*childSink, 1))
			{
				DrxLog("[ReadXML CreationMode]: Failed loading element %d of array %s", 1, name);
				return false;
			}
		}
	}

	if (!pSink->EndArray(name))
	{
		DrxLog("Failed to finish array named %s", name);
		return false;
	}

	return true;
}

bool LoadTable(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink)
{
	if (strlen(definition->getAttr("deprecated")))
	{
		// skip deprecated properties by not loading them and returning success
		return true;
	}

	tukk name = definition->getAttr("name");
	if (0 == strlen(name))
	{
		DrxLog("Child-table has no name");
		return false;
	}

	XmlNodeRef childData;

	if (!pSink->IsCreationMode())
	{
		childData = data->findChild(name);
		if (!childData)
		{
			bool ok = IsOptionalReadXML(parseParams, definition);
			if (!ok)
				DrxLog("Failed to load child table %s", name);
			return ok;
		}
	}

	IReadXMLSinkPtr childSink = pSink->BeginTable(name, definition);
	if (!childSink)
	{
		DrxLog("Sink creation failed for table %s", name);
		return false;
	}

	if (!LoadTableInner(parseParams, definition, childData, childSink))
	{
		DrxLog("Failed to load data for child table %s", name);
		return false;
	}

	if (!pSink->EndTable(name))
	{
		DrxLog("Table %s failed to complete in sink", name);
		return false;
	}

	return true;
}

bool LoadSomething(const SParseParams& parseParams, XmlNodeRef& nodeDefinition, XmlNodeRef& data, IReadXMLSink* pSink)
{
	// Ignore if it's the useAlways array
	if (parseParams.useAlways == nodeDefinition)
		return true;

	static struct
	{
		tukk            name;
		LoadDefinitionFunction loader;
	} loaderTypes[] = {
		{ "Property", &LoadProperty     },
		{ "Array",    &LoadArray        },
		{ "Table",    &LoadTable        },
		{ "Use",      &LoadReferencedId },
	};
	static i32k numLoaderTypes = sizeof(loaderTypes) / sizeof(*loaderTypes);

	tukk nodeDefinitionTag = nodeDefinition->getTag();
	bool ok = false;
	i32 i;

	for (i = 0; i < numLoaderTypes; i++)
	{
		if (0 == strcmp(loaderTypes[i].name, nodeDefinitionTag))
		{
			ok = loaderTypes[i].loader(parseParams, nodeDefinition, data, pSink);
			break;
		}
	}

	if (0 == stricmp("Settings", nodeDefinitionTag))
		return true;

	if (!ok)
	{
		if (i == numLoaderTypes)
			DrxLog("Invalid definition node type %s, line %d", nodeDefinitionTag, nodeDefinition->getLine());
	}
	return ok;
}

bool LoadReferencedId(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink)
{
	IdTable::const_iterator iter = parseParams.idTable.find(definition->getAttr("id"));
	if (iter == parseParams.idTable.end())
	{
		DrxLog("No definition with id '%s'", definition->getAttr("id"));
		return false;
	}
	XmlNodeRef useDefinition = Clone(iter->second);
	CopyAttributes(definition, useDefinition);

	return LoadSomething(parseParams, useDefinition, data, pSink);
}

bool LoadTableInner(const SParseParams& parseParams, XmlNodeRef& definition, XmlNodeRef& data, IReadXMLSink* pSink)
{
	i32k nChildrenDefinition = definition->getChildCount();

	for (i32 nChildDefinition = 0; nChildDefinition < nChildrenDefinition; nChildDefinition++)
	{
		XmlNodeRef nodeDefinition = definition->getChild(nChildDefinition);

		if (!LoadSomething(parseParams, nodeDefinition, data, pSink))
		{
			LoadSomething(parseParams, nodeDefinition, data, pSink);
			return false;
		}
	}

	tukk tag = definition->getTag();
	if (parseParams.useAlways != (IXmlNode*)NULL)
	{
		assert(!definition->haveAttr("type"));

		i32k nUseAlwaysDefCount = parseParams.useAlways->getChildCount();
		for (i32 i = 0; i < nUseAlwaysDefCount; ++i)
		{
			XmlNodeRef nodeDefinition = parseParams.useAlways->getChild(i);

			// Don't continue loading useAlways nodes in creation mode
			if (!pSink->IsCreationMode())
			{
				if (!LoadSomething(parseParams, nodeDefinition, data, pSink))
					return false;
			}
		}
	}

	return true;
}

bool CReadWriteXMLSink::ReadXML(XmlNodeRef rootDefinition, XmlNodeRef rootData, IReadXMLSink* pSink)
{
	if (!pSink->IsCreationMode())
	{
		if (0 == rootData)
			return false;

		if (0 != strcmp(rootDefinition->getTag(), "Definition"))
		{
			DrxLog("Root tag of definition file was %s; expected Definition", rootDefinition->getTag());
			return false;
		}
		if (rootDefinition->haveAttr("root"))
		{
			if (0 != strcmp(rootDefinition->getAttr("root"), rootData->getTag()))
			{
				DrxLog("Root data has wrong tag; was %s expected %s", rootData->getTag(), rootDefinition->getAttr("root"));
				return false;
			}
		}
	}

	SParseParams parseParams;
	parseParams.useAlways = rootDefinition->findChild("AllowAlways");

	if (XmlNodeRef settingsParams = rootDefinition->findChild("Settings"))
	{
		settingsParams->getAttr("strict", parseParams.strict);
	}

	// scan for id's in the structure (for the Use member)
	std::stack<XmlNodeRef> scanStack;
	scanStack.push(rootDefinition);
	while (!scanStack.empty())
	{
		XmlNodeRef refNode = scanStack.top();
		scanStack.pop();

		i32 numChildren = refNode->getChildCount();
		tukk tag = refNode->getTag();

		for (i32 i = 0; i < numChildren; i++)
		{
			const XmlNodeRef& childNodeRef = refNode->getChild(i);
			if (parseParams.useAlways != childNodeRef)
				scanStack.push(childNodeRef);
		}

		// If the element has an attribute id="" and is not a "<Use>" element add it to the idTable map
		if (refNode->haveAttr("id") && 0 != strcmp("Use", tag))
			parseParams.idTable[refNode->getAttr("id")] = refNode;
	}

	if (pSink->IsCreationMode() && rootDefinition->haveAttr("type"))
	{
		// if creating from a 0-child definition node, load itself
		if (!LoadSomething(parseParams, rootDefinition, rootData, pSink))
			return false;
	}
	else
	{
		// load content
		if (!LoadTableInner(parseParams, rootDefinition, rootData, pSink))
			return false;
	}

	bool ok = pSink->Complete();
	if (!ok)
		DrxLog("Warning: sink failed to complete reading");

	return ok;
}

bool CReadWriteXMLSink::ReadXML(XmlNodeRef definition, tukk dataFile, IReadXMLSink* pSink)
{
	XmlNodeRef rootData = GetISystem()->LoadXmlFromFile(dataFile);
	if (!rootData)
	{
		DrxLog("Unable to load XML-Lua data file: %s", dataFile);
		return false;
	}
	return ReadXML(definition, rootData, pSink);
}

bool CReadWriteXMLSink::ReadXML(tukk definitionFile, XmlNodeRef rootData, IReadXMLSink* pSink)
{
	XmlNodeRef rootDefinition = GetISystem()->LoadXmlFromFile(definitionFile);
	if (!rootDefinition)
	{
		DrxLog("Unable to load XML-Lua definition file: %s", definitionFile);
		return false;
	}
	return ReadXML(rootDefinition, rootData, pSink);
}

bool CReadWriteXMLSink::ReadXML(tukk definitionFile, tukk dataFile, IReadXMLSink* pSink)
{
	XmlNodeRef rootData = GetISystem()->LoadXmlFromFile(dataFile);
	if (!rootData)
	{
		DrxLog("Unable to load XML-Lua data file: %s", dataFile);
		return false;
	}
	if (!ReadXML(definitionFile, rootData, pSink))
	{
		DrxLog("Unable to load file %s", dataFile);
		return false;
	}
	return true;
}
