// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   GameCodeCoverageUpr.h
//  Created:     18/06/2008 by Matthew
//  Описание: High-level manager class for code coverage system
//               including file handing of checkpoint lisst
//               Tracks whether key points of code have been hit in testing
// -------------------------------------------------------------------------
//  История:     Tim Furnish, 11/11/2009:
//               Moved into game DLL from AI system
//               Wrapped contents in ENABLE_GAME_CODE_COVERAGE
//
//               Tim Furnish, 19/11/2009:
//               Reads context from XML file instead of raw text
//               Support for checkpoints which are only valid for certain game modes
//               Support for checkpoints which are only valid in certain states (server, hasLocalPlayer, multiplayer etc.)
//
////////////////////////////////////////////////////////////////////////////

/**
* Design notes:
*   For now, reading contexts is done in one shot. I'm tempted to work through the file slowly to avoid a spike.
*   A (separate) tracking GUI is crucial to this system - an efficient interface to service this is not trivial
*   Quite a basic principle to underlie this code is that we're only interested in checkpoints we haven't yet hit
*/

#ifndef __GAME_CODE_COVERAGE_MANAGER_H_
#define __GAME_CODE_COVERAGE_MANAGER_H_

#pragma once

#include <drx3D/Game/GameCodeCoverage/GameCodeCoverageTracker.h>
#include <drx3D/Game/Utility/SingleAllocTextBlock.h>
#include <drx3D/Game/AutoEnum.h>
#include <drx3D/Game/GameMechanismBase.h>

#if ENABLE_GAME_CODE_COVERAGE

#define GameCodeCoverageFlagList(f)  \
	f(kGCCFlag_server)                 \
	f(kGCCFlag_nonServer)              \
	f(kGCCFlag_hasLocalPlayer)         \
	f(kGCCFlag_multiplayer)            \
	f(kGCCFlag_singleplayer)           \
	f(kGCCFlag_noLevelLoaded)          \

AUTOENUM_BUILDFLAGS_WITHZERO(GameCodeCoverageFlagList, kGCCFlag_none);

typedef u16  TGameCodeCoverageCustomListBitfield;
typedef u8   TGameCodeCoverageGroupNum;

#define GAMECODECOVERAGE_STOREHITTHISFRAME  5

struct SCustomCheckpointList
{
	tukk  m_name;
	u32 m_numInList;
};

struct SLabelInfoFromFile
{
	tukk  m_gameModeNames;
	tukk  m_labelName;
	SLabelInfoFromFile * m_nextInAutoNamedGroup;

	TBitfield m_flags;
	TGameCodeCoverageCustomListBitfield m_customListBitfield;
	TGameCodeCoverageGroupNum m_groupNum;

	bool m_validForCurrentGameState, m_hasBeenHit, m_ignoreMe;
};

struct SMatchThisPattern;
class IGameCodeCoverageOutput;
class IGameCodeCoverageListener;

class CNamedCheckpointGroup
{
	tukk  m_name;
	SLabelInfoFromFile * m_firstInGroup;
	i32 m_numInGroup, m_numValid, m_numHit;

	public:
	CNamedCheckpointGroup()
	{
		m_name = NULL;
		m_firstInGroup = NULL;
		m_numInGroup = 0;
		m_numValid = 0;
		m_numHit = 0;
	}

	ILINE void SetName(tukk  namePtr)              { m_name = namePtr;                                 }
	ILINE tukk  GetName() const                    { return m_name;                                    }
	ILINE i32 GetNumInGroup() const                       { return m_numInGroup;                              }
	ILINE i32 GetNumValidForCurrentGameState() const      { return m_numValid;                                }
	ILINE i32 GetNumHit() const                           { return m_numHit;                                  }
	ILINE void IncreaseNumHit()                           { m_numHit ++; assert (m_numHit <= m_numValid);     }
	const SLabelInfoFromFile * GetFirstUnhitValidCheckpoint() const;
	void AddCheckpoint(SLabelInfoFromFile * addThis);
	i32 FixNumValidForCurrentGameState();
};

/**
* The code coverage manager
*/
class CGameCodeCoverageUpr : public CGameMechanismBase
{
	// String comparison for set and map
	struct cmp_str : public std::binary_function<tukk , tukk , bool>
	{
		bool operator()(char const * a, char const * b) const
		{
			return strcmp(a, b) < 0;
		}
	};

	typedef std::vector<CGameCodeCoverageCheckPoint *> CheckPointVector;
	typedef std::map < tukk , CGameCodeCoverageCheckPoint*, cmp_str > CheckPointMap;
	typedef std::set < tukk , cmp_str > CheckPointSet;

public:
	CGameCodeCoverageUpr(tukk  filename);
	~CGameCodeCoverageUpr();

	struct SRecentlyHitList
	{
		struct SStrAndTime
		{
			const char	*pStr;
			float				fTime;
			bool				bExpected;
		};

		SStrAndTime m_array[GAMECODECOVERAGE_STOREHITTHISFRAME];
		i32					m_count;
	};

	ILINE static CGameCodeCoverageUpr * GetInstance()         { return s_instance;                                                  }
	ILINE bool IsContextValid() const                             { return m_bContextValid;                                             }
	ILINE i32 GetTotalCheckpointsReadFromFileAndValid() const     { return m_nTotalCheckpointsReadFromFileAndValid;                     }
	ILINE const CheckPointSet &GetUnexpectedCheckpoints() const	  { return m_setUnexpectedCheckPoints;                                  }
	ILINE const SRecentlyHitList & GetRecentlyHitList() const     {	return m_recentlyHit;                                               }
	ILINE i32 GetTotalValidCheckpointsHit() const                 { return m_totalValidCheckpointsHit;                                  }
	ILINE i32 GetNumAutoNamedGroups() const                       { return m_numAutoNamedCheckpointGroups;                              }
	ILINE i32 GetTotalCustomListsReadFromFile() const             { return m_nTotalCustomListsReadFromFile;                             }
	ILINE tukk  GetCustomListName(i32 n) const             { return m_pCustomCheckpointLists_Array[n].m_name;                    }

	const CNamedCheckpointGroup * GetAutoNamedCheckpointGroup(i32 i) const;
	void GetRemainingCheckpointLabels( std::vector < tukk  > &vLabels ) const;
	void Register( CGameCodeCoverageCheckPoint * pPoint );
	void Hit( CGameCodeCoverageCheckPoint * pPoint );
	void Reset();
	void UploadHitCheckpointsToServer();
	void EnableListener(IGameCodeCoverageListener * listener);
	void DisableListener(IGameCodeCoverageListener * listener);

protected:

	virtual void Update(float frameTime);
	virtual void Inform(EGameMechanismEvent gmEvent, const SGameMechanismEventData * data);

	void CheckWhichCheckpointsAreValidForGameState();
	SLabelInfoFromFile * FindWatchedCheckpointStructByName(tukk szCheckPoint) const;
	bool ReadCodeCoverageContext( IItemParamsNode *paramNode );
	void ParseCustomList(tukk  name, tukk  list, i32 numListsParsedSoFar);
	void AddCheckpointToCustomList(SCustomCheckpointList * checkpointList, i32 checkpointNum, TGameCodeCoverageCustomListBitfield bit);
	void RemoveFirstEntryFromRecentlyHitList();
	void AddToRecentlyHitList(tukk  which, bool expected);
	void IgnoreAllCheckpointsWhichMatchPattern(bool onOff, tukk  argument);
	bool DoOutput(IGameCodeCoverageOutput & outputClass) const;
	bool GetShouldIgnoreCheckpoint(const SLabelInfoFromFile * cp) const;
	void CacheCurrentTime();
	void ClearMem();

	static bool GetMatchThisPatternData(tukk  patternTxt, SMatchThisPattern * pattOut);
	static bool MatchesPattern(tukk  checkpointName, const SMatchThisPattern * patt);
	static void CmdDumpCodeCoverage(IConsoleCmdArgs *pArgs);
	static void CmdUploadHitCheckpoints(IConsoleCmdArgs *pArgs);
	static void CmdGCCIgnore(IConsoleCmdArgs *pArgs);
	static void CmdGCCUseList(IConsoleCmdArgs *pArgs);

	static CGameCodeCoverageUpr * s_instance;

	CheckPointSet             m_setUnexpectedCheckPoints;
	CheckPointMap             m_mCheckPoints;
	CheckPointVector          m_vecCheckPoints;
	SRecentlyHitList          m_recentlyHit;
	CSingleAllocTextBlock     m_singleAllocTextBlock;

	// Pointers to memory controlled elsewhere - DON'T free these up when destroying the class
	tukk                   m_filepath;
	IGameCodeCoverageListener *   m_onlyListener;

	// Pointers to memory allocated within this class - DO free these up when destroying the class
	SLabelInfoFromFile *      m_pExpectedCheckpoints_Array;
	CNamedCheckpointGroup *   m_pAutoNamedCheckpointGroups_Array;
	SCustomCheckpointList *   m_pCustomCheckpointLists_Array;

	string                    m_outputDirectory;
	string                    m_timeWhenStarted;

	i32 											m_bCodeCoverageEnabled;
	i32 											m_bCodeCoverageDisplay;
	i32 											m_bCodeCoverageLogAll;
	i32                       m_nTotalCustomListsReadFromFile;
	i32                       m_nTotalCheckpointsReadFromFile;
	i32                       m_nTotalCheckpointsReadFromFileAndValid;
	i32                       m_numAutoNamedCheckpointGroups;
	i32                       m_totalValidCheckpointsHit;

	TGameCodeCoverageCustomListBitfield   m_listsBeingWatched_Bitfield;

	// TODO: Combine these into a 'state' enum... there's no way both will be true at the same time
	bool											m_bContextValid;
	bool											m_bCodeCoverageFailed;
};

#endif // ENABLE_GAME_CODE_COVERAGE

#endif // __GAME_CODE_COVERAGE_MANAGER_H_