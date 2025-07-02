// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/Ledge.h>
#include <drx3D/Game/LedgeUpr.h>

#include <drx3D/CoreX/Game/IGameVolumes.h>
#include <drx3D/CoreX/Renderer/IRenderAuxGeom.h>

#include <drx3D/Game/EntityUtility/EntityScriptCalls.h>

namespace
{
	bool GetLedgeMarkersInfo(EntityId entityId, IGameVolumes::VolumeInfo& volumeInfo)
	{
		IGameVolumes* pGameVolumesMgr = gEnv->pGame->GetIGameFramework()->GetIGameVolumesUpr();
		if (pGameVolumesMgr != NULL)
		{
			return pGameVolumesMgr->GetVolumeInfoForEntity(entityId, &volumeInfo);
		}

		return false;
	}
}

namespace Ledge
{
	void RegisterEvents( IGameObjectExtension& goExt, IGameObject& gameObject )
	{
		i32k eventID = eGFE_ScriptEvent;
		gameObject.UnRegisterExtForEvents( &goExt, NULL, 0 );
		gameObject.RegisterExtForEvents( &goExt, &eventID, 1 );
	}
}

CLedgeObject::LedgeProperties::LedgeProperties( const IEntity& entity )
{
	memset(this, 0, sizeof(LedgeProperties));

	SmartScriptTable properties;
	IScriptTable* pScriptTable = entity.GetScriptTable();

	if ((pScriptTable != NULL) && pScriptTable->GetValue("Properties", properties))
	{
		// Properties shared by both sides
		bool enabled = true;
		properties->GetValue("bEnabled", enabled);

		bool isThin = false;
		properties->GetValue("bIsThin", isThin);

		bool isWindow = false;
		properties->GetValue("bIsWindow", isWindow);

		properties->GetValue("bLedgeFlipped", ledgeFlipped);
		
		bool ledgeDoubleSide = false;
		properties->GetValue("bLedgeDoubleSide", ledgeDoubleSide);

		ledgeCornerMaxAngle = 0.f;
		properties->GetValue("fCornerMaxAngle", ledgeCornerMaxAngle);

		ledgeCornerEndAdjustAmount = 0.f; 
		properties->GetValue("fCornerEndAdjustAmount", ledgeCornerEndAdjustAmount);

		u16 commonFlags = kLedgeFlag_none;
		commonFlags |= enabled         ? kLedgeFlag_enabled       : 0;
		commonFlags |= isThin          ? kLedgeFlag_isThin        : 0;
		commonFlags |= isWindow        ? kLedgeFlag_isWindow      : 0;
		commonFlags |= ledgeDoubleSide ? kLedgeFlag_isDoubleSided : 0;

		//Properties exclusive to each side

		SmartScriptTable mainSideProperties;
		if (properties->GetValue("MainSide", mainSideProperties))
		{
			bool endCrouched = false;
			mainSideProperties->GetValue("bEndCrouched", endCrouched);

			bool endFalling = false;
			mainSideProperties->GetValue("bEndFalling", endFalling);

			bool usableByMarines = false;
			mainSideProperties->GetValue("bUsableByMarines", usableByMarines);

			tukk ledgeType = NULL;
			mainSideProperties->GetValue("esLedgeType", ledgeType);

			// MainSide flags
			ledgeFlags_MainSide = commonFlags;
			ledgeFlags_MainSide |= endCrouched     ? kLedgeFlag_endCrouched   : 0;
			ledgeFlags_MainSide |= endFalling      ? kLedgeFlag_endFalling    : 0;
			ledgeFlags_MainSide |= usableByMarines ? kledgeFlag_usableByMarines : 0;
			
			if ((ledgeType != NULL))
			{
				if (strcmp(ledgeType, "Vault") == 0)
				{
					ledgeFlags_MainSide |= kLedgeFlag_useVault;
				}
				else if (strcmp(ledgeType, "HighVault") == 0)
				{
					ledgeFlags_MainSide |= kLedgeFlag_useHighVault;
				}
			}
		}

		if (ledgeDoubleSide)
		{
			SmartScriptTable oppositeSideProperties;
			if (properties->GetValue("OppositeSide", oppositeSideProperties))
			{
				bool endCrouched = false;
				oppositeSideProperties->GetValue("bEndCrouched", endCrouched);

				bool endFalling = false;
				oppositeSideProperties->GetValue("bEndFalling", endFalling);

				bool usableByMarines = false;
				oppositeSideProperties->GetValue("bUsableByMarines", usableByMarines);

				tukk ledgeType = NULL;
				oppositeSideProperties->GetValue("esLedgeType", ledgeType);

				// MainSide flags
				ledgeFlags_OppositeSide = commonFlags;
				ledgeFlags_OppositeSide |= endCrouched     ? kLedgeFlag_endCrouched   : 0;
				ledgeFlags_OppositeSide |= endFalling      ? kLedgeFlag_endFalling    : 0;
				ledgeFlags_OppositeSide |= usableByMarines ? kledgeFlag_usableByMarines : 0;
				
				if ((ledgeType != NULL))
				{
					if (strcmp(ledgeType, "Vault") == 0)
					{
						ledgeFlags_OppositeSide |= kLedgeFlag_useVault;
					}
					else if (strcmp(ledgeType, "HighVault") == 0)
					{
						ledgeFlags_OppositeSide |= kLedgeFlag_useHighVault;
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CLedgeObject::CLedgeObject()
	: m_flipped( false )
{

}

CLedgeObject::~CLedgeObject()
{
	if (g_pGame != NULL)
	{
		CLedgeUprEdit* pLedgeUprEdit = g_pGame->GetLedgeUpr()->GetEditorUpr();
		if (pLedgeUprEdit != NULL)
		{
			pLedgeUprEdit->UnregisterLedge( GetEntityId() );
		}
	}
}

bool CLedgeObject::Init( IGameObject * pGameObject )
{
	SetGameObject(pGameObject);

	return true;
}

void CLedgeObject::PostInit( IGameObject * pGameObject )
{
	Ledge::RegisterEvents( *this, *pGameObject );
	// In pure game, all ledge data is loaded by the manager from the level.pak
	if (gEnv->IsEditor())
	{
		ComputeLedgeMarkers();
	}
	else
	{
		EntityScripts::GetEntityProperty( GetEntity(), "bLedgeFlipped", m_flipped );
	}
}

bool CLedgeObject::ReloadExtension( IGameObject * pGameObject, const SEntitySpawnParams &params )
{
	ResetGameObject();
	Ledge::RegisterEvents( *this, *pGameObject );

	DRX_ASSERT_MESSAGE(false, "CLedgeObject::ReloadExtension not implemented");

	return false;
}

bool CLedgeObject::GetEntityPoolSignature( TSerialize signature )
{
	DRX_ASSERT_MESSAGE(false, "CLedgeObject::GetEntityPoolSignature not implemented");

	return true;
}

void CLedgeObject::Release()
{
	delete this;
}

void CLedgeObject::HandleEvent( const SGameObjectEvent& gameObjectEvent )
{
	if ((gameObjectEvent.event == eGFE_ScriptEvent) && (gameObjectEvent.param != NULL))
	{
		tukk eventName = static_cast<tukk >(gameObjectEvent.param);
		if (strcmp(eventName, "enable") == 0)
		{
			g_pGame->GetLedgeUpr()->EnableLedge( GetEntityId(), true );
		}
		else if (strcmp(eventName, "disable") == 0)
		{
			g_pGame->GetLedgeUpr()->EnableLedge( GetEntityId(), false );
		}
	}
}

void CLedgeObject::ProcessEvent( SEntityEvent& entityEvent )
{
	switch( entityEvent.event )
	{

	case ENTITY_EVENT_XFORM:
		{
			UpdateLocation();
		}
		break;

	case ENTITY_EVENT_EDITOR_PROPERTY_CHANGED:
		{
			ComputeLedgeMarkers();
		}
		break;
	}
}

void CLedgeObject::GetMemoryUsage( IDrxSizer *pSizer ) const
{

}

void CLedgeObject::UpdateLocation()
{
	IGameVolumes::VolumeInfo markersInfo;
	if ( (GetLedgeMarkersInfo( GetEntityId(), markersInfo) == false) || (markersInfo.verticesCount < 2) )
		return;

	std::vector<SLedgeMarker> markers;
	markers.resize( markersInfo.verticesCount );

	const Matrix34 entityWorldTM = GetEntity()->GetWorldTM();

	for (u32 markerIdx = 0; markerIdx < markersInfo.verticesCount; ++markerIdx)
	{
		markers[markerIdx].m_worldPosition = entityWorldTM.TransformPoint( markersInfo.pVertices[markerIdx] );
	}

	const float side = IsFlipped() ? 1.0f : -1.0f;
	for (u32 markerIdx = 0; markerIdx < (markersInfo.verticesCount - 1); ++markerIdx)
	{
		const Vec3 ledgeDirection = (markers[markerIdx + 1].m_worldPosition - markers[markerIdx].m_worldPosition);

		markers[markerIdx].m_facingDirection = side * Quat::CreateRotationVDir( ledgeDirection.GetNormalizedSafe( FORWARD_DIRECTION ) ).GetColumn0();
	}
	markers[markersInfo.verticesCount -1].m_facingDirection = FORWARD_DIRECTION;

	g_pGame->GetLedgeUpr()->UpdateLedgeMarkers( GetEntityId(), &markers[0], markersInfo.verticesCount );
}

void CLedgeObject::ComputeLedgeMarkers()
{
	IGameVolumes::VolumeInfo markersInfo;
	if ( (GetLedgeMarkersInfo( GetEntityId(), markersInfo) == false) || (markersInfo.verticesCount < 2) )
		return;

	LedgeProperties ledgeProperties( *GetEntity() );
	
	std::vector<SLedgeMarker> markers;
	markers.resize( markersInfo.verticesCount );
	
	const Matrix34 entityWorldTM = GetEntity()->GetWorldTM();

	for (u32 markerIdx = 0; markerIdx < markersInfo.verticesCount; ++markerIdx)
	{
		markers[markerIdx].m_worldPosition = entityWorldTM.TransformPoint( markersInfo.pVertices[markerIdx] );

		if (markerIdx == 0 || markerIdx == markersInfo.verticesCount-1)
		{
			markers[markerIdx].m_endOrCorner = true;
		}
		else
		{
			markers[markerIdx].m_endOrCorner = false;

			if (markerIdx > 0 && markerIdx < markersInfo.verticesCount-1)
			{
				u32k prevIndex = markerIdx-1;
				u32k nextIndex = markerIdx+1;

				DRX_ASSERT(prevIndex >= 0);
				DRX_ASSERT(nextIndex <= markersInfo.verticesCount-1);

				// we have an edge both sides of this marker. Are we at a corner
				// use entitySpace vertices as we've not calculated worldPos for nextIndex yet
				Vec3 toPrev = markersInfo.pVertices[prevIndex] - markersInfo.pVertices[markerIdx];
				Vec3 toNext = markersInfo.pVertices[nextIndex] - markersInfo.pVertices[markerIdx];

				toPrev.Normalize();
				toNext.Normalize();

				const float result = toPrev.dot(toNext);
				const float angle = RAD2DEG(acos(result));

				if (angle < ledgeProperties.ledgeCornerMaxAngle) 
				{
					markers[markerIdx].m_endOrCorner = true;
				}
			}
		}
	}
	
	const float side = ledgeProperties.ledgeFlipped ? 1.0f : -1.0f;
	for (u32 markerIdx = 0; markerIdx < (markersInfo.verticesCount - 1); ++markerIdx)
	{
		const Vec3 ledgeDirection = (markers[markerIdx + 1].m_worldPosition - markers[markerIdx].m_worldPosition);

		markers[markerIdx].m_facingDirection = side * Quat::CreateRotationVDir( ledgeDirection.GetNormalizedSafe( FORWARD_DIRECTION ) ).GetColumn0();
	}
	markers[markersInfo.verticesCount -1].m_facingDirection = FORWARD_DIRECTION;

	CLedgeUprEdit* pLedgeUprEdit = g_pGame->GetLedgeUpr()->GetEditorUpr();

	u16k staticFlag = IsStatic() ? kLedgeFlag_static : kLedgeFlag_none;

	if (pLedgeUprEdit != NULL)
	{
		pLedgeUprEdit->RegisterLedge( GetEntityId(), &markers[0], markersInfo.verticesCount, (ledgeProperties.ledgeFlags_MainSide | staticFlag), (ledgeProperties.ledgeFlags_OppositeSide | staticFlag), ledgeProperties.ledgeCornerMaxAngle, ledgeProperties.ledgeCornerEndAdjustAmount );
	}

	m_flipped = ledgeProperties.ledgeFlipped;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CLedgeObjectStatic::CLedgeObjectStatic()
{
	DRX_ASSERT_MESSAGE( gEnv->IsEditor(), "Static ledge object should  only be instantiated in the editor!" );
}

CLedgeObjectStatic::~CLedgeObjectStatic()
{

}

