// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Entity/stdafx.h>
#include <drx3D/Entity/ScriptProperties.h>
#include <drx3D/Script/IScriptSystem.h>

//////////////////////////////////////////////////////////////////////////
bool CScriptProperties::SetProperties(XmlNodeRef& entityNode, IScriptTable* pEntityTable)
{
	XmlNodeRef propsNode = entityNode->findChild("Properties");
	if (propsNode)
	{
		SmartScriptTable pPropsTable;
		if (pEntityTable->GetValue("Properties", pPropsTable))
		{
			Assign(propsNode, pPropsTable);
		}
	}

	propsNode = entityNode->findChild("Properties2");
	if (propsNode)
	{
		SmartScriptTable pPropsTable;
		if (pEntityTable->GetValue("PropertiesInstance", pPropsTable))
		{
			Assign(propsNode, pPropsTable);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CScriptProperties::Assign(XmlNodeRef& propsNode, IScriptTable* pPropsTable)
{
	tukk key = "";
	tukk value = "";
	i32 nAttrs = propsNode->getNumAttributes();
	for (i32 attr = 0; attr < nAttrs; attr++)
	{
		if (!propsNode->getAttributeByIndex(attr, &key, &value))
			continue;

		ScriptVarType varType = pPropsTable->GetValueType(key);
		switch (varType)
		{
		case svtNull:
			break;
		case svtString:
			pPropsTable->SetValue(key, value);
			break;
		case svtNumber:
			{
				float fValue = (float)atof(value);
				pPropsTable->SetValue(key, fValue);
			}
			break;
		case svtBool:
			{
				bool const bValue = (stricmp(value, "true") == 0) || (stricmp(value, "1") == 0);
				pPropsTable->SetValue(key, bValue);
			}
			break;
		case svtObject:
			{
				Vec3 vec;
				propsNode->getAttr(key, vec);
				CScriptVector vecTable;
				pPropsTable->GetValue(key, vecTable);
				vecTable.Set(vec);
				//pPropsTable->SetValue( key,vec );
			}
			break;
		case svtPointer:
		case svtUserData:
		case svtFunction:
			// Ignore invalid property types.
			break;
		}
	}

	for (i32 i = 0; i < propsNode->getChildCount(); i++)
	{
		XmlNodeRef childNode = propsNode->getChild(i);
		SmartScriptTable pChildPropTable;
		if (pPropsTable->GetValue(childNode->getTag(), pChildPropTable))
		{
			// Recurse.
			Assign(childNode, pChildPropTable);
		}
	}
}
