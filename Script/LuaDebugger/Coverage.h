// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _COVERAGE_H_
#define _COVERAGE_H_

#if _MSC_VER > 1000
	#pragma once
#endif

#include <drx3D/CoreX/Containers/VectorMap.h>

class CLUACodeCoverage
{
	typedef std::set<i32>             TLines;
	typedef VectorMap<string, TLines> TCoverage;
	typedef std::vector<TCoverage>    TCoverageStack;
public:
	void VisitLine(tukk source, i32 line);
	void ResetFile(tukk source);
	bool GetCoverage(tukk source, i32 line) const;
	void DumpCoverage() const;
private:
	TCoverage m_coverage;
};

#endif //#ifndef _COVERAGE_H_
