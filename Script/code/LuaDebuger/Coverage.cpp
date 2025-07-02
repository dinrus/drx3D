// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Script/StdAfx.h>
#include <drx3D/Script/Coverage.h>

void CLUACodeCoverage::VisitLine(tukk source, i32 line)
{
	TCoverage::iterator it = m_coverage.find(CONST_TEMP_STRING(source));
	if (it == m_coverage.end())
		it = m_coverage.insert(std::make_pair(string(source), TLines())).first;
	it->second.insert(line);
}

void CLUACodeCoverage::ResetFile(tukk source)
{
	TCoverage::iterator it = m_coverage.find(CONST_TEMP_STRING(source));
	if (it != m_coverage.end())
		it->second = TLines();
}

bool CLUACodeCoverage::GetCoverage(tukk source, i32 line) const
{
	TCoverage::const_iterator it = m_coverage.find(CONST_TEMP_STRING(source));
	if (it != m_coverage.end())
		return it->second.find(line) != it->second.end();

	return false;
}

void CLUACodeCoverage::DumpCoverage() const
{
	for (TCoverage::const_iterator it = m_coverage.begin(), eit = m_coverage.end(); it != eit; ++it)
	{
		u32 num = it->second.size();
		if (num)
		{
			DrxLog("%s : %u lines", it->first.c_str(), num);
		}
	}
}
