// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Act/StdAfx.h>
#include <drx3D/Act/Analyst.h>
#include <drx3D/Act/ILevelSystem.h>
#include <drx3D/Act/IGameRulesSystem.h>

//------------------------------------------------------------------------
CGameplayAnalyst::CGameplayAnalyst()
{
}

//------------------------------------------------------------------------
CGameplayAnalyst::~CGameplayAnalyst()
{

}

void CGameplayAnalyst::ProcessPlayerEvent(EntityId id, const GameplayEvent& event)
{
	switch (event.event)
	{
	case eGE_Connected:
		{
			IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id);
			NewPlayer(pEntity);
		}
		break;
	case eGE_Renamed:
		GetPlayer(id).name = event.description;
		break;
	case eGE_Kill:
		{
			PlayerAnalysis& player = GetPlayer(id);
			player.kills++;
			player.suit.kills[player.suit.mode]++;
			GetWeapon(id, ((EntityId*)event.extra)[0]).kills++;
		}
		break;
	case eGE_Death:
		{
			PlayerAnalysis& player = GetPlayer(id);
			player.deaths++;

			GetCurrentWeapon(id).deaths++;
			player.suit.deaths[player.suit.mode]++;

			CTimeValue now = gEnv->pTimer->GetFrameStartTime();

			if (player.deathStart.GetMilliSeconds() == 0)
				player.deathStart = now;

			if (player.alive)
				player.timeAlive += now - player.deathStart;

			player.deathStart = now;
			player.alive = false;
		}
		break;

	case eGE_Revive:
		{
			PlayerAnalysis& player = GetPlayer(id);
			CTimeValue now = gEnv->pTimer->GetFrameStartTime();

			if (player.deathStart.GetMilliSeconds() == 0)
				player.deathStart = now;

			if (!player.alive)
				player.timeDead += now - player.deathStart;
			player.deathStart = now;
			player.alive = true;
		}
		break;

	case eGE_WeaponHit:
		GetPlayer(id).hits++;
		GetWeapon(id, ((EntityId*)event.extra)[0]).hits++;
		break;
	case eGE_WeaponShot:
		GetPlayer(id).shots += (i32)event.value;
		GetWeapon(id, (EntityId)(TRUNCATE_PTR)event.extra).shots += (i32)event.value;
		break;
	case eGE_WeaponMelee:
		GetPlayer(id).melee++;
		GetWeapon(id, (EntityId)(TRUNCATE_PTR)event.extra).melee++;
		break;
	case eGE_WeaponReload:
		GetPlayer(id).reloads++;
		GetWeapon(id, (EntityId)(TRUNCATE_PTR)event.extra).reloads++;
		break;
	case eGE_WeaponFireModeChanged:
		GetPlayer(id).firemodes++;
		GetWeapon(id, (EntityId)(TRUNCATE_PTR)event.extra).firemodes++;
		break;
	case eGE_ZoomedIn:
		GetPlayer(id).zoomUsage++;
		GetWeapon(id, (EntityId)(TRUNCATE_PTR)event.extra).zoomUsage++;
		break;

	case eGE_ItemSelected:
		GetPlayer(id).itemUsage;
		GetWeapon(id, (EntityId)(TRUNCATE_PTR)event.extra).usage++;
		break;

	case eGE_Damage:
		GetPlayer(id).damage += (i32)event.value;
		GetWeapon(id, (EntityId)(TRUNCATE_PTR)event.extra).damage += (i32)event.value;
		break;

	case eGE_SuitModeChanged:
		{
			PlayerAnalysis& player = GetPlayer(id);
			SuitAnalysis& suit = player.suit;

			CTimeValue now = gEnv->pTimer->GetFrameStartTime();

			if (suit.usageStart.GetMilliSeconds() == 0)
				suit.usageStart = player.timeStart;

			suit.timeUsed[suit.mode] += now - suit.usageStart;

			suit.mode = (i32)event.value;
			suit.usage[(i32)event.value]++;
			suit.usageStart = now;
		}
		break;

	case eGE_Rank:
		{
			PlayerAnalysis& player = GetPlayer(id);

			CTimeValue now = gEnv->pTimer->GetFrameStartTime();

			if (player.rankStart.GetMilliSeconds() == 0)
				player.rankStart = player.timeStart;

			if (player.maxRank < event.value)
				player.maxRank = (i32)event.value;

			if (player.rank > event.value)
				++player.demotions;
			if (player.rank < event.value)
				++player.promotions;

			DRX_ASSERT(player.rank < 10);
			player.rankTime[player.rank] += now - player.rankStart;
			player.rank = (i32)event.value;
			player.rankStart = now;
		}
		break;
	default:
		break;
	}
}

//------------------------------------------------------------------------
void CGameplayAnalyst::OnGameplayEvent(IEntity* pEntity, const GameplayEvent& event)
{
	if (!gEnv->bServer)
		return;

	EntityId id = pEntity ? pEntity->GetId() : 0;

	if (id && IsPlayer(id))
		ProcessPlayerEvent(id, event);

	switch (event.event)
	{
	case eGE_GameStarted:
		if (event.value)//only for server event
			Reset();
		break;
	case eGE_GameEnd:
		if (event.value)//only for server event
			DumpToTXT();
		break;
	default:
		break;
	}
}

//------------------------------------------------------------------------
void CGameplayAnalyst::NewPlayer(IEntity* pEntity)
{
	if (!pEntity)
		return;

	std::pair<Players::iterator, bool> result = m_gameanalysis.players.insert(Players::value_type(pEntity->GetId(), PlayerAnalysis()));
	result.first->second.name = pEntity->GetName();
	result.first->second.timeStart = gEnv->pTimer->GetFrameStartTime();
}

//------------------------------------------------------------------------
bool CGameplayAnalyst::IsPlayer(EntityId entityId) const
{
	if (IActor* pActor = CDrxAction::GetDrxAction()->GetIActorSystem()->GetActor(entityId))
		return pActor->IsPlayer();
	return false;
}

//------------------------------------------------------------------------
CGameplayAnalyst::PlayerAnalysis& CGameplayAnalyst::GetPlayer(EntityId playerId)
{
	Players::iterator it = m_gameanalysis.players.find(playerId);
	if (it != m_gameanalysis.players.end())
		return it->second;

	static PlayerAnalysis def;
	return def;
}

//------------------------------------------------------------------------
CGameplayAnalyst::WeaponAnalysis& CGameplayAnalyst::GetWeapon(EntityId playerId, EntityId weaponId)
{
	if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(weaponId))
		return GetWeapon(playerId, pEntity->GetClass()->GetName());

	static WeaponAnalysis def;
	return def;
}

//------------------------------------------------------------------------
CGameplayAnalyst::WeaponAnalysis& CGameplayAnalyst::GetWeapon(EntityId playerId, tukk weapon)
{
	PlayerAnalysis& player = GetPlayer(playerId);

	Weapons::iterator it = player.weapons.find(CONST_TEMP_STRING(weapon));
	if (it == player.weapons.end())
	{
		std::pair<Weapons::iterator, bool> result = player.weapons.insert(Weapons::value_type(weapon, WeaponAnalysis()));
		return result.first->second;
	}
	else
		return it->second;
}

//------------------------------------------------------------------------
CGameplayAnalyst::WeaponAnalysis& CGameplayAnalyst::GetCurrentWeapon(EntityId playerId)
{
	if (IActor* pActor = CDrxAction::GetDrxAction()->GetIActorSystem()->GetActor(playerId))
	{
		if (IInventory* pInventory = pActor->GetInventory())
			return GetWeapon(playerId, pInventory->GetCurrentItem());
	}

	static WeaponAnalysis def;
	return def;
}

//------------------------------------------------------------------------
void CGameplayAnalyst::Reset()
{
	m_gameanalysis = GameAnalysis();

	IActorSystem* pActorSystem = CDrxAction::GetDrxAction()->GetIActorSystem();
	IActorIteratorPtr pActorIt = pActorSystem->CreateActorIterator();
	while (IActor* pActor = pActorIt->Next())
	{
		if (pActor && pActor->GetChannelId())
			NewPlayer(pActor->GetEntity());
	}

	DrxLog("Player statistics reset...");
}

//------------------------------------------------------------------------
void CGameplayAnalyst::DumpToTXT()
{
	tukk level = CDrxAction::GetDrxAction()->GetILevelSystem()->GetCurrentLevel()->GetName();
	tukk gamerules = CDrxAction::GetDrxAction()->GetIGameRulesSystem()->GetCurrentGameRulesEntity()->GetClass()->GetName();

	string lines;
	lines.append(level);
	lines.append("\n");
	lines.append(gamerules);
	lines.append("\n\n");

	for (Players::iterator it = m_gameanalysis.players.begin(); it != m_gameanalysis.players.end(); ++it)
		DumpWeapon(it->first, lines);

	lines.append("\n\n");

	DumpRank(lines);
	lines.append("\n\n");

	DumpSuit(lines);
	lines.append("\n\n");

	DumpCurrency(lines);
	lines.append("\n\n");

	IDrxPak* pDrxPak = gEnv->pDrxPak;
	FILE* pFile = 0;

	tukk fileFmt = "gameplaystats%03d.txt";
	string fname;

	i32 i = 0;
	while (++i < 1000)
	{
		fname.Format(fileFmt, i);
		if (pFile = pDrxPak->FOpen(fname.c_str(), "r"))
		{
			pDrxPak->FClose(pFile);
			pFile = 0;

			continue;
		}

		fname.Format(fileFmt, i);
		if (pFile = pDrxPak->FOpen(fname, "wb"))
			break;

		return;
	}

	if (!pFile)
		return;

	pDrxPak->FWrite(lines.c_str(), lines.size(), 1, pFile);
	pDrxPak->FClose(pFile);

	DrxLog("Player statistics saved to %s...", fname.c_str());
}

//------------------------------------------------------------------------
void CGameplayAnalyst::DumpWeapon(EntityId playerId, string& lines)
{
	PlayerAnalysis& player = GetPlayer(playerId);

	tukk header = "\tKills\tDeaths\tKillsPerDeath\tShots\tHits\tShootsPerKill\tAccuracy\tDamage\tUsage\tReloads\tMeleeAttacks\tFireModeChanges\tZoomIn\n";
	tukk lineFmt = "%s\t%d\t%d\t%.2f\t%d\t%d\t%.2f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\n";

	string lineFormatter;

	lines.append(header);
	lines.append(lineFormatter.Format(lineFmt, player.name.c_str(), player.kills, player.deaths,
	                                  player.deaths ? player.kills / (float)player.deaths : player.kills,
	                                  player.shots, player.hits,
	                                  player.kills ? (player.shots / (float)player.kills) : player.shots,
	                                  player.shots ? (100 * player.hits / (float)player.shots) : 0.0f,
	                                  player.damage, player.itemUsage, player.reloads, player.melee, player.firemodes, player.zoomUsage));

	for (Weapons::iterator wit = player.weapons.begin(); wit != player.weapons.end(); ++wit)
	{
		WeaponAnalysis& weapon = wit->second;
		lines.append(lineFormatter.Format(lineFmt, wit->first.c_str(), weapon.kills, weapon.deaths,
		                                  weapon.deaths ? weapon.kills / (float)weapon.deaths : weapon.kills,
		                                  weapon.shots, weapon.hits,
		                                  weapon.kills ? weapon.shots / (float)weapon.kills : weapon.shots,
		                                  weapon.shots ? 100 * weapon.hits / (float)weapon.shots : 0.0f,
		                                  weapon.damage, weapon.usage, weapon.reloads, weapon.melee, weapon.firemodes, weapon.zoomUsage));
	}
	lines.append("\n");
}

//------------------------------------------------------------------------
void CGameplayAnalyst::DumpRank(string& lines)
{
	string header("\tHighestRank\tPromotions\tDemotions");
	string lineFmt("%s\t%d\t%d\t%d");

	string headerFormatter;
	for (i32 i = 1; i < 10; i++)
	{
		header.append(headerFormatter.Format("\tRank %d Time", i));
		lineFmt.append("\t%.1f");
	}
	header.append("\n");
	lineFmt.append("\n");

	lines.append(header);

	string lineFormatter;

	for (Players::iterator it = m_gameanalysis.players.begin(); it != m_gameanalysis.players.end(); ++it)
	{
		PlayerAnalysis& player = it->second;

		CTimeValue now = gEnv->pTimer->GetFrameStartTime();

		if (player.rankStart.GetMilliSeconds() == 0)
			player.rankStart = now;

		player.rankTime[player.rank] += now - player.rankStart;
		player.rankStart = now;

		lines.append(lineFormatter.Format(lineFmt, player.name.c_str(), player.maxRank, player.promotions, player.demotions,
		                                  player.rankTime[1].GetSeconds(), player.rankTime[2].GetSeconds(), player.rankTime[3].GetSeconds(),
		                                  player.rankTime[4].GetSeconds(), player.rankTime[5].GetSeconds(), player.rankTime[6].GetSeconds(),
		                                  player.rankTime[7].GetSeconds(), player.rankTime[8].GetSeconds(), player.rankTime[9].GetSeconds()
		                                  ));
	}
}

//------------------------------------------------------------------------
void CGameplayAnalyst::DumpSuit(string& lines)
{
	string header("\tModeChanges\tSpeedKills\tSpeedDeaths\tSpeedUsage\tSpeedTime\tStrengthKills\tStrengthDeaths\tStrengthUsage\tStrengthTime\tCloakKills\tCloakDeaths\tCloakUsage\tCloakTime\tArmorKills\tArmorDeaths\tArmorUsage\tArmorTime\n");
	string lineFmt("%s\t%d\t%d\t%d\t%d\t%.1f\t%d\t%d\t%d\t%.1f\t%d\t%d\t%d\t%.1f\t%d\t%d\t%d\t%.1f\n");

	lines.append(header);

	string lineFormatter;

	for (Players::iterator it = m_gameanalysis.players.begin(); it != m_gameanalysis.players.end(); ++it)
	{
		SuitAnalysis& suit = it->second.suit;

		CTimeValue now = gEnv->pTimer->GetFrameStartTime();

		if (suit.usageStart.GetMilliSeconds() == 0)
			suit.usageStart = now;

		suit.timeUsed[suit.mode] += now - suit.usageStart;
		suit.usageStart = now;

		i32 totalUsage = suit.usage[0] + suit.usage[1] + suit.usage[2] + suit.usage[3];
		CTimeValue totalTime = suit.timeUsed[0] + suit.timeUsed[1] + suit.timeUsed[2] + suit.timeUsed[3];

		lines.append(lineFormatter.Format(lineFmt, it->second.name.c_str(), totalUsage,
		                                  suit.kills[0], suit.deaths[0], suit.usage[0], suit.timeUsed[0].GetSeconds(),
		                                  suit.kills[1], suit.deaths[1], suit.usage[1], suit.timeUsed[1].GetSeconds(),
		                                  suit.kills[2], suit.deaths[2], suit.usage[2], suit.timeUsed[2].GetSeconds(),
		                                  suit.kills[3], suit.deaths[3], suit.usage[3], suit.timeUsed[3].GetSeconds()
		                                  ));
	}
}

//------------------------------------------------------------------------
void CGameplayAnalyst::DumpCurrency(string& lines)
{
	/*
	   tukk header="\tHighestRank\tPromotions\tDemotions\n"
	   tukk lineFmt="";

	   string lineFormatter;

	   for (Players::iterator it=m_gameanalysis.players.begin(); it!=m_gameanalysis.players.end(); it++)
	   {
	   PlayerAnalysis &player=it->second;

	   lines.append(header);
	   lines.append(lineFormatter.Format(lineFmt,
	     ));
	   }
	 */
}
