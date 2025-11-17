#include "bzfsAPI.h"
#include "plugin_utils.h"
#include <math.h>

const double pi = 3.14159265358979323846;

using namespace std;

class SuperBulletBackwardsFlag : public bz_Plugin
{
    virtual const char* Name ()
    {
        return "BS Flag";
    }
    virtual void Init(const char*);
	virtual void Event(bz_EventData*);
	~SuperBulletBackwardsFlag();

	virtual void Cleanup(void)
	{
		Flush();
	}
};

BZ_PLUGIN(SuperBulletBackwardsFlag)

void SuperBulletBackwardsFlag::Init(const char*) {
	Register(bz_eShotFiredEvent);
	Register(bz_ePlayerDieEvent);

	bz_RegisterCustomFlag("BS", "Backwards Shot", "Fire a super bullet backwards in addition to your normal shot.", 0, eGoodFlag);

	bz_registerCustomBZDBDouble("_backwardsShotDistance", 40);
}

SuperBulletBackwardsFlag::~SuperBulletBackwardsFlag() {}

void SuperBulletBackwardsFlag::Event(bz_EventData *ed)
{
	switch (ed->eventType)
	{
		case bz_eShotFiredEvent:
		{
			bz_ShotFiredEventData_V1* data = (bz_ShotFiredEventData_V1*) ed;
			bz_BasePlayerRecord* playerRecord = bz_getPlayerByIndex(data->playerID);

			if (playerRecord && playerRecord->currentFlag == "Backwards Shot (+BS)")
			{
				float pos[3];
				pos[0] = playerRecord->lastKnownState.pos[0] - cos(playerRecord->lastKnownState.rotation)*bz_getBZDBDouble("_backwardsShotDistance");
				pos[1] = playerRecord->lastKnownState.pos[1] - sin(playerRecord->lastKnownState.rotation)*bz_getBZDBDouble("_backwardsShotDistance");
				pos[2] = playerRecord->lastKnownState.pos[2] + bz_getBZDBDouble("_muzzleHeight");

				float vel[3];
				vel[0] = cos(playerRecord->lastKnownState.rotation - pi + playerRecord->lastKnownState.velocity[0]/(bz_getBZDBDouble("_shotSpeed")*2));
				vel[1] = sin(playerRecord->lastKnownState.rotation - pi + playerRecord->lastKnownState.velocity[1]/(bz_getBZDBDouble("_shotSpeed")*2));
				vel[2] = 0;

				uint32_t shotGUID = bz_fireServerShot("SB", pos, vel, playerRecord->team);
				bz_setShotMetaData(shotGUID, "type", "BS");
				bz_setShotMetaData(shotGUID, "owner", data->playerID);
			}

			bz_freePlayerRecord(playerRecord);
		} break;
		case bz_ePlayerDieEvent:
		{
			bz_PlayerDieEventData_V1* data = (bz_PlayerDieEventData_V1*) ed;
			uint32_t shotGUID = bz_getShotGUID(data->killerID, data->shotID);

			if (bz_shotHasMetaData(shotGUID, "type") && bz_shotHasMetaData(shotGUID, "owner"))
			{
			    std::string flagType = bz_getShotMetaDataS(shotGUID, "type");

			    if (flagType == "BS")
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
