#ifndef ROBOT_LOGGING_UTIL_H
#define ROBOT_LOGGING_UTIL_H

#include <drx3D/Maths/Linear/AlignedObjectArray.h>
#include <string>

struct MinitaurLogValue
{
	MinitaurLogValue()
		: m_intVal(0xcdcdcdcd)
	{
	}
	MinitaurLogValue(i32 iv)
		: m_intVal(iv)
	{
	}
	MinitaurLogValue(float fv)
		: m_floatVal(fv)
	{
	}
	MinitaurLogValue(char fv)
		: m_charVal(fv)
	{
	}

	union {
		char m_charVal;
		i32 m_intVal;
		float m_floatVal;
	};
};

struct MinitaurLogRecord
{
	AlignedObjectArray<MinitaurLogValue> m_values;
};

enum MINITAUR_LOG_ERROR
{
	eMinitaurFileNotFound = -1,
	eCorruptHeader = -2,
	eUnknownType = -3,
	eCorruptValue = -4,
	eInvalidAABBAlignCheck = -5,
};

i32 readMinitaurLogFile(tukk fileName, AlignedObjectArray<STxt>& structNames, STxt& structTypes, AlignedObjectArray<MinitaurLogRecord>& logRecords, bool verbose);

FILE* createMinitaurLogFile(tukk fileName, AlignedObjectArray<STxt>& structNames, STxt& structTypes);
void appendMinitaurLogData(FILE* f, STxt& structTypes, const MinitaurLogRecord& logData);
void closeMinitaurLogFile(FILE* f);

#endif  //ROBOT_LOGGING_UTIL_H
