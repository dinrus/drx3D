// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once
////////////////////////////////////////////////////////////////////////////
//
//  DinrusPro 3D Engine Source File.
//  Copyright (C), DinrusPro 3D Studios, 2001-2012
// -------------------------------------------------------------------------
//  File name:   UsedResources.h
//  Created:     28/11/2003 by Timur.
//  Description: Class to gather used resources
//
////////////////////////////////////////////////////////////////////////////

//! Class passed to resource gathering functions
class EDITOR_COMMON_API CUsedResources
{
public:
	typedef std::set<string, stl::less_stricmp<string>> TResourceFiles;

	CUsedResources();
	void Add(tukk pResourceFileName);
	//! validate gathered resources, reports warning if resource is not found
	void Validate();

	TResourceFiles files;
};

