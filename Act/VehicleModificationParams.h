// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __Vehicle_Modification_Params__h__
#define __Vehicle_Modification_Params__h__

class CVehicleModificationParams
{
public:
	CVehicleModificationParams();
	CVehicleModificationParams(XmlNodeRef xmlVehicleData, tukk modificationName);
	virtual ~CVehicleModificationParams();

	template<typename T>
	void ApplyModification(tukk nodeId, tukk attrName, T& attrValueOut) const
	{
		XmlNodeRef modificationNode = GetModificationNode(nodeId, attrName);
		if (modificationNode)
		{
			modificationNode->getAttr("value", attrValueOut);
		}
	}

private:
	void               InitModification(XmlNodeRef xmlModificationData);

	static XmlNodeRef  FindModificationNodeByName(tukk name, XmlNodeRef xmlModificationsGroup);

	void               InitModificationElem(XmlNodeRef xmlElem);

	virtual XmlNodeRef GetModificationNode(tukk nodeId, tukk attrName) const;

private:
	struct Implementation;
	Implementation* m_pImpl;
};

#endif
