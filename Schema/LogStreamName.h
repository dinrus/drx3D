// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/Schema/TypeDesc.h>
#include <drx3D/Schema/ILog.h>

namespace sxema
{
struct SLogStreamName
{
	inline SLogStreamName()
		: value(gEnv->pSchematyc->GetLog().GetStreamName(LogStreamId::Default))
	{}

	inline SLogStreamName(tukk _szValue)
		: value(_szValue)
	{}

	inline bool operator==(const SLogStreamName& rhs) const
	{
		return value == rhs.value;
	}

	static void ReflectType(CTypeDesc<SLogStreamName>& desc)
	{
		desc.SetGUID("e43f1341-ac7c-4dd7-892f-097810eee478"_drx_guid);
	}

	string value;
};

inline bool Serialize(Serialization::IArchive& archive, SLogStreamName& value, tukk szName, tukk szLabel)
{
	if (archive.isEdit())
	{
		Serialization::StringList logStreamNames;

		auto visitLogStream = [&logStreamNames](tukk szStreamName, LogStreamId streamId) -> EVisitStatus
		{
			logStreamNames.push_back(szStreamName);
			return EVisitStatus::Continue;
		};
		gEnv->pSchematyc->GetLog().VisitStreams(visitLogStream);

		if (archive.isInput())
		{
			Serialization::StringListValue temp(logStreamNames, 0);
			archive(temp, szName, szLabel);
			value.value = temp.c_str();
		}
		else if (archive.isOutput())
		{
			i32k pos = logStreamNames.find(value.value.c_str());
			archive(Serialization::StringListValue(logStreamNames, pos), szName, szLabel);
		}
		return true;
	}
	else
	{
		return archive(value.value, szName, szLabel);
	}
}
} // sxema