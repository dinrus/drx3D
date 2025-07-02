// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/Script.h>

#include <drx3D/Sys/IDrxPak.h>
#include <drx3D/Sys/ITimer.h>
#include <drx3D/Schema/ICore.h>
#include <drx3D/Schema/IScriptElement.h>
#include <drx3D/Schema/SerializationToString.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/StringUtils.h>

#include <drx3D/Schema/IScriptRegistry.h>

#include <drx3D/Schema/ScriptSerializers.h>
#include <drx3D/Schema/ScriptRegistry.h>

namespace sxema
{
CScript::CScript(const DrxGUID& guid, tukk szFilePath)
	: m_guid(guid)
	, m_filePath(szFilePath)
	, m_timeStamp(gEnv->pTimer->GetAsyncTime())
	, m_pRoot(nullptr)
{}

CScript::CScript(tukk szFilePath)
	: m_guid()
	, m_filePath(szFilePath)
	, m_timeStamp(gEnv->pTimer->GetAsyncTime())
	, m_pRoot(nullptr)
{

}

DrxGUID CScript::GetGUID() const
{
	return m_guid;
}

tukk CScript::SetFilePath(tukk szFilePath)
{
	if (szFilePath)
	{
		stack_string gameFolder = gEnv->pDrxPak->GetGameFolder();
		gameFolder.MakeLower();
		stack_string filePath = szFilePath;
		filePath.MakeLower();

		if (filePath.find(gameFolder.c_str()) != 0)
		{

			filePath = gEnv->pDrxPak->GetGameFolder();
			filePath.append("/");
			filePath.append(szFilePath);

			switch (m_pRoot->GetType())
			{
			case EScriptElementType::Class:
				{
					filePath.append(".schematyc_ent");
					break;
				}
			default:
				{
					filePath.append(".schematyc_lib");
					break;
				}
			}
			filePath.MakeLower();
		}

		m_filePath = filePath.c_str();
	}

	return m_filePath.c_str();
}

const CTimeValue& CScript::GetTimeStamp() const
{
	return m_timeStamp;
}

void CScript::SetRoot(IScriptElement* pRoot)
{
	m_pRoot = pRoot;
}

IScriptElement* CScript::GetRoot()
{
	return m_pRoot;
}

EVisitStatus CScript::VisitElements(const ScriptElementVisitor& visitor)
{
	SXEMA_CORE_ASSERT(visitor);
	if (m_pRoot && visitor)
	{
		return VisitElementsRecursive(visitor, *m_pRoot);
	}
	return EVisitStatus::Stop;
}

EVisitStatus CScript::VisitElementsRecursive(const ScriptElementVisitor& visitor, IScriptElement& element)
{
	EVisitStatus visitStatus = visitor(element);
	if (visitStatus == EVisitStatus::Continue)
	{
		for (IScriptElement* pChild = element.GetFirstChild(); pChild; pChild = pChild->GetNextSibling())
		{
			if (!pChild->GetScript())
			{
				visitStatus = VisitElementsRecursive(visitor, *pChild);
				if (visitStatus == EVisitStatus::Stop)
				{
					break;
				}
			}
		}
	}
	return visitStatus;
}

void CScript::SetNameFromRootRecursive(CStackString& name, IScriptElement& element)
{
	IScriptElement* pParent = element.GetParent();
	if (pParent && (pParent->GetType() != EScriptElementType::Root))
	{
		SetNameFromRootRecursive(name, *pParent);
		name.append("/");
	}
	name.append(element.GetName());
}
}
