#include "bzfsAPI.h"

class RedKillZone : public bz_CustomZoneObject
{
public:
	RedKillZone() : bz_CustomZoneObject()
	{
	}
};

class GreenKillZone : public bz_CustomZoneObject
{
public:
	GreenKillZone() : bz_CustomZoneObject()
	{
	}
};

class BlueKillZone : public bz_CustomZoneObject
{
public:
	BlueKillZone() : bz_CustomZoneObject()
	{
	}
};

class PurpleKillZone : public bz_CustomZoneObject
{
public:
	PurpleKillZone() : bz_CustomZoneObject()
	{
	}
};

class KillZone : public bz_Plugin, public bz_CustomMapObjectHandler
{
public:
	virtual const char* Name();
	virtual void Init(const char*);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);
	virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data);

	std::vector<RedKillZone> redKillZones;
	std::vector<GreenKillZone> greenKillZones;
	std::vector<BlueKillZone> blueKillZones;
	std::vector<PurpleKillZone> purpleKillZones;
};

BZ_PLUGIN(KillZone)

const char* KillZone::Name()
{
	return "Death Zones";
}

void KillZone::Init(const char*)
{
	bz_registerCustomMapObject("REDKILL", this);
	bz_registerCustomMapObject("GREENKILL", this);
	bz_registerCustomMapObject("BLUEKILL", this);
	bz_registerCustomMapObject("PURPLEKILL", this);
}

void KillZone::Cleanup()
{
	Flush();

	bz_removeCustomMapObject("REDKILL");
	bz_removeCustomMapObject("GREENKILL");
	bz_removeCustomMapObject("BLUEKILL");
	bz_removeCustomMapObject("PURPLEKILL");
}

void KillZone::Event(bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_ePlayerUpdateEvent:
		{
			bz_PlayerUpdateEventData_V1* data = (bz_PlayerUpdateEventData_V1*)eventData;
			int playerID = data->playerID;

			for (unsigned int i = 0; i < redKillZones.size(); i++)
			{
				if (redKillZones[i].pointInZone(data->state.pos) && bz_getPlayerTeam(playerID) != eRedTeam)
					bz_killPlayer(data->playerID, false, data->playerID, NULL);
			}
			for (unsigned int i = 0; i < greenKillZones.size(); i++)
			{
				if (greenKillZones[i].pointInZone(data->state.pos) && bz_getPlayerTeam(playerID) != eGreenTeam)
					bz_killPlayer(data->playerID, false, data->playerID, NULL);
			}
			for (unsigned int i = 0; i < blueKillZones.size(); i++)
			{
				if (blueKillZones[i].pointInZone(data->state.pos) && bz_getPlayerTeam(playerID) != eBlueTeam)
					bz_killPlayer(data->playerID, false, data->playerID, NULL);
			}
			for (unsigned int i = 0; i < purpleKillZones.size(); i++)
			{
				if (purpleKillZones[i].pointInZone(data->state.pos) && bz_getPlayerTeam(playerID) != ePurpleTeam)
					bz_killPlayer(data->playerID, false, data->playerID, NULL);
			}
		}
		break;

		default:
			break;
	}
}

bool KillZone::MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data)
{
	// Note, this value will be in uppercase
	if (!data || (object != "REDKILL" && object != "GREENKILL" && object != "BLUEKILL" && object != "PURPLEKILL"))
	{
		return false;
	}

	if (object == "REDKILL")
	{
		RedKillZone redKillZone;
		redKillZone.handleDefaultOptions(data);
		redKillZones.push_back(redKillZone);
	}
	else if (object == "GREENKILL")
	{
		GreenKillZone greenKillZone;
		greenKillZone.handleDefaultOptions(data);
		greenKillZones.push_back(greenKillZone);
	}
	else if (object == "BLUEKILL")
	{
		BlueKillZone blueKillZone;
		blueKillZone.handleDefaultOptions(data);
		blueKillZones.push_back(blueKillZone);
	}
	else if (object == "PURPLEKILL")
	{
		PurpleKillZone purpleKillZone;
		purpleKillZone.handleDefaultOptions(data);
		purpleKillZones.push_back(purpleKillZone);
	}

	return true;
}
