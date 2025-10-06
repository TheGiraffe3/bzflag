#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <cstring>
#include <math.h>

double lastTeamMessageTime[40] = { 0.0 };

class BattleCombatStrategyPlugin : public bz_Plugin
{
	virtual const char* Name()
	{
		return "Battle Combat Strategy Plugin";
	}
	virtual void Init(const char*);
	virtual void Event(bz_EventData*);
	~BattleCombatStrategyPlugin();

	virtual void Cleanup(void)
	{
		Flush();
	}
};

BZ_PLUGIN(BattleCombatStrategyPlugin)

void BattleCombatStrategyPlugin::Init(const char*)
{
	bz_registerCustomBZDBDouble("_seerMessageTime", 50.0);
	Register(bz_ePlayerDieEvent);
	Register(bz_eFlagGrabbedEvent);
	Register(bz_eRawChatMessageEvent);
}

BattleCombatStrategyPlugin::~BattleCombatStrategyPlugin() {}

int getSeerPlayerOnTeam(bz_eTeamType team)
{
	float now = bz_getCurrentTime();
	int count = bz_getPlayerCount();
	for (int i = 0; i < count; ++i)
	{
		bz_BasePlayerRecord* player = bz_getPlayerByIndex(i);

		int id = player->playerID;
		const char* flag = bz_getPlayerFlag(id);

		bool hasSeer = flag && strcmp(flag, "Seer") == 0;
		bool recentMsg = (now - lastTeamMessageTime[id]) <= bz_getBZDBDouble("_seerMessageTime");

		if (player->team == team && hasSeer && recentMsg)
		{
			bz_freePlayerRecord(player);
			return id;
		}

		bz_freePlayerRecord(player);
	}
	return -1;
}


void BattleCombatStrategyPlugin::Event(bz_EventData *eventData)
{
	switch (eventData->eventType)
	{
		case bz_ePlayerDieEvent:
		{
			bz_PlayerDieEventData_V2* data = (bz_PlayerDieEventData_V2*) eventData;

			// Don't give points in these scenarios
			if (data->killerID == data->playerID) // Suicide
				break;
			if (data->killerID == BZ_SERVER) // Server kill
				break;
			if (data->killerTeam == data->team) // Team kill
				break;

			int seerPlayer = -1;
			int flagHeldWhenKilled = data->flagHeldWhenKilled;
			bz_ApiString flagAbbreviation = bz_getFlagName(flagHeldWhenKilled);
			if (flagAbbreviation == "ST") {
				bz_BasePlayerRecord* killer = bz_getPlayerByIndex(data->killerID);
				bz_BasePlayerRecord* victim = bz_getPlayerByIndex(data->playerID);
				seerPlayer = getSeerPlayerOnTeam(data->killerTeam);
				if (seerPlayer != -1) {
					bz_incrementPlayerWins(seerPlayer, 1);
					bz_sendTextMessagef(BZ_SERVER, seerPlayer, "%s killed %s, thanks to your efforts. Take a point as a reward!", killer->callsign.c_str(), victim->callsign.c_str());
				}
				bz_freePlayerRecord(killer);
				bz_freePlayerRecord(victim);
			}
		} break;

		case bz_eFlagGrabbedEvent:
		{
			bz_FlagGrabbedEventData_V1* grabEvent = (bz_FlagGrabbedEventData_V1*) eventData;

			if (strcmp(grabEvent->flagType, "SE") == 0)
			{
				for (int i = 0; i < bz_getPlayerCount(); ++i)
				{
					bz_BasePlayerRecord* other = bz_getPlayerByIndex(i);
					if (!other) continue;

					if (other->playerID != grabEvent->playerID && other->team == bz_getPlayerTeam(grabEvent->playerID) && other->currentFlag == "SE")
					{
						bz_sendTextMessagef(BZ_SERVER, grabEvent->playerID,
							"%s on your team already has a Seer flag!", other->callsign.c_str());

						bz_removePlayerFlag(grabEvent->playerID);
						bz_freePlayerRecord(other);
						break;
					}

					bz_freePlayerRecord(other);
				}
			}

			break;
		}

		case bz_eRawChatMessageEvent:
		{
			bz_ChatEventData_V1 *chatData = (bz_ChatEventData_V1*)eventData;
			if (!chatData) break;

			// Only track team messages, not global
			if (chatData->team != eNoTeam)
			{
				int playerID = chatData->from;
				lastTeamMessageTime[playerID] = bz_getCurrentTime();
			}
		}
		break;

		default:
			break;
	}
}
