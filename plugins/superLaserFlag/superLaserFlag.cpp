#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <math.h>

using namespace std;

class SuperLaserFlag : public bz_Plugin
{
public:
	virtual const char* Name()
	{
		return "Super Laser";
	}
	virtual void Init(const char*);
	virtual void Event(bz_EventData*);
	~SuperLaserFlag();

	virtual void Cleanup(void)
	{
		Flush();
	}
};

BZ_PLUGIN(SuperLaserFlag)

void SuperLaserFlag::Init(const char*)
{
	bz_RegisterCustomFlag("SL", "Super Laser", "Fires a string of shockwaves ahead of you.", 0, eGoodFlag);
	bz_registerCustomBZDBDouble("_superLaserDistance", 7.0);
	bz_registerCustomBZDBDouble("_superLaserHeight", 0.0);
	bz_registerCustomBZDBDouble("_superLaserSpacing", 5.0);
	bz_registerCustomBZDBInt("_superLaserCount", 10);

	Register(bz_eShotFiredEvent);
	Register(bz_ePlayerDieEvent);
}

SuperLaserFlag::~SuperLaserFlag() {}

void SuperLaserFlag::Event(bz_EventData *eventData)
{
	switch (eventData->eventType)
	{
		case bz_eShotFiredEvent:
		{
			bz_ShotFiredEventData_V1* data = (bz_ShotFiredEventData_V1*) eventData;
			bz_BasePlayerRecord* playerRecord = bz_getPlayerByIndex(data->playerID);

			if (playerRecord && playerRecord->currentFlag == "Super Laser (+SL)")
			{
				int count = bz_getBZDBInt("_superLaserCount");
				for (int i = 0; i < count; ++i)
				{
					float pos[3];
					float vel[3] = {0, 0, 0};

					pos[0] = playerRecord->lastKnownState.pos[0] + cos(playerRecord->lastKnownState.rotation) * (bz_getBZDBDouble("_superLaserDistance") + bz_getBZDBDouble("_superLaserSpacing") * i);
					pos[1] = playerRecord->lastKnownState.pos[1] + sin(playerRecord->lastKnownState.rotation) * (bz_getBZDBDouble("_superLaserDistance") + bz_getBZDBDouble("_superLaserSpacing") * i);
					pos[2] = playerRecord->lastKnownState.pos[2] + bz_getBZDBDouble("_muzzleHeight") + bz_getBZDBDouble("_superLaserHeight");

					uint32_t shotID = bz_fireServerShot("SW", pos, vel, bz_getPlayerTeam(data->playerID));
					bz_setShotMetaData(shotID, "type", "SL");
					bz_setShotMetaData(shotID, "owner", data->playerID);
				}
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

				if (flagType == "SL")
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
