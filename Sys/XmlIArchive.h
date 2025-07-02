// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __XML_I_ARCHIVE__H__
#define __XML_I_ARCHIVE__H__

#include <drx3D/CoreX/Serialization/IArchive.h>

namespace Serialization
{
class CXmlIArchiveVer1 : public IArchive
{
public:
	CXmlIArchiveVer1();
	CXmlIArchiveVer1(const CXmlIArchiveVer1 &parent,const XmlNodeRef &pRootNode);
	~CXmlIArchiveVer1();

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
	bool operator()(SBlackBox& ser, tukk name = "", tukk label = 0) override;
	bool operator()(IContainer& ser, tukk name = "", tukk label = 0) override;

	void validatorMessage(bool error, ukk handle, const TypeID& type, tukk message) override;
	// ~IArchive

	using IArchive::operator();

private:
	XmlNodeRef m_pRootNode;
	i32        m_childIndexOverride;
	i32        m_childIndexHint;
};

class CXmlInputArchive : public IArchive
{
public:
	CXmlInputArchive();
	CXmlInputArchive(const CXmlInputArchive &parent,const XmlNodeRef &pRootNode);
	~CXmlInputArchive();

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
	bool operator()(SBlackBox& ser, tukk name = "", tukk label = 0) override;
	bool operator()(IContainer& ser, tukk name = "", tukk label = 0) override;

	void validatorMessage(bool error, ukk handle, const TypeID& type, tukk message) override;
	// ~IArchive

	using IArchive::operator();

private:
	XmlNodeRef m_pRootNode;
	i32        m_childIndexOverride;
	i32        m_childIndexHint;
};
}

#endif
