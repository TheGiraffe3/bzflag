/*
 * Copyright (C) 2025 Loymdayddaud
 * All rights reserved.
 */

#include "bzfsAPI.h"
#include "plugin_utils.h"

class TronPlugin : public bz_Plugin
{
public:
	virtual const char* Name();
	virtual void Init(const char*);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);
};

BZ_PLUGIN(TronPlugin)

const char* TronPlugin::Name()
{
	return "Tron Plugin";
}

void TronPlugin::Init(const char*)
{
	Register(bz_eFlagGrabbedEvent);
	Register(bz_ePlayerDieEvent);
	Register(bz_ePlayerSpawnEvent);
}

void TronPlugin::Cleanup()
{
	Flush();
}

void TronPlugin::Event(bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_eFlagGrabbedEvent:
		{
			bz_FlagGrabbedEventData_V1* data = (bz_FlagGrabbedEventData_V1*)eventData;

			std::string flag = data->flagType;
			if (flag == "CL")
				bz_sendTextMessage(BZ_SERVER, data->playerID, "Note: you may only make one kill with this flag.");
		}
		break;

		case bz_ePlayerDieEvent:
		{
			bz_PlayerDieEventData_V2* data = (bz_PlayerDieEventData_V2*)eventData;

			int killerFlagID = bz_getPlayerFlagID(data->killerID);
			bz_ApiString killerFlag = bz_getFlagName(killerFlagID);

			if (killerFlag == "CL")
			{
				bz_removePlayerFlag(data->killerID);
				bz_sendTextMessage(BZ_SERVER, data->killerID, "Your Cloaking flag has been taken, since you have made a kill.");
			}
		}
		break;

		case bz_ePlayerSpawnEvent:
		{
			bz_PlayerSpawnEventData_V1* data = (bz_PlayerSpawnEventData_V1*)eventData;

			bz_givePlayerFlag(data->playerID, "MG", true);
		}
		break;

		default:
			break;
	}
}
