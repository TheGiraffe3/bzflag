/*
 * Copyright (C) 2018 Vladimir "allejo" Jimenez
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

#include "bzfsAPI.h"

// Define plugin name
const std::string PLUGIN_NAME = "Death Zones";

// Define plugin version numbering
const int MAJOR = 1;
const int MINOR = 0;
const int REV = 1;
const int BUILD = 7;

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

class KillZone : public bz_CustomZoneObject
{
public:
    KillZone() : bz_CustomZoneObject() {}

    std::vector<bz_eTeamType> teamsAffected;
};

class KillZones : public bz_Plugin, public bz_CustomMapObjectHandler
{
public:
    virtual const char* Name();
    virtual void Init(const char* config);
    virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo *data);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);

private:
    std::vector<KillZone> registeredkillZones;
};

BZ_PLUGIN(KillZones)

const char* KillZones::Name()
{
    static std::string pluginName;

    if (pluginName.empty())
    {
        pluginName = bz_format("%s %d.%d.%d (%d)", PLUGIN_NAME.c_str(), MAJOR, MINOR, REV, BUILD);
    }

    return pluginName.c_str();
}

void KillZones::Init(const char* /*config*/)
{
    Register(bz_ePlayerUpdateEvent);

    bz_registerCustomMapObject("KILLZONE", this);
}

bool KillZones::MapObject(bz_ApiString object, bz_CustomMapObjectInfo *data)
{
    // Note, this parameter will be in uppercase
    if (object != "KILLZONE" || !data)
    {
        return false;
    }

    KillZone killZone;
    killZone.handleDefaultOptions(data);

    for (unsigned int i = 0; i < data->data.size(); i++)
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
                bz_eTeamType team = convertTeamType(teamNum);

                if (team != eNoTeam) {
                    killZone.teamsAffected.push_back(team);
                }
            }
        }
    }

    registeredkillZones.push_back(killZone);

    return true;
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
            // This event is called each time a shot is fired
            bz_PlayerUpdateEventData_V1* data = (bz_PlayerUpdateEventData_V1*)eventData;

            bz_eTeamType playerTeam = bz_getPlayerTeam(data->playerID);
            float *pos = data->state.pos;

            for (auto &killZone : registeredkillZones)
            {
                if (!killZone.teamsAffected.empty() && !doesVectorContain<bz_eTeamType>(killZone.teamsAffected, playerTeam))
                {
                    continue;
                }

                if (killZone.pointInZone(pos))
                {
                    bz_killPlayer(data->playerID, false, data->playerID, NULL);
                }
            }
        }
        break;

        default:
            break;
    }
}
