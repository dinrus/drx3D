// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/DynRespSys/stdafx.h>
#include <drx3D/DynRespSys/IDynamicResponseCondition.h>
#include <drx3D/CoreX/String/StringUtils.h>

#include <drx3D/DynRespSys/Response.h>
#include <drx3D/DynRespSys/ResponseInstance.h>
#include <drx3D/DynRespSys/ResponseUpr.h>
#include <drx3D/DynRespSys/ResponseSegment.h>
#include <drx3D/DynRespSys/VariableCollection.h>
#include <drx3D/DynRespSys/ResponseSystem.h>
#include <drx3D/DynRespSys/ConditionsCollection.h>
#include <drx3D/CoreX/Serialization/Enum.h>
#include <drx3D/CoreX/Serialization/Decorators/BitFlags.h>

using namespace DrxDRS;

void CConditionParserHelper::GetResponseVariableValueFromString(tukk szValueString, CVariableValue* pOutValue)
{
	for (i32 i = 0; szValueString[i] != '\0'; i++)
	{
		if (szValueString[i] == '.' && i > 0 && isdigit(szValueString[i+1]))
		{
			//it's (probably) a float
			float valueAsFloat = (float)atof(szValueString);
			if (valueAsFloat == 0.0f && szValueString[i-1] != '0') //atof will return 0 as an error code, so we do a very simple check, if 0 here means the value 0 or the error code
			{
				continue;
			}
			*pOutValue = valueAsFloat;
			return;

		}
		else if (szValueString[i] >= 'A' && szValueString[i] <= 'z')
		{
			if (stricmp(szValueString, "true") == 0)
			{
				//it's a boolean
				*pOutValue = CVariableValue(true);
			}
			else if (stricmp(szValueString, "false") == 0)
			{
				//it's a boolean
				*pOutValue = CVariableValue(false);
			}
			else
			{
				//its a string
				*pOutValue = CVariableValue(CHashedString(szValueString));
			}
			return;
		}
	}

	*pOutValue = CVariableValue(atoi(szValueString));
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

CResponseInstance* CResponse::StartExecution(SSignal& signal)
{
	DRS_DEBUG_DATA_ACTION(AddResponseStarted(signal.m_signalName.GetText()));
	DRS_DEBUG_DATA_ACTION(AddResponseSegmentEvaluated(&m_baseSegment));

	++m_executionCounter;
	CResponseInstance temp(signal, this);
	if (m_baseSegment.AreConditionsMet(&temp))
	{
		m_lastStartTime = CResponseSystem::GetInstance()->GetCurrentDrsTime();
		CResponseInstance* newResponseInstance = CResponseSystem::GetInstance()->GetResponseUpr()->CreateInstance(signal, this);
		newResponseInstance->Execute();
		return newResponseInstance;
	}
	else
	{
		--m_executionCounter;
		DRS_DEBUG_DATA_ACTION(AddResponseInstanceFinished(CResponseSystemDebugDataProvider::eER_NoValidSegment));
	}
	return nullptr;
}

//--------------------------------------------------------------------------------------------------
void CResponse::Serialize(Serialization::IArchive& ar)
{
	ar(m_baseSegment, "BaseSegment", "+<");
}
