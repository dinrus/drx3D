// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __XML_O_ARCHIVE__H__
#define __XML_O_ARCHIVE__H__

#include <drx3D/CoreX/Serialization/IArchive.h>

namespace Serialization
{
class CXmlOArchiveVer1 : public IArchive
{
public:
	CXmlOArchiveVer1();
	CXmlOArchiveVer1(XmlNodeRef pRootNode);
	~CXmlOArchiveVer1();

	void       SetXmlNode(XmlNodeRef pNode);
	XmlNodeRef GetXmlNode() const;

	// IArchive
	bool operator()(bool& value, tukk name = "", tukk label = 0) override;
	bool operator()(IString& value, tukk name = "", tukk label = 0) override;
	bool operator()(IWString& value, tukk name = "", tukk label = 0) override;
	bool operator()(float& value, tukk name = "", tukk label = 0) override;
	bool operator()(double& value, tukk name = "", tukk label = 0) override;
	bool operator()(i16& value, tukk name = "", tukk label = 0) override;
	bool operator()(u16& value, tukk name = "", tukk label = 0) override;
	bool operator()(i32& value, tukk name = "", tukk label = 0) override;
	bool operator()(u32& value, tukk name = "", tukk label = 0) override;
	bool operator()(int64& value, tukk name = "", tukk label = 0) override;
	bool operator()(uint64& value, tukk name = "", tukk label = 0) override;

	bool operator()(int8& value, tukk name = "", tukk label = 0) override;
	bool operator()(u8& value, tukk name = "", tukk label = 0) override;
	bool operator()(char& value, tukk name = "", tukk label = 0) override;

	bool operator()(const SStruct& ser, tukk name = "", tukk label = 0) override;
	bool operator()(IContainer& ser, tukk name = "", tukk label = 0) override;
	bool operator()(SBlackBox& box, tukk name = "", tukk label = 0) override;
	// ~IArchive

	using IArchive::operator();

private:
	XmlNodeRef m_pRootNode;
};

class CXmlOutputArchive : public IArchive
{
public:
	CXmlOutputArchive();
	CXmlOutputArchive(const CXmlOutputArchive &ar);
	~CXmlOutputArchive();

	void       SetXmlNode(XmlNodeRef pNode);
	XmlNodeRef GetXmlNode() const;

	// IArchive
	bool operator()(bool& value, tukk name = "", tukk label = 0) override;
	bool operator()(IString& value, tukk name = "", tukk label = 0) override;
	bool operator()(IWString& value, tukk name = "", tukk label = 0) override;
	bool operator()(float& value, tukk name = "", tukk label = 0) override;
	bool operator()(double& value, tukk name = "", tukk label = 0) override;
	bool operator()(i16& value, tukk name = "", tukk label = 0) override;
	bool operator()(u16& value, tukk name = "", tukk label = 0) override;
	bool operator()(i32& value, tukk name = "", tukk label = 0) override;
	bool operator()(u32& value, tukk name = "", tukk label = 0) override;
	bool operator()(int64& value, tukk name = "", tukk label = 0) override;
	bool operator()(uint64& value, tukk name = "", tukk label = 0) override;

	bool operator()(int8& value, tukk name = "", tukk label = 0) override;
	bool operator()(u8& value, tukk name = "", tukk label = 0) override;
	bool operator()(char& value, tukk name = "", tukk label = 0) override;

	bool operator()(const SStruct& ser, tukk name = "", tukk label = 0) override;
	bool operator()(IContainer& ser, tukk name = "", tukk label = 0) override;
	bool operator()(SBlackBox& box, tukk name = "", tukk label = 0) override;
	// ~IArchive

	using IArchive::operator();

private:
	XmlNodeRef m_pRootNode;
	bool m_bArray = false;
};
}

#endif
