// TODO: update teleport coordinates, platform names, platform coordinates

#include "bzfsAPI.h"

#include "../../src/bzfs/bzfs.h"

#include <cctype>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <set>

float spawnLocation[256][5] = {0};
int playerPlatform[256] = {0};

// Platform name mapping.
// Syntax: platformname
const char* platformNames[14] = {
	"none",			// 0
	"platform1",		// 1
	"platform2",		// 2
	"platform3",		// 3
	"platform4",		// 4
	"platform5",		// 5
	"platform6",		// 6
	"Rusty Seabed",		// 7
	"Kelp Forest",		// 8
	"platform9",		// 9
	"platform10",		// 10
	"platform11",		// 11
	"platform12",		// 12
	"platform13",		// 13
};

// Location where players will teleport to if they attempt to teleport to the given platform.
// Syntax: x, y, z
const float platformCoords[14][3] = {
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
	{12,12,0}, // 12
	{13,13,0}, // 13
};

// Areas of each platform.
// Syntax: xmin, xmax, ymin, ymax, zmin, zmax
const float platformZones[14][6] = {
	{0,0,0,0,1000,1000}, // 0 (unused/no platform)
	{0, 0, 0, 0, 0, 0}, // 1
	{0, 0, 0, 0, 0, 0}, // 2
	{0, 0, 0, 0, 0, 0}, // 3
	{0, 0, 0, 0, 0, 0}, // 4
	{0, 0, 0, 0, 0, 0}, // 5
	{0, 0, 0, 0, 0, 0}, // 6
	{0, 0, 0, 0, 0, 0}, // 7
	{0, 0, 0, 0, 0, 0}, // 8
	{0, 0, 0, 0, 0, 0}, // 9
	{0, 0, 0, 0, 0, 0}, // 10
	{0, 0, 0, 0, 0, 0}, // 11
	{0, 0, 0, 0, 0, 0}, // 12
	{0, 0, 0, 0, 0, 0}, // 13
};

class TeleportZone : public bz_CustomZoneObject
{
public:
	TeleportZone() : bz_CustomZoneObject()
	{
		zoneName = "";
		teleportTo = "";
	}

	std::string zoneName;
	std::string teleportTo;
};

std::vector<TeleportZone> teleportZones;
std::map<std::string, int> zoneNameMap;

struct PlayerZoneState {
	std::set<int> currentZones;
};

std::map<int, PlayerZoneState> playerZoneStates;

int getRandomNumber(int min, int max)
{
	return min + (std::rand() % (max - min + 1));
}

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

int getPlatform(float pos[3])
{
	for (int i = 1; i <= 13; i++)
		if (pointIn(pos, platformZones[i][0], platformZones[i][1], platformZones[i][2], platformZones[i][3], platformZones[i][4], platformZones[i][5]))
			return i;
	return 0;
}

int getPlatformByName(const char* name)
{
	std::string lowerName = bz_tolower(name);

	for (int i = 0; i <= 13; i++)
		if (lowerName == bz_tolower(platformNames[i]))
			return i;

	return -1;
}

bool playerOnPlatform(int playerID, const char* platformName)
{
	int platformNumber = getPlatformByName(platformName);

	if (platformNumber == -1)
		return false;

	return (playerPlatform[playerID] == platformNumber);
}

class StationCollaborationPlugin : public bz_Plugin, public bz_CustomSlashCommandHandler, public bz_CustomMapObjectHandler
{
public:
	virtual const char* Name();
	virtual void Init(const char*);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);
	virtual bool SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);
	virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data);
};

BZ_PLUGIN(StationCollaborationPlugin)

const char* StationCollaborationPlugin::Name()
{
	return "Station Collaboration";
}

void StationCollaborationPlugin::Init(const char*)
{
	Register(bz_eGetPlayerSpawnPosEvent);
	Register(bz_ePlayerPartEvent);
	Register(bz_ePlayerUpdateEvent);
	Register(bz_eShotFiredEvent);

	bz_registerCustomSlashCommand("teleport", this);
	bz_registerCustomSlashCommand("tp", this);

	bz_registerCustomMapObject("TELEPORTZONE", this);

	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

void StationCollaborationPlugin::Cleanup()
{
	Flush();

	bz_removeCustomSlashCommand("teleport");
	bz_removeCustomSlashCommand("tp");

	bz_removeCustomMapObject("TELEPORTZONE");
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

		case bz_ePlayerPartEvent:
		{
			bz_PlayerJoinPartEventData_V1* data = (bz_PlayerJoinPartEventData_V1*)eventData;

			playerPlatform[data->playerID] = 0;

			playerZoneStates.erase(data->playerID);
		}
		break;

		case bz_ePlayerUpdateEvent:
		{
			bz_PlayerUpdateEventData_V1* data = (bz_PlayerUpdateEventData_V1*)eventData;
			bz_BasePlayerRecord* pr = bz_getPlayerByIndex(data->playerID);

			int currentPlatform = getPlatform(pr->lastKnownState.pos);
			playerPlatform[data->playerID] = currentPlatform;

			PlayerZoneState& playerState = playerZoneStates[data->playerID];
			std::set<int> newZones;

			for (size_t i = 0; i < teleportZones.size(); i++)
			{
				if (teleportZones[i].pointInZone(pr->lastKnownState.pos))
				{
					newZones.insert(i);

					if (playerState.currentZones.find(i) == playerState.currentZones.end())
					{
						std::map<std::string, int>::iterator it = zoneNameMap.find(teleportZones[i].teleportTo);

						if (it != zoneNameMap.end())
						{
							int destIndex = it->second;
							TeleportZone& destZone = teleportZones[destIndex];

							spawnLocation[data->playerID][0] = (destZone.xMax + destZone.xMin) / 2.0f;
							spawnLocation[data->playerID][1] = (destZone.yMax + destZone.yMin) / 2.0f;
							spawnLocation[data->playerID][2] = (destZone.zMax + destZone.zMin) / 2.0f;
							spawnLocation[data->playerID][3] = pr->lastKnownState.rotation;
							spawnLocation[data->playerID][4] = 1; // tell it we want it to change the spawn location next time

							bz_killPlayer(data->playerID, false, -1, NULL);
							bz_incrementTeamLosses(pr->team, -1);
							bz_incrementPlayerLosses(data->playerID, -1);
							playerAlive(data->playerID);

							newZones.insert(destIndex);
						}
						break;
					}
				}
			}

			playerState.currentZones = newZones;

			bz_freePlayerRecord(pr);
		}
		break;

		case bz_eShotFiredEvent:
		{
			bz_ShotFiredEventData_V1* data = (bz_ShotFiredEventData_V1*)eventData;

			if (playerOnPlatform(data->playerID, "Rusty Seabed") && data->type == "SW")
			{
				bz_killPlayer(data->playerID, false, BZ_SERVER, NULL);
				bz_sendTextMessage(BZ_SERVER, data->playerID, "Congratulations on making it to the treasure chest and killing all other players on the platform!");
				bz_incrementPlayerLosses(data->playerID, -1);
			}
		}

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
			bz_freePlayerRecord(pr);
			return true;
		}

		if (pr->spawned == false)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "You can only teleport if you are alive!");
			bz_freePlayerRecord(pr);
			return true;
		}

		const std::string arg = bz_tolower(params->get(0).c_str());

		int platform = 0;

		if (arg == "random")
		{
			platform = getRandomNumber(1,13);
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

		if (platform < 1 || platform > 13)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "You can only choose a platform between 1 and 13!");
			return true;
		}

		spawnLocation[playerID][0] = platformCoords[platform][0];
		spawnLocation[playerID][1] = platformCoords[platform][1];
		spawnLocation[playerID][2] = platformCoords[platform][2];
		spawnLocation[playerID][3] = pr->lastKnownState.rotation;
		spawnLocation[playerID][4] = 1; // tell it we want it to change the spawn location next time

		bz_killPlayer(playerID, false, -1, NULL);
		bz_incrementTeamLosses(pr->team, -1);
		bz_incrementPlayerLosses(playerID, -1);
		bz_freePlayerRecord(pr);
		playerAlive(playerID);

		return true;
	}

	return false;
}

bool StationCollaborationPlugin::MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data)
{
	if (!data || object != "TELEPORTZONE")
	{
		return false;
	}

	TeleportZone teleportZone;
	teleportZone.handleDefaultOptions(data);

	for (unsigned int i = 0; i < data->data.size(); i++)
	{
		std::string line = data->data.get(i);

		bz_APIStringList nubs;
		nubs.tokenize(line.c_str(), " ", 0, true);

		if (nubs.size() > 0)
		{
			std::string key = bz_toupper(nubs.get(0).c_str());

			if (key == "NAME" && nubs.size() > 1)
			{
				teleportZone.zoneName = nubs.get(1).c_str();
			}
			else if (key == "TELEPORTTO" && nubs.size() > 1)
			{
				teleportZone.teleportTo = nubs.get(1).c_str();
			}
		}
	}

	int zoneIndex = teleportZones.size();
	teleportZones.push_back(teleportZone);

	if (!teleportZone.zoneName.empty())
	{
		zoneNameMap[teleportZone.zoneName] = zoneIndex;
	}

	return true;
}
