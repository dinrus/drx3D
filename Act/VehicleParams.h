// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __Vehicle_Params__h__
#define __Vehicle_Params__h__

#include "VehicleModificationParams.h"

class CVehicleParams
{
public:
	CVehicleParams(XmlNodeRef root, const CVehicleModificationParams& modificationParams)
		: m_xmlNode(root)
		, m_modificationParams(modificationParams)
	{
	}

	virtual ~CVehicleParams() {}

	tukk getTag() const
	{
		assert(IsValid());

		return m_xmlNode->getTag();
	}

	bool haveAttr(tukk name) const
	{
		assert(IsValid());

		return m_xmlNode->haveAttr(name);
	}

	tukk getAttr(tukk name) const
	{
		assert(IsValid());

		tukk attributeValue = m_xmlNode->getAttr(name);
		tukk* attributeValueAddress = &attributeValue;
		ApplyModification(name, attributeValueAddress);

		return attributeValue;
	}

	bool getAttr(tukk name, tukk* valueOut) const
	{
		return GetAttrImpl(name, valueOut);
	}

	bool getAttr(tukk name, i32& valueOut) const
	{
		return GetAttrImpl(name, valueOut);
	}

	bool getAttr(tukk name, float& valueOut) const
	{
		return GetAttrImpl(name, valueOut);
	}

	bool getAttr(tukk name, bool& valueOut) const
	{
		return GetAttrImpl(name, valueOut);
	}

	bool getAttr(tukk name, Vec3& valueOut) const
	{
		return GetAttrImpl(name, valueOut);
	}

	i32 getChildCount() const
	{
		assert(IsValid());

		return m_xmlNode->getChildCount();
	}

	CVehicleParams getChild(i32 i) const
	{
		assert(IsValid());

		XmlNodeRef childNode = m_xmlNode->getChild(i);
		return CVehicleParams(childNode, m_modificationParams);
	}

	CVehicleParams findChild(tukk id) const
	{
		assert(IsValid());

		XmlNodeRef childNode = m_xmlNode->findChild(id);
		return CVehicleParams(childNode, m_modificationParams);
	}

	operator bool() const { return m_xmlNode != NULL; }

	bool IsValid() const { return m_xmlNode != NULL; }

private:
	template<typename T>
	bool GetAttrImpl(tukk name, T& valueOut) const
	{
		assert(IsValid());

		bool attributeGetSuccess = m_xmlNode->getAttr(name, valueOut);
		ApplyModification(name, valueOut);

		return attributeGetSuccess;
	}

	template<typename T>
	void ApplyModification(tukk name, T& valueOut) const
	{
		assert(IsValid());

		tukk id;
		bool hasId = m_xmlNode->getAttr("id", &id);
		if (hasId)
		{
			m_modificationParams.ApplyModification(id, name, valueOut);
		}
	}

private:
	XmlNodeRef                        m_xmlNode;
	const CVehicleModificationParams& m_modificationParams;
};

#endif
