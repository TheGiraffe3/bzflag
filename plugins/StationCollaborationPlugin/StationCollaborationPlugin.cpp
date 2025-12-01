#include "bzfsAPI.h"
#include "plugin_utils.h"

#include <cctype>
#include <cstdlib>
#include <ctime>
#include <string>

float spawnLocation[256][5] = {0};

int getRandomNumber(int min, int max)
{
	return min + (std::rand() % (max - min + 1));
}

const float platformCoords[12][3] = {
	{0,0,0}, // 0 (unused and kept at 0)
	{1,1,0}, // 1
	{2,2,0}, // 2
	{3,3,0}, // 3
	{4,4,0}, // 4
	{5,5,0}, // 5
	{6,6,0}, // 6
	{7,7,0}, // 7
	{8,8,0}, // 8
	{9,9,0}, // 9
	{10,10,0}, // 10
	{11,11,0}, // 11
};

class StationCollaborationPlugin : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:
	virtual const char* Name();
	virtual void Init(const char*);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);
	virtual bool SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);
};

BZ_PLUGIN(StationCollaborationPlugin)

const char* StationCollaborationPlugin::Name()
{
	return "Station Collaboration";
}

void StationCollaborationPlugin::Init(const char*)
{
	Register(bz_eGetPlayerSpawnPosEvent);

	bz_registerCustomSlashCommand("teleport", this);
	bz_registerCustomSlashCommand("tp", this);

	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

void StationCollaborationPlugin::Cleanup()
{
	Flush();

	bz_removeCustomSlashCommand("teleport");
	bz_removeCustomSlashCommand("tp");
}

void StationCollaborationPlugin::Event(bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_eGetPlayerSpawnPosEvent:
		{
			bz_GetPlayerSpawnPosEventData_V1* data = (bz_GetPlayerSpawnPosEventData_V1*)eventData;

			if (spawnLocation[data->playerID][4] == 0)
				break;

			data->handled = true;
			data->pos[0] = spawnLocation[data->playerID][0];
			data->pos[1] = spawnLocation[data->playerID][1];
			data->pos[2] = spawnLocation[data->playerID][2];
			data->rot = spawnLocation[data->playerID][3];

			spawnLocation[data->playerID][4] = 0; // don't mess with the spawn next time, unless they run the command again
		}
		break;

		default:
			break;
	}
}

bool StationCollaborationPlugin::SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
	if (command == "teleport" || command == "tp")
	{
		if (params->size() != 1)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "/teleport <random|platform #>");
			return true;
		}

		bz_BasePlayerRecord* pr = bz_getPlayerByIndex(playerID);

		if (pr->team == eObservers)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "Observers can't teleport!");
			return true;
		}

		const std::string arg = bz_tolower(params->get(0).c_str());

		int platform = 0;

		if (arg == "random")
		{
			platform = getRandomNumber(1,11);
		}
		else if (arg.find_first_not_of("0123456789") == std::string::npos)
		{
			platform = std::stoi(arg);
		}
		else
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "/teleport <random|platform #>");
			return true;
		}

		if (platform < 1 || platform > 11)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "You can only choose a platform between 1 and 11!");
			return true;
		}

		spawnLocation[playerID][0] = platformCoords[platform][0];
		spawnLocation[playerID][1] = platformCoords[platform][1];
		spawnLocation[playerID][2] = platformCoords[platform][2];
		spawnLocation[playerID][3] = pr->lastKnownState.rotation;
		spawnLocation[playerID][4] = 1; // tell it we want it to change the spawn location next time

		bz_killPlayer(playerID, false, -1, NULL);
		bz_incrementPlayerLosses(playerID, -1);
		bz_freePlayerRecord(pr);

		return true;
	}

	return false;
}
