// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/HUDUtils.h>
#include <drx3D/Game/UI/UIUpr.h>
#include <drx3D/Game/UI/UICVars.h>
#include <drx3D/Game/UI/Utils/ScreenLayoutUpr.h>

#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/CoreX/String/StringUtils.h>
#include <drx3D/Sys/Scaleform/IFlashPlayer.h>

#include <drx3D/Game/GameRules.h>
#include <drx3D/Game/GameRulesModules/IGameRulesRoundsModule.h>

#include <drx3D/Game/Network/Lobby/GameLobby.h>
#include <drx3D/Game/Network/Squad/SquadUpr.h>

#include <drx3D/Game/Player.h>


static const float MIN_ASPECT_RATIO = (16.0f / 9.0f);

namespace CHUDUtils
{
////////////////////////////////////////////////////////////////////////
static const ColorF s_hUDColor(0.6015625f, 0.83203125f, 0.71484375f, 1.0f);

static tukk g_subtitleCharacters[] =
{
	"PSYCHO",
	"RASCH",
	"PROPHET",
	"CLAIRE",
	"NAX",
	"HIVEMIND",
	"SUIT_VOICE",
};

static const size_t g_subtitleCharactersCount = (DRX_ARRAY_COUNT(g_subtitleCharacters) );

static tukk g_subtitleColors[] =
{
	"92D050", //PSYCHO
	"00B0F0", //RASCH
	"8F68AC", //PROPHET
	"FFCCFF", //CLAIRE
	"F79646", //NAX
	"FF0000", //HIVEMIND
	"FFFF00", //SUIT_VOICE
};


const size_t GetNumSubtitleCharacters()
{
	return g_subtitleCharactersCount;
}

tukk GetSubtitleCharacter(const size_t index)
{
	assert(index>=0 && index<g_subtitleCharactersCount);
	return g_subtitleCharacters[index];
}

tukk GetSubtitleColor(const size_t index)
{
	assert(index>=0 && index<g_subtitleCharactersCount);
	return g_subtitleColors[index];
}



//////////////////////////////////////////////////////////////////////////
void LocalizeString( string &out, tukk text, tukk arg1, tukk arg2, tukk arg3, tukk arg4)
{
#if ENABLE_HUD_EXTRA_DEBUG
	i32k numberOfWs = g_pGame->GetHUD()->GetCVars()->hud_localize_ws_instead;
	if( numberOfWs>0 )
	{
		static i32 lastNumberOfWs=0;
		if( lastNumberOfWs!=numberOfWs )
		{
			for(i32 i=0; i<numberOfWs; i++)
			{
				out.append("W");
			}

			lastNumberOfWs = numberOfWs;
		}
		return;
	}
#endif

	if(!text)
	{
		out = "";
		return;
	}

	string localizedString, param1, param2, param3, param4;
	ILocalizationUpr* pLocMgr = gEnv->pSystem->GetLocalizationUpr();

	if(text[0]=='@')
		pLocMgr->LocalizeString(text, localizedString);
	else
		localizedString = text;

	if(arg1)
	{
		if(arg1[0]=='@')
			pLocMgr->LocalizeString(arg1, param1);
		else
			param1 = arg1;
	}

	if(arg2)
	{
		if(arg2[0]=='@')
			pLocMgr->LocalizeString(arg2, param2);
		else
			param2 = arg2;
	}

	if(arg3)
	{
		if(arg3[0]=='@')
			pLocMgr->LocalizeString(arg3, param3);
		else
			param3 = arg3;
	}

	if(arg4)
	{
		if(arg4[0]=='@')
			pLocMgr->LocalizeString(arg4, param4);
		else
			param4 = arg4;
	}

	out.resize(0);
	pLocMgr->FormatStringMessage(out, localizedString, param1.c_str(), param2.c_str(), param3.c_str(), param4.c_str());
}
//////////////////////////////////////////////////////////////////////////
tukk  LocalizeString( tukk text, tukk arg1, tukk arg2, tukk arg3, tukk arg4 )
{
	//ScopedSwitchToGlobalHeap globalHeap;
	static string charstr;
	LocalizeString( charstr, text, arg1, arg2, arg3, arg4 );

	return charstr.c_str();
}
//////////////////////////////////////////////////////////////////////////
void LocalizeStringn( tuk dest, size_t bufferSizeInBytes, tukk text, tukk arg1 /*= NULL*/, tukk arg2 /*= NULL*/, tukk arg3 /*= NULL*/, tukk arg4 /*= NULL*/ )
{
	drx_strcpy( dest, bufferSizeInBytes, LocalizeString(text, arg1, arg2, arg3, arg4) );
}
//////////////////////////////////////////////////////////////////////////
tukk LocalizeNumber(i32k number)
{
	ILocalizationUpr* pLocMgr = gEnv->pSystem->GetLocalizationUpr();

	//ScopedSwitchToGlobalHeap globalHeap;
	static string charstr;
	pLocMgr->LocalizeNumber(number, charstr);

	return charstr.c_str();
}
//////////////////////////////////////////////////////////////////////////
void LocalizeNumber(string& out, i32k number)
{
	ILocalizationUpr* pLocMgr = gEnv->pSystem->GetLocalizationUpr();

	pLocMgr->LocalizeNumber(number, out);
}
//////////////////////////////////////////////////////////////////////////
void LocalizeNumbern(tuk dest, size_t bufferSizeInBytes, i32k number)
{
	drx_strcpy(dest, bufferSizeInBytes, LocalizeNumber(number));
}
//////////////////////////////////////////////////////////////////////////
tukk LocalizeNumber(const float number, i32 decimals)
{
	ILocalizationUpr* pLocMgr = gEnv->pSystem->GetLocalizationUpr();

	//ScopedSwitchToGlobalHeap globalHeap;
	static string charstr;
	pLocMgr->LocalizeNumber(number, decimals, charstr);

	return charstr.c_str();
}
//////////////////////////////////////////////////////////////////////////
void LocalizeNumber(string& out, const float number, i32 decimals)
{
	ILocalizationUpr* pLocMgr = gEnv->pSystem->GetLocalizationUpr();

	pLocMgr->LocalizeNumber(number, decimals, out);
}
//////////////////////////////////////////////////////////////////////////
void LocalizeNumbern(tuk dest, size_t bufferSizeInBytes, const float number, i32 decimals)
{
	drx_strcpy(dest, bufferSizeInBytes, LocalizeNumber(number, decimals));
}
//////////////////////////////////////////////////////////////////////////

void ConvertSecondsToTimerString( i32k s, string* in_out_string, const bool stripZeroElements/*=false*/, bool keepMinutes/*=false*/, tukk const hex_colour/*=NULL*/ )
{
	i32 hours=0, mins=0, secs=0;
	secs = s;
	hours = (i32)floor(((float)secs)*(1/60.0f)*(1/60.0f));
	secs -= hours*60*60;
	mins = (i32)floor(((float)secs)*(1/60.0f));
	secs -= mins*60;
	string& l_time = (*in_out_string);
	if (stripZeroElements)
	{
		if (hours > 0)
		{
			l_time.Format( "%.2d:%.2d:%.2d", hours, mins, secs );
		}
		else if (mins > 0 || keepMinutes )
		{
			l_time.Format( "%.2d:%.2d", mins, secs );
		}
		else
		{
			l_time.Format( "%.2d", secs );
		}
	}
	else
	{
		l_time.Format( "%.2d:%.2d:%.2d", hours, mins, secs );
	}

	if (hex_colour)
	{
		string formatted_time;
		formatted_time.Format("<FONT color=\"%s\">%s</FONT>", hex_colour, l_time.c_str());
		l_time = formatted_time;
	}
	
	return;
}

////////////////////////////////////////////////////////////////////////

tukk GetFriendlyStateColour( EFriendState friendState )
{
	if (CUICVars* pHUDCvars = g_pGame->GetUI()->GetCVars())
	{
		switch(friendState)
		{
		case eFS_Friendly:
			return pHUDCvars->hud_colour_friend->GetString();
		case eFS_Enemy:
			return pHUDCvars->hud_colour_enemy->GetString();
		case eFS_LocalPlayer:
			return pHUDCvars->hud_colour_localclient->GetString();
		case eFS_Squaddie:
			return pHUDCvars->hud_colour_squaddie->GetString();
		case eFS_Server:
			return pHUDCvars->hud_colour_localclient->GetString();
		};
	}

	return "FFFFFF";
}

const EFriendState GetFriendlyState( const EntityId entityId, CActor* pLocalActor )
{
	if( pLocalActor )
	{
		if (pLocalActor->GetEntityId()==entityId)
		{
			return eFS_LocalPlayer;
		}
		else if( pLocalActor->IsFriendlyEntity( entityId ) )
		{
			if(CGameLobby* pLobby = g_pGame->GetGameLobby())
			{
				if(CSquadUpr* pSM = g_pGame->GetSquadUpr())
				{
					IActor* pActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(entityId);
					if(pActor)
					{
						u16 channelId = pActor->GetChannelId();
						DrxUserID userId = pLobby->GetUserIDFromChannelID(channelId);
						const bool isSquadMember = pSM->IsSquadMateByUserId( userId );
						if (isSquadMember)
						{
							return eFS_Squaddie;
						}
					}
				}
			}

			return eFS_Friendly;
		}
		else
		{
			return eFS_Enemy;
		}
	}

	return eFS_Unknown;
}

IFlashPlayer* GetFlashPlayerFromMaterial( IMaterial* pMaterial, bool bGetTemporary )
{
	IFlashPlayer* pRetFlashPlayer(NULL);
	const SShaderItem& shaderItem(pMaterial->GetShaderItem());
	if (shaderItem.m_pShaderResources)
	{
		SEfResTexture* pTex = shaderItem.m_pShaderResources->GetTexture(EEfResTextures(0));
		if (pTex)
		{
			IDynTextureSource* pDynTexSrc = pTex->m_Sampler.m_pDynTexSource;
			if (pDynTexSrc)
			{
				if (bGetTemporary)
				{
					pRetFlashPlayer = (IFlashPlayer*) pDynTexSrc->GetSourceTemp(IDynTextureSource::DTS_I_FLASHPLAYER);
				}
				else
				{
					pRetFlashPlayer = (IFlashPlayer*) pDynTexSrc->GetSourcePerm(IDynTextureSource::DTS_I_FLASHPLAYER);
				}
			}
		}
	}

	return pRetFlashPlayer;
}

// assume the cgf is loaded at slot 0
// assume that each cgf has only one dynamic flash material
// assume that the dynamic texture is texture 0 on teh shader
IFlashPlayer* GetFlashPlayerFromCgfMaterial( IEntity* pCgfEntity, bool bGetTemporary/*, tukk materialName*/ )
{
	IFlashPlayer* pRetFlashPlayer(NULL);

	// First get the Material from slot 0 on the entity
	assert(pCgfEntity); // should never be NULL

	//IMaterial* pSrcMat = gEnv->p3DEngine->GetMaterialUpr()->LoadMaterial( materialName, false );

	IStatObj* pStatObj = pCgfEntity->GetStatObj(0);	

	//i32k subObjCount = pStatObj->GetSubObjectCount();

	//for( i32 subObjId=0; subObjId<subObjCount; subObjId++ )
	{
		//IStatObj::SSubObject* pSubObject = pStatObj->GetSubObject( subObjId );
		//IStatObj* pSubStatObj = pSubObject->pStatObj;

		IMaterial* pMaterial = pStatObj->GetMaterial();
		if (!pMaterial)
		{
			GameWarning( "HUD: Static object '%s' does not have a material!", pStatObj->GetGeoName() );
			return NULL;
		}

		pRetFlashPlayer = CHUDUtils::GetFlashPlayerFromMaterialIncludingSubMaterials(pMaterial, bGetTemporary);
	}
	return pRetFlashPlayer;
}


IFlashPlayer* GetFlashPlayerFromMaterialIncludingSubMaterials( IMaterial* pMaterial, bool bGetTemporary )
{
	IFlashPlayer* pRetFlashPlayer = CHUDUtils::GetFlashPlayerFromMaterial( pMaterial, bGetTemporary );

	i32k subMtlCount = pMaterial->GetSubMtlCount();
	for (i32 i = 0; i != subMtlCount; ++i)
	{
		IMaterial* pSubMat = pMaterial->GetSubMtl(i);
		if (!pSubMat)
		{
			GameWarning( "HUD: Failed to get unified asset 3D submaterial #%d.", i );
			continue;
		}

		IFlashPlayer* pFlashPlayer = CHUDUtils::GetFlashPlayerFromMaterial( pSubMat, bGetTemporary );
		if(pFlashPlayer)
		{
			if(pRetFlashPlayer)
			{
				GameWarning( "HUD: Multiple flash assets in texture!");
			}

			pRetFlashPlayer = pFlashPlayer;
		}
	}

	return pRetFlashPlayer;
}

void UpdateFlashPlayerViewport(IFlashPlayer* pFlashPlayer, i32 iWidth, i32 iHeight)
{
	if (pFlashPlayer)
	{
		const float assetWidth = (float)pFlashPlayer->GetWidth();
		const float assetHeight = (float)pFlashPlayer->GetHeight();

		const float renderWidth = (float)iWidth;
		const float renderHeight = (float)iHeight;

		if( (renderWidth * __fres(renderHeight)) > MIN_ASPECT_RATIO )
		{
			i32k proposedWidth = int_round( (renderHeight * __fres(assetHeight)) * assetWidth);
			i32k offset = int_round(0.5f * (float)(iWidth - proposedWidth));
			pFlashPlayer->SetViewport(offset, 0, proposedWidth, iHeight);
		}
		else
		{
			i32k proposedHeight = int_round( (renderWidth / assetWidth) * assetHeight);
			i32k offset = int_round(0.5f * (float)(iHeight - proposedHeight));
			pFlashPlayer->SetViewport(0, offset, iWidth, proposedHeight);
		}
	}
}


const ColorF& GetHUDColor()
{
	return s_hUDColor;
}

const float GetIconDepth(const float distance)
{
	// Made into a separate function in case the calculation becomes more complicated at any point
	CUICVars* pCVars = g_pGame->GetUI()->GetCVars();
	return max(distance * pCVars->hud_stereo_icon_depth_multiplier, pCVars->hud_stereo_minDist);
}

i32 GetBetweenRoundsTimer( i32 previousTimer )
{
	i32 roundTime = previousTimer;

	CGameRules *pGameRules = g_pGame->GetGameRules();
	if (pGameRules)
	{
		IGameRulesRoundsModule *pRoundsModule = pGameRules->GetRoundsModule();
		if (pRoundsModule)
		{
			if (pRoundsModule->IsRestarting() && (pRoundsModule->GetRoundEndHUDState() == IGameRulesRoundsModule::eREHS_HUDMessage))
			{
				if (pRoundsModule->GetPreviousRoundWinReason() == EGOR_TimeLimitReached)
				{
					// Round finished by the time limit being hit, force the timer to 0 (may not have reached 0 on clients yet due to network lag)
					roundTime = 0;
				}
			}
			else
			{
				roundTime = static_cast<i32>(60.0f*pGameRules->GetTimeLimit());
			}
		}
	}

	return roundTime;
}

bool IsSensorPackageActive()
{
	return false;
}

static CHUDUtils::TCenterSortArray m_helperArray;
TCenterSortArray& GetCenterSortHelper()
{
	return m_helperArray;
}

uk GetNearestToCenter()
{
	return GetNearestToCenter(20.0f);
}

uk GetNearestToCenter(const float maxValidDistance)
{
	return GetNearestTo(Vec2(50.0f, 50.0f), maxValidDistance);
}

uk GetNearestTo(const Vec2& center, const float maxValidDistance)
{
	return GetNearestTo(GetCenterSortHelper(), center, maxValidDistance);
}

uk GetNearestTo(const TCenterSortArray& array, const Vec2& center, const float maxValidDistance)
{
	i32 nearest = -1;
	float nearestDistSq = sqr(maxValidDistance*1.1f);

	Vec2 renderSize = Vec2(800.0f, 600.0f);

	ScreenLayoutUpr* pLayoutMgr = g_pGame->GetUI()->GetLayoutUpr();
	if(pLayoutMgr)
	{
		renderSize.x = pLayoutMgr->GetRenderWidth();
		renderSize.y = pLayoutMgr->GetRenderHeight();
	}
	else if(gEnv->pRenderer)
	{
		renderSize.x = (float)gEnv->pRenderer->GetWidth();
		renderSize.y = (float)gEnv->pRenderer->GetHeight();
	}

	float xCompression = 1.0f;
	if(renderSize.y>0.0f)
	{
		xCompression = renderSize.x / renderSize.y;
	}

	i32k numPoints = array.size();
	for(i32 i=0; i<numPoints; ++i)
	{
		const SCenterSortPoint& point = array[i];

		Vec2 dir = (point.m_screenPos - center);
		dir.x *= xCompression;

		const float distanceSq = dir.GetLength2();

		if(distanceSq>nearestDistSq)
		{
			continue;
		}

		nearest = i;
		nearestDistSq = distanceSq;
	}

	if(numPoints>0 && nearest<0)
	{
		i32 a=1;
	}

	if(nearest>=0)
	{
		return array[nearest].m_pData;
	}
	return NULL;
}

}
