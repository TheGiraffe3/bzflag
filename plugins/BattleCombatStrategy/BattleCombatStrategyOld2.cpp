/*
 * BattleCombatStrategy
 *   Copyright (C) 2025 Loymdayddaud
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bzfsAPI.h"
#include <cstring>

int getSeerOnTeam(bz_eTeamType team, int triggerPlayer)
{
	int playerCount = bz_getPlayerCount();
	bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Reached getSeerOnTeam function.");

	for (int i = 0; i < playerCount; ++i)
	{
		bz_BasePlayerRecord* player = bz_getPlayerByIndex(i);

		if (!player)
			continue;

		if (player->team != team)
			continue;

		if (i == triggerPlayer)
			continue;

		bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Alright, we've made sure that the Seer player is on the right team.");
		if (player->currentFlag == "SEer (+SE)")
		{
			bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Returning a player's ID who has Seer.");
			return i;
		}
		bz_freePlayerRecord(player);
	}

	return -1;
}

class BattleCombatStrategy : public bz_Plugin
{
public:
	virtual const char* Name();
	virtual void Init(const char*);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);

	int playerMessageTime[256];
};

BZ_PLUGIN(BattleCombatStrategy)

const char* BattleCombatStrategy::Name()
{
	return "Battle Combat Strategy";
}

void BattleCombatStrategy::Init(const char*)
{
	Register(bz_eFilteredChatMessageEvent);
	Register(bz_eFlagGrabbedEvent);
	Register(bz_ePlayerDieEvent);
	Register(bz_ePlayerPartEvent);

	for (int &i : playerMessageTime)
	{
		i = -1;
	}
}

void BattleCombatStrategy::Cleanup()
{
	Flush();
}

void BattleCombatStrategy::Event(bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_eFilteredChatMessageEvent:
		{
			bz_ChatEventData_V2* cdata = (bz_ChatEventData_V2*)eventData;

//			bz_eTeamType teamTo = bz_getPlayerTeam(cdata->from);
//			if(cdata->team == teamTo)
//			{
			playerMessageTime[cdata->from] = cdata->eventTime;
			bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Resolved effects of chat message.");
//			}
		}
		break;

		case bz_eFlagGrabbedEvent:
		{
			bz_FlagGrabbedEventData_V1* fdata = (bz_FlagGrabbedEventData_V1*) eventData;

			bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Someone grabbed a flag!");
			if (strcmp(fdata->flagType, "SE") == 0)
			{
				bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "The flag was Seer!");
				bz_eTeamType playerTeam = bz_getPlayerTeam(fdata->playerID);
				int seerPlayerID = getSeerOnTeam(playerTeam, fdata->playerID);
				if(seerPlayerID != -1)
				{
					bz_removePlayerFlag(fdata->playerID);
					bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Alright, we've removed a flag.");
					bz_sendTextMessagef(BZ_SERVER, fdata->playerID, "Someone on your team already has a Seer flag!");
				}
			}
		}
		break;

		case bz_ePlayerDieEvent:
		{
			bz_PlayerDieEventData_V2* data = (bz_PlayerDieEventData_V2*)eventData;
			bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Oh no, someone just died!");

			if (data->killerID == BZ_SERVER || data->killerID == -1)
				break;

			bz_BasePlayerRecord* killed = bz_getPlayerByIndex(data->playerID);
			bz_BasePlayerRecord* killer = bz_getPlayerByIndex(data->killerID);

			bz_ApiString deathFlag = bz_getFlagName(data->flagHeldWhenKilled);
			if (deathFlag == "ST")
			{
				bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "And they had Stealth, too!");
				int seerID = getSeerOnTeam(killer->team, data->killerID);

				if (seerID != -1)
				{
					bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "And someone had a Seer flag, on the murderer's team!");
					double eventTime = data->eventTime;
					if (playerMessageTime[seerID] != -1 && (eventTime - playerMessageTime[seerID] <= 30.0))
					{
						bz_incrementPlayerWins(seerID, 1);
						bz_sendTextMessagef(BZ_SERVER, seerID, "With your help, teammate %s killed %s! You gain a point!", killer->callsign, killed->callsign);
					}
				}
			}

			bz_freePlayerRecord(killed);
			bz_freePlayerRecord(killer);
		}
		break;

		case bz_ePlayerPartEvent:
		{
			bz_PlayerJoinPartEventData_V1* pdata = (bz_PlayerJoinPartEventData_V1*)eventData;
			bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "Uh-oh, someone left.");

			playerMessageTime[pdata->playerID] = -1;
		}
		break;

		default:
			break;
	}
}
