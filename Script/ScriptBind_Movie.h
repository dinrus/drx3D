// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ScriptBind_Movie.h
//  Version:     v1.00
//  Created:     8/7/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ScriptBind_Movie_h__
#define __ScriptBind_Movie_h__
#pragma once

#include <drx3D/Script/IScriptSystem.h>

//! <description>Interface to movie system.</description>
class CScriptBind_Movie : public CScriptableBase
{
public:
	CScriptBind_Movie(IScriptSystem* pScriptSystem, ISystem* pSystem);
	virtual ~CScriptBind_Movie();
	virtual void GetMemoryUsage(IDrxSizer* pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
	}

	//! <code>Movie.PlaySequence(sSequenceName)</code>
	//!		<param name="sSequenceName">Sequence name.</param>
	//! <description>Plays the specified sequence.</description>
	i32 PlaySequence(IFunctionHandler* pH, tukk sSequenceName);

	//! <code>Movie.StopSequence(sSequenceName)</code>
	//!		<param name="sSequenceName">Sequence name.</param>
	//! <description>Stops the specified sequence.</description>
	i32 StopSequence(IFunctionHandler* pH, tukk sSequenceName);

	//! <code>Movie.AbortSequence(sSequenceName)</code>
	//!		<param name="sSequenceName">Sequence name.</param>
	//! <description>Aborts the specified sequence.</description>
	i32 AbortSequence(IFunctionHandler* pH, tukk sSequenceName);

	//! <code>Movie.StopAllSequences()</code>
	//! <description>Stops all the video sequences.</description>
	i32 StopAllSequences(IFunctionHandler* pH);

	//! <code>Movie.StopAllCutScenes()</code>
	//! <description>Stops all the cut scenes.</description>
	i32 StopAllCutScenes(IFunctionHandler* pH);

	//! <code>Movie.PauseSequences()</code>
	//! <description>Pauses all the sequences.</description>
	i32 PauseSequences(IFunctionHandler* pH);

	//! <code>Movie.ResumeSequences()</code>
	//! <description>Resume all the sequences.</description>
	i32 ResumeSequences(IFunctionHandler* pH);
private:
	ISystem*      m_pSystem;
	IMovieSystem* m_pMovieSystem;
};

#endif // __ScriptBind_Movie_h__
