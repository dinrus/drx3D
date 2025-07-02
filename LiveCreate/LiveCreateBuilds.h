// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

namespace LiveCreate
{
// Date and time of the build
struct SDateTime
{
	SDateTime()
		: year(0)
		, month(0)
		, dayOfWeek(0)
		, day(0)
		, hour(0)
		, minute(0)
		, second(0)
		, milliseconds(0)
	{}

	u16 year;
	u16 month;
	u16 dayOfWeek;
	u16 day;
	u16 hour;
	u16 minute;
	u16 second;
	u16 milliseconds;
};

// Used to get information about the builds on the machines (from BuildInfo.txt)
struct SGameBuildInfo
{
	static i32k kMaxBuildNameSize = 256;
	static i32k kMaxBranchNameSize = 256;
	static i32k kMaxPathSize = 256;
	static i32k kMaxExeNameListSize = 512;

	SGameBuildInfo()
		: buildNumber(0)
		, buildChangelist(0)
		, codeChangelist(0)
		, assetChangelist(0)
	{
		executables[0] = 0;
		buildName[0] = 0;
		codeBranch[0] = 0;
		assetBranch[0] = 0;
		buildPath[0] = 0;
	}

	i32       buildNumber;
	char      buildName[kMaxBuildNameSize];
	SDateTime buildTime;
	i32       buildChangelist;
	i32       codeChangelist;
	i32       assetChangelist;
	char      codeBranch[kMaxBranchNameSize];
	char      assetBranch[kMaxBranchNameSize];
	char      buildPath[kMaxPathSize];
	// comma separated exe names, like: "Drxsis3.xex,Drxsis3Profile.xex"
	char      executables[kMaxExeNameListSize];
};
}
