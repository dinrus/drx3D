// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
Описание:
	- Contains various shared network util functions
-------------------------------------------------------------------------
История:
	- 19/07/2010 : Created by Colin Gulliver

*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/GameNetworkUtils.h>
#include <drx3D/Act/IPlayerProfiles.h>
#include <drx3D/Game/Lobby/SessionNames.h>

namespace GameNetworkUtils
{
	EDrxLobbyError SendToAll( CDrxLobbyPacket* pPacket, DrxSessionHandle h, SSessionNames &clients, bool bCheckConnectionState )
	{
		EDrxLobbyError result = eCLE_Success;

		IDrxLobby *pLobby = gEnv->pNetwork->GetLobby();
		if (pLobby)
		{
			IDrxMatchMaking *pMatchMaking = pLobby->GetMatchMaking();
			if (pMatchMaking)
			{
				u32k numClients = clients.m_sessionNames.size();
				// Start from 1 since we don't want to send to ourselves (unless we're a dedicated server)
				i32k startIndex = gEnv->IsDedicated() ? 0 : 1;
				for (u32 i = startIndex; (i < numClients) && (result == eCLE_Success); ++ i)
				{
					SSessionNames::SSessionName &client = clients.m_sessionNames[i];

					if (!bCheckConnectionState || client.m_bFullyConnected)
					{
						result = pMatchMaking->SendToClient(pPacket, h, client.m_conId);
					}
				}
			}
		}

		return result;
	}

	//---------------------------------------
	const bool CompareDrxSessionId(const DrxSessionID &lhs, const DrxSessionID &rhs)
	{
		if (!lhs && !rhs)
		{
			return true;
		}
		if ((lhs && !rhs) || (!lhs && rhs))
		{
			return false;
		}
		return (*lhs == *rhs);
	}

	void WebSafeEscapeString(string				*ioString)
	{
		/*	from RFC2396
			escape the following by replacing them with %<hex of their ascii code>
		 
				delims      = "<" | ">" | "#" | "%" | <">
				unwise      = "{" | "}" | "|" | "\" | "^" | "[" | "]" | "`"
				reserved    = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
									"$" | ","
			also replace spaces with %20
				space->%20
		*/
		const char				k_escapeCharacters[]="%<>#\"{}|\\^[]`;/?:@&=+$, ";		// IMPORTANT: replace % first!
		i32k				k_numEscapedCharacters=sizeof(k_escapeCharacters)-1;
		i32						numToReplace=0;
		i32						origLen=ioString->length();

		{
			const char		*rawStr=ioString->c_str();

			for (i32 i=0; i<k_numEscapedCharacters; i++)
			{
				char	escChar=k_escapeCharacters[i];

				for (i32 j=0; j<origLen; j++)
				{
					if (rawStr[j]==escChar)
					{
						numToReplace++;
					}
				}
			}
		}

		if (numToReplace == 0)
		{
			return;
		}

		ioString->reserve(origLen+numToReplace*2);		// replace with 3 chars, as escaped chars are replaced with a % then two hex numbers. replace len = 3, old len = 1, need numReplace*2 extra bytes

		{
			DrxFixedStringT<8>		find,replace;

			for (i32 i=0; i<k_numEscapedCharacters; i++)
			{
				char	escChar=k_escapeCharacters[i];

				find.Format("%c",escChar);
				replace.Format("%%%x",i32(escChar));
				ioString->replace(find,replace);
			}
		}
}

} //~GameNetworkUtils