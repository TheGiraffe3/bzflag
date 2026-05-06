/*
    Copyright (C) 2016 Vladimir "allejo" Jimenez

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
 */

#include <iterator>
#include <map>
#include <random>
#include <sstream>
#include <vector>

#include "bzfsAPI.h"

// Define plugin name
const std::string PLUGIN_NAME = "Spawn at Custom Base";

// Define plugin version numbering
const int MAJOR = 1;
const int MINOR = 0;
const int REV = 0;
const int BUILD = 6;

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
    std::uniform_int_distribution<> dis(0, (int)(std::distance(start, end) - 1));
    std::advance(start, dis(g));

    return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    return select_randomly(start, end, gen);
}

class BaseSpawnZone : public bz_CustomZoneObject
{
public:
    BaseSpawnZone() : bz_CustomZoneObject(),
        team(eNoTeam) {}

    bz_eTeamType team;
};

class SpawnAtBase : public bz_Plugin, bz_CustomMapObjectHandler
{
public:
    virtual const char* Name ();
    virtual void Init (const char* config);
    virtual void Event (bz_EventData *eventData);
    virtual void Cleanup (void);
    virtual bool MapObject (bz_ApiString object, bz_CustomMapObjectInfo *data);

    typedef std::vector<BaseSpawnZone> ZoneVector;
    std::map<bz_eTeamType, ZoneVector> TeamZones;
};

BZ_PLUGIN(SpawnAtBase)

const char* SpawnAtBase::Name (void)
{
    static std::string pluginBuild = "";

    if (!pluginBuild.size())
    {
        std::ostringstream pluginBuildStream;

        pluginBuildStream << PLUGIN_NAME << " " << MAJOR << "." << MINOR << "." << REV << " (" << BUILD << ")";
        pluginBuild = pluginBuildStream.str();
    }

    return pluginBuild.c_str();
}

void SpawnAtBase::Init (const char* /*commandLine*/)
{
    bz_registerCustomMapObject("BASESPAWNZONE", this);

    // Register our events with Register()
    Register(bz_eGetPlayerSpawnPosEvent);
}

void SpawnAtBase::Cleanup (void)
{
    Flush(); // Clean up all the events
}

bool SpawnAtBase::MapObject (bz_ApiString object, bz_CustomMapObjectInfo *data)
{
    if (object != "BASESPAWNZONE" || !data)
    {
        return false;
    }

    BaseSpawnZone newZone;
    newZone.handleDefaultOptions(data);

    for (unsigned int i = 0; i < data->data.size(); i++)
    {
        std::string line = data->data.get(i).c_str();

        bz_APIStringList *nubs = bz_newStringList();
        nubs->tokenize(line.c_str(), " ", 0, true);

        if (nubs->size() > 0)
        {
            std::string key = bz_toupper(nubs->get(0).c_str());

            if (key == "COLOR" && nubs->size() == 2)
            {
                newZone.team = (bz_eTeamType)atoi(nubs->get(1).c_str());
            }
        }

        bz_deleteStringList(nubs);
    }

    TeamZones[newZone.team].push_back(newZone);

    return true;
}

void SpawnAtBase::Event (bz_EventData *eventData)
{
    switch (eventData->eventType)
    {
        case bz_eGetPlayerSpawnPosEvent: // This event is called each time the server needs a new spawn postion
        {
            bz_GetPlayerSpawnPosEventData_V1* spawnData = (bz_GetPlayerSpawnPosEventData_V1*)eventData;

            if (bz_getPlayerSpawnAtBase(spawnData->playerID) && !TeamZones[spawnData->team].empty())
            {
                spawnData->handled = true;

                float spawnLocation[3];

                BaseSpawnZone zone = *select_randomly(TeamZones[spawnData->team].begin(), TeamZones[spawnData->team].end());
                bz_getRandomPoint((bz_CustomZoneObject*)&zone, spawnLocation);

                spawnData->pos[0] = spawnLocation[0];
                spawnData->pos[1] = spawnLocation[1];
                spawnData->pos[2] = spawnLocation[2];
            }
        }
        break;

        default: break;
    }
}
