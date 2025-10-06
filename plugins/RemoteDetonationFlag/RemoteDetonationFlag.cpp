#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <math.h>

using namespace std;

class RemoteDetonationFlag : public bz_Plugin
{
	virtual const char* Name()
	{
		return "Remote Detonation Flag";
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
	bz_RegisterCustomFlag("RD", "Remote Detonation", "Firing detonates a shock wave four hundred and fifty world units ahead of you.", 0, eGoodFlag);
	bz_registerCustomBZDBDouble("_remoteDetonationDistance", 450.0);
	bz_registerCustomBZDBDouble("_remoteDetonationHeight", 50.0);
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

			if (playerRecord && playerRecord->currentFlag == "Remote Detonation (+RD)")
			{
				float pos[3];
				float vel[3];
				pos[0] = playerRecord->lastKnownState.pos[0] + cos(playerRecord->lastKnownState.rotation)*bz_getBZDBDouble("_remoteDetonationDistance");
				pos[1] = playerRecord->lastKnownState.pos[1] + sin(playerRecord->lastKnownState.rotation)*bz_getBZDBDouble("_remoteDetonationDistance");
				pos[2] = playerRecord->lastKnownState.pos[2] + bz_getBZDBDouble("_muzzleHeight") + bz_getBZDBDouble("_remoteDetonationHeight");
				vel[0] = 0;
				vel[1] = 0;
				vel[2] = 0;
				uint32_t remoteDetonationShotID = bz_fireServerShot("SW", pos, vel, bz_getPlayerTeam(data->playerID));
				bz_setShotMetaData(remoteDetonationShotID, "type", bz_getPlayerFlag(data->playerID));
				bz_setShotMetaData(remoteDetonationShotID, "owner", data->playerID);
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

			    if (flagType == "RD")
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
