#include "bzfsAPI.h"
#include "../plugin_utils/plugin_utils.h"
#include <sstream>

class DeathZones : public bz_Plugin, public bz_CustomMapObjectHandler
{
public:
	std::vector<DeathZone> deathZones;

	virtual const char* Name() { return "Death Zones"; }

	virtual void Init(const char*)
	{
		bz_registerCustomMapObject("DEATHZONE", this);

		Register(bz_ePlayerUpdateEvent);
	}

	virtual void Cleanup()
	{
		Flush();
	}

	virtual void Event(bz_EventData* eventData)
	{
		if (eventData->eventType == bz_ePlayerUpdateEvent)
		{
			bz_PlayerUpdateEventData_V1* data = (bz_PlayerUpdateEventData_V1*)eventData;
			int playerID = data->playerID;

			bz_BasePlayerRecord* pr = bz_getPlayerByIndex(playerID);

			int playerTeam = (int)pr->team;
			float pos[3];
			pos[0] = pr->lastKnownState.pos[0];
			pos[1] = pr->lastKnownState.pos[1];
			pos[2] = pr->lastKnownState.pos[2];

			bz_freePlayerRecord(pr);

			for (size_t i = 0; i < deathZones.size(); ++i)
			{
				DeathZone dz;
				if (!dz->pointInZone(pos))
					continue;

				bool onKillTeam = false;
				std::vector<int>& killTeams = (std::vector<int>&)dz->data;

				if (!killTeams.empty())
				{
					for (size_t t = 0; t < killTeams.size(); ++t)
					{
						if (killTeams[t] == playerTeam)
						{
							onKillTeam = true;
							break;
						}
					}
					if (!onKillTeam)
						continue;
				}

				bz_killPlayer(playerID, false, BZ_SERVER);
				bz_sendTextMessage(BZ_SERVER, playerID, ((std::string*)dz->userData)->c_str());
				break;
			}
		}
	}

	virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data)
	{
		if (!data || object != "DEATHZONE")
			return false;

		bz_CustomZoneObject* zone = new bz_CustomZoneObject();
		zone->handleDefaultOptions(data);

		std::vector<int>* killTeams = std::vector<int>();
		std::string* message = std::string("You entered a death zone!");

		for (unsigned int i = 0; i < data->data.size(); ++i)
		{
			std::string line = data->data.get(i).c_str();

			bz_APIStringList nubs;
			nubs.tokenize(line.c_str(), " ", 0, true);

			if (nubs.size() > 0)
			{
				std::string key = bz_toupper(nubs.get(0).c_str());

				if (key == "TEAM" && nubs.size() == 1)
				{
					int teamNum = std::atoi(nubs.get(1).c_str());
					zone->killTeams.push_back(teamNum);
				}
			}
			else if (key == "MESSAGE")
			{
				*message = nubs.get(1).c_str();
			}
		}

		zone->data = (void*)killTeams;
		zone->userData = (void*)message;
		deathZones.push_back(zone);
		return true;
	}
};

BZ_PLUGIN(DeathZones)
