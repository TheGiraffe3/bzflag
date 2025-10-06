#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <math.h>

using namespace std;

class BombFlag : public bz_Plugin
{
	virtual const char* Name()
	{
		return "Bomb Flag";
	}
	virtual void Init(const char*);
	virtual void Event(bz_EventData*);
	~BombFlag();

	virtual void Cleanup(void)
	{
		Flush();
	}
};

BZ_PLUGIN(BombFlag)

void BombFlag::Init(const char*)
{
	bz_RegisterCustomFlag("BR", "Bomber", "Drops a super bullet bomb upon fire.", 0, eGoodFlag);
//	bz_registerCustomBZDBDouble("_bombSpeed", -10.0);
	Register(bz_eShotFiredEvent);
	Register(bz_ePlayerDieEvent);
}

BombFlag::~BombFlag() {}

void BombFlag::Event(bz_EventData *eventData)
{
	switch (eventData->eventType)
	{
		case bz_eShotFiredEvent:
		{
			bz_ShotFiredEventData_V1* data = (bz_ShotFiredEventData_V1*) eventData;
			bz_BasePlayerRecord* playerRecord = bz_getPlayerByIndex(data->playerID);

			if (playerRecord && playerRecord->currentFlag == "Bomber (+BR)")
			{
				float pos[3];
				float vel[3];
				bz_debugMessage(1, "DEBUG :: Bomb Flag :: Created Floats");
				bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "DEBUG :: Created Floats");
				pos[0] = playerRecord->lastKnownState.pos[0] + cos(playerRecord->lastKnownState.rotation)*2;
				pos[1] = playerRecord->lastKnownState.pos[1] + sin(playerRecord->lastKnownState.rotation)*2;
				pos[2] = playerRecord->lastKnownState.pos[2];
				vel[0] = 0;
				vel[1] = 0;
				vel[2] = 0;//bz_getBZDBDouble("_bombSpeed");
				uint32_t bombShotID = bz_fireServerShot("SB", pos, vel, bz_getPlayerTeam(data->playerID));
				bz_setShotMetaData(bombShotID, "type", bz_getPlayerFlag(data->playerID));
				bz_setShotMetaData(bombShotID, "owner", data->playerID);
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

			    if (flagType == "BR")
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
