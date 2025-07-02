// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SCRIPTBIND_GAMEAUDIO_H__
#define __SCRIPTBIND_GAMEAUDIO_H__

#include <drx3D/Script/ScriptHelpers.h>
#include <drx3D/Game/AudioSignalPlayer.h>

struct ISystem;
struct IFunctionHandler;

class CScriptbind_GameAudio : public CScriptableBase
{
public:
	CScriptbind_GameAudio();
	virtual ~CScriptbind_GameAudio();
	void Reset();

	virtual void GetMemoryUsage(IDrxSizer *pSizer) const
	{
		pSizer->AddObject(this, sizeof(*this));
		pSizer->AddContainer(m_signalPlayers);		
		pSizer->AddContainer(m_signalPlayersSearchMap);		
	}
private:
	i32 GetSignal( IFunctionHandler *pH, tukk pSignalName );
	
	i32 JustPlaySignal( IFunctionHandler *pH, TAudioSignalID audioSignalID );  // a signal played with JustPlaySignal cant be stopped.
	i32 JustPlayEntitySignal( IFunctionHandler *pH, TAudioSignalID audioSignalID, ScriptHandle entityId );  // a signal played with JustPlaySignal cant be stopped.
	i32 JustPlayPosSignal(IFunctionHandler *pH, TAudioSignalID signalId, Vec3 pos );
	i32 PlayEntitySignal( IFunctionHandler *pH, TAudioSignalID audioSignalID, ScriptHandle entityId );
	i32 IsPlayingEntitySignal( IFunctionHandler *pH, TAudioSignalID audioSignalID, ScriptHandle entityId );
	i32 StopEntitySignal(  IFunctionHandler *pH, TAudioSignalID audioSignalID, ScriptHandle entityId );
	i32 SetEntitySignalParam(  IFunctionHandler *pH, TAudioSignalID audioSignalID, ScriptHandle entityId, tukk param, float fValue );
	i32 PlaySignal( IFunctionHandler *pH, TAudioSignalID audioSignalID );
	i32 StopSignal(  IFunctionHandler *pH, TAudioSignalID audioSignalID );
	i32 Announce( IFunctionHandler *pH, tukk announcement, i32 context );

	void RegisterMethods();
	
	void PlaySignal_Internal( TAudioSignalID audioSignalID, EntityId entityId );
	bool IsPlayingSignal_Internal( TAudioSignalID signalId, EntityId entityId);
	void StopSignal_Internal( TAudioSignalID audioSignalID, EntityId entityId );
	void SetSignalParam_Internal( TAudioSignalID audioSignalID, EntityId entityId, tukk param, float fValue );
	
	struct SKey
	{
		SKey()
		: m_signalID( INVALID_AUDIOSIGNAL_ID )
		, m_entityId(0)
		{}

		SKey( TAudioSignalID signalID, EntityId entityId )
			: m_signalID( signalID )
			, m_entityId( entityId )
		{}
		
		bool operator < ( const SKey& otherKey ) const
		{
			if (m_signalID<otherKey.m_signalID)
				return true;
			else
				if (m_signalID>otherKey.m_signalID)
					return false;
				else
					return m_entityId<otherKey.m_entityId;
		}

		void GetMemoryUsage(IDrxSizer *pSizer) const {/*nothing*/}

		TAudioSignalID m_signalID;
		EntityId m_entityId;
	};

	std::vector<CAudioSignalPlayer> m_signalPlayers;
	typedef std::map<SKey, i32> TSignalPlayersSearchMap;
	TSignalPlayersSearchMap m_signalPlayersSearchMap;
};

#endif //__SCRIPTBIND_GAME_H__