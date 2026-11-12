#include "bzfsAPI.h"
#include "plugin_utils.h"

#include <ctime>

static constexpr int PLAYER_COUNT = 256;

// Holds information about the player's timer.
// -1 if we should not be counting, 0 if we should be counting, positive otherwise.
int playerTimer[PLAYER_COUNT] = {-1};
bool sentPlayerWarning[PLAYER_COUNT] = {false};
std::string escapeMessage[PLAYER_COUNT];

class TimedDeathZone : public bz_CustomZoneObject
{
public:
	TimedDeathZone() : bz_CustomZoneObject()
	{
	}

	int time = 5;

	std::string flag;
	bz_eTeamType team;
	bool teamRestriction = false;
	std::string message = "Warning: you are in a timed death zone. You may be killed if you do not move out of it soon.";
	std::string deathMessage = "You stayed in a death zone for too long!";
	std::string escapeMessage = "You have escaped the bounds of the timed death zone.";
};

class timedDeathZone : public bz_Plugin, public bz_CustomMapObjectHandler
{
public:
	virtual const char* Name();
	virtual void Init(const char* /*config*/);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);
	virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data);

	std::vector<TimedDeathZone> timedDeathZones;

	time_t lastCountdownCheck;
};

BZ_PLUGIN(timedDeathZone)

const char* timedDeathZone::Name()
{
	return "Timed Death Zones";
}

void timedDeathZone::Init(const char* /*config*/)
{
	Register(bz_ePlayerDieEvent);
	Register(bz_ePlayerPartEvent);
	Register(bz_ePlayerUpdateEvent);
	Register(bz_eTickEvent);

	bz_registerCustomMapObject("timeddeathzone", this);

	lastCountdownCheck = time(NULL);
	std::fill(std::begin(playerTimer), std::end(playerTimer), -1);
}

void timedDeathZone::Cleanup()
{
	Flush();

	bz_removeCustomMapObject("timeddeathzone");
}

void timedDeathZone::Event(bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_ePlayerDieEvent:
		{
			bz_PlayerDieEventData_V2* data = (bz_PlayerDieEventData_V2*)eventData;

			playerTimer[data->playerID] = -1;
			sentPlayerWarning[data->playerID] = false;
			escapeMessage[data->playerID] = "";
		}
		break;

		case bz_ePlayerPartEvent:
		{
			bz_PlayerJoinPartEventData_V1* data = (bz_PlayerJoinPartEventData_V1*)eventData;

			playerTimer[data->playerID] = -1;
			sentPlayerWarning[data->playerID] = false;
			escapeMessage[data->playerID] = "";
		}
		break;

		case bz_ePlayerUpdateEvent:
		{
			bz_PlayerUpdateEventData_V1* data = (bz_PlayerUpdateEventData_V1*)eventData;

			bz_BasePlayerRecord* pr = bz_getPlayerByIndex(data->playerID);

			if (!pr)
				break;

			if (!pr->spawned)
				break;

			bool isInDeathZone = false;
			std::string message = "";
			std::string zoneEscapeMessage = "";

			for (auto &zone : timedDeathZones)
			{
				if (zone.pointInZone(data->lastState.pos))
				{
					if (!zone.flag.empty() && pr->currentFlag == zone.flag)
						continue;

					if (zone.teamRestriction && pr->team != zone.team)
						continue;

					message = zone.message;

					zoneEscapeMessage = zone.escapeMessage;

					isInDeathZone = true;
				}
			}

			bz_freePlayerRecord(pr);

			if (isInDeathZone == true)
			{
				if (playerTimer[data->playerID] == -1)
					playerTimer[data->playerID] = 0;
				if (sentPlayerWarning[data->playerID] == false)
				{
					sentPlayerWarning[data->playerID] = true;
					bz_sendTextMessage(BZ_SERVER, data->playerID, message.c_str());
				}
				escapeMessage[data->playerID] = zoneEscapeMessage;
			}
			else
			{
				playerTimer[data->playerID] = -1;
				if (sentPlayerWarning[data->playerID] == true)
				{
					sentPlayerWarning[data->playerID] = false;
					bz_sendTextMessage(BZ_SERVER, data->playerID, escapeMessage[data->playerID].c_str());
				}
			}
		}
		break;

		case bz_eTickEvent:
		{
			time_t currentTime;
			time(&currentTime);

			if (difftime(currentTime, lastCountdownCheck) >= 1)
			{
				for (int i = 0; i < PLAYER_COUNT; i++)
				{
					bz_BasePlayerRecord* pr = bz_getPlayerByIndex(i);

					if (!pr)
						continue;

					if (playerTimer[i] >= 0)
						playerTimer[i]++;

					for (auto &zone : timedDeathZones)
					{
						if (!zone.pointInZone(pr->lastKnownState.pos))
							continue;

						std::string flagAbbv = bz_getFlagName(pr->currentFlagID);
						if (!zone.flag.empty() && flagAbbv == zone.flag)
							continue;

						if (zone.teamRestriction && pr->team != zone.team)
							continue;

						if (playerTimer[i] >= zone.time)
						{
							bz_sendTextMessage(BZ_SERVER, i, zone.deathMessage.c_str());
							bz_killPlayer(i, false, -1, NULL);
							playerTimer[i] = -1;
							sentPlayerWarning[i] = false;
						}
					}

					bz_freePlayerRecord(pr);
				}
				lastCountdownCheck = currentTime;
			}
		}
		break;

		default:
			break;
	}
}

bool timedDeathZone::MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data)
{
	if (!data || object != "TIMEDDEATHZONE")
		return false;

	TimedDeathZone zone;
	zone.handleDefaultOptions(data);

	for (unsigned int i = 0; i < data->data.size(); i++)
	{
		std::string line = data->data.get(i);

		bz_APIStringList nubs;
		nubs.tokenize(line.c_str(), " ", 0, true);

		if (nubs.size() > 0)
		{
			std::string key = bz_toupper(nubs.get(0).c_str());

			if (key == "TIME")
			{
				int time = atoi(nubs.get(1).c_str());
				if (time > 0)
					zone.time = time;
				else
					bz_debugMessage(0, "A timed death zone did not have a time specified for it. Defaulting to 5.");
			}
			else if (key == "TEAM")
			{
				zone.team = (bz_eTeamType)atoi(nubs.get(1).c_str());
				zone.teamRestriction = true;
			}
			else if (key == "MESSAGE")
			{
				if (!nubs.get(1).empty())
					zone.message = nubs.get(1).c_str();
				else
					zone.message = "";
			}
			else if (key == "DEATHMESSAGE")
			{
				if (!nubs.get(1).empty())
					zone.deathMessage = nubs.get(1).c_str();
				else
					zone.deathMessage = "";
			}
			else if (key == "ESCAPEMESSAGE")
			{
				if (!nubs.get(1).empty())
					zone.escapeMessage = nubs.get(1).c_str();
				else
					zone.escapeMessage = "";
			}
			else if (key == "FLAG")
			{
				std::string flag = bz_toupper(nubs.get(1).c_str());
				zone.flag = flag.c_str();
			}
		}
	}

	timedDeathZones.push_back(zone);

	return true;
}
