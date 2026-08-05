#include "bzfsAPI.h"
#include "plugin_utils.h"

#include <iostream>
#include <cstdlib>
#include <ctime>

const char* playerFlag[256] = {};

int getRandomNumber(int min, int max)
{
	return min + (std::rand() % (max - min + 1));
}

const char* possibleFlags[25] = {
	"A",
	"BU",
	"CL",
	"GM",
	"V",
	"ID",
	"IB",
	"JP",
	"L",
	"MG",
	"MQ",
	"N",
	"OO",
	"PZ",
	"QT",
	"F",
	"R",
	"SH",
	"SB",
	"SR",
	"SE",
	"ST",
	"SW",
	"T",
	"WG",
};

class SpawnWithRandomFlag : public bz_Plugin
{
public:
	virtual const char* Name();
	virtual void Init(const char*);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);
};

void getFlagForPlayer(int playerID)
{
	int flagInt = getRandomNumber(0,24);
	const char* flag = possibleFlags[flagInt];
	playerFlag[playerID] = flag;
}

BZ_PLUGIN(SpawnWithRandomFlag)

const char* SpawnWithRandomFlag::Name()
{
	return "Spawn With Random Flag";
}

void SpawnWithRandomFlag::Init(const char*)
{
	Register(bz_eFlagDroppedEvent);
	Register(bz_ePlayerPartEvent);
	Register(bz_ePlayerSpawnEvent);

	std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

void SpawnWithRandomFlag::Cleanup()
{
	Flush();
}

void SpawnWithRandomFlag::Event(bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_eFlagDroppedEvent:
		{
			bz_FlagDroppedEventData_V1* data = (bz_FlagDroppedEventData_V1*)eventData;
			bz_BasePlayerRecord* pr = bz_getPlayerByIndex(data->playerID);

			if(!pr)
				break;
			if(!pr->spawned)
			{
				bz_freePlayerRecord(pr);
				break;
			}
			if(!playerFlag[data->playerID])
				getFlagForPlayer(data->playerID);

			bz_givePlayerFlag(data->playerID, playerFlag[data->playerID], false);

			bz_freePlayerRecord(pr);
		}
		break;

		case bz_ePlayerPartEvent:
		{
			bz_PlayerJoinPartEventData_V1* data = (bz_PlayerJoinPartEventData_V1*)eventData;

			playerFlag[data->playerID] = nullptr;
		}
		break;

		case bz_ePlayerSpawnEvent:
		{
			bz_PlayerSpawnEventData_V1* data = (bz_PlayerSpawnEventData_V1*)eventData;

			getFlagForPlayer(data->playerID);
			bz_givePlayerFlag(data->playerID, playerFlag[data->playerID], false);
		}
		break;

		default:
			break;
	}
}
