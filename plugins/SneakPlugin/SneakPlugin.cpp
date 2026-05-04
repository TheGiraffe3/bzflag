#include "bzfsAPI.h"

#include <chrono>
#include <random>

class SneakPlugin : public bz_Plugin, public bz_CustomSlashCommandHandler
{
	virtual const char* Name ();
	virtual void Init (const char*);
	virtual void Cleanup ();
	virtual void Event (bz_EventData* eventData);
	virtual bool SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);

	int sneakAttack[256];

	std::mt19937 gen;
	std::uniform_int_distribution<int> random_rotation;
	std::uniform_int_distribution<int> random_x;
	std::uniform_int_distribution<int> random_y;
};

BZ_PLUGIN(SneakPlugin)

const char* SneakPlugin::Name ()
{
	return "Sneak Plugin";
}

void SneakPlugin::Init (const char*)
{
	Register(bz_eGetPlayerSpawnPosEvent);
	Register(bz_ePlayerPartEvent);

	bz_registerCustomSlashCommand("sneak", this);

	for (int &i : sneakAttack)
	{
		i = -1;
	}

	std::random_device rd;
	auto seed = rd() ^ static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
	gen.seed(seed);

	random_rotation = std::uniform_int_distribution<int>(-120, 120);
	random_x = std::uniform_int_distribution<int>(-15, 15);
	random_y = std::uniform_int_distribution<int>(-15, 15);
}

void SneakPlugin::Cleanup ()
{
	Flush();

	bz_removeCustomSlashCommand("sneak");
}

void SneakPlugin::Event (bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_eGetPlayerSpawnPosEvent:
		{
			bz_GetPlayerSpawnPosEventData_V1 *data = (bz_GetPlayerSpawnPosEventData_V1*)eventData;

			if (sneakAttack[data->playerID] >= 0 && data->handled == false)
			{
				bz_BasePlayerRecord *pr = bz_getPlayerByIndex(sneakAttack[data->playerID]);

				if (!pr)
				{
					bz_sendTextMessage(BZ_SERVER, data->playerID, "Your sneak attack failed! Victim not found.");
					return;
				}

				bool valid = false;
				float newPos[3];

				do {
					int x_diff = random_x(gen);
					int y_diff = random_y(gen);

					newPos[0] = pr->lastKnownState.pos[0] + x_diff;
					newPos[1] = pr->lastKnownState.pos[1] + y_diff;
					newPos[2] = pr->lastKnownState.pos[2] + 2;

					if (bz_isValidSpawnPoint(newPos))
						valid = true;
				} while (valid == false);

				if (valid == true)
				{
					int rot_diff = random_rotation(gen);
					data->handled = true;
					data->pos[0] = newPos[0];
					data->pos[1] = newPos[1];
					data->pos[2] = newPos[2];
					data->rot = pr->lastKnownState.rotation + rot_diff;
				}

				bz_freePlayerRecord(pr);

				sneakAttack[data->playerID] = -1;
			}
		}
		break;

		case bz_ePlayerPartEvent:
		{
			bz_PlayerJoinPartEventData_V1 *data = (bz_PlayerJoinPartEventData_V1*)eventData;

			sneakAttack[data->playerID] = -1;
		}
		break;

		default:
			break;
	}
}

bool SneakPlugin::SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
	if (command == "sneak")
	{
		bz_BasePlayerRecord *runPR = bz_getPlayerByIndex(playerID);

		if (runPR->spawned == false)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "You can only /sneak if you are alive!");
			bz_freePlayerRecord(runPR);
			return true;
		}

		if (params->size() != 1)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "/sneak <player ID|callsign>");
			return true;
		}

		const char* victimID = params->get(0).c_str();

		bz_BasePlayerRecord *pr = bz_getPlayerBySlotOrCallsign(victimID);

		if (!pr)
		{
			bz_sendTextMessagef(BZ_SERVER, playerID, "player %s not found", victimID);
			return true;
		}

		if (pr->playerID == runPR->playerID)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "You cannot sneak yourself!");
			return true;
		}

		else
		{
			sneakAttack[playerID] = pr->playerID;
			bz_sendTextMessagef(BZ_SERVER, playerID, "Sneaking %s. Next time you die, you will spawn near their tank.", pr->callsign.c_str());
			bz_sendTextMessagef(BZ_SERVER, pr->playerID, "Warning! %s is sneaking you!", runPR->callsign.c_str());
			bz_freePlayerRecord(pr);
			bz_freePlayerRecord(runPR);
		}

		return true;
	}

	return false;
}
