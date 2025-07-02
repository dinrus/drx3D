// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   IDialogSystem.h
//  Version:     v1.00
//  Created:     26/7/2006 by AlexL.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание: Interface of the DialogSystem [mainly for Editor->DinrusAction]
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#pragma once

struct IDialogScriptIterator
{
	struct SDialogScript
	{
		tukk id;
		tukk desc;
		i32         numRequiredActors;
		i32         numLines;
		bool        bIsLegacyExcel;
	};
	// <interfuscator:shuffle>
	virtual ~IDialogScriptIterator(){}
	virtual void AddRef() = 0;
	virtual void Release() = 0;
	virtual bool Next(SDialogScript&) = 0;
	// </interfuscator:shuffle>
};
typedef _smart_ptr<IDialogScriptIterator> IDialogScriptIteratorPtr;

struct IDialogSystem
{
	// <interfuscator:shuffle>
	virtual ~IDialogSystem(){}
	virtual bool                     Init() = 0;
	virtual void                     Update(const float dt) = 0;
	virtual void                     Reset(bool bUnload) = 0;
	virtual bool                     ReloadScripts(tukk levelName) = 0;
	virtual IDialogScriptIteratorPtr CreateScriptIterator() = 0;

	//! \return true if entity is currently playing a dialog.
	virtual bool IsEntityInDialog(EntityId entityId) const = 0;
	// </interfuscator:shuffle>
};
