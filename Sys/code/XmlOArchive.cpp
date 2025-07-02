// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Sys/StdAfx.h>
#include <drx3D/CoreX/Extension/ClassWeaver.h>

#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/CoreX/Serialization/ClassFactory.h>
#include <drx3D/CoreX/Serialization/BlackBox.h>

#include <drx3D/Sys/XmlOArchive.h>

namespace XmlUtil
{
XmlNodeRef CreateChildNode(XmlNodeRef pParent, tukk const name)
{
	DRX_ASSERT(pParent);
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);

	XmlNodeRef pChild = pParent->createNode(name);
	DRX_ASSERT(pChild);

	pParent->addChild(pChild);
	return pChild;
}

template<typename T, typename TIn>
bool WriteChildNodeAs(XmlNodeRef pParent, tukk const name, const TIn& value)
{
	XmlNodeRef pChild = XmlUtil::CreateChildNode(pParent, name);
	DRX_ASSERT(pChild);

	pChild->setAttr("value", static_cast<T>(value));
	return true;
}

template<typename T>
bool WriteChildNode(XmlNodeRef pParent, tukk const name, const T& value)
{
	return WriteChildNodeAs<T>(pParent, name, value);
}
}

Serialization::CXmlOArchiveVer1::CXmlOArchiveVer1()
	: IArchive(OUTPUT | NO_EMPTY_NAMES | XML_VERSION_1)
{
}

Serialization::CXmlOArchiveVer1::CXmlOArchiveVer1(XmlNodeRef pRootNode)
	: IArchive(OUTPUT | NO_EMPTY_NAMES | XML_VERSION_1)
	, m_pRootNode(pRootNode)
{
	DRX_ASSERT(m_pRootNode);
}

Serialization::CXmlOArchiveVer1::~CXmlOArchiveVer1()
{
}

void Serialization::CXmlOArchiveVer1::SetXmlNode(XmlNodeRef pNode)
{
	m_pRootNode = pNode;
}

XmlNodeRef Serialization::CXmlOArchiveVer1::GetXmlNode() const
{
	return m_pRootNode;
}

bool Serialization::CXmlOArchiveVer1::operator()(bool& value, tukk name, tukk label)
{
	tukk const stringValue = value ? "true" : "false";
	return XmlUtil::WriteChildNode(m_pRootNode, name, stringValue);
}

bool Serialization::CXmlOArchiveVer1::operator()(IString& value, tukk name, tukk label)
{
	tukk const stringValue = value.get();
	return XmlUtil::WriteChildNode(m_pRootNode, name, stringValue);
}

bool Serialization::CXmlOArchiveVer1::operator()(IWString& value, tukk name, tukk label)
{
	DrxFatalError("CXmlOArchive::operator() with IWString is not implemented");
	return false;
}

bool Serialization::CXmlOArchiveVer1::operator()(float& value, tukk name, tukk label)
{
	return XmlUtil::WriteChildNode(m_pRootNode, name, value);
}

bool Serialization::CXmlOArchiveVer1::operator()(double& value, tukk name, tukk label)
{
	return XmlUtil::WriteChildNode(m_pRootNode, name, value);
}

bool Serialization::CXmlOArchiveVer1::operator()(i16& value, tukk name, tukk label)
{
	return XmlUtil::WriteChildNodeAs<i32>(m_pRootNode, name, value);
}

bool Serialization::CXmlOArchiveVer1::operator()(u16& value, tukk name, tukk label)
{
	return XmlUtil::WriteChildNodeAs<uint>(m_pRootNode, name, value);
}

bool Serialization::CXmlOArchiveVer1::operator()(i32& value, tukk name, tukk label)
{
	return XmlUtil::WriteChildNode(m_pRootNode, name, value);
}

bool Serialization::CXmlOArchiveVer1::operator()(u32& value, tukk name, tukk label)
{
	return XmlUtil::WriteChildNode(m_pRootNode, name, value);
}

bool Serialization::CXmlOArchiveVer1::operator()(int64& value, tukk name, tukk label)
{
	return XmlUtil::WriteChildNode(m_pRootNode, name, value);
}

bool Serialization::CXmlOArchiveVer1::operator()(uint64& value, tukk name, tukk label)
{
	return XmlUtil::WriteChildNode(m_pRootNode, name, value);
}

bool Serialization::CXmlOArchiveVer1::operator()(int8& value, tukk name, tukk label)
{
	return XmlUtil::WriteChildNodeAs<i32>(m_pRootNode, name, value);
}

bool Serialization::CXmlOArchiveVer1::operator()(u8& value, tukk name, tukk label)
{
	return XmlUtil::WriteChildNodeAs<uint>(m_pRootNode, name, value);
}

bool Serialization::CXmlOArchiveVer1::operator()(char& value, tukk name, tukk label)
{
	return XmlUtil::WriteChildNodeAs<i32>(m_pRootNode, name, value);
}

bool Serialization::CXmlOArchiveVer1::operator()(const SStruct& ser, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);

	XmlNodeRef pChild = XmlUtil::CreateChildNode(m_pRootNode, name);
	CXmlOArchiveVer1 childArchive(pChild);
	childArchive.setFilter(getFilter());
	childArchive.setLastContext(lastContext());

	const bool serializeSuccess = ser(childArchive);

	return serializeSuccess;
}

bool Serialization::CXmlOArchiveVer1::operator()(SBlackBox& box, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);
	DRX_ASSERT(m_pRootNode->isTag(name));

	if ((strcmp(box.format, "xml") != 0) || !box.data)
		return false;

	m_pRootNode->addChild(*static_cast<XmlNodeRef*>(box.data));

	return true;
}

bool Serialization::CXmlOArchiveVer1::operator()(IContainer& ser, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);

	bool serializeSuccess = true;

	XmlNodeRef pChild = XmlUtil::CreateChildNode(m_pRootNode, name);
	CXmlOArchiveVer1 childArchive(pChild);
	childArchive.setFilter(getFilter());
	childArchive.setLastContext(lastContext());

	const size_t containerSize = ser.size();
	if (0 < containerSize)
	{
		do
		{
			serializeSuccess &= ser(childArchive, "Element", "Element");
		}
		while (ser.next());
	}

	return serializeSuccess;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Serialization::CXmlOutputArchive::CXmlOutputArchive()
	: IArchive(OUTPUT | NO_EMPTY_NAMES)
{
}

Serialization::CXmlOutputArchive::CXmlOutputArchive(const CXmlOutputArchive &ar)
	: IArchive(ar.caps_)
	, m_pRootNode(ar.m_pRootNode)
{
	filter_ = ar.filter_;
	modifiedRow_ = ar.modifiedRow_;
	lastContext_ = ar.lastContext_;
	DRX_ASSERT(m_pRootNode);
}

Serialization::CXmlOutputArchive::~CXmlOutputArchive()
{
}

void Serialization::CXmlOutputArchive::SetXmlNode(XmlNodeRef pNode)
{
	m_pRootNode = pNode;
}

XmlNodeRef Serialization::CXmlOutputArchive::GetXmlNode() const
{
	return m_pRootNode;
}

bool Serialization::CXmlOutputArchive::operator()(bool& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	tukk const stringValue = value ? "true" : "false";
	node->setAttr(name, stringValue);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(IString& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	tukk const stringValue = value.get();
	node->setAttr(name, stringValue);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(IWString& value, tukk name, tukk label)
{
	DRX_ASSERT_MESSAGE(0,"CXmlOutputArchive::operator() with IWString is not implemented");
	DrxFatalError("CXmlOutputArchive::operator() with IWString is not implemented");
	return false;
}

bool Serialization::CXmlOutputArchive::operator()(float& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	node->setAttr(name, value);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(double& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	node->setAttr(name, value);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(i16& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	node->setAttr(name, value);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(u16& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	node->setAttr(name, value);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(i32& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	node->setAttr(name, value);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(u32& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	node->setAttr(name, value);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(int64& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	node->setAttr(name, value);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(uint64& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	node->setAttr(name, value);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(int8& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	node->setAttr(name, value);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(u8& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	node->setAttr(name, value);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(char& value, tukk name, tukk label)
{
	XmlNodeRef node = (!m_bArray) ? m_pRootNode : XmlUtil::CreateChildNode(m_pRootNode, name);
	node->setAttr(name, value);
	return true;
}

bool Serialization::CXmlOutputArchive::operator()(const SStruct& ser, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);

	XmlNodeRef pChild = XmlUtil::CreateChildNode(m_pRootNode, name);
	CXmlOutputArchive childArchive(*this);
	childArchive.SetXmlNode(pChild);

	const bool serializeSuccess = ser(childArchive);

	return serializeSuccess;
}

bool Serialization::CXmlOutputArchive::operator()(SBlackBox& box, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);
	DRX_ASSERT(m_pRootNode->isTag(name));

	if ((strcmp(box.format, "xml") != 0) || !box.data)
		return false;

	m_pRootNode->addChild(*static_cast<XmlNodeRef*>(box.data));

	return true;
}

bool Serialization::CXmlOutputArchive::operator()(IContainer& ser, tukk name, tukk label)
{
	DRX_ASSERT(name);
	DRX_ASSERT(name[0]);

	bool serializeSuccess = true;

	CXmlOutputArchive childArchive(*this);
	childArchive.SetXmlNode(XmlUtil::CreateChildNode(m_pRootNode, name));
	childArchive.m_bArray = true;

	const size_t containerSize = ser.size();
	if (0 < containerSize)
	{
		do
		{
			serializeSuccess &= ser(childArchive, "element", "element");
		} while (ser.next());
	}

	return serializeSuccess;
}
