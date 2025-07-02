// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Manages one-off reader/writer usages for Xml serialization

   -------------------------------------------------------------------------
   История:
   - 02:06:2010: Created by Kevin Kirst

*************************************************************************/

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/XmlSerializeHelper.h>

//////////////////////////////////////////////////////////////////////////
CXmlSerializedObject::CXmlSerializedObject(tukk szSection)
	: m_nRefCount(0)
	, m_sTag(szSection)
{
	assert(szSection && szSection[0]);
}

//////////////////////////////////////////////////////////////////////////
void CXmlSerializedObject::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
	if (m_XmlNode)
	{
		m_XmlNode.GetMemoryUsage(pSizer);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CXmlSerializedObject::IsEmpty() const
{
	return (!m_XmlNode || (m_XmlNode->getChildCount() <= 0 && m_XmlNode->getNumAttributes() <= 0));
}

//////////////////////////////////////////////////////////////////////////
void CXmlSerializedObject::Reset()
{
	m_XmlNode = XmlNodeRef();
}

//////////////////////////////////////////////////////////////////////////
void CXmlSerializedObject::Serialize(TSerialize& serialize)
{
	serialize.Value(m_sTag.c_str(), m_XmlNode);
}

//////////////////////////////////////////////////////////////////////////
void CXmlSerializedObject::CreateRootNode()
{
	m_XmlNode = gEnv->pSystem->CreateXmlNode(m_sTag.c_str());
	assert(m_XmlNode);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CXmlSerializeHelper::CXmlSerializeHelper()
	: m_nRefCount(1)
	, m_pSerializer()
{
	m_pSerializer = GetISystem()->GetXmlUtils()->CreateXmlSerializer();
}

//////////////////////////////////////////////////////////////////////////
CXmlSerializeHelper::~CXmlSerializeHelper()
{

}

//////////////////////////////////////////////////////////////////////////
void CXmlSerializeHelper::GetMemoryUsage(IDrxSizer* pSizer) const
{
	pSizer->Add(*this);
	pSizer->AddObject(m_pSerializer);
}

//////////////////////////////////////////////////////////////////////////
_smart_ptr<ISerializedObject> CXmlSerializeHelper::CreateSerializedObject(tukk szSection)
{
	assert(szSection && szSection[0]);
	return _smart_ptr<ISerializedObject>(new CXmlSerializedObject(szSection));
}

//////////////////////////////////////////////////////////////////////////
CXmlSerializedObject* CXmlSerializeHelper::GetXmlSerializedObject(ISerializedObject* pObject)
{
	assert(pObject);

	CXmlSerializedObject* pXmlObject = NULL;
	if (pObject && CXmlSerializedObject::GUID == pObject->GetGUID())
	{
		pXmlObject = static_cast<CXmlSerializedObject*>(pObject);
		assert(pXmlObject);
	}

	return pXmlObject;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlSerializeHelper::Write(ISerializedObject* pObject, TSerializeFunc serializeFunc, uk pArgument /*= NULL*/)
{
	assert(pObject);

	bool bResult = false;

	if (CXmlSerializedObject* pXmlObject = GetXmlSerializedObject(pObject))
	{
		pXmlObject->CreateRootNode();
		ISerialize* pWriter = GetWriter(pXmlObject->GetXmlNode());

		TSerialize stateWriter(pWriter);
		bResult = serializeFunc(stateWriter, pArgument);
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////
bool CXmlSerializeHelper::Read(ISerializedObject* pObject, TSerializeFunc serializeFunc, uk pArgument /*= NULL*/)
{
	assert(pObject);

	bool bResult = false;

	if (CXmlSerializedObject* pXmlObject = GetXmlSerializedObject(pObject))
	{
		ISerialize* pReader = GetReader(pXmlObject->GetXmlNode());

		TSerialize stateReader(pReader);
		bResult = serializeFunc(stateReader, pArgument);
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////
ISerialize* CXmlSerializeHelper::GetWriter(XmlNodeRef& node)
{
	return m_pSerializer->GetWriter(node);
}

//////////////////////////////////////////////////////////////////////////
ISerialize* CXmlSerializeHelper::GetReader(XmlNodeRef& node)
{
	return m_pSerializer->GetReader(node);
}
