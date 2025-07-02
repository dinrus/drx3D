// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ScriptBind_Movie.cpp
//  Version:     v1.00
//  Created:     8/7/2004 by Timur.
//  Компиляторы:       drux (DinrusPro's Universal Compiler),   mingw-w64-clang-x86_64 2003
//  Описание:
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////
#include <drx3D/Script/StdAfx.h>
#include <drx3D/Script/ScriptBind_Movie.h>
#include <drx3D/Sys/ISystem.h>
#include <drx3D/Movie/IMovieSystem.h>

//////////////////////////////////////////////////////////////////////////
CScriptBind_Movie::CScriptBind_Movie(IScriptSystem* pScriptSystem, ISystem* pSystem)
{
	CScriptableBase::Init(pScriptSystem, pSystem);
	m_pSystem = pSystem;
	m_pMovieSystem = gEnv->pMovieSystem;
	SetGlobalName("Movie");

#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_Movie::

	SCRIPT_REG_TEMPLFUNC(PlaySequence, "sSequenceName");
	SCRIPT_REG_TEMPLFUNC(StopSequence, "sSequenceName");
	SCRIPT_REG_TEMPLFUNC(AbortSequence, "sSequenceName");
	SCRIPT_REG_FUNC(StopAllSequences);
	SCRIPT_REG_FUNC(StopAllCutScenes);
	SCRIPT_REG_FUNC(PauseSequences);
	SCRIPT_REG_FUNC(ResumeSequences);
}

//////////////////////////////////////////////////////////////////////////
CScriptBind_Movie::~CScriptBind_Movie()
{
}

/*! Start a sequence
    @param pszName Name of sequence
 */
//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Movie::PlaySequence(IFunctionHandler* pH, tukk sSequenceName)
{
	bool bResetFx = true;
	if (pH->GetParamCount() == 2)
		pH->GetParam(2, bResetFx);

	IMovieSystem* movieSys = gEnv->pMovieSystem;
	if (movieSys != NULL)
		movieSys->PlaySequence(sSequenceName, NULL, bResetFx, false);

	return pH->EndFunction();
}

/*! Stop a sequence
    @param pszName Name of sequence
 */
//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Movie::StopSequence(IFunctionHandler* pH, tukk sSequenceName)
{
	if (m_pMovieSystem != NULL)
		m_pMovieSystem->StopSequence(sSequenceName);
	return pH->EndFunction();
}
/*! Abort a sequence
   @param pszName Name of sequence
 */
//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Movie::AbortSequence(IFunctionHandler* pH, tukk sSequenceName)
{
	bool bLeaveTime = false;
	if (pH->GetParamCount() == 2)
		pH->GetParam(2, bLeaveTime);
	//
	IAnimSequence* seq = m_pMovieSystem->FindSequence(sSequenceName);
	if (seq)
	{
		if (m_pMovieSystem != NULL)
			m_pMovieSystem->AbortSequence(seq, bLeaveTime);
	}
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Movie::StopAllSequences(IFunctionHandler* pH)
{
	if (m_pMovieSystem != NULL)
		m_pMovieSystem->StopAllSequences();
	return pH->EndFunction();
}

//////////////////////////////////////////////////////////////////////////
i32 CScriptBind_Movie::StopAllCutScenes(IFunctionHandler* pH)
{
	//pGame->StopCurrentCutscene();
	//if (m_pMovieSystem != NULL)
	//  m_pMovieSystem->StopAllCutScenes();
	gEnv->pMovieSystem->StopAllCutScenes();

	return pH->EndFunction();
}

i32 CScriptBind_Movie::PauseSequences(IFunctionHandler* pH)
{
	if (m_pMovieSystem != NULL)
		m_pMovieSystem->Pause();
	return pH->EndFunction();
}

i32 CScriptBind_Movie::ResumeSequences(IFunctionHandler* pH)
{
	if (m_pMovieSystem != NULL)
		m_pMovieSystem->Resume();
	return pH->EndFunction();
}
