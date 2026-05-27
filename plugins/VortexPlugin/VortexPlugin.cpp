#include "bzfsAPI.h"

#include "../../src/bzfs/bzfs.h"

#include <chrono>
#include <random>

bool pointIn(float pos[3], float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
{
	if ( pos[0] > xmax || pos[0] < xmin )
		return false;
	if ( pos[1] > ymax || pos[1] < ymin )
		return false;
	if ( pos[2] > zmax || pos[2] < zmin )
		return false;
	return true;
}

bool playerInVortex(float pos[3])
{
	return pointIn(pos, -150, 150, -150, 150, 0, 10);
}

class VortexPlugin : public bz_Plugin, public bz_CustomSlashCommandHandler
{
	virtual const char* Name ();
	virtual void Init (const char*);
	virtual void Cleanup ();
	virtual void Event (bz_EventData* eventData);
	virtual bool SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);

	int sneakAttack[256];
	float spawnLocation[256][4] = {0};

	std::mt19937 gen;
	std::uniform_int_distribution<int> random_rotation;
	std::uniform_int_distribution<int> random_x;
	std::uniform_int_distribution<int> random_y;
};

BZ_PLUGIN(VortexPlugin)

const char* VortexPlugin::Name ()
{
	return "Vortex Plugin";
}

void VortexPlugin::Init (const char*)
{
	Register(bz_eGetPlayerSpawnPosEvent);
	Register(bz_ePlayerPartEvent);

	bz_registerCustomSlashCommand("sneak", this);
	bz_registerCustomSlashCommand("teleport", this);

	bz_RegisterCustomFlag("PH", "Placeholder", "Kill someone else to get rid of this flag.", 0, eBadFlag);
	bz_RegisterCustomFlag("TP", "Teleporter", "Use /teleport <x> <y> <z> to teleport to those coordinates. Disappears after use.", 0, eGoodFlag);

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

void VortexPlugin::Cleanup ()
{
	Flush();

	bz_removeCustomSlashCommand("sneak");
	bz_removeCustomSlashCommand("teleport");
}

void VortexPlugin::Event (bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_eGetPlayerSpawnPosEvent:
		{
			bz_GetPlayerSpawnPosEventData_V1 *data = (bz_GetPlayerSpawnPosEventData_V1*)eventData;

			if (spawnLocation[data->playerID][3] > 0)
			{
				data->handled = true;
				data->pos[0] = spawnLocation[data->playerID][0];
				data->pos[1] = spawnLocation[data->playerID][1];
				data->pos[2] = spawnLocation[data->playerID][2];
				spawnLocation[data->playerID][3] = 0; // don't mess with the spawn next time, unless they run /teleport again
			}

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

bool VortexPlugin::SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
	if (command == "sneak")
	{
		if (params->size() != 1)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "/sneak <player ID|callsign>");
			return true;
		}

		bz_BasePlayerRecord *runPR = bz_getPlayerByIndex(playerID);

		if (runPR->spawned == false)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "You can only /sneak if you are alive!");
			bz_freePlayerRecord(runPR);
			return true;
		}

		const char* victimID = params->get(0).c_str();

		bz_BasePlayerRecord *pr = bz_getPlayerBySlotOrCallsign(victimID);

		if (!pr)
		{
			bz_sendTextMessagef(BZ_SERVER, playerID, "player %s not found", victimID);
			bz_freePlayerRecord(runPR);
			return true;
		}

		if (pr->playerID == runPR->playerID)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "You cannot sneak yourself!");
			bz_freePlayerRecord(pr);
			bz_freePlayerRecord(runPR);
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
	if (command == "teleport")
	{
		if (params->size() != 3)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "/teleport <x> <y> <z>");
			return true;
		}

		float newPos[3];
		newPos[0] = std::stof(params->get(0));
		newPos[1] = std::stof(params->get(1));
		newPos[2] = std::stof(params->get(2));

		if (!bz_isWithinWorldBoundaries(newPos))
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "You cannot teleport to that location.");
			return true;
		}

		bz_BasePlayerRecord *player = bz_getPlayerByIndex(playerID);

		if (player->spawned == false)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "You can only teleport if you are alive!");
			bz_freePlayerRecord(player);
			return true;
		}

		if ((!playerInVortex(player->lastKnownState.pos)) && (player->currentFlag != "TelePorter (+TP)"))
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "You can only teleport if you are in the Vortex or have the Teleporter flag!");
			bz_freePlayerRecord(player);
			return true;
		}

		spawnLocation[playerID][0] = newPos[0];
		spawnLocation[playerID][1] = newPos[1];
		spawnLocation[playerID][2] = newPos[2];
		spawnLocation[playerID][3] = 1; // tell it we want it to change the spawn location next time

		bz_killPlayer(playerID, false, -1, NULL);
		bz_incrementTeamLosses(player->team, -1);
		bz_incrementPlayerLosses(playerID, -1);
		playerAlive(playerID);
		bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "%s has teleported through the Vortex! The tank reappeared at coordinates (%d, %d, %d)", player->callsign.c_str(), (int)newPos[0], (int)newPos[1], (int)newPos[2]);
		bz_freePlayerRecord(player);

		return true;
	}

	return false;
}
