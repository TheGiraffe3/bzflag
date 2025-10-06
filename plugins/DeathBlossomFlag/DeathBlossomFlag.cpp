#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <math.h>

using namespace std;

class DeathBlossomFlag : public bz_Plugin
{
	virtual const char* Name()
	{
		return "Death Blossom Flag";
	}
	virtual void Init(const char*);
	virtual void Event(bz_EventData*);
	~DeathBlossomFlag();

	virtual void Cleanup(void)
	{
		Flush();
	}
};

BZ_PLUGIN(DeathBlossomFlag)

void DeathBlossomFlag::Init(const char*)
{
	bz_RegisterCustomFlag("DB", "Death Blossom", "Fires shrapnel in all directions.", 0, eGoodFlag);
	Register(bz_eShotFiredEvent);
	Register(bz_ePlayerDieEvent);
}

DeathBlossomFlag::~DeathBlossomFlag() {}

void DeathBlossomFlag::Event(bz_EventData *eventData)
{
	switch (eventData->eventType)
	{
		case bz_eShotFiredEvent:
		{
			bz_ShotFiredEventData_V1* data = (bz_ShotFiredEventData_V1*) eventData;
			bz_BasePlayerRecord* playerRecord = bz_getPlayerByIndex(data->playerID);

			if (playerRecord && playerRecord->currentFlag == "Death Blossom (+DB)")
			{
				float pos[3]; // Player position

				pos[0] = playerRecord->lastKnownState.pos[0] + cos(playerRecord->lastKnownState.rotation);
				pos[1] = playerRecord->lastKnownState.pos[1] + sin(playerRecord->lastKnownState.rotation);
				pos[2] = playerRecord->lastKnownState.pos[2] + bz_getBZDBDouble("_muzzleHeight");

				// Left shot
				float leftvel[3];
				leftvel[0] = cos(playerRecord->lastKnownState.rotation - 1.57 + playerRecord->lastKnownState.velocity[0]/bz_getBZDBDouble("_shotSpeed"));
				leftvel[1] = sin(playerRecord->lastKnownState.rotation - 1.57 + playerRecord->lastKnownState.velocity[1]/bz_getBZDBDouble("_shotSpeed"));
				leftvel[2] = 0;
				uint32_t leftShotID = bz_fireServerShot("DB", pos, leftvel, playerRecord->team);
				bz_setShotMetaData(leftShotID, "type", bz_getPlayerFlag(data->playerID));
				bz_setShotMetaData(leftShotID, "owner", data->playerID);

				// Back shot
				float backvel[3];
				backvel[0] = cos(playerRecord->lastKnownState.rotation - 3.14 + playerRecord->lastKnownState.velocity[0]/bz_getBZDBDouble("_shotSpeed"));
				backvel[1] = sin(playerRecord->lastKnownState.rotation - 3.14 + playerRecord->lastKnownState.velocity[1]/bz_getBZDBDouble("_shotSpeed"));
				backvel[2] = 0;
				uint32_t backShotID = bz_fireServerShot("DB", pos, backvel, playerRecord->team);
				bz_setShotMetaData(backShotID, "type", bz_getPlayerFlag(data->playerID));
				bz_setShotMetaData(backShotID, "owner", data->playerID);

				// Right shot
				float rightvel[3];
				rightvel[0] = cos(playerRecord->lastKnownState.rotation + 1.57 + playerRecord->lastKnownState.velocity[0]/bz_getBZDBDouble("_shotSpeed"));
				rightvel[1] = sin(playerRecord->lastKnownState.rotation + 1.57 + playerRecord->lastKnownState.velocity[1]/bz_getBZDBDouble("_shotSpeed"));
				rightvel[2] = 0;
				uint32_t rightShotID = bz_fireServerShot("DB", pos, rightvel, playerRecord->team);
				bz_setShotMetaData(rightShotID, "type", bz_getPlayerFlag(data->playerID));
				bz_setShotMetaData(rightShotID, "owner", data->playerID);

				// Shockwave
				float SWvel[3]; // Shockwave
				SWvel[0] = 0;
				SWvel[1] = 0;
				SWvel[2] = 0;
				uint32_t SWShotID = bz_fireServerShot("SW", pos, SWvel, bz_getPlayerTeam(data->playerID));
				bz_setShotMetaData(SWShotID, "type", bz_getPlayerFlag(data->playerID));
				bz_setShotMetaData(SWShotID, "owner", data->playerID);
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

			    if (flagType == "DB")
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
