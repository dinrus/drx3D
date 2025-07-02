// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// These are helper classes for containing the data from the generic overwrite dialog.

#include "stdafx.h"
#include "UserOptions.h"

//////////////////////////////////////////////////////////////////////////
CUserOptions::CUserOptionsReferenceCountHelper::CUserOptionsReferenceCountHelper(CUserOptions& roUserOptions) :
	m_roReferencedUserOptionsObject(roUserOptions)
{
	m_roReferencedUserOptionsObject.IncRef();
}
//////////////////////////////////////////////////////////////////////////
CUserOptions::CUserOptionsReferenceCountHelper::~CUserOptionsReferenceCountHelper()
{
	m_roReferencedUserOptionsObject.DecRef();
}
//////////////////////////////////////////////////////////////////////////
CUserOptions::CUserOptions()
{
	m_boToAll = false;
	m_nCurrentOption = ENotSet;
}
//////////////////////////////////////////////////////////////////////////
bool CUserOptions::IsOptionValid()
{
	return m_nCurrentOption != ENotSet;
}
//////////////////////////////////////////////////////////////////////////
i32 CUserOptions::GetOption()
{
	return m_nCurrentOption;
}
//////////////////////////////////////////////////////////////////////////
bool CUserOptions::IsOptionToAll()
{
	return m_boToAll;
}
//////////////////////////////////////////////////////////////////////////
void CUserOptions::SetOption(i32 nNewOption, bool boToAll)
{
	m_nCurrentOption = nNewOption;
	m_boToAll = boToAll;
}
//////////////////////////////////////////////////////////////////////////
i32 CUserOptions::DecRef()
{
	if (m_nNumberOfReferences >= 1)
	{
		--m_nNumberOfReferences;
		if (m_nNumberOfReferences == 0)
		{
			SetOption(CUserOptions::ENotSet, false);
		}
	}
	return m_nNumberOfReferences;
}
//////////////////////////////////////////////////////////////////////////
i32 CUserOptions::IncRef()
{
	++m_nNumberOfReferences;
	return m_nNumberOfReferences;
}
//////////////////////////////////////////////////////////////////////////

