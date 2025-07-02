// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IFileChangeMonitor.h
//  Version:     v1.00
//  Created:     27/07/2007 by Adam Rutkowski
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef _IFILECHANGEMONITOR_H_
#define _IFILECHANGEMONITOR_H_
#pragma once

struct IFileChangeListener
{
	enum EChangeType
	{
		eChangeType_Unknown,          //! Error or unknown change type.
		eChangeType_Created,          //! The file was created.
		eChangeType_Deleted,          //! The file was deleted.
		eChangeType_Modified,         //! The file was modified (size changed,write).
		eChangeType_RenamedOldName,   //! This is the old name of a renamed file.
		eChangeType_RenamedNewName    //! This is the new name of a renamed file.
	};

	virtual void OnFileChange(tukk sFilename, EChangeType eType) = 0;
};

struct IFileChangeMonitor
{
	// <interfuscator:shuffle>
	//! Register the path of a file or directory to monitor.
	//! Path is relative to game directory, e.g. "Libs/WoundSystem/" or "Libs/WoundSystem/HitLocations.xml".
	virtual bool RegisterListener(IFileChangeListener* pListener, tukk sMonitorItem) = 0;

	//! This function can be used to monitor files of specific type, e.g.
	//! RegisterListener(pListener, "Animations", "caf").
	virtual bool RegisterListener(IFileChangeListener* pListener, tukk sFolder, tukk sExtension) = 0;

	virtual bool UnregisterListener(IFileChangeListener* pListener) = 0;
	// </interfuscator:shuffle>
};

#endif //_IFILECHANGEMONITOR_H_
