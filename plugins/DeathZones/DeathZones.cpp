#include <algorithm>

#include "bzfsAPI.h"
#include "plugin_utils.h"

static bz_eTeamType convertTeamType(int teamNumber)
{
	switch (teamNumber)
	{
		case 0:
			return eRogueTeam;

		case 1:
			return eRedTeam;

		case 2:
			return eGreenTeam;

		case 3:
			return eBlueTeam;

		case 4:
			return ePurpleTeam;

		case 5:
			return eRabbitTeam;

		case 6:
			return eHunterTeam;

		default:
			return eNoTeam;
	}
}

template <typename T>
static bool doesVectorContain(std::vector<T> &vec, T val)
{
	return (std::find(vec.begin(), vec.end(), val) != vec.end());
}

class DeathZone : public bz_CustomZoneObject
{
public:
	DeathZone() : bz_CustomZoneObject() {}

	std::vector<bz_eTeamType> teamsAffected;

	std::string privatemessage = "You entered a death zone!";
	std::string publicmessage = "";
};

class DeathZones : public bz_Plugin, public bz_CustomMapObjectHandler
{
public:
	virtual const char* Name();
	virtual void Init(const char* config);
	virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo *data);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);

private:
	std::vector<DeathZone> registeredDeathZones;
};

BZ_PLUGIN(DeathZones)

const char* DeathZones::Name()
{
	return "Death Zones";
}

void DeathZones::Init(const char* /*config*/)
{
	Register(bz_ePlayerUpdateEvent);

	bz_registerCustomMapObject("deathzone", this);
}

bool DeathZones::MapObject(bz_ApiString object, bz_CustomMapObjectInfo *data)
{
	if (object != "DEATHZONE" || !data)
	{
		return false;
	}

	DeathZone deathZone;
	deathZone.handleDefaultOptions(data);

	for (unsigned int i = 0; i < data->data.size(); i++)
	{
		std::string line = data->data.get(i).c_str();

		bz_APIStringList nubs;
		nubs.tokenize(line.c_str(), " ", 0, true);

		if (nubs.size() > 0)
		{
			std::string key = bz_toupper(nubs.get(0).c_str());

			if (key == "TEAM" && nubs.size() == 2)
			{
				int teamNum = std::atoi(nubs.get(1).c_str());
				bz_eTeamType team = convertTeamType(teamNum);

				if (team != eNoTeam) {
					deathZone.teamsAffected.push_back(team);
				}
			}

			if (key == "PRIVATEMESSAGE")
			{
				if (nubs.size() == 2)
					deathZone.privatemessage = nubs.get(1);
				else
					deathZone.privatemessage = "";
			}

			if (key == "PUBLICMESSAGE")
			{
				if (nubs.size() == 2)
					deathZone.publicmessage = nubs.get(1);
				else
					deathZone.publicmessage = "";
			}
		}
	}

	registeredDeathZones.push_back(deathZone);

	return true;
}

void DeathZones::Cleanup()
{
	Flush();

	bz_removeCustomMapObject("deathzone");
}

void DeathZones::Event(bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_ePlayerUpdateEvent:
		{
			bz_PlayerUpdateEventData_V1* data = (bz_PlayerUpdateEventData_V1*)eventData;

			bz_BasePlayerRecord* pr = bz_getPlayerByIndex(data->playerID);

			if (!pr)
				break;

			if (!pr->spawned)
			{
				bz_freePlayerRecord(pr);
				break;
			}

			for (auto &zone : registeredDeathZones)
			{
				if (!zone.pointInZone(data->lastState.pos))
					continue;

				if (!zone.teamsAffected.empty() && !doesVectorContain<bz_eTeamType>(zone.teamsAffected, pr->team))
					continue;

				bz_killPlayer(data->playerID, false, -1, NULL);

				if (!zone.privatemessage.empty())
					bz_sendTextMessage(BZ_SERVER, data->playerID, zone.privatemessage.c_str());

				if (!zone.publicmessage.empty())
				{
					bz_ApiString publicMessage = zone.publicmessage.c_str();
					publicMessage.replaceAll("%player%", pr->callsign.c_str());
					bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, publicMessage.c_str());
				}
			}

			bz_freePlayerRecord(pr);
		}
		break;

		default:
			break;
	}
}
