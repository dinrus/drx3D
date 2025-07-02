// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/CoreX/Serialization/ClassFactory.h>

#include <drx3D/Sys/XmlIArchive.h>

#include <drx3D/CoreX/Serialization/BlackBox.h>

namespace XmlUtil
{
i32 g_hintSuccess = 0;
i32 g_hintFail = 0;

XmlNodeRef FindChildNode(const XmlNodeRef &pParent, i32k childIndexOverride, i32& childIndexHint, tukk const name)
{
	DRX_ASSERT(pParent);

	if (0 <= childIndexOverride)
	{
		DRX_ASSERT(childIndexOverride < pParent->getChildCount());
		return pParent->getChild(childIndexOverride);
	}
	else
	{
		DRX_ASSERT(name);
		DRX_ASSERT(name[0]);
		DRX_ASSERT(0 <= childIndexHint);

		i32k childCount = pParent->getChildCount();
		const bool hasValidChildHint = (childIndexHint < childCount);
		if (hasValidChildHint)
		{
			XmlNodeRef pChildNode = pParent->getChild(childIndexHint);
			if (pChildNode->isTag(name))
			{
				g_hintSuccess++;
				i32k nextChildIndexHint = childIndexHint + 1;
				childIndexHint = (nextChildIndexHint < childCount) ? nextChildIndexHint : 0;
				return pChildNode;
			}
			else
			{
				g_hintFail++;
			}
		}

		for (i32 i = 0; i < childCount; ++i)
		{
			XmlNodeRef pChildNode = pParent->getChild(i);
			if (pChildNode->isTag(name))
			{
				i32k nextChildIndexHint = i + 1;
				childIndexHint = (nextChildIndexHint < childCount) ? nextChildIndexHint : 0;
				return pChildNode;
			}
		}
	}
	return XmlNodeRef();
}

template<typename T, typename TOut>
bool ReadChildNodeAs(const XmlNodeRef &pParent, i32k childIndexOverride, i32& childIndexHint, tukk const name, TOut& valueOut)
{
	XmlNodeRef pChild = FindChildNode(pParent, childIndexOverride, childIndexHint, name);
	if (pChild)
	{
		T tmp;
		const bool readValueSuccess = pChild->getAttr("value", tmp);
		if (readValueSuccess)
		{
			valueOut = tmp;
		}
		return readValueSuccess;
	}
	return false;
}

template<typename T>
bool ReadChildNode(const XmlNodeRef &pParent, i32k childIndexOverride, i32& childIndexHint, tukk const name, T& valueOut)
{
	return ReadChildNodeAs<T>(pParent, childIndexOverride, childIndexHint, name, valueOut);
}
}

Serialization::CXmlIArchiveVer1::CXmlIArchiveVer1()
	: IArchive(INPUT | NO_EMPTY_NAMES | VALIDATION | XML_VERSION_1)
	, m_childIndexOverride(-1)
	, m_childIndexHint(0)
{
}

Serialization::CXmlIArchiveVer1::CXmlIArchiveVer1(const CXmlIArchiveVer1 &parent,const XmlNodeRef &pRootNode)
	: IArchive(parent.caps_)
	, m_pRootNode(pRootNode)
	, m_childIndexOverride(-1)
	, m_childIndexHint(0)
{
	filter_ = parent.filter_;
	modifiedRow_ = parent.modifiedRow_;
	lastContext_ = parent.lastContext_;

	DRX_ASSERT(m_pRootNode);
}

Serialization::CXmlIArchiveVer1::~CXmlIArchiveVer1()
{
}

void Serialization::CXmlIArchiveVer1::SetXmlNode(XmlNodeRef pNode)
{
	m_pRootNode = pNode;
}

XmlNodeRef Serialization::CXmlIArchiveVer1::GetXmlNode() const
{
	return m_pRootNode;
}

bool Serialization::CXmlIArchiveVer1::operator()(bool& value, tukk name, tukk label)
{
	XmlNodeRef pChild = XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name);
	if (pChild)
	{
		tukk const stringValue = pChild->getAttr("value");
		if (stringValue)
		{
			value = (strcmp("true", stringValue) == 0);
			value = value || (strcmp("1", stringValue) == 0);
			return true;
		}
		return false;
	}
	return false;
}

bool Serialization::CXmlIArchiveVer1::operator()(IString& value, tukk name, tukk label)
{
	XmlNodeRef pChild = XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name);
	if (pChild)
	{
		tukk const stringValue = pChild->getAttr("value");
		if (stringValue)
		{
			value.set(stringValue);
			return true;
		}
		return false;
	}
	return false;
}

bool Serialization::CXmlIArchiveVer1::operator()(IWString& value, tukk name, tukk label)
{
	DrxFatalError("CXmlIArchive::operator() with IWString is not implemented");
	return false;
}

bool Serialization::CXmlIArchiveVer1::operator()(float& value, tukk name, tukk label)
{
	return XmlUtil::ReadChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name, value);
}

bool Serialization::CXmlIArchiveVer1::operator()(double& value, tukk name, tukk label)
{
	return XmlUtil::ReadChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name, value);
}

bool Serialization::CXmlIArchiveVer1::operator()(i16& value, tukk name, tukk label)
{
	return XmlUtil::ReadChildNodeAs<i32>(m_pRootNode, m_childIndexOverride, m_childIndexHint, name, value);
}

bool Serialization::CXmlIArchiveVer1::operator()(u16& value, tukk name, tukk label)
{
	return XmlUtil::ReadChildNodeAs<uint>(m_pRootNode, m_childIndexOverride, m_childIndexHint, name, value);
}

bool Serialization::CXmlIArchiveVer1::operator()(i32& value, tukk name, tukk label)
{
	return XmlUtil::ReadChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name, value);
}

bool Serialization::CXmlIArchiveVer1::operator()(u32& value, tukk name, tukk label)
{
	return XmlUtil::ReadChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name, value);
}

bool Serialization::CXmlIArchiveVer1::operator()(int64& value, tukk name, tukk label)
{
	return XmlUtil::ReadChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name, value);
}

bool Serialization::CXmlIArchiveVer1::operator()(uint64& value, tukk name, tukk label)
{
	return XmlUtil::ReadChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name, value);
}

bool Serialization::CXmlIArchiveVer1::operator()(int8& value, tukk name, tukk label)
{
	return XmlUtil::ReadChildNodeAs<i32>(m_pRootNode, m_childIndexOverride, m_childIndexHint, name, value);
}

bool Serialization::CXmlIArchiveVer1::operator()(u8& value, tukk name, tukk label)
{
	return XmlUtil::ReadChildNodeAs<uint>(m_pRootNode, m_childIndexOverride, m_childIndexHint, name, value);
}

bool Serialization::CXmlIArchiveVer1::operator()(char& value, tukk name, tukk label)
{
	return XmlUtil::ReadChildNodeAs<i32>(m_pRootNode, m_childIndexOverride, m_childIndexHint, name, value);
}

bool Serialization::CXmlIArchiveVer1::operator()(const SStruct& ser, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);

	XmlNodeRef pChild = XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name);
	if (pChild)
	{
		CXmlIArchiveVer1 childArchive(*this,pChild);

		const bool serializeSuccess = ser(childArchive);
		return serializeSuccess;
	}
	return false;
}

bool Serialization::CXmlIArchiveVer1::operator()(SBlackBox& box, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);

	XmlNodeRef pChild = XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name);
	if (pChild)
	{
		box.set<XmlNodeRef>("xml", pChild);
		box.xmlVersion = 1;
		return true;
	}
	return false;
}

bool Serialization::CXmlIArchiveVer1::operator()(IContainer& ser, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);

	XmlNodeRef pChild = XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name);
	if (pChild)
	{
		bool serializeSuccess = true;

		i32k elementCount = pChild->getChildCount();
		ser.resize(elementCount);

		if (0 < elementCount)
		{
			CXmlIArchiveVer1 childArchive(*this,pChild);

			for (i32 i = 0; i < elementCount; ++i)
			{
				childArchive.m_childIndexOverride = i;

				serializeSuccess &= ser(childArchive, "Element", "Element");
				ser.next();
			}
		}

		return serializeSuccess;
	}
	return false;
}

void Serialization::CXmlIArchiveVer1::validatorMessage(bool error, ukk handle, const TypeID& type, tukk message)
{
	const EValidatorModule module = VALIDATOR_MODULE_UNKNOWN;
	const EValidatorSeverity severity = error ? VALIDATOR_ERROR : VALIDATOR_WARNING;
	DrxWarning(module, severity, "CXmlIArchive: %s", message);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Serialization::CXmlInputArchive::CXmlInputArchive()
	: IArchive(INPUT | NO_EMPTY_NAMES | VALIDATION)
	, m_childIndexOverride(-1)
	, m_childIndexHint(0)
{
}

Serialization::CXmlInputArchive::CXmlInputArchive(const CXmlInputArchive &parent,const XmlNodeRef &pRootNode)
	: IArchive(parent.caps_)
	, m_pRootNode(pRootNode)
	, m_childIndexOverride(-1)
	, m_childIndexHint(0)
{
	filter_ = parent.filter_;
	modifiedRow_ = parent.modifiedRow_;
	lastContext_ = parent.lastContext_;

	DRX_ASSERT(m_pRootNode);
}

Serialization::CXmlInputArchive::~CXmlInputArchive()
{
}

void Serialization::CXmlInputArchive::SetXmlNode(XmlNodeRef pNode)
{
	m_pRootNode = pNode;
}

XmlNodeRef Serialization::CXmlInputArchive::GetXmlNode() const
{
	return m_pRootNode;
}

bool Serialization::CXmlInputArchive::operator()(bool& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	return node->getAttr(name,value);
}

bool Serialization::CXmlInputArchive::operator()(IString& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	if (!node->haveAttr(name))
		return false;
	
	tukk stringValue = node->getAttr(name);
	if (stringValue)
	{
		value.set(stringValue);
		return true;
	}
	return false;
}

bool Serialization::CXmlInputArchive::operator()(IWString& value, tukk name, tukk label)
{
	DRX_ASSERT_MESSAGE(0,"CXmlInputArchive::operator() with IWString is not implemented");
	DrxFatalError("CXmlInputArchive::operator() with IWString is not implemented");
	return false;
}

bool Serialization::CXmlInputArchive::operator()(float& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	return node->getAttr(name,value);
}

bool Serialization::CXmlInputArchive::operator()(double& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	return node->getAttr(name,value);
}

bool Serialization::CXmlInputArchive::operator()(i16& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	return node->getAttr(name,value);
}

bool Serialization::CXmlInputArchive::operator()(u16& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	return node->getAttr(name,value);
}

bool Serialization::CXmlInputArchive::operator()(i32& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	return node->getAttr(name,value);
}

bool Serialization::CXmlInputArchive::operator()(u32& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
 	return node->getAttr(name,value);
}

bool Serialization::CXmlInputArchive::operator()(int64& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	return node->getAttr(name,value);
}

bool Serialization::CXmlInputArchive::operator()(uint64& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	return node->getAttr(name,value);
}

bool Serialization::CXmlInputArchive::operator()(int8& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	i32 v = value;
	bool res = node->getAttr(name,v);
	value = v;
	return res;
}

bool Serialization::CXmlInputArchive::operator()(u8& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	return node->getAttr(name,value);
}

bool Serialization::CXmlInputArchive::operator()(char& value, tukk name, tukk label)
{
	XmlNodeRef node = (m_childIndexOverride >= 0) ? XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name) : m_pRootNode;
	return node->getAttr(name,value);
}

bool Serialization::CXmlInputArchive::operator()(const SStruct& ser, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);

	XmlNodeRef pChild = XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name);
	if (pChild)
	{
		CXmlInputArchive childArchive(*this,pChild);

		const bool serializeSuccess = ser(childArchive);
		return serializeSuccess;
	}
	return false;
}

bool Serialization::CXmlInputArchive::operator()(SBlackBox& box, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);

	XmlNodeRef pChild = XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name);
	if (pChild)
	{
		pChild->AddRef();
		box.set<XmlNodeRef>("xml", pChild);
		box.xmlVersion = 2;
		return true;
	}
	return false;
}

bool Serialization::CXmlInputArchive::operator()(IContainer& ser, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);

	XmlNodeRef pChild = XmlUtil::FindChildNode(m_pRootNode, m_childIndexOverride, m_childIndexHint, name);
	if (pChild)
	{
		bool serializeSuccess = true;

		i32k elementCount = pChild->getChildCount();
		ser.resize(elementCount);

		if (0 < elementCount)
		{
			CXmlInputArchive childArchive(*this,pChild);

			for (i32 i = 0; i < elementCount; ++i)
			{
				childArchive.m_childIndexOverride = i;

				serializeSuccess &= ser(childArchive, "element", "element");
				ser.next();
			}
		}

		return serializeSuccess;
	}
	return false;
}

void Serialization::CXmlInputArchive::validatorMessage(bool error, ukk handle, const TypeID& type, tukk message)
{
	const EValidatorModule module = VALIDATOR_MODULE_UNKNOWN;
	const EValidatorSeverity severity = error ? VALIDATOR_ERROR : VALIDATOR_WARNING;
	DrxWarning(module, severity, "CXmlInputArchive: %s", message);
}
