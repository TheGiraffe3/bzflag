/*
 * Copyright (C) 2025 Loymdayddaud
 * All rights reserved.
 */

#include "bzfsAPI.h"
#include "plugin_utils.h"

class PhantomZoneFlagOnSpawn : public bz_Plugin
{
public:
	virtual const char* Name();
	virtual void Init(const char*);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);
};

BZ_PLUGIN(PhantomZoneFlagOnSpawn)

const char* PhantomZoneFlagOnSpawn::Name()
{
	return "Phantom Zone Flag On Spawn";
}

void PhantomZoneFlagOnSpawn::Init(const char*)
{
	Register(bz_ePlayerSpawnEvent);
}

void PhantomZoneFlagOnSpawn::Cleanup()
{
	Flush();
}

void PhantomZoneFlagOnSpawn::Event(bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_ePlayerSpawnEvent:
		{
			// This event is called each time a playing tank is being spawned into the world
			bz_PlayerSpawnEventData_V1* data = (bz_PlayerSpawnEventData_V1*)eventData;

			bz_givePlayerFlag(data->playerID, "PZ", true);

			// Data
			// ----
			// (int)                  playerID  - ID of the player who was added to the world.
			// (bz_eTeamType)         team      - The team the player is a member of.
			// (bz_PlayerUpdateState) state     - The state record for the spawning player
			// (double)               eventTime - Time local server time for the event.
		}
		break;

		default:
			break;
	}
}
