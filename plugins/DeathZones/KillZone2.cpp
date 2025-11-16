#include <algorithm>

#include "bzfsAPI.h"

// Define plug-in name
const std::string PLUGIN_NAME = "Death Zones";

// Define plug-in version numbering
const int MAJOR = 1;
const int MINOR = 0;
const int REV = 1;
const int BUILD = 7;
const std::string SUFFIX = "";

// Define build settings
const int VERBOSITY_LEVEL = 4;

class KillZone : public bz_CustomZoneObject
{
public:
    KillZone() : bz_CustomZoneObject()
    {
    }

    std::string message;
    std::vector<bz_eTeamType> teams;

    bool targetsTeam(const std::vector<bz_eTeamType>& teamsToCheck, bz_eTeamType team)
    {
        auto doesTeamExist = std::find(teamsToCheck.begin(), teamsToCheck.end(), team);
        return doesTeamExist != teamsToCheck.end();
    }
};

class KillZones : public bz_Plugin, public bz_CustomMapObjectHandler
{
public:
    virtual const char* Name();
    virtual void Init(const char*);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);
    virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data);

private:
    std::vector<KillZone> killZones;
};

BZ_PLUGIN(KillZones)

const char* KillZones::Name()
{
    static const char *pluginBuild;

    if (!pluginBuild)
    {
        pluginBuild = bz_format("%s %d.%d.%d (%d)", PLUGIN_NAME.c_str(), MAJOR, MINOR, REV, BUILD);

        if (!SUFFIX.empty())
        {
            pluginBuild = bz_format("%s - %s", pluginBuild, SUFFIX.c_str());
        }
    }

    return pluginBuild;
}

void KillZones::Init(const char*)
{
    Register(bz_ePlayerUpdateEvent);

    bz_registerCustomMapObject("KILLZONE", this);
}

void KillZones::Cleanup()
{
    Flush();

    bz_removeCustomMapObject("KILLZONE");
}

void KillZones::Event(bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_ePlayerUpdateEvent:
        {
            bz_PlayerUpdateEventData_V1* data = (bz_PlayerUpdateEventData_V1*)eventData;
            bz_BasePlayerRecord* pr = bz_getPlayerByIndex(data->playerID);
            bz_eTeamType playerTeam = pr->team;
            bz_freePlayerRecord(pr);

            for (auto &zone : killZones)
            {
                if (zone.targetsTeam(zone.teams, playerTeam) && zone.pointInZone(data->state.pos))
                {
                    bz_debugMessage(0, "We have found somebody in a zone.");
                    bz_killPlayer(data->playerID, false, -1, NULL);
                    bz_sendTextMessage(BZ_SERVER, data->playerID, zone.message.c_str());

                    break;
                }
            }
        }
        break;

        default:
            break;
    }
}

bool KillZones::MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data)
{
    // Note, this value will be in uppercase
    if (!data || object != "KILLZONE")
    {
        return false;
    }

    KillZone killZone;
    killZone.handleDefaultOptions(data);

    bz_APIStringList *nubs = bz_newStringList();

    for (unsigned int i = 0; i < data->data.size(); i++)
    {
        std::string line = data->data.get(i);
        nubs->clear();

        nubs->tokenize(line.c_str(), " ", 0, true);

        if (nubs->size() > 0)
        {
            std::string key = bz_toupper(nubs->get(0).c_str());

            if (key == "TEAM")
            {
                killZone.teams.push_back((bz_eTeamType)atoi(nubs->get(1).c_str()));
            }
            else if ((key == "MESSAGE" || key == "MSG") && nubs->size() > 1)
                killZone.message = nubs->get(1).c_str();
        }
    }

    bz_deleteStringList(nubs);

    killZones.push_back(killZone);

    return true;
}
