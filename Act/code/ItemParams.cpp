// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$

   -------------------------------------------------------------------------
   История:
   - 18:8:2005   17:27 : Created by Márcio Martins

*************************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/ItemParams.h>

//------------------------------------------------------------------------
//i32 g_paramNodeDebugCount = 0;

CItemParamsNode::CItemParamsNode()
	: m_refs(1)
{
	//g_paramNodeDebugCount++;
};

//------------------------------------------------------------------------
CItemParamsNode::~CItemParamsNode()
{
	for (TChildVector::iterator it = m_children.begin(); it != m_children.end(); ++it)
		delete (*it);

	//g_paramNodeDebugCount--;
}

//------------------------------------------------------------------------
i32 CItemParamsNode::GetAttributeCount() const
{
	return m_attributes.size();
}

//------------------------------------------------------------------------
tukk CItemParamsNode::GetAttributeName(i32 i) const
{
	TAttributeMap::const_iterator it = GetConstIterator<TAttributeMap>(m_attributes, i);
	if (it != m_attributes.end())
		return it->first.c_str();
	return 0;
}

//------------------------------------------------------------------------
tukk CItemParamsNode::GetAttribute(i32 i) const
{
	TAttributeMap::const_iterator it = GetConstIterator<TAttributeMap>(m_attributes, i);
	if (it != m_attributes.end())
	{
		const string* str = it->second.GetPtr<string>();
		return str ? str->c_str() : 0;
	}
	return 0;
}

//------------------------------------------------------------------------
bool CItemParamsNode::GetAttribute(i32 i, Vec3& attr) const
{
	TAttributeMap::const_iterator it = GetConstIterator<TAttributeMap>(m_attributes, i);
	if (it != m_attributes.end())
		return it->second.GetValueWithConversion(attr);
	return false;
}

//------------------------------------------------------------------------
bool CItemParamsNode::GetAttribute(i32 i, Ang3& attr) const
{
	Vec3 temp;
	bool r = GetAttribute(i, temp);
	attr = Ang3(temp);
	return r;
}

//------------------------------------------------------------------------
bool CItemParamsNode::GetAttribute(i32 i, float& attr) const
{
	TAttributeMap::const_iterator it = GetConstIterator<TAttributeMap>(m_attributes, i);
	if (it != m_attributes.end())
		return it->second.GetValueWithConversion(attr);
	return false;
}

//------------------------------------------------------------------------
bool CItemParamsNode::GetAttribute(i32 i, i32& attr) const
{
	TAttributeMap::const_iterator it = GetConstIterator<TAttributeMap>(m_attributes, i);
	if (it != m_attributes.end())
		return it->second.GetValueWithConversion(attr);
	return false;
}

//------------------------------------------------------------------------
i32 CItemParamsNode::GetAttributeType(i32 i) const
{
	TAttributeMap::const_iterator it = GetConstIterator<TAttributeMap>(m_attributes, i);
	if (it != m_attributes.end())
		return it->second.GetType();
	return eIPT_None;
}

//------------------------------------------------------------------------
tukk CItemParamsNode::GetNameAttribute() const
{
	return m_nameAttribute.c_str();
}

//------------------------------------------------------------------------
tukk CItemParamsNode::GetAttribute(tukk name) const
{
	TAttributeMap::const_iterator it = FindAttrIterator(m_attributes, name);
	if (it != m_attributes.end())
	{
		const string* str = it->second.GetPtr<string>();
		return str ? str->c_str() : 0;
	}
	return 0;
}

//------------------------------------------------------------------------
tukk CItemParamsNode::GetAttributeSafe(tukk name) const
{
	TAttributeMap::const_iterator it = FindAttrIterator(m_attributes, name);
	if (it != m_attributes.end())
	{
		const string* str = it->second.GetPtr<string>();
		return str ? str->c_str() : "";
	}
	return "";
}

//------------------------------------------------------------------------
bool CItemParamsNode::GetAttribute(tukk name, Vec3& attr) const
{
	TAttributeMap::const_iterator it = FindAttrIterator(m_attributes, name);
	if (it != m_attributes.end())
		return it->second.GetValueWithConversion(attr);
	return false;
}

//------------------------------------------------------------------------
bool CItemParamsNode::GetAttribute(tukk name, Ang3& attr) const
{
	Vec3 temp;
	bool r = GetAttribute(name, temp);
	if (r)
		attr = Ang3(temp);
	return r;
}

//------------------------------------------------------------------------
bool CItemParamsNode::GetAttribute(tukk name, float& attr) const
{
	TAttributeMap::const_iterator it = FindAttrIterator(m_attributes, name);
	if (it != m_attributes.end())
		return it->second.GetValueWithConversion(attr);
	return false;
}

//------------------------------------------------------------------------
bool CItemParamsNode::GetAttribute(tukk name, i32& attr) const
{
	TAttributeMap::const_iterator it = FindAttrIterator(m_attributes, name);
	if (it != m_attributes.end())
		return it->second.GetValueWithConversion(attr);
	return false;
}

//------------------------------------------------------------------------
i32 CItemParamsNode::GetAttributeType(tukk name) const
{
	TAttributeMap::const_iterator it = FindAttrIterator(m_attributes, name);
	if (it != m_attributes.end())
		return it->second.GetType();
	return eIPT_None;
}

//------------------------------------------------------------------------
i32 CItemParamsNode::GetChildCount() const
{
	return (i32)m_children.size();
}

//------------------------------------------------------------------------
tukk CItemParamsNode::GetChildName(i32 i) const
{
	if (i >= 0 && i < m_children.size())
		return m_children[i]->GetName();
	return 0;
}

//------------------------------------------------------------------------
const IItemParamsNode* CItemParamsNode::GetChild(i32 i) const
{
	if (i >= 0 && i < m_children.size())
		return m_children[i];
	return 0;
}

//------------------------------------------------------------------------
const IItemParamsNode* CItemParamsNode::GetChild(tukk name) const
{
	for (TChildVector::const_iterator it = m_children.begin(); it != m_children.end(); ++it)
	{
		if (!strcmpi((*it)->GetName(), name))
			return (*it);
	}
	return 0;
}

//------------------------------------------------------------------------
void CItemParamsNode::SetAttribute(tukk name, tukk attr)
{
	//m_attributes.insert(TAttributeMap::value_type(name, string(attr)));
	if (!strcmpi(name, "name"))
	{
		m_nameAttribute = attr;
		AddAttribute(name, TItemParamValue(m_nameAttribute));
	}
	else
		AddAttribute(name, TItemParamValue(string(attr)));
}

//------------------------------------------------------------------------
void CItemParamsNode::SetAttribute(tukk name, const Vec3& attr)
{
	//m_attributes.insert(TAttributeMap::value_type(name, attr));
	AddAttribute(name, TItemParamValue(attr));
}

//------------------------------------------------------------------------
void CItemParamsNode::SetAttribute(tukk name, float attr)
{
	//m_attributes.insert(TAttributeMap::value_type(name, attr));
	AddAttribute(name, TItemParamValue(attr));
}

//------------------------------------------------------------------------
void CItemParamsNode::SetAttribute(tukk name, i32 attr)
{
	//m_attributes.insert(TAttributeMap::value_type(name, attr));
	AddAttribute(name, TItemParamValue(attr));
}

//------------------------------------------------------------------------
IItemParamsNode* CItemParamsNode::InsertChild(tukk name)
{
	m_children.push_back(new CItemParamsNode());
	IItemParamsNode* inserted = m_children.back();
	inserted->SetName(name);
	return inserted;
}

//------------------------------------------------------------------------
bool IsInteger(tukk v, i32* i = 0)
{
	errno = 0;
	tuk endptr = 0;
	i32 r = strtol(v, &endptr, 0);
	if (errno && (errno != ERANGE))
		return false;
	if (endptr == v || *endptr != '\0')
		return false;
	if (i)
		*i = r;
	return true;
}

//------------------------------------------------------------------------
bool IsFloat(tukk v, float* f = 0)
{
	errno = 0;
	tuk endptr = 0;
	float r = (float)strtod(v, &endptr);
	if (errno && (errno != ERANGE))
		return false;
	if (endptr == v || *endptr != '\0')
		return false;
	if (f)
		*f = r;
	return true;
}

//------------------------------------------------------------------------
bool IsVec3(tukk v, Vec3* vec)
{
	float x, y, z;
	if (sscanf(v, "%f,%f,%f", &x, &y, &z) != 3)
		return false;
	if (vec)
		vec->Set(x, y, z);
	return true;
}

//------------------------------------------------------------------------
void CItemParamsNode::ConvertFromXML(const XmlNodeRef& root)
{
	if (gEnv->bMultiplayer)
	{
		ConvertFromXMLWithFiltering(root, "MP");
	}
	else
	{
		ConvertFromXMLWithFiltering(root, "SP");
	}
}

bool CItemParamsNode::ConvertFromXMLWithFiltering(const XmlNodeRef& root, tukk keepWithThisAttrValue)
{
	bool filteringRequired = false;
	i32 nattributes = root->getNumAttributes();
	m_attributes.reserve(nattributes);
	for (i32 a = 0; a < nattributes; a++)
	{
		tukk name = 0;
		tukk value = 0;
		if (root->getAttributeByIndex(a, &name, &value))
		{
			float f;
			i32 i;
			Vec3 v;
			if (!stricmp(value, "true"))
				SetAttribute(name, 1);
			else if (!stricmp(value, "false"))
				SetAttribute(name, 0);
			else if (IsInteger(value, &i))
				SetAttribute(name, i);
			else if (IsFloat(value, &f))
				SetAttribute(name, f);
			else if (IsVec3(value, &v))
				SetAttribute(name, v);
			else
				SetAttribute(name, value);
		}
	}

	i32 nchildren = root->getChildCount();
	m_children.reserve(nchildren);
	for (i32 c = 0; c < nchildren; c++)
	{
		XmlNodeRef child = root->getChild(c);
		EXMLFilterType filterType = ShouldConvertNodeFromXML(child, keepWithThisAttrValue);
		filteringRequired = (filterType != eXMLFT_none) || filteringRequired ? true : false;

		if (filterType != eXMLFT_remove)
		{
			filteringRequired = (InsertChild(child->getTag())->ConvertFromXMLWithFiltering(child, keepWithThisAttrValue) || filteringRequired);
		}
	}

	return filteringRequired;
}

CItemParamsNode::EXMLFilterType CItemParamsNode::ShouldConvertNodeFromXML(const XmlNodeRef& xmlNode, tukk keepWithThisAttrValue) const
{
	if (xmlNode->haveAttr("GAME"))
	{
		tukk game = xmlNode->getAttr("GAME");

		return (strcmp(game, keepWithThisAttrValue) == 0 ? eXMLFT_add : eXMLFT_remove);
	}

	return eXMLFT_none;
}

void CItemParamsNode::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->AddObject(this, sizeof(*this));
	pSizer->AddObject(m_name);
	pSizer->AddObject(m_nameAttribute);
	pSizer->AddContainer(m_attributes);
	pSizer->AddContainer(m_children);
}
