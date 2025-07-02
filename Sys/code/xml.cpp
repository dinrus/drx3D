// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>

//#define _CRT_SECURE_NO_DEPRECATE 1
//#define _CRT_NONSTDC_NO_DEPRECATE
#include <stdlib.h>
#include <cctype>

#pragma warning(disable : 6031) // Return value ignored: 'sscanf'

#include <expat.h>
#include <drx3D/Sys/xml.h>
#include <algorithm>
#include <stdio.h>
#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Sys/XMLBinaryReader.h>

#define FLOAT_FMT  "%.8g"
#define DOUBLE_FMT "%.17g"

#include <drx3D/Sys/SimpleStringPool.h>

// Global counter for memory allocated in XML string pools.
size_t CSimpleStringPool::g_nTotalAllocInXmlStringPools = 0;

//////////////////////////////////////////////////////////////////////////
static i32 __cdecl ascii_stricmp(tukk dst, tukk src)
{
	i32 f, l;
	do
	{
		if (((f = (u8)(*(dst++))) >= 'A') && (f <= 'Z'))
			f -= 'A' - 'a';
		if (((l = (u8)(*(src++))) >= 'A') && (l <= 'Z'))
			l -= 'A' - 'a';
	}
	while (f && (f == l));
	return(f - l);
}

//////////////////////////////////////////////////////////////////////////
XmlStrCmpFunc g_pXmlStrCmp = &ascii_stricmp;
bool g_bEnableBinaryXmlLoading = true;

//////////////////////////////////////////////////////////////////////////
class CXmlStringData : public IXmlStringData
{
public:
	i32       m_nRefCount;
	XmlString m_string;

	CXmlStringData() { m_nRefCount = 0; }
	virtual void        AddRef()          { ++m_nRefCount; }
	virtual void        Release()         { if (--m_nRefCount <= 0) delete this; }

	virtual tukk GetString()       { return m_string.c_str(); };
	virtual size_t      GetStringLength() { return m_string.size(); };
};

//////////////////////////////////////////////////////////////////////////
class CXmlStringPool : public IXmlStringPool
{
public:
	explicit CXmlStringPool(bool bReuseStrings) : m_stringPool(bReuseStrings) {}

	tukk AddString(tukk str)            { return m_stringPool.Append(str, (i32)strlen(str)); }
	void        Clear()                               { m_stringPool.Clear(); }
	void        SetBlockSize(u32 nBlockSize) { m_stringPool.SetBlockSize(nBlockSize); }

	void        GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(m_stringPool);
	}
private:
	CSimpleStringPool m_stringPool;
};

//xml_node_allocator XmlDynArrayAlloc::allocator;
//size_t XmlDynArrayAlloc::m_iAllocated = 0;
/**
 ******************************************************************************
 * CXmlNode implementation.
 ******************************************************************************
 */

void CXmlNode::DeleteThis()
{
	delete this;
}

CXmlNode::~CXmlNode()
{
	m_nRefCount = 1;    // removeAllChildsImpl can make an XmlNodeRef to this node whilst it is deleting the children
	                    // doing this will cause the ref count to be increment and decremented and cause delete to be called again,
	                    // leading to a recursion crash. upping the ref count here once destruction has started avoids this problem
	removeAllChildsImpl();

	SAFE_DELETE(m_pAttributes);

	m_pStringPool->Release();
}

CXmlNode::CXmlNode()
	: m_pStringPool(NULL) // must be changed later.
	, m_tag("")
	, m_content("")
	, m_parent(NULL)
	, m_pChilds(NULL)
	, m_pAttributes(NULL)
	, m_line(0)
{
	m_nRefCount = 0; //TODO: move initialization to IXmlNode constructor
}

CXmlNode::CXmlNode(tukk tag, bool bReuseStrings)
	: m_content("")
	, m_parent(NULL)
	, m_pChilds(NULL)
	, m_pAttributes(NULL)
	, m_line(0)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "XML");
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "New node (constructor)");
	m_nRefCount = 0; //TODO: move initialization to IXmlNode constructor

	m_pStringPool = new CXmlStringPool(bReuseStrings);
	m_pStringPool->AddRef();
	m_tag = m_pStringPool->AddString(tag);
}

//////////////////////////////////////////////////////////////////////////
// collect allocated memory  informations
void CXmlNode::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_pStringPool);

	if (m_pChilds)
	{
		pSizer->AddObject(*m_pChilds);
	}
	if (m_pAttributes)
	{
		pSizer->AddContainer(*m_pAttributes);
	}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CXmlNode::createNode(tukk tag)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "XML");
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "New node (createNode)");

	CXmlNode* pNewNode;
	{
		MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Node construction");
		pNewNode = new CXmlNode;
	}
	pNewNode->m_pStringPool = m_pStringPool;
	m_pStringPool->AddRef();
	pNewNode->m_tag = m_pStringPool->AddString(tag);
	return XmlNodeRef(pNewNode);
}

//////////////////////////////////////////////////////////////////////////
void CXmlNode::setTag(tukk tag)
{
	m_tag = m_pStringPool->AddString(tag);
}

//////////////////////////////////////////////////////////////////////////
void CXmlNode::setContent(tukk str)
{
	m_content = m_pStringPool->AddString(str);
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::isTag(tukk tag) const
{
	return g_pXmlStrCmp(tag, m_tag) == 0;
}

tukk CXmlNode::getAttr(tukk key) const
{
	tukk svalue = GetValue(key);
	if (svalue)
		return svalue;
	return "";
}

bool CXmlNode::getAttr(tukk key, tukk* value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		*value = svalue;
		return true;
	}
	else
	{
		*value = "";
		return false;
	}
}

bool CXmlNode::haveAttr(tukk key) const
{
	if (m_pAttributes)
	{
		XmlAttrConstIter it = GetAttrConstIterator(key);
		if (it != m_pAttributes->end())
		{
			return true;
		}
	}
	return false;
}

void CXmlNode::delAttr(tukk key)
{
	if (m_pAttributes)
	{
		XmlAttrIter it = GetAttrIterator(key);
		if (it != m_pAttributes->end())
		{
			m_pAttributes->erase(it);
		}
	}
}

void CXmlNode::removeAllAttributes()
{
	if (m_pAttributes)
	{
		m_pAttributes->clear();
		SAFE_DELETE(m_pAttributes);
	}
}

void CXmlNode::setAttr(tukk key, tukk value)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "XML");
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "setAttr");

	if (!m_pAttributes)
	{
		m_pAttributes = new XmlAttributes;
	}
	assert(m_pAttributes);

	XmlAttrIter it = GetAttrIterator(key);
	if (it == m_pAttributes->end())
	{
		XmlAttribute tempAttr;
		tempAttr.key = m_pStringPool->AddString(key);
		tempAttr.value = m_pStringPool->AddString(value);
		m_pAttributes->push_back(tempAttr);
		// Sort attributes.
		//std::sort( m_pAttributes->begin(),m_pAttributes->end() );
	}
	else
	{
		// If already exist, overide this member.
		it->value = m_pStringPool->AddString(value);
	}
}

void CXmlNode::setAttr(tukk key, i32 value)
{
	char str[128];
	itoa(value, str, 10);
	setAttr(key, str);
}

void CXmlNode::setAttr(tukk key, u32 value)
{
	char str[128];
	_ui64toa(value, str, 10);
	setAttr(key, str);
}

void CXmlNode::setAttr(tukk key, float value)
{
	char str[128];
	drx_sprintf(str, FLOAT_FMT, value);
	setAttr(key, str);
}

void CXmlNode::setAttr(tukk key, double value)
{
	char str[128];
	drx_sprintf(str, DOUBLE_FMT, value);
	setAttr(key, str);
}

//////////////////////////////////////////////////////////////////////////
void CXmlNode::setAttr(tukk key, int64 value)
{
	char str[32];
	drx_sprintf(str, "%" PRId64, value);
	setAttr(key, str);
}

//////////////////////////////////////////////////////////////////////////
void CXmlNode::setAttr(tukk key, uint64 value, bool useHexFormat)
{
	char str[32];
	if (useHexFormat)
		drx_sprintf(str, "%" PRIX64, value);
	else
		drx_sprintf(str, "%" PRIu64, value);
	setAttr(key, str);
}

void CXmlNode::setAttr(tukk key, const Ang3& value)
{
	char str[128];
	drx_sprintf(str, FLOAT_FMT "," FLOAT_FMT "," FLOAT_FMT, value.x, value.y, value.z);
	setAttr(key, str);
}
void CXmlNode::setAttr(tukk key, const Vec3& value)
{
	char str[128];
	drx_sprintf(str, FLOAT_FMT "," FLOAT_FMT "," FLOAT_FMT, value.x, value.y, value.z);
	setAttr(key, str);
}
void CXmlNode::setAttr(tukk key, const Vec4& value)
{
	char str[128];
	drx_sprintf(str, FLOAT_FMT "," FLOAT_FMT "," FLOAT_FMT "," FLOAT_FMT, value.x, value.y, value.z, value.w);
	setAttr(key, str);
}

void CXmlNode::setAttr(tukk key, const Vec3d& value)
{
	char str[128];
	drx_sprintf(str, DOUBLE_FMT "," DOUBLE_FMT "," DOUBLE_FMT, value.x, value.y, value.z);
	setAttr(key, str);
}
void CXmlNode::setAttr(tukk key, const Vec2& value)
{
	char str[128];
	drx_sprintf(str, FLOAT_FMT "," FLOAT_FMT, value.x, value.y);
	setAttr(key, str);
}
void CXmlNode::setAttr(tukk key, const Vec2d& value)
{
	char str[128];
	drx_sprintf(str, DOUBLE_FMT "," DOUBLE_FMT, value.x, value.y);
	setAttr(key, str);
}

void CXmlNode::setAttr(tukk key, const Quat& value)
{
	char str[128];
	drx_sprintf(str, FLOAT_FMT "," FLOAT_FMT "," FLOAT_FMT "," FLOAT_FMT, value.w, value.v.x, value.v.y, value.v.z);
	setAttr(key, str);
}

void CXmlNode::setAttr(tukk key, const DrxGUID& value)
{
	setAttr(key, value.ToString());
}

bool CXmlNode::getAttr(tukk key, DrxGUID& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		tukk guidStr = getAttr(key);
		value = DrxGUID::FromString(svalue);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttr(tukk key, i32& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		value = atoi(svalue);
		return true;
	}
	return false;
}

bool CXmlNode::getAttr(tukk key, u32& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		value = strtoul(svalue, NULL, 10);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttr(tukk key, int64& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		sscanf(svalue, "%" PRId64, &value);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttr(tukk key, uint64& value, bool useHexFormat) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		if (useHexFormat)
			sscanf(svalue, "%" PRIX64, &value);
		else
			sscanf(svalue, "%" PRIu64, &value);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttr(tukk key, bool& value) const
{
	bool isSuccess = false;
	char const* const szValue = GetValue(key);

	if (szValue != nullptr && szValue[0] != '\0')
	{
		if (std::isalpha(*szValue) != 0)
		{
			if (g_pXmlStrCmp(szValue, "false") == 0)
			{
				value = false;
				isSuccess = true;
			}
			else if (g_pXmlStrCmp(szValue, "true") == 0)
			{
				value = true;
				isSuccess = true;
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "Encountered invalid value during CXmlNode::getAttr! Value: %s Tag: %s", szValue, m_tag);
			}
		}
		else
		{
			i32 const number = std::atoi(szValue);

			if (number == 0)
			{
				value = false;
				isSuccess = true;
			}
			else if (number == 1)
			{
				value = true;
				isSuccess = true;
			}
			else
			{
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "Encountered invalid value during CXmlNode::getAttr! Value: %s Tag: %s", szValue, m_tag);
			}
		}
	}

	return isSuccess;
}

bool CXmlNode::getAttr(tukk key, float& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		value = (float)atof(svalue);
		return true;
	}
	return false;
}

bool CXmlNode::getAttr(tukk key, double& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		value = atof(svalue);
		return true;
	}
	return false;
}

bool CXmlNode::getAttr(tukk key, Ang3& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		float x, y, z;
		if (sscanf(svalue, "%f,%f,%f", &x, &y, &z) == 3)
		{
			value(x, y, z);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttr(tukk key, Vec3& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		float x, y, z;
		if (sscanf(svalue, "%f,%f,%f", &x, &y, &z) == 3)
		{
			value = Vec3(x, y, z);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttr(tukk key, Vec4& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		float x, y, z, w;
		if (sscanf(svalue, "%f,%f,%f,%f", &x, &y, &z, &w) == 4)
		{
			value = Vec4(x, y, z, w);
			return true;
		}
	}

	return false;
}

bool CXmlNode::getAttr(tukk key, Vec3d& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		double x, y, z;
		if (sscanf(svalue, "%lf,%lf,%lf", &x, &y, &z) == 3)
		{
			value = Vec3d(x, y, z);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttr(tukk key, Vec2& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		float x, y;
		if (sscanf(svalue, "%f,%f", &x, &y) == 2)
		{
			value = Vec2(x, y);
			return true;
		}
	}
	return false;
}

bool CXmlNode::getAttr(tukk key, Vec2d& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		double x, y;
		if (sscanf(svalue, "%lf,%lf", &x, &y) == 2)
		{
			value = Vec2d(x, y);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttr(tukk key, Quat& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		float w, x, y, z;
		if (sscanf(svalue, "%f,%f,%f,%f", &w, &x, &y, &z) == 4)
		{
			if (fabs(w) > VEC_EPSILON || fabs(x) > VEC_EPSILON || fabs(y) > VEC_EPSILON || fabs(z) > VEC_EPSILON)
			{
				//[AlexMcC|02.03.10] directly assign to members to avoid triggering the assert in Quat() with data from bad assets
				value.w = w;
				value.v = Vec3(x, y, z);
				return value.IsValid();
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttr(tukk key, ColorB& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		u32 r, g, b, a = 255;
		i32 numFound = sscanf(svalue, "%u,%u,%u,%u", &r, &g, &b, &a);
		if (numFound == 3 || numFound == 4)
		{
			// If we only found 3 values, a should be unchanged, and still be 255
			if (r < 256 && g < 256 && b < 256 && a < 256)
			{
				value = ColorB(r, g, b, a);
				return true;
			}
		}
	}
	return false;
}

XmlNodeRef CXmlNode::findChild(tukk tag) const
{
	if (m_pChilds)
	{
		XmlNodes& childs = *m_pChilds;
		for (i32 i = 0, num = childs.size(); i < num; ++i)
		{
			if (childs[i]->isTag(tag))
				return childs[i];
		}
	}
	return 0;
}

void CXmlNode::removeChild(const XmlNodeRef& node)
{
	if (m_pChilds)
	{
		XmlNodes::iterator it = std::find(m_pChilds->begin(), m_pChilds->end(), (IXmlNode*)node);
		if (it != m_pChilds->end())
		{
			ReleaseChild(*it);
			m_pChilds->erase(it);
		}
	}
}

void CXmlNode::removeAllChilds()
{
	removeAllChildsImpl();
}

//////////////////////////////////////////////////////////////////////////
void CXmlNode::deleteChild(tukk tag)
{
	if (m_pChilds)
	{
		XmlNodes& childs = *m_pChilds;
		for (i32 i = 0, num = childs.size(); i < num; ++i)
		{
			if (childs[i]->isTag(tag))
			{
				ReleaseChild(childs[i]);
				childs.erase(childs.begin() + i);
				return;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CXmlNode::deleteChildAt(i32 nIndex)
{
	if (m_pChilds)
	{
		XmlNodes& childs = *m_pChilds;
		if (nIndex >= 0 && nIndex < (i32)childs.size())
		{
			ReleaseChild(childs[nIndex]);
			childs.erase(childs.begin() + nIndex);
		}
	}
}

//! Adds new child node.
void CXmlNode::addChild(const XmlNodeRef& node)
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "addChild");
	if (!m_pChilds)
	{
		m_pChilds = new XmlNodes;
	}

	assert(node != 0);
	IXmlNode* pNode = ((IXmlNode*)node);
	pNode->AddRef();
	m_pChilds->push_back(pNode);
	pNode->setParent(this);
};

void CXmlNode::shareChildren(const XmlNodeRef& inFromMe)
{
	i32 numChildren = inFromMe->getChildCount();

	removeAllChilds();

	if (numChildren > 0)
	{
		XmlNodeRef child;

		m_pChilds = new XmlNodes;
		m_pChilds->reserve(numChildren);
		for (i32 i = 0; i < numChildren; i++)
		{
			child = inFromMe->getChild(i);

			child->AddRef();
			// not overwriting parent assignment of child, we share the node but do not exclusively own it
			m_pChilds->push_back(child);
		}
	}
}

void CXmlNode::setParent(const XmlNodeRef& inNewParent)
{
	// note, parent ptrs are not ref counted
	m_parent = inNewParent;
}

void CXmlNode::insertChild(i32 inIndex, const XmlNodeRef& inNewChild)
{
	assert(inIndex >= 0 && inIndex <= getChildCount());
	assert(inNewChild != 0);
	if (inIndex >= 0 && inIndex <= getChildCount() && inNewChild)
	{
		if (getChildCount() == 0)
		{
			addChild(inNewChild);
		}
		else
		{
			IXmlNode* pNode = ((IXmlNode*)inNewChild);
			pNode->AddRef();
			m_pChilds->insert(m_pChilds->begin() + inIndex, pNode);
			pNode->setParent(this);
		}
	}
}

void CXmlNode::replaceChild(i32 inIndex, const XmlNodeRef& inNewChild)
{
	assert(inIndex >= 0 && inIndex < getChildCount());
	assert(inNewChild != 0);
	if (inIndex >= 0 && inIndex < getChildCount() && inNewChild)
	{
		IXmlNode* wasChild = (*m_pChilds)[inIndex];

		if (wasChild->getParent() == this)
		{
			wasChild->setParent(XmlNodeRef());      // child is orphaned, will be freed by Release() below if this parent is last holding a reference to it
		}
		wasChild->Release();
		inNewChild->AddRef();
		(*m_pChilds)[inIndex] = inNewChild;
		inNewChild->setParent(this);
	}
}

XmlNodeRef CXmlNode::newChild(tukk tagName)
{
	XmlNodeRef node = createNode(tagName);
	addChild(node);
	return node;
}

//! Get XML Node child nodes.
XmlNodeRef CXmlNode::getChild(i32 i) const
{
	assert(m_pChilds);
	XmlNodes& childs = *m_pChilds;
	assert(i >= 0 && i < (i32)childs.size());
	return childs[i];
}

//////////////////////////////////////////////////////////////////////////
void CXmlNode::copyAttributes(XmlNodeRef fromNode)
{
	IXmlNode* inode = fromNode;
	CXmlNode* n = (CXmlNode*)inode;
	assert(n);
	PREFAST_ASSUME(n);

	if (n != this)
	{
		if (n->m_pAttributes)
		{
			if (!m_pAttributes)
			{
				m_pAttributes = new XmlAttributes;
			}

			if (n->m_pStringPool == m_pStringPool)
				*m_pAttributes = *(n->m_pAttributes);
			else
			{
				XmlAttributes& lhs = *m_pAttributes;
				const XmlAttributes& rhs = *(n->m_pAttributes);
				lhs.resize(rhs.size());
				for (size_t i = 0; i < rhs.size(); ++i)
				{
					lhs[i].key = m_pStringPool->AddString(rhs[i].key);
					lhs[i].value = m_pStringPool->AddString(rhs[i].value);
				}
			}
		}
		else
		{
			SAFE_DELETE(m_pAttributes);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttributeByIndex(i32 index, tukk* key, tukk* value)
{
	if (m_pAttributes)
	{
		XmlAttributes::iterator it = m_pAttributes->begin();
		if (it != m_pAttributes->end())
		{
			std::advance(it, index);
			if (it != m_pAttributes->end())
			{
				*key = it->key;
				*value = it->value;
				return true;
			}
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool CXmlNode::getAttributeByIndex(i32 index, XmlString& key, XmlString& value)
{
	if (m_pAttributes)
	{
		XmlAttributes::iterator it = m_pAttributes->begin();
		if (it != m_pAttributes->end())
		{
			std::advance(it, index);
			if (it != m_pAttributes->end())
			{
				key = it->key;
				value = it->value;
				return true;
			}
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
XmlNodeRef CXmlNode::clone()
{
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "XML");
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "clone");
	CXmlNode* node = new CXmlNode;
	XmlNodeRef result(node);
	node->m_pStringPool = m_pStringPool;
	m_pStringPool->AddRef();
	node->m_tag = m_tag;
	node->m_content = m_content;
	// Clone attributes.
	CXmlNode* n = (CXmlNode*)(IXmlNode*)node;
	n->copyAttributes(this);
	// Clone sub nodes.
	MEMSTAT_CONTEXT(EMemStatContextTypes::MSC_Other, 0, "Clone children");

	if (m_pChilds)
	{
		const XmlNodes& childs = *m_pChilds;

		node->m_pChilds = new XmlNodes;
		node->m_pChilds->reserve(childs.size());
		for (i32 i = 0, num = childs.size(); i < num; ++i)
		{
			node->addChild(childs[i]->clone());
		}
	}

	return result;
}

//////////////////////////////////////////////////////////////////////////
static void AddTabsToString(XmlString& xml, i32 level)
{
	static tukk tabs[] = {
		"",
		" ",
		"  ",
		"   ",
		"    ",
		"     ",
		"      ",
		"       ",
		"        ",
		"         ",
		"          ",
		"           ",
	};
	// Add tabs.
	if (level < DRX_ARRAY_COUNT(tabs))
	{
		xml += tabs[level];
	}
	else
	{
		for (i32 i = 0; i < level; i++)
			xml += "  ";
	}
}

//////////////////////////////////////////////////////////////////////////
bool CXmlNode::IsValidXmlString(tukk str) const
{
	const size_t len = strlen(str);
	return strcspn(str, "\"\'&><\n") == len;
}

//////////////////////////////////////////////////////////////////////////
XmlString CXmlNode::MakeValidXmlString(const XmlString& instr) const
{
	XmlString str = instr;

	// check if str contains any invalid characters
	str.replace("&", "&amp;");
	str.replace("\"", "&quot;");
	str.replace("\'", "&apos;");
	str.replace("<", "&lt;");
	str.replace(">", "&gt;");
	str.replace("...", "&gt;");
	str.replace("\n", "&#10;");

	return str;
}

void CXmlNode::ReleaseChild(IXmlNode* pChild)
{
	if (pChild)
	{
		if (pChild->getParent() == this)    // if check to handle shared children which are supported by the CXmlNode impl
		{
			pChild->setParent(NULL);
		}
		pChild->Release();
	}
}

void CXmlNode::removeAllChildsImpl()
{
	if (m_pChilds)
	{
		for (XmlNodes::iterator iter = m_pChilds->begin(), endIter = m_pChilds->end(); iter != endIter; ++iter)
		{
			ReleaseChild(*iter);
		}
		SAFE_DELETE(m_pChilds);
	}
}

//////////////////////////////////////////////////////////////////////////
void CXmlNode::AddToXmlString(XmlString& xml, i32 level, FILE* pFile, IPlatformOS::ISaveWriterPtr pSaveWriter, size_t chunkSize) const
{
	if ((pFile || pSaveWriter) && chunkSize > 0)
	{
		size_t len = xml.length();
		if (len >= chunkSize)
		{
			if (pSaveWriter)
			{
				pSaveWriter->AppendBytes(xml.c_str(), len);
			}
			else
			{
				gEnv->pDrxPak->FWrite(xml.c_str(), len, 1, pFile);
			}
			xml.assign("");  // should not free memory and does not!
		}
	}

	AddTabsToString(xml, level);

	const bool bHasChildren = (m_pChilds && !m_pChilds->empty());

	// Begin Tag
	if (!m_pAttributes || m_pAttributes->empty())
	{
		xml += "<";
		xml += m_tag;
		if (*m_content == 0 && !bHasChildren)
		{
			// Compact tag form.
			xml += " />\n";
			return;
		}
		xml += ">";
	}
	else
	{
		xml += "<";
		xml += m_tag;
		xml += " ";

		// Put attributes.
		for (XmlAttributes::const_iterator it = m_pAttributes->begin(); it != m_pAttributes->end(); )
		{
			xml += it->key;
			xml += "=\"";
			if (IsValidXmlString(it->value))
				xml += it->value;
			else
				xml += MakeValidXmlString(it->value);
			++it;
			if (it != m_pAttributes->end())
				xml += "\" ";
			else
				xml += "\"";
		}
		if (*m_content == 0 && !bHasChildren)
		{
			// Compact tag form.
			xml += "/>\n";
			return;
		}
		xml += ">";
	}

	// Put node content.
	if (IsValidXmlString(m_content))
		xml += m_content;
	else
		xml += MakeValidXmlString(m_content);

	if (!bHasChildren)
	{
		xml += "</";
		xml += m_tag;
		xml += ">\n";
		return;
	}

	xml += "\n";

	// Add sub nodes.
	for (XmlNodes::iterator it = m_pChilds->begin(), itEnd = m_pChilds->end(); it != itEnd; ++it)
	{
		IXmlNode* node = *it;
		((CXmlNode*)node)->AddToXmlString(xml, level + 1, pFile, pSaveWriter, chunkSize);
	}

	// Add tabs.
	AddTabsToString(xml, level);
	xml += "</";
	xml += m_tag;
	xml += ">\n";
}

inline static tuk drx_stpcpy(tuk dst, tukk src)
{
	while (src[0])
	{
		dst[0] = src[0];
		dst++;
		src++;
	}
	dst[0] = 0;
	return dst;
}

tuk CXmlNode::AddToXmlStringUnsafe(tuk xml, i32 level, tuk endPtr, FILE* pFile, IPlatformOS::ISaveWriterPtr pSaveWriter, size_t chunkSize) const
{
	const bool bHasChildren = (m_pChilds && !m_pChilds->empty());

	for (i32 i = 0; i < level; i++)
	{
		*(xml++) = ' ';
		*(xml++) = ' ';
	}

	// Begin Tag
	if (!m_pAttributes || m_pAttributes->empty())
	{
		*(xml++) = '<';
		xml = drx_stpcpy(xml, m_tag);
		if (*m_content == 0 && !bHasChildren)
		{
			*(xml++) = '/';
			*(xml++) = '>';
			*(xml++) = '\n';
			return xml;
		}
		*(xml++) = '>';
	}
	else
	{
		*(xml++) = '<';
		xml = drx_stpcpy(xml, m_tag);
		*(xml++) = ' ';

		// Put attributes.
		for (XmlAttributes::const_iterator it = m_pAttributes->begin(); it != m_pAttributes->end(); )
		{
			xml = drx_stpcpy(xml, it->key);
			*(xml++) = '=';
			*(xml++) = '\"';
#ifndef _RELEASE
			if (it->value[strcspn(it->value, "\"\'&><")])
			{
				__debugbreak();
			}
#endif
			xml = drx_stpcpy(xml, it->value);
			++it;
			*(xml++) = '\"';
			if (it != m_pAttributes->end())
			{
				*(xml++) = ' ';
			}
		}
		if (*m_content == 0 && !bHasChildren)
		{
			// Compact tag form.
			*(xml++) = '/';
			*(xml++) = '>';
			*(xml++) = '\n';
			return xml;
		}
		*(xml++) = '>';
	}

#ifndef _RELEASE
	if (m_content[strcspn(m_content, "\"\'&><")])
	{
		__debugbreak();
	}
#endif
	xml = drx_stpcpy(xml, m_content);

	if (!bHasChildren)
	{
		*(xml++) = '<';
		*(xml++) = '/';
		xml = drx_stpcpy(xml, m_tag);
		*(xml++) = '>';
		*(xml++) = '\n';
		return xml;
	}

	*(xml++) = '\n';

	// Add sub nodes.
	for (XmlNodes::iterator it = m_pChilds->begin(), itEnd = m_pChilds->end(); it != itEnd; ++it)
	{
		IXmlNode* node = *it;
		xml = ((CXmlNode*)node)->AddToXmlStringUnsafe(xml, level + 1, endPtr, pFile, pSaveWriter, chunkSize);
	}

	for (i32 i = 0; i < level; i++)
	{
		*(xml++) = ' ';
		*(xml++) = ' ';
	}
	*(xml++) = '<';
	*(xml++) = '/';
	xml = drx_stpcpy(xml, m_tag);
	*(xml++) = '>';
	*(xml++) = '\n';

	assert(xml < endPtr);

	return xml;
}

//////////////////////////////////////////////////////////////////////////
IXmlStringData* CXmlNode::getXMLData(i32 nReserveMem) const
{
	CXmlStringData* pStrData = new CXmlStringData;
	pStrData->m_string.reserve(nReserveMem);
	AddToXmlString(pStrData->m_string, 0);
	return pStrData;
}

//////////////////////////////////////////////////////////////////////////
XmlString CXmlNode::getXML(i32 level) const
{
	XmlString xml;
	xml = "";
	xml.reserve(1024);

	AddToXmlString(xml, level);
	return xml;
}

XmlString CXmlNode::getXMLUnsafe(i32 level, tuk tmpBuffer, u32 sizeOfTmpBuffer) const
{
	tuk endPtr = tmpBuffer + sizeOfTmpBuffer - 1;
	tuk endOfBuffer = AddToXmlStringUnsafe(tmpBuffer, level, endPtr);
	endOfBuffer[0] = '\0';
	XmlString ret(tmpBuffer);
	return ret;
}

// TODO: those 2 saving functions are a bit messy. should probably make a separate one for the use of PlatformAPI
bool CXmlNode::saveToFile(tukk fileName)
{
	const size_t chunkSizeBytes = (15 * 1024);
	IPlatformOS* pPlatformOS = gEnv->pSystem->GetPlatformOS();

	if (pPlatformOS->UsePlatformSavingAPI())
	{
#if DRX_PLATFORM_WINDOWS
		DrxSetFileAttributes(fileName, 0x00000080); // FILE_ATTRIBUTE_NORMAL
#endif

		IPlatformOS::ISaveWriterPtr pSaveWriter;
		pSaveWriter = pPlatformOS->SaveGetWriter(fileName);

		if (!pSaveWriter)
			return false;

		XmlString xml;
		xml.assign("");
		xml.reserve(chunkSizeBytes * 2);
		xml = getXML();

		size_t len = xml.length();
		if (len > 0)
		{
			if (pSaveWriter)
			{
				pSaveWriter->AppendBytes(xml.c_str(), len);
			}
		}
		xml.clear();

		return (len > 0);
	}
	else
	{
		FILE* file = gEnv->pDrxPak->FOpen(fileName, "wt");
		if (file)
		{
#if DRX_PLATFORM_WINDOWS
			XmlString xml = getXML();   // this would not work in consoles because the size limits in strings
			tukk sxml = (tukk)xml;
			gEnv->pDrxPak->FWrite(sxml, xml.length(), file);
			gEnv->pDrxPak->FClose(file);
			return true;
#else
			bool ret = saveToFile(fileName, chunkSizeBytes, file);
			gEnv->pDrxPak->FClose(file);
			return ret;
#endif
		}
		return false;
	}
}

bool CXmlNode::saveToFile(tukk fileName, size_t chunkSize, FILE* pFile)
{
#if DRX_PLATFORM_WINDOWS
	DrxSetFileAttributes(fileName, 0x00000080);  // FILE_ATTRIBUTE_NORMAL
#endif

	if (chunkSize < 256 * 1024)   // make at least 256k
		chunkSize = 256 * 1024;

	XmlString xml;
	xml.assign("");
	xml.reserve(chunkSize * 2); // we reserve double memory, as writing in chunks is not really writing in fixed blocks but a bit fuzzy
	IDrxPak* pDrxPak = gEnv->pDrxPak;
	IPlatformOS::ISaveWriterPtr pSaveWriter;
	if (!pFile)
	{
		pSaveWriter = gEnv->pSystem->GetPlatformOS()->SaveGetWriter(fileName);
	}
	if (!pFile && !pSaveWriter)
		return false;
	AddToXmlString(xml, 0, pFile, pSaveWriter, chunkSize);
	size_t len = xml.length();
	if (len > 0)
	{
		if (pSaveWriter)
		{
			pSaveWriter->AppendBytes(xml.c_str(), len);
		}
		else
		{
			pDrxPak->FWrite(xml.c_str(), len, 1, pFile);
		}
	}
	xml.clear(); // xml.resize(0) would not reclaim memory

	if (pSaveWriter)
		pSaveWriter->Close();

	return true;
}

/**
 ******************************************************************************
 * XmlParserImp class.
 ******************************************************************************
 */
class XmlParserImp : public IXmlStringPool
{
public:
	explicit XmlParserImp(bool bReuseStrings);
	~XmlParserImp();

	void       ParseBegin(bool bCleanPools);
	XmlNodeRef ParseFile(tukk filename, XmlString& errorString, bool bCleanPools);
	XmlNodeRef ParseBuffer(tukk buffer, size_t bufLen, XmlString& errorString, bool bCleanPools);
	void       ParseEnd();

	// Add new string to pool.
	tukk AddString(tukk str) { return m_stringPool.Append(str, (i32)strlen(str)); }
	//tuk AddString( tukk str ) { return (tuk)str; }

	void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddObject(m_stringPool);
		pSizer->AddObject(m_nodeStack);
	}
protected:
	void        onStartElement(tukk tagName, tukk* atts);
	void        onEndElement(tukk tagName);
	void        onRawData(tukk data);

	static void startElement(uk userData, tukk name, tukk* atts)
	{
		((XmlParserImp*)userData)->onStartElement(name, atts);
	}
	static void endElement(uk userData, tukk name)
	{
		((XmlParserImp*)userData)->onEndElement(name);
	}
	static void characterData(uk userData, tukk s, i32 len) PREFAST_SUPPRESS_WARNING(6262)
	{
		char str[32768];

		if (len > sizeof(str) - 1)
		{
			DrxFatalError("XML Parser buffer too small in \'characterData\' function. (%s)", s);
		}

		// Note that XML buffer userData has no terminating '\0'.
		memcpy(str, s, len);
		str[len] = '\0';
		((XmlParserImp*)userData)->onRawData(str);
	}

	void CleanStack();

	struct SStackEntity
	{
		XmlNodeRef             node;
		std::vector<IXmlNode*> childs; //TODO: is it worth lazily initializing this, like CXmlNode::m_pChilds?

		void                   GetMemoryUsage(IDrxSizer* pSizer) const
		{
			pSizer->AddObject(node);
			pSizer->AddObject(childs);
		}
	};

	// First node will become root node.
	std::vector<SStackEntity> m_nodeStack;
	i32                       m_nNodeStackTop;

	XmlNodeRef                m_root;

	XML_Parser                m_parser;
	CSimpleStringPool         m_stringPool;
};

//////////////////////////////////////////////////////////////////////////
void XmlParserImp::CleanStack()
{
	m_nNodeStackTop = 0;
	for (i32 i = 0, num = m_nodeStack.size(); i < num; i++)
	{
		m_nodeStack[i].node = 0;
		m_nodeStack[i].childs.resize(0);
	}
}

/**
 ******************************************************************************
 * XmlParserImp
 ******************************************************************************
 */
void XmlParserImp::onStartElement(tukk tagName, tukk* atts)
{
	CXmlNode* pCNode = new CXmlNode;
	pCNode->m_pStringPool = this;
	pCNode->m_pStringPool->AddRef();
	pCNode->m_tag = AddString(tagName);

	XmlNodeRef node = pCNode;

	m_nNodeStackTop++;
	if (m_nNodeStackTop >= (i32)m_nodeStack.size())
		m_nodeStack.resize(m_nodeStack.size() * 2);

	m_nodeStack[m_nNodeStackTop].node = pCNode;
	m_nodeStack[m_nNodeStackTop - 1].childs.push_back(pCNode);

	if (!m_root)
	{
		m_root = node;
	}
	else
	{
		pCNode->m_parent = (IXmlNode*)m_nodeStack[m_nNodeStackTop - 1].node;
		node->AddRef(); // Childs need to be add refed.
	}

	node->setLine(XML_GetCurrentLineNumber((XML_Parser)m_parser));

	// Call start element callback.
	i32 i = 0;
	i32 numAttrs = 0;
	while (atts[i] != 0)
	{
		numAttrs++;
		i += 2;
	}
	if (numAttrs > 0)
	{
		i = 0;
		if (!pCNode->m_pAttributes)
		{
			pCNode->m_pAttributes = new XmlAttributes;
		}

		XmlAttributes& nodeAtts = *(pCNode->m_pAttributes);
		nodeAtts.resize(numAttrs);
		i32 nAttr = 0;
		while (atts[i] != 0)
		{
			nodeAtts[nAttr].key = AddString(atts[i]);
			nodeAtts[nAttr].value = AddString(atts[i + 1]);
			nAttr++;
			i += 2;
		}
		// Sort attributes.
		//std::sort( pCNode->m_attributes.begin(),pCNode->m_attributes.end() );
	}
}

void XmlParserImp::onEndElement(tukk tagName)
{
	assert(m_nNodeStackTop > 0);

	if (m_nNodeStackTop > 0)
	{
		// Copy current childs to the parent.
		SStackEntity& entry = m_nodeStack[m_nNodeStackTop];
		CXmlNode* currNode = static_cast<CXmlNode*>(static_cast<IXmlNode*>(entry.node));
		if (!entry.childs.empty())
		{
			if (!currNode->m_pChilds)
			{
				currNode->m_pChilds = new CXmlNode::XmlNodes;
			}
			*currNode->m_pChilds = entry.childs;
		}
		entry.childs.resize(0);
		entry.node = NULL;
	}
	m_nNodeStackTop--;
}

void XmlParserImp::onRawData(tukk data)
{
	assert(m_nNodeStackTop >= 0);
	if (m_nNodeStackTop >= 0)
	{
		size_t len = strlen(data);
		if (len > 0)
		{
			if (strspn(data, "\r\n\t ") != len)
			{
				CXmlNode* node = (CXmlNode*)(IXmlNode*)m_nodeStack[m_nNodeStackTop].node;
				if (*node->m_content != '\0')
				{
					node->m_content = m_stringPool.ReplaceString(node->m_content, data);
				}
				else
					node->m_content = AddString(data);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
XmlParserImp::XmlParserImp(bool bReuseStrings)
	: m_nNodeStackTop(0)
	, m_parser(NULL)
	, m_stringPool(bReuseStrings)
{
	m_nodeStack.resize(32);
	CleanStack();
}

XmlParserImp::~XmlParserImp()
{
	ParseEnd();
}

namespace
{
uk custom_xml_malloc(size_t nSize)
{
	return malloc(nSize);
}
uk custom_xml_realloc(uk p, size_t nSize)
{
	return realloc(p, nSize);
}
void custom_xml_free(uk p)
{
	free(p);
}
}

//////////////////////////////////////////////////////////////////////////
void XmlParserImp::ParseBegin(bool bCleanPools)
{
	m_root = 0;
	CleanStack();

	if (bCleanPools)
		m_stringPool.Clear();

	XML_Memory_Handling_Suite memHandler;
	memHandler.malloc_fcn = custom_xml_malloc;
	memHandler.realloc_fcn = custom_xml_realloc;
	memHandler.free_fcn = custom_xml_free;

	m_parser = XML_ParserCreate_MM(NULL, &memHandler, NULL);

	XML_SetUserData(m_parser, this);
	XML_SetElementHandler(m_parser, startElement, endElement);
	XML_SetCharacterDataHandler(m_parser, characterData);
	XML_SetEncoding(m_parser, "utf-8");
}

//////////////////////////////////////////////////////////////////////////
void XmlParserImp::ParseEnd()
{
	if (m_parser)
		XML_ParserFree(m_parser);
	m_parser = 0;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef XmlParserImp::ParseBuffer(tukk buffer, size_t bufLen, XmlString& errorString, bool bCleanPools)
{
	static tukk const errorPrefix = "XML parser: ";

	XmlNodeRef root = 0;

	// Let's try to parse the buffer as binary XML
	{
		XMLBinary::XMLBinaryReader reader;
		XMLBinary::XMLBinaryReader::EResult result;
		root = reader.LoadFromBuffer(XMLBinary::XMLBinaryReader::eBufferMemoryHandling_MakeCopy, buffer, bufLen, result);
		if (root)
		{
			return root;
		}
		if (result != XMLBinary::XMLBinaryReader::eResult_NotBinXml)
		{
			tukk const str = reader.GetErrorDescription();
			errorString = str;
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "%s%s (data size: %u)", errorPrefix, str, static_cast<unsigned>(bufLen));
			return 0;
		}
	}

	// This is not binary XML, so let's use text XML parser.
	{
		ParseBegin(bCleanPools);
		m_stringPool.SetBlockSize(static_cast<unsigned>(bufLen) / 16);

		if (XML_Parse(m_parser, buffer, static_cast<i32>(bufLen), 1))
		{
			root = m_root;
		}
		else
		{
			char str[1024];
			drx_sprintf(str, "%s%s at line %d", errorPrefix, XML_ErrorString(XML_GetErrorCode(m_parser)), (i32)XML_GetCurrentLineNumber(m_parser));
			errorString = str;
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "%s", str);
		}

		m_root = 0;
		ParseEnd();
	}

	return root;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef XmlParserImp::ParseFile(tukk filename, XmlString& errorString, bool bCleanPools)
{
	LOADING_TIME_PROFILE_SECTION(GetISystem());

	if (!filename)
		return 0;

	static tukk const errorPrefix = "XML reader: ";

	XmlNodeRef root = 0;

	tuk pFileContents = 0;
	size_t fileSize = 0;

	char str[1024];

	// Check the hard link %USER% to use IPlatformOS::ISaveReader
	static const char USER_ENV[] = "%USER%";
	if (!strnicmp(filename, USER_ENV, sizeof(USER_ENV) - 1))
	{
		IPlatformOS* const pPlatformOS = gEnv->pSystem->GetPlatformOS();
		if (pPlatformOS)
		{
			IPlatformOS::ISaveReaderPtr const pSaveReader = pPlatformOS->SaveGetReader(filename);

			if (pSaveReader)
			{
				if ((IPlatformOS::eFOC_Success != pSaveReader->GetNumBytes(fileSize)) || (fileSize <= 0))
				{
					return 0;
				}

				if (fileSize == ~size_t(0))
				{
					drx_sprintf(str, "%sReported file size is -1 (%s)", errorPrefix, filename);
					errorString = str;
					DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "%s", str);
					return 0;
				}

				pFileContents = new char[fileSize];
				if (!pFileContents)
				{
					drx_sprintf(str, "%sCan't allocate %u bytes of memory (%s)", errorPrefix, static_cast<unsigned>(fileSize), filename);
					errorString = str;
					DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "%s", str);
					return 0;
				}

				pSaveReader->ReadBytes(pFileContents, fileSize);
			}
		}
	}
	DrxStackStringT<char, 256> adjustedFilename;
	DrxStackStringT<char, 256> pakPath;
	if (fileSize <= 0)
	{
		CDrxFile xmlFile;

		if (!xmlFile.Open(filename, "rb"))
		{
			drx_sprintf(str, "%sCan't open file (%s)", errorPrefix, filename);
			errorString = str;
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "%s", str);
			return 0;
		}

		fileSize = xmlFile.GetLength();
		if (fileSize <= 0)
		{
			drx_sprintf(str, "%sFile is empty (%s)", errorPrefix, filename);
			errorString = str;
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "%s", str);
			return 0;
		}

		pFileContents = new char[fileSize];
		if (!pFileContents)
		{
			drx_sprintf(str, "%sCan't allocate %u bytes of memory (%s)", errorPrefix, static_cast<unsigned>(fileSize), filename);
			errorString = str;
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "%s", str);
			return 0;
		}

		if (xmlFile.ReadRaw(pFileContents, fileSize) != fileSize)
		{
			delete[] pFileContents;
			drx_sprintf(str, "%sCan't read file (%s)", errorPrefix, filename);
			errorString = str;
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "%s", str);
			return 0;
		}
		adjustedFilename = xmlFile.GetAdjustedFilename();
		adjustedFilename.replace('\\', '/');
		pakPath = xmlFile.GetPakPath();
		pakPath.replace('\\', '/');
	}

	if (g_bEnableBinaryXmlLoading)
	{
		LOADING_TIME_PROFILE_SECTION_NAMED("XMLBinaryReader::Parse");

		XMLBinary::XMLBinaryReader reader;
		XMLBinary::XMLBinaryReader::EResult result;
		root = reader.LoadFromBuffer(XMLBinary::XMLBinaryReader::eBufferMemoryHandling_TakeOwnership, pFileContents, fileSize, result);
		if (root)
		{
			return root;
		}
		if (result != XMLBinary::XMLBinaryReader::eResult_NotBinXml)
		{
			delete[] pFileContents;
			drx_sprintf(str, "%s%s (%s)", errorPrefix, reader.GetErrorDescription(), filename);
			errorString = str;
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "%s", str);
			return 0;
		}
		else
		{
			// not binary XML - refuse to load if in scripts dir and not in bin xml to help reduce hacking
			// wish we could compile the text xml parser out, but too much work to get everything moved over
			static const char SCRIPTS_DIR[] = "Scripts/";
			DrxFixedStringT<32> strScripts("S");
			strScripts += "c";
			strScripts += "r";
			strScripts += "i";
			strScripts += "p";
			strScripts += "t";
			strScripts += "s";
			strScripts += "/";
			// exclude files and PAKs from Mods folder
			DrxFixedStringT<8> modsStr("M");
			modsStr += "o";
			modsStr += "d";
			modsStr += "s";
			modsStr += "/";
			if (strnicmp(filename, strScripts.c_str(), strScripts.length()) == 0 &&
			    strnicmp(adjustedFilename.c_str(), modsStr.c_str(), modsStr.length()) != 0 &&
			    strnicmp(pakPath.c_str(), modsStr.c_str(), modsStr.length()) != 0)
			{
#ifdef _RELEASE
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "Non binary XML found in scripts dir (%s)", filename);
#endif
			}
		}
	}

	{
		ParseBegin(bCleanPools);
		m_stringPool.SetBlockSize(static_cast<unsigned>(fileSize / 16));

		if (XML_Parse(m_parser, pFileContents, static_cast<i32>(fileSize), 1))
		{
			root = m_root;
		}
		else
		{
			drx_sprintf(str, "%s%s at line %d (%s)", errorPrefix, XML_ErrorString(XML_GetErrorCode(m_parser)), (i32)XML_GetCurrentLineNumber(m_parser), filename);
			errorString = str;
			DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "%s", str);
		}

		m_root = 0;
		ParseEnd();
	}

	SYNCHRONOUS_LOADING_TICK();

	delete[] pFileContents;

	return root;
}

XmlParser::XmlParser(bool bReuseStrings)
{
	m_nRefCount = 0;
	m_pImpl = new XmlParserImp(bReuseStrings);
	m_pImpl->AddRef();
}

XmlParser::~XmlParser()
{
	m_pImpl->Release();
}

void XmlParser::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_pImpl);
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef XmlParser::ParseBuffer(tukk buffer, i32 nBufLen, bool bCleanPools)
{
	m_errorString = "";
	return m_pImpl->ParseBuffer(buffer, nBufLen, m_errorString, bCleanPools);
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef XmlParser::ParseFile(tukk filename, bool bCleanPools)
{
	m_errorString = "";
	return m_pImpl->ParseFile(filename, m_errorString, bCleanPools);
}

//////////////////////////////////////////////////////////////////////////
//
// Implements special reusable XmlNode for XmlNode pool
//
//////////////////////////////////////////////////////////////////////////
CXmlNodeReuse::CXmlNodeReuse(tukk tag, CXmlNodePool* pPool) : m_pPool(pPool)
{
	SAFE_RELEASE(m_pStringPool);
	m_pStringPool = m_pPool->GetStringPool();
	m_pStringPool->AddRef();
	m_tag = m_pStringPool->AddString(tag);
}

void CXmlNodeReuse::Release()
{
	m_pPool->OnRelease(m_nRefCount, this);
	CXmlNode::Release();
}

//////////////////////////////////////////////////////////////////////////
//
// Pool of reusable XML nodes with shared string pool
//
//////////////////////////////////////////////////////////////////////////
CXmlNodePool::CXmlNodePool(u32 nBlockSize, bool bReuseStrings)
{
	m_pStringPool = new CXmlStringPool(bReuseStrings);
	assert(m_pStringPool != 0);

	// in order to avoid memory fragmentation
	// allocates 1Mb buffer for shared string pool
	static_cast<CXmlStringPool*>(m_pStringPool)->SetBlockSize(nBlockSize);
	m_pStringPool->AddRef();
	m_nAllocated = 0;
}

CXmlNodePool::~CXmlNodePool()
{
	while (!m_pNodePool.empty())
	{
		CXmlNodeReuse* pNode = m_pNodePool.top();
		m_pNodePool.pop();
		pNode->Release();
	}
	m_pStringPool->Release();
}

XmlNodeRef CXmlNodePool::GetXmlNode(tukk sNodeName)
{
	CXmlNodeReuse* pNode = 0;

	// NOTE: at the moment xml node pool is dedicated for statistics nodes only

	// first at all check if we have already free node
	if (!m_pNodePool.empty())
	{
		pNode = m_pNodePool.top();
		m_pNodePool.pop();

		// init it to new node name
		pNode->setTag(sNodeName);

		m_nAllocated++;
		//if (0 == m_nAllocated % 1000)
		//DrxLog("[CXmlNodePool]: already reused nodes [%d]", m_nAllocated);
	}
	else
	{
		// there is no free nodes so create new one
		// later it will be reused as soon as no external references left
		pNode = new CXmlNodeReuse(sNodeName, this);
		assert(pNode != 0);

		// increase ref counter for reusing node later
		pNode->AddRef();

		m_nAllocated++;
		//if (0 == m_nAllocated % 1000)
		//DrxLog("[CXmlNodePool]: already allocated nodes [%d]", m_nAllocated);
	}
	return pNode;
}

void CXmlNodePool::OnRelease(i32 iRefCount, uk pThis)
{
	// each reusable node call OnRelease before parent release
	// since we keep reference on xml node so when ref count equals
	// to 2 means that it is last external object releases reference
	// to reusable node and it can be save for reuse later
	if (2 == iRefCount)
	{
		CXmlNodeReuse* pNode = static_cast<CXmlNodeReuse*>(pThis);

		pNode->removeAllChilds();
		pNode->removeAllAttributes();

		m_pNodePool.push(pNode);

		// decrease totally allocated by xml node pool counter
		// when counter equals to zero it means that all external
		// objects do not have references to allocated reusable node
		// at that point it is safe to clear shared string pool
		m_nAllocated--;

		if (0 == m_nAllocated)
		{
			//DrxLog("[CXmlNodePool]: clear shared string pool");
			static_cast<CXmlStringPool*>(m_pStringPool)->Clear();
		}
	}
}
