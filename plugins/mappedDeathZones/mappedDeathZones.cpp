/*
 * Copyright (C) 2020 Vladimir "allejo" Jimenez
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <algorithm>
#include <map>
#include <random>
#include <set>

#include "bzfsAPI.h"
#include "plugin_utils.h"

// Define plug-in name
const std::string PLUGIN_NAME = "Mapped Death Zones";

// Define plug-in version numbering
const int MAJOR = 1;
const int MINOR = 0;
const int REV = 2;
const int BUILD = 22;
const std::string SUFFIX = "DEV";

// Define build settings
const int VERBOSITY_LEVEL = 4;

class DeathZone : public bz_CustomZoneObject
{
public:
    DeathZone() : bz_CustomZoneObject()
    {
    }

    std::string name;
    std::set<bz_eTeamType> affectedTeams;
    std::set<std::string> spawnZones;

    bool doesAffectTeam(bz_eTeamType team)
    {
        if (affectedTeams.size() == 0)
        {
            return true;
        }

        return affectedTeams.find(team) != affectedTeams.end();
    }

    std::string getRandomSpawnZone()
    {
        if (spawnZones.size() == 0)
        {
            return "";
        }

        int target = rand() % spawnZones.size();
        auto it = spawnZones.begin();

        std::advance(it, target);

        return *it;
    }
};

class SpawnZone : public bz_CustomZoneObject
{
public:
    SpawnZone() : bz_CustomZoneObject()
    {
    }

    std::string name;
};

class MappedDeathZones : public bz_Plugin, public bz_CustomMapObjectHandler
{
public:
    virtual const char* Name();
    virtual void Init(const char* config);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);
    virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data);

private:
    std::map<std::string, DeathZone> deathZones;
    std::map<std::string, SpawnZone> spawnZones;

    std::map<int, std::string> nextSpawnZone;
};

BZ_PLUGIN(MappedDeathZones)

const char* MappedDeathZones::Name()
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

void MappedDeathZones::Init(const char* /*config*/)
{
    Register(bz_eGetPlayerSpawnPosEvent);
    Register(bz_ePlayerDieEvent);
    Register(bz_eWorldFinalized);

    bz_registerCustomMapObject("DEATHZONE", this);
    bz_registerCustomMapObject("SPAWNZONE", this);
}

void MappedDeathZones::Cleanup()
{
    Flush();

    bz_removeCustomMapObject("DEATHZONE");
    bz_removeCustomMapObject("SPAWNZONE");
}

void MappedDeathZones::Event(bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_eGetPlayerSpawnPosEvent:
        {
            bz_GetPlayerSpawnPosEventData_V1* data = (bz_GetPlayerSpawnPosEventData_V1*)eventData;

            if (nextSpawnZone.find(data->playerID) != nextSpawnZone.end())
            {
                std::string target = nextSpawnZone[data->playerID];

                if (target.size() == 0)
                {
                    bz_debugMessagef(
                        VERBOSITY_LEVEL,
                        "WARNING :: Mapped Death Zones :: Player '%s' set to spawn at spawnzone with no name. Bailing out.",
                        bz_getPlayerCallsign(data->playerID)
                    );

                    return;
                }

                if (spawnZones.find(target) == spawnZones.end())
                {
                    bz_debugMessagef(
                        VERBOSITY_LEVEL,
                        "WARNING :: Mapped Death Zones :: Player '%s' set to spawn at nonexistent spawnzone: %s. Bailing out.",
                        target.c_str()
                    );

                    return;
                }

                float spawnPos[3];
                bool couldFindPos = bz_getSpawnPointWithin(&spawnZones[target], spawnPos);

                if (couldFindPos)
                {
                    data->handled = true;
                    data->pos[0] = spawnPos[0];
                    data->pos[1] = spawnPos[1];
                    data->pos[2] = spawnPos[2];
                }
                else
                {
                    bz_debugMessagef(
                        0,
                        "ERROR :: Mapped Death Zones :: A safe spawn position could not be found in spawnzone: %s. Bailing out.",
                        target.c_str()
                    );
                }

                nextSpawnZone.erase(data->playerID);
            }
        }
        break;

        case bz_ePlayerDieEvent:
        {
            // This event is called each time a tank is killed.
            bz_PlayerDieEventData_V2* data = (bz_PlayerDieEventData_V2*)eventData;


            for (auto &deathZone : deathZones)
            {
                if (
                    deathZone.second.doesAffectTeam(data->team) &&
                    deathZone.second.pointInZone(data->state.pos)
                ) {
                    bz_debugMessagef(
                        VERBOSITY_LEVEL,
                        "DEBUG :: Mapped Death Zones :: %s team player '%s' (#%d) died in '%s' deathzone.",
                        bzu_GetTeamName(data->team),
                        bz_getPlayerCallsign(data->playerID),
                        data->playerID,
                        deathZone.first.c_str()
                    );

                    nextSpawnZone[data->playerID] = deathZone.second.getRandomSpawnZone();

                    break;
                }
            }
        }
        break;

        case bz_eWorldFinalized:
        {
            std::set<std::string> linkedSpawnZones;

            for (auto &deathZone : deathZones)
            {
                for (auto name : deathZone.second.spawnZones)
                {
                    linkedSpawnZones.insert(name);

                    if (spawnZones.find(name) == spawnZones.end())
                    {
                        bz_debugMessagef(0, "ERROR :: Mapped Death Zones :: There is no SPAWNZONE object with the name: %s", name.c_str());
                    }
                }
            }

            for (auto &spawnZone : spawnZones)
            {
                if (linkedSpawnZones.find(spawnZone.first) == linkedSpawnZones.end())
                {
                    bz_debugMessagef(0, "WARNING :: Mapped Death Zones :: Orphaned SPAWNZONE object with name: %s", spawnZone.first.c_str());
                }
            }
        }
        break;

        default:
            break;
    }
}

bool MappedDeathZones::MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data)
{
    // Note, this value will be in uppercase
    if (!data || (object != "DEATHZONE" && object != "SPAWNZONE"))
    {
        return false;
    }

    if (object == "DEATHZONE")
    {
        DeathZone deathZone;
        deathZone.handleDefaultOptions(data);

        for (unsigned int i = 0; i < data->data.size(); i++)
        {
            std::string line = data->data.get(i);

            bz_APIStringList nubs;
            nubs.tokenize(line.c_str(), " ", 0, true);

            if (nubs.size() > 0)
            {
                std::string key = bz_toupper(nubs.get(0).c_str());

                if (key == "TEAM")
                {
                    deathZone.affectedTeams.insert((bz_eTeamType)atoi(nubs.get(1).c_str()));
                }
                else if (key == "SPAWNZONE")
                {
                    deathZone.spawnZones.insert(nubs.get(1).c_str());
                }
                else if (key == "NAME")
                {
                    deathZone.name = nubs.get(1).c_str();
                }
            }
        }

        if (deathZones.find(deathZone.name) != deathZones.end())
        {
            bz_debugMessagef(0, "ERROR :: Mapped Death Zones :: Conflicting DEATHZONE name: %s", deathZone.name.c_str());
        }

        deathZones[deathZone.name] = deathZone;
    }
    else if (object == "SPAWNZONE")
    {
        SpawnZone spawnZone;
        spawnZone.handleDefaultOptions(data);

        for (unsigned int i = 0; i < data->data.size(); i++)
        {
            std::string line = data->data.get(i);

            bz_APIStringList nubs;
            nubs.tokenize(line.c_str(), " ", 0, true);

            if (nubs.size() > 0)
            {
                std::string key = bz_toupper(nubs.get(0).c_str());

                if (key == "NAME")
                {
                    spawnZone.name = nubs.get(1).c_str();
                }
            }
        }

        if (spawnZones.find(spawnZone.name) != spawnZones.end())
        {
            bz_debugMessagef(0, "ERROR :: Mapped Death Zones :: Conflicting SPAWNZONE name: %s", spawnZone.name.c_str());
        }

        spawnZones[spawnZone.name] = spawnZone;
    }

    return true;
}
