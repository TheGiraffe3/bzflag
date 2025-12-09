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
#include "plugin_utils.h"

// Define plugin name
const std::string PLUGIN_NAME = "Demilitarized Zones";

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

class DemilitarizedZone : public bz_CustomZoneObject
{
public:
    DemilitarizedZone() : bz_CustomZoneObject() {}

    std::vector<bz_eTeamType> teamsAffected;
};

class DemilitarizedZones : public bz_Plugin, public bz_CustomMapObjectHandler
{
public:
    virtual const char* Name();
    virtual void Init(const char* config);
    virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo *data);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);

private:
    std::vector<DemilitarizedZone> registeredDMZs;
};

BZ_PLUGIN(DemilitarizedZones)

const char* DemilitarizedZones::Name()
{
    static std::string pluginName;

    if (pluginName.empty())
    {
        pluginName = bz_format("%s %d.%d.%d (%d)", PLUGIN_NAME.c_str(), MAJOR, MINOR, REV, BUILD);
    }

    return pluginName.c_str();
}

void DemilitarizedZones::Init(const char* /*config*/)
{
    Register(bz_eAllowServerShotFiredEvent);
    Register(bz_eShotFiredEvent);

    bz_registerCustomMapObject("dmz", this);
}

bool DemilitarizedZones::MapObject(bz_ApiString object, bz_CustomMapObjectInfo *data)
{
    // Note, this parameter will be in uppercase
    if (object != "DMZ" || !data)
    {
        return false;
    }

    DemilitarizedZone dmz;
    dmz.handleDefaultOptions(data);

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
                    dmz.teamsAffected.push_back(team);
                }
            }
        }
    }

    registeredDMZs.push_back(dmz);

    return true;
}

void DemilitarizedZones::Cleanup()
{
    Flush();

    bz_removeCustomMapObject("dmz");
}

void DemilitarizedZones::Event(bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_eAllowServerShotFiredEvent:
        {
            bz_AllowServerShotFiredEventData_V1 *data = (bz_AllowServerShotFiredEventData_V1*)eventData;

            for (auto &dmz : registeredDMZs)
            {
                if (!dmz.teamsAffected.empty() && !doesVectorContain<bz_eTeamType>(dmz.teamsAffected, data->team))
                {
                    continue;
                }

                if (dmz.pointInZone(data->pos))
                {
                    data->allow = false;
                }
            }
        }
        break;

        case bz_eShotFiredEvent:
        {
            // This event is called each time a shot is fired
            bz_ShotFiredEventData_V1 *data = (bz_ShotFiredEventData_V1*)eventData;

            bz_eTeamType playerTeam = bz_getPlayerTeam(data->playerID);

            for (auto &dmz : registeredDMZs)
            {
                if (!dmz.teamsAffected.empty() && !doesVectorContain<bz_eTeamType>(dmz.teamsAffected, playerTeam))
                {
                    continue;
                }

                if (dmz.pointInZone(data->pos))
                {
                    data->changed = true;
                    data->type = "PZ";
                }
            }
        }
        break;

        default:
            break;
    }
}
