// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/Sys/XMLBinaryNode.h>
#include <drx3D/CoreX/Memory/DrxSizer.h>
#include <cctype>

#pragma warning(disable : 6031) // Return value ignored: 'sscanf'

//////////////////////////////////////////////////////////////////////////
CBinaryXmlData::CBinaryXmlData()
	: pNodes(0)
	, pAttributes(0)
	, pChildIndices(0)
	, pStringData(0)
	, pFileContents(0)
	, nFileSize(0)
	, bOwnsFileContentsMemory(true)
	, pBinaryNodes(0)
	, nRefCount(0)
{
}

//////////////////////////////////////////////////////////////////////////
CBinaryXmlData::~CBinaryXmlData()
{
	if (bOwnsFileContentsMemory)
	{
		delete[] pFileContents;
	}
	pFileContents = 0;

	delete[] pBinaryNodes;
	pBinaryNodes = 0;
}

void CBinaryXmlData::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(pFileContents, nFileSize);
	const XMLBinary::BinaryFileHeader* pHeader = reinterpret_cast<const XMLBinary::BinaryFileHeader*>(pFileContents);
	pSizer->AddObject(pBinaryNodes, sizeof(CBinaryXmlNode) * pHeader->nNodeCount);
}

//////////////////////////////////////////////////////////////////////////
// CBinaryXmlNode implementation.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// collect allocated memory  informations
void CBinaryXmlNode::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(m_pData);
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CBinaryXmlNode::getParent() const
{
	const XMLBinary::Node* const pNode = _node();
	if (pNode->nParentIndex != (XMLBinary::NodeIndex)-1)
	{
		return &m_pData->pBinaryNodes[pNode->nParentIndex];
	}
	return XmlNodeRef();
}

XmlNodeRef CBinaryXmlNode::createNode(tukk tag)
{
	assert(0);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CBinaryXmlNode::isTag(tukk tag) const
{
	return g_pXmlStrCmp(tag, getTag()) == 0;
}

tukk CBinaryXmlNode::getAttr(tukk key) const
{
	tukk svalue = GetValue(key);
	if (svalue)
		return svalue;
	return "";
}

bool CBinaryXmlNode::getAttr(tukk key, tukk* value) const
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

bool CBinaryXmlNode::haveAttr(tukk key) const
{
	return (GetValue(key) != 0);
}

//////////////////////////////////////////////////////////////////////////
bool CBinaryXmlNode::getAttr(tukk key, i32& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		value = atoi(svalue);
		return true;
	}
	return false;
}

bool CBinaryXmlNode::getAttr(tukk key, u32& value) const
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
bool CBinaryXmlNode::getAttr(tukk key, int64& value) const
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
bool CBinaryXmlNode::getAttr(tukk key, uint64& value, bool useHexFormat) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		if (useHexFormat)
		{
			sscanf(svalue, "%" PRIX64, &value);
		}
		else
		{
			sscanf(svalue, "%" PRIu64, &value);
		}
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBinaryXmlNode::getAttr(tukk key, bool& value) const
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
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "Encountered invalid value during CBinaryXmlNode::getAttr! Value: %s Tag: %s", szValue, getTag());
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
				DrxWarning(VALIDATOR_MODULE_SYSTEM, VALIDATOR_WARNING, "Encountered invalid value during CBinaryXmlNode::getAttr! Value: %s Tag: %s", szValue, getTag());
			}
		}
	}

	return isSuccess;
}

bool CBinaryXmlNode::getAttr(tukk key, float& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		value = (float)atof(svalue);
		return true;
	}
	return false;
}

bool CBinaryXmlNode::getAttr(tukk key, double& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		value = atof(svalue);
		return true;
	}
	return false;
}

bool CBinaryXmlNode::getAttr(tukk key, Ang3& value) const
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
bool CBinaryXmlNode::getAttr(tukk key, Vec3& value) const
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
bool CBinaryXmlNode::getAttr(tukk key, Vec4& value) const
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

bool CBinaryXmlNode::getAttr(tukk key, Vec3d& value) const
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
bool CBinaryXmlNode::getAttr(tukk key, Vec2& value) const
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

//////////////////////////////////////////////////////////////////////////
bool CBinaryXmlNode::getAttr(tukk key, Vec2d& value) const
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
bool CBinaryXmlNode::getAttr(tukk key, Quat& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		float w, x, y, z;
		if (sscanf(svalue, "%f,%f,%f,%f", &w, &x, &y, &z) == 4)
		{
			value = Quat(w, x, y, z);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBinaryXmlNode::getAttr(tukk key, ColorB& value) const
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

void CBinaryXmlNode::setAttr(tukk key, const DrxGUID& value)
{
	setAttr(key, value.ToString());
}

bool CBinaryXmlNode::getAttr(tukk key, DrxGUID& value) const
{
	tukk svalue = GetValue(key);
	if (svalue)
	{
		value = DrxGUID::FromString(svalue);
		if ((value.hipart >> 32) == 0)
		{
			memset(&value, 0, sizeof(value));
			// If bad GUID, use old guid system.
			// Not sure if this will apply well in DrxGUID!
			value.hipart = (uint64)atoi(svalue) << 32;
		}
		return true;
	}
	return false;
}

XmlNodeRef CBinaryXmlNode::findChild(tukk tag) const
{
	const XMLBinary::Node* const pNode = _node();
	u32k nFirst = pNode->nFirstChildIndex;
	u32k nAfterLast = pNode->nFirstChildIndex + pNode->nChildCount;
	for (u32 i = nFirst; i < nAfterLast; ++i)
	{
		tukk sChildTag = m_pData->pStringData + m_pData->pNodes[m_pData->pChildIndices[i]].nTagStringOffset;
		if (g_pXmlStrCmp(tag, sChildTag) == 0)
		{
			return m_pData->pBinaryNodes + m_pData->pChildIndices[i];
		}
	}
	return 0;
}

//! Get XML Node child nodes.
XmlNodeRef CBinaryXmlNode::getChild(i32 i) const
{
	const XMLBinary::Node* const pNode = _node();
	assert(i >= 0 && i < (i32)pNode->nChildCount);
	return m_pData->pBinaryNodes + m_pData->pChildIndices[pNode->nFirstChildIndex + i];
}

//////////////////////////////////////////////////////////////////////////
bool CBinaryXmlNode::getAttributeByIndex(i32 index, tukk* key, tukk* value)
{
	const XMLBinary::Node* const pNode = _node();
	if (index >= 0 && index < pNode->nAttributeCount)
	{
		const XMLBinary::Attribute& attr = m_pData->pAttributes[pNode->nFirstAttributeIndex + index];
		*key = _string(attr.nKeyStringOffset);
		*value = _string(attr.nValueStringOffset);
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
bool CBinaryXmlNode::getAttributeByIndex(i32 index, XmlString& key, XmlString& value)
{
	const XMLBinary::Node* const pNode = _node();
	if (index >= 0 && index < pNode->nAttributeCount)
	{
		const XMLBinary::Attribute& attr = m_pData->pAttributes[pNode->nFirstAttributeIndex + index];
		key = _string(attr.nKeyStringOffset);
		value = _string(attr.nValueStringOffset);
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
