// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/********************************************************************
   DrxGame Source File.
   -------------------------------------------------------------------------
   Имя файла:   AIFaceUpr.cpp
   Version:     v1.00
   Описание:

   -------------------------------------------------------------------------
   История:
   - 05:05:2007   12:05 : Created by Kirill Bulatsev

 *********************************************************************/
#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/AIFaceUpr.h>
#include <drx3D/Animation/IDrxAnimation.h>
#include <drx3D/Network/ISerialize.h>
#include <drx3D/Act/IActorSystem.h>

CAIFaceUpr::TExprState CAIFaceUpr::s_Expressions[EE_Count + 1];
//
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CAIFaceUpr::CAIFaceUpr(IEntity* pEntity) :
	m_pEntity(pEntity),
	m_CurrentState(EE_IDLE),
	m_CurrentExprIdx(0),
	m_ExprStartTime(0.f),
	m_ChangeExpressionTimeMs(0)
{

}

//
//------------------------------------------------------------------------------
CAIFaceUpr::~CAIFaceUpr(void)
{

}

//
//------------------------------------------------------------------------------
void CAIFaceUpr::OnReused(IEntity* pEntity)
{
	m_pEntity = pEntity;

	Reset();
}

//
//------------------------------------------------------------------------------
i32 CAIFaceUpr::SelectExpressionTime() const
{
	return drx_random(30000, 50000);
}

//
//------------------------------------------------------------------------------
void CAIFaceUpr::Reset()
{
	m_CurrentState = EE_IDLE;
	m_CurrentExprIdx = 0;
	m_ExprStartTime = 0.f;
}

//
//------------------------------------------------------------------------------
void CAIFaceUpr::PrecacheSequences()
{
	IActorSystem* pASystem = gEnv->pGameFramework->GetIActorSystem();
	IActor* pActor = (pASystem && m_pEntity ? pASystem->GetActor(m_pEntity->GetId()) : 0);

	for (e_ExpressionEvent eventType = e_ExpressionEvent(0); eventType <= EE_Count; eventType = e_ExpressionEvent(eventType + 1))
	{
		const TExprState& stateSequences = s_Expressions[eventType];
		for (i32 stateSequenceIndex = 0, stateSequenceCount = stateSequences.size(); stateSequenceIndex < stateSequenceCount; ++stateSequenceIndex)
		{
			const string& expressionName = stateSequences[stateSequenceIndex];

			if (pActor)
				pActor->PrecacheFacialExpression(expressionName.c_str());
		}
	}
}

//
//------------------------------------------------------------------------------
bool CAIFaceUpr::LoadStatic()
{
	stl::for_each_array(s_Expressions, stl::container_freer());
	string sPath = PathUtil::Make("Libs/Readability/Faces", "Faces.xml");
	return Load(sPath);
	/*
	   m_SoundPacks.clear();
	   //	Load("korean_01.xml");

	   string path( "Libs/Readability/Sound" );
	   IDrxPak * pDrxPak = gEnv->pDrxPak;
	   _finddata_t fd;
	   string fileName;

	   string searchPath(path + "/*.xml");
	   intptr_t handle = pDrxPak->FindFirst( searchPath.c_str(), &fd );
	   if (handle != -1)
	   {
	    do
	    {
	      fileName = path;
	      fileName += "/" ;
	      fileName += fd.name;
	      Load(fileName);
	    } while ( pDrxPak->FindNext( handle, &fd ) >= 0 );
	    pDrxPak->FindClose( handle );
	   }
	   m_reloadID++;
	 */
}

//
//----------------------------------------------------------------------------------------------------------
bool CAIFaceUpr::Load(tukk szPackName)
{
	string sPath = PathUtil::Make("Libs/Readability/Faces", "Faces.xml");

	XmlNodeRef root = GetISystem()->LoadXmlFromFile(sPath);
	if (!root)
		return false;

	XmlNodeRef nodeWorksheet = root->findChild("Worksheet");
	if (!nodeWorksheet)
		return false;

	XmlNodeRef nodeTable = nodeWorksheet->findChild("Table");
	if (!nodeTable)
		return false;

	e_ExpressionEvent curExpression(EE_NONE);
	tukk szSignal = 0;
	for (i32 rowCntr = 0, childN = 0; childN < nodeTable->getChildCount(); ++childN)
	{
		XmlNodeRef nodeRow = nodeTable->getChild(childN);
		if (!nodeRow->isTag("Row"))
			continue;
		++rowCntr;
		if (rowCntr == 1) // Skip the first row, it only have description/header.
			continue;
		tukk szFaceName = 0;

		for (i32 childrenCntr = 0, cellIndex = 1; childrenCntr < nodeRow->getChildCount(); ++childrenCntr, ++cellIndex)
		{
			XmlNodeRef nodeCell = nodeRow->getChild(childrenCntr);
			if (!nodeCell->isTag("Cell"))
				continue;

			if (nodeCell->haveAttr("ss:Index"))
			{
				cellIndex = 0;
				nodeCell->getAttr("ss:Index", cellIndex);
			}
			XmlNodeRef nodeCellData = nodeCell->findChild("Data");
			if (!nodeCellData)
				continue;

			switch (cellIndex)
			{
			case 1:   // Readability Signal name
				szSignal = nodeCellData->getContent();
				curExpression = StringToEnum(szSignal);
				continue;
			case 2:   // The expression name
				szFaceName = nodeCellData->getContent();
				break;
			}
		}
		if (szFaceName != NULL)
			s_Expressions[curExpression].push_back(szFaceName);
	}
	return true;
}

//
//------------------------------------------------------------------------------
CAIFaceUpr::e_ExpressionEvent CAIFaceUpr::StringToEnum(tukk szFaceName)
{
	if (!strcmp(szFaceName, "IDLE"))
		return EE_IDLE;
	else if (!strcmp(szFaceName, "ALERT"))
		return EE_ALERT;
	else if (!strcmp(szFaceName, "COMBAT"))
		return EE_COMBAT;

	return EE_NONE;
}

//
//------------------------------------------------------------------------------
void CAIFaceUpr::Update()
{
	if (m_CurrentState == EE_NONE)
	{
		MakeFace(NULL);
		return;
	}
	int64 timePassedMs((gEnv->pTimer->GetFrameStartTime() - m_ExprStartTime).GetMilliSecondsAsInt64());
	if (timePassedMs > m_ChangeExpressionTimeMs)
		SetExpression(m_CurrentState, true);
}

//
//------------------------------------------------------------------------------
void CAIFaceUpr::SetExpression(e_ExpressionEvent expression, bool forceChange)
{
	if (m_CurrentState == expression && !forceChange)
		return;

	assert(expression >= 0 && expression < EE_Count);
	TExprState& theState = s_Expressions[expression];
	if (!theState.empty())
	{
		m_CurrentExprIdx = drx_random(0, (i32)theState.size() - 1);
		MakeFace(theState[m_CurrentExprIdx]);
	}
	m_ExprStartTime = gEnv->pTimer->GetFrameStartTime();
	m_CurrentState = expression;
}

//
//------------------------------------------------------------------------------
void CAIFaceUpr::MakeFace(tukk pFaceName)
{
	IActorSystem* pASystem = gEnv->pGameFramework->GetIActorSystem();
	if (pASystem)
	{
		IActor* pActor = pASystem->GetActor(m_pEntity->GetId());
		if (pActor)
		{
			f32 length = -1.0f;
			pActor->RequestFacialExpression(pFaceName, &length);
			if (length > 0.0f)
				m_ChangeExpressionTimeMs = (u32)(length * 1000.0f);
		}
	}
}

//----------------------------------------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------------------------------------
