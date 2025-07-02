// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/PlayerModifiableValues.h>


CSingleModifiedValue::~CSingleModifiedValue()
{
#if ENABLE_PLAYER_MODIFIABLE_VALUES_DEBUGGING
	for (i32 i = 0; i < MAX_MODIFIERS_PER_VALUE; ++ i)
	{
		DRX_ASSERT_MESSAGE (m_modifiers[i] == NULL, string().Format("%s: Sanity check failed! List of modifiers not cleared out but instance is being destroyed! Shouldn't cause any problems but, equally, should never happen!", m_dbgName));
	}
#endif
}

bool CSingleModifiedValue::IsModifiedBy(const float* thisModifier)
{
	for (i32 i = 0; i < MAX_MODIFIERS_PER_VALUE; ++ i)
	{
		if(thisModifier == m_modifiers[i])
		{
			return true;
		}
	}
	return false;
}

void CSingleModifiedValue::ChangeModifierStateIfRequired(const float * thisModifier, bool onOff)
{
	if(IsModifiedBy(thisModifier) != onOff)
	{
		ChangeModifierState(thisModifier, onOff);
	}
}

void CSingleModifiedValue::AddModifier(const float * thisModifier)
{
#if ENABLE_PLAYER_MODIFIABLE_VALUES_DEBUGGING
	for (i32 i = 0; i < MAX_MODIFIERS_PER_VALUE; ++ i)
	{
		DRX_ASSERT_MESSAGE (thisModifier != m_modifiers[i], string().Format("%s: Pointer already in list!", m_dbgName));
	}
#endif

	m_currentResult = 1.f;

	for (i32 j = 0; j < MAX_MODIFIERS_PER_VALUE; ++ j)
	{
		if (m_modifiers[j] == NULL)
		{
			m_modifiers[j] = thisModifier;
			thisModifier = NULL;
		}

		if (m_modifiers[j] != NULL)
		{
			m_currentResult *= *m_modifiers[j];
		}
	}

#if ENABLE_PLAYER_MODIFIABLE_VALUES_DEBUGGING
	DrxLog ("[PMV] %s value is now %f (just added a modifier)", m_dbgName, m_currentResult);
	DRX_ASSERT_MESSAGE (thisModifier == NULL, string().Format("%s: New pointer not added to list (list already full)", m_dbgName));
#endif
}

void CSingleModifiedValue::RemoveModifier(const float * thisModifier)
{
#if ENABLE_PLAYER_MODIFIABLE_VALUES_DEBUGGING
	bool doneIt = false;
#endif

	m_currentResult = 1.f;

	for (i32 j = 0; j < MAX_MODIFIERS_PER_VALUE; ++ j)
	{
		if (m_modifiers[j] == thisModifier)
		{
			m_modifiers[j] = NULL;

#if ENABLE_PLAYER_MODIFIABLE_VALUES_DEBUGGING
			DRX_ASSERT_MESSAGE (! doneIt, string().Format("%s: Same pointer found twice in list!", m_dbgName));
			doneIt = true;
#endif
		}

		if (m_modifiers[j] != NULL)
		{
			m_currentResult *= *m_modifiers[j];
		}
	}

#if ENABLE_PLAYER_MODIFIABLE_VALUES_DEBUGGING
	DrxLog ("[PMV] %s value is now %f (just removed a modifier)", m_dbgName, m_currentResult);
	DRX_ASSERT_MESSAGE (doneIt, string().Format("%s: Pointer not found in list!", m_dbgName));
#endif
}

//=================================================================================
// Debug-only functions...
//=================================================================================

#if ENABLE_PLAYER_MODIFIABLE_VALUES_DEBUGGING
void CPlayerModifiableValues::DbgInit(IEntity * owner)
{
	AUTOENUM_BUILDNAMEARRAY(s_modifiableValueNames, PlayerModifiableValueIDList);

	for (i32 i = 0; i < kPMV_Num; ++ i)
	{
		m_modifiableValuesArray[i].m_dbgName = s_modifiableValueNames[i];
		m_modifiableValuesArray[i].m_dbgOwnerEntity = owner;
	}
}

void CPlayerModifiableValues::DbgTick()
{
	/*if (g_pGameCVars->g_displayDbgText_pmv)
	{
		for (i32 i = 0; i < kPMV_Num; ++ i)
		{
			float val = m_modifiableValuesArray[i];
			XDbgDisplayForEntityIDFunc (m_modifiableValuesArray[i].m_dbgOwnerEntity->GetId(), string().Format("%s = %f", m_modifiableValuesArray[i].m_dbgName, val));
		}
	}*/
}
#endif
