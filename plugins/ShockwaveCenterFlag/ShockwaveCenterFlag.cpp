#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <math.h>

using namespace std;

class RemoteDetonationFlag : public bz_Plugin
{
	virtual const char* Name()
	{
		return "Remote Detonation Flag 1.3.1";
	}
	virtual void Init(const char*);
	virtual void Event(bz_EventData*);
	~RemoteDetonationFlag();

	virtual void Cleanup(void)
	{
		Flush();
	}
};

BZ_PLUGIN(RemoteDetonationFlag)

void RemoteDetonationFlag::Init(const char*)
{
	bz_RegisterCustomFlag("SC", "Shockwave Center", "Firing detonates a shock wave in the middle of the map. Note: you only get one shot.", 0, eGoodFlag);
	Register(bz_eShotFiredEvent);
	Register(bz_ePlayerDieEvent);
}

RemoteDetonationFlag::~RemoteDetonationFlag() {}

void RemoteDetonationFlag::Event(bz_EventData *eventData)
{
	switch (eventData->eventType)
	{
		case bz_eShotFiredEvent:
		{
			bz_ShotFiredEventData_V1* data = (bz_ShotFiredEventData_V1*) eventData;
			bz_BasePlayerRecord* playerRecord = bz_getPlayerByIndex(data->playerID);

			if (playerRecord && playerRecord->currentFlag == "Shockwave Center (+SC)")
			{
				float pos[3];
				float vel[3];
				pos[0] = 0;
				pos[1] = 0;
				pos[2] = 50;
				vel[0] = 0;
				vel[1] = 0;
				vel[2] = 0;
				uint32_t shockwaveGUID = bz_fireServerShot("SW", pos, vel, playerRecord->team);
				bz_setShotMetaData(shockwaveGUID, "type", bz_getPlayerFlag(data->playerID));
				bz_setShotMetaData(shockwaveGUID, "owner", data->playerID);
				bz_removePlayerFlag(data->playerID);
			}

			bz_freePlayerRecord(playerRecord);
		} break;
		case bz_ePlayerDieEvent:
		{
			bz_PlayerDieEventData_V1* data = (bz_PlayerDieEventData_V1*) eventData;
			uint32_t shotGUID = bz_getShotGUID(data->killerID, data->shotID);

			if (bz_shotHasMetaData(shotGUID, "type") && bz_shotHasMetaData(shotGUID, "owner"))
			{
				std::string flagType = bz_getShotMetaDataS(shotGUID, "type");

				if (flagType == "SC")
				{
					data->killerID = bz_getShotMetaDataI(shotGUID, "owner");
					data->killerTeam = bz_getPlayerTeam(data->killerID);
				}
			}
		} break;
		default:
			break;
	}
}
