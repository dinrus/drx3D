// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/************************************************************************

   The dynamic Response itself is responsible for managing its Response segments
   and checking base conditions

************************************************************************/

#pragma once

#include "ResponseSegment.h"

namespace DrxDRS
{
class CVariableValue;
class CVariableCollection;
struct SSignal;

class CConditionParserHelper
{
public:
	static void GetResponseVariableValueFromString(tukk szValueString, CVariableValue* pOutValue);
};

class CResponse
{
public:
	friend class CDataImportHelper;

	CResponse() : m_executionCounter(0), m_lastStartTime(0.0f), m_lastEndTime(0.0f) {}

	CResponseInstance* StartExecution(SSignal& signal);   //the execution itself will/might take some time, therefore we return a ResponseInstance object, which needs to be updated, until the execution has finished.
	CResponseSegment& GetBaseSegment()                      { return m_baseSegment; }
	void              Reset()                               { m_executionCounter = 0; m_lastStartTime = 0.0f; m_lastEndTime = 0.0f; }
	u32            GetExecutionCounter()                 { return m_executionCounter; }
	void              SetExecutionCounter(u32 newAmount) { m_executionCounter = newAmount; }
	float             GetLastStartTime()                    { return m_lastStartTime; }
	void              SetLastStartTime(float newValue)      { m_lastStartTime = newValue; }
	float             GetLastEndTime()                      { return m_lastEndTime; }
	void              SetLastEndTime(float newValue)        { m_lastEndTime = newValue; }

	void              Serialize(Serialization::IArchive& ar);

private:
	float            m_lastStartTime;
	float            m_lastEndTime;
	u32           m_executionCounter;
	CResponseSegment m_baseSegment;
};
}
