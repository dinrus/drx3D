// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------

   Описание: Wrapper class for XmlNodeRef which allows to parse different contents
   from xml depending on game mode SP or MP

   -------------------------------------------------------------------------
   История:
   - 23:09:2010   : Created by Benito Gangoso Rodriguez

*************************************************************************/

#pragma once

#ifndef _GAME_XML_PARAM_READER_H_
	#define _GAME_XML_PARAM_READER_H_

class CGameXmlParamReader
{
public:

	explicit CGameXmlParamReader(const XmlNodeRef& xmlNode)
		: m_xmlNode(xmlNode)
	{
		m_gameModeFilter = gEnv->bMultiplayer ? "MP" : "SP";
	#if defined(_RELEASE)
		m_devmodeFilter = true;
	#else
		m_devmodeFilter = false;
	#endif //_RELEASE
	}

	i32 GetUnfilteredChildCount() const
	{
		if (m_xmlNode)
		{
			return m_xmlNode->getChildCount();
		}
		return 0;
	}

	i32 GetFilteredChildCount() const
	{
		i32 filteredChildCount = 0;
		if (m_xmlNode)
		{
			i32k childCount = m_xmlNode->getChildCount();
			for (i32 i = 0; i < childCount; ++i)
			{
				filteredChildCount += !IsNodeFiltered(m_xmlNode->getChild(i));
			}
		}
		return filteredChildCount;
	}

	XmlNodeRef GetFilteredChildAt(i32 index) const
	{
		if (m_xmlNode)
		{
			XmlNodeRef childNode = m_xmlNode->getChild(index);

			return !IsNodeFiltered(childNode) ? childNode : XmlNodeRef((IXmlNode*)NULL);
		}

		return NULL;
	}

	XmlNodeRef FindFilteredChild(tukk childName) const
	{
		if (m_xmlNode)
		{
			i32k childCount = m_xmlNode->getChildCount();
			for (i32 i = 0; i < childCount; ++i)
			{
				XmlNodeRef childNode = m_xmlNode->getChild(i);

				if (!IsNodeWithTag(childNode, childName) || IsNodeFiltered(childNode))
				{
					continue;
				}

				return childNode;
			}
		}
		return NULL;
	}

	tukk ReadParamValue(tukk paramName) const
	{
		return ReadParamValue(paramName, "");
	}

	tukk ReadParamValue(tukk paramName, tukk defaultValue) const
	{
		if (m_xmlNode)
		{
			i32k childCount = m_xmlNode->getChildCount();
			for (i32 i = 0; i < childCount; ++i)
			{
				XmlNodeRef childNode = m_xmlNode->getChild(i);

				if (!HasNodeParamAttribute(childNode, paramName) || IsNodeFiltered(childNode))
				{
					continue;
				}

				return childNode->getAttr("value");
			}
		}
		return defaultValue;
	}

	tukk ReadParamAttributeValue(tukk paramName, tukk attributeName) const
	{
		if (m_xmlNode)
		{
			i32k childCount = m_xmlNode->getChildCount();
			for (i32 i = 0; i < childCount; ++i)
			{
				XmlNodeRef childNode = m_xmlNode->getChild(i);

				if (!HasNodeParamAttribute(childNode, paramName) || IsNodeFiltered(childNode))
				{
					continue;
				}

				return childNode->getAttr(attributeName);
			}
		}
		return "";
	}

	template<typename T>
	bool ReadParamValue(tukk paramName, T& value) const
	{
		if (m_xmlNode)
		{
			i32k childCount = m_xmlNode->getChildCount();
			for (i32 i = 0; i < childCount; ++i)
			{
				XmlNodeRef childNode = m_xmlNode->getChild(i);

				if (!HasNodeParamAttribute(childNode, paramName) || IsNodeFiltered(childNode))
				{
					continue;
				}

				return childNode->getAttr("value", value);
			}
		}
		return false;
	}

	template<typename T>
	bool ReadParamValue(tukk paramName, T& value, const T& defaultValue) const
	{
		value = defaultValue;
		return ReadParamValue<T>(paramName, value);
	}

	template<typename T>
	bool ReadParamAttributeValue(tukk paramName, tukk attributeName, T& value) const
	{
		if (m_xmlNode)
		{
			i32k childCount = m_xmlNode->getChildCount();
			for (i32 i = 0; i < childCount; ++i)
			{
				XmlNodeRef childNode = m_xmlNode->getChild(i);

				if (!HasNodeParamAttribute(childNode, paramName) || IsNodeFiltered(childNode))
				{
					continue;
				}

				return childNode->getAttr(attributeName, value);
			}
		}
		return false;
	}

	template<typename T>
	bool ReadParamAttributeValue(tukk paramName, tukk attributeName, T& value, const T& defaultValue) const
	{
		value = defaultValue;

		return ReadParamAttributeValue<T>(paramName, attributeName, value);
	}

private:

	bool IsNodeFiltered(const XmlNodeRef& xmlNode) const
	{
		DRX_ASSERT(xmlNode != (IXmlNode*)NULL);

		tukk gameAttribute = xmlNode->getAttr("GAME");

		i32 devmodeFilter = 0;
		xmlNode->getAttr("DEVMODE", devmodeFilter);
		if (devmodeFilter != 0)
		{
			i32 i = 0;
		}

		const bool devmodeFiltered = devmodeFilter != 0 && m_devmodeFilter;
		const bool gameModeFiltered = (gameAttribute != NULL) && gameAttribute[0] && (strcmp(gameAttribute, m_gameModeFilter.c_str()) != 0);

		return devmodeFiltered || gameModeFiltered;
	}

	bool IsNodeWithTag(const XmlNodeRef& xmlNode, tukk tag) const
	{
		DRX_ASSERT(xmlNode != (IXmlNode*)NULL);

		return (stricmp(xmlNode->getTag(), tag) == 0);
	}

	bool HasNodeParamAttribute(const XmlNodeRef& xmlNode, tukk paramName) const
	{
		DRX_ASSERT(xmlNode != (IXmlNode*)NULL);

		tukk attributeName = xmlNode->getAttr("name");

		return (attributeName != NULL && (stricmp(attributeName, paramName) == 0));
	}

	const XmlNodeRef&  m_xmlNode;
	DrxFixedStringT<4> m_gameModeFilter;
	bool               m_devmodeFilter;
};

#endif //_GAME_XML_PARAM_READER_H_
