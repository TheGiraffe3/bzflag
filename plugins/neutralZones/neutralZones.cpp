/*
 * Copyright (C) 2022 Vladimir "allejo" Jimenez
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

// Define plug-in name
const std::string PLUGIN_NAME = "Neutral Zones";

// Define plug-in version numbering
const int MAJOR = 1;
const int MINOR = 0;
const int REV = 1;
const int BUILD = 7;
const std::string SUFFIX = "";

// Define build settings
const int VERBOSITY_LEVEL = 4;

class NeutralZone : public bz_CustomZoneObject
{
public:
    NeutralZone() : bz_CustomZoneObject()
    {
    }

    bool disallowAllFlags = false;
    std::vector<bz_eTeamType> teams;
    std::vector<bz_ApiString> flags;

    bool hasDisallowedFlag(const int flagID)
    {
        if (disallowAllFlags)
        {
            return true;
        }

        auto flag = bz_getFlagName(flagID);

        return std::find(flags.begin(), flags.end(), flag) != flags.end();
    }

    bool targetsTeam(bz_eTeamType team)
    {
        // If there's no targeted teams, then affect all teams
        return teams.size() == 0 || std::find(teams.begin(), teams.end(), team) != teams.end();
    }
};

class NeutralZones : public bz_Plugin, public bz_CustomMapObjectHandler
{
public:
    virtual const char* Name();
    virtual void Init(const char* config);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);
    virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data);

private:
    std::vector<NeutralZone> neutralZones;
};

BZ_PLUGIN(NeutralZones)

const char* NeutralZones::Name()
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

void NeutralZones::Init(const char* /* config */)
{
    Register(bz_ePlayerUpdateEvent);

    bz_registerCustomMapObject("NEUTRALZONE", this);
}

void NeutralZones::Cleanup()
{
    Flush();

    bz_removeCustomMapObject("NEUTRALZONE");
}

void NeutralZones::Event(bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_ePlayerUpdateEvent:
        {
            bz_PlayerUpdateEventData_V1* data = (bz_PlayerUpdateEventData_V1*)eventData;
            bz_BasePlayerRecord* pr = bz_getPlayerByIndex(data->playerID);

            // If a player doesn't have a flag, then there's no need to do anything
            if (pr->currentFlagID < 0)
            {
                bz_freePlayerRecord(pr);
                return;
            }

            for (auto &zone : neutralZones)
            {
                if (zone.targetsTeam(pr->team) && zone.hasDisallowedFlag(pr->currentFlagID) && zone.pointInZone(data->state.pos))
                {
                    bz_removePlayerFlag(data->playerID);
                    
                    auto abbr = bz_getFlagName(pr->currentFlagID);
                    auto isTeamFlag = (abbr == "R*" || abbr == "G*" || abbr == "B*" || abbr == "P*");
                    
                    if (isTeamFlag)
                    {
                        float pos[3];

                        if (bz_getNearestFlagSafetyZone(pr->currentFlagID, pos))
                        {
                            bz_moveFlag(pr->currentFlagID, pos);
                        }
                        else
                        {
                            // If there is no safety zone nearby, then reset it to the team base
                            bz_resetFlag(pr->currentFlagID);
                        }
                    }
                    
                    bz_sendTextMessagef(BZ_SERVER, data->playerID, "You've entered a neutral zone, your %s has been removed.", pr->currentFlag.c_str());

                    break;
                }
            }

            bz_freePlayerRecord(pr);
        }
        break;

        default:
            break;
    }
}

bool NeutralZones::MapObject(bz_ApiString object, bz_CustomMapObjectInfo* data)
{
    // Note, this value will be in uppercase
    if (!data || object != "NEUTRALZONE")
    {
        return false;
    }

    NeutralZone neutralZone;
    neutralZone.handleDefaultOptions(data);

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
                neutralZone.teams.push_back((bz_eTeamType)atoi(nubs->get(1).c_str()));
            }
            else if (key == "FLAG")
            {
                auto abbv = nubs->get(1);

                if (abbv == "*") {
                    neutralZone.disallowAllFlags = true;
                } else {
                    neutralZone.flags.push_back(abbv);
                }
            }
        }
    }

    bz_deleteStringList(nubs);

    neutralZones.push_back(neutralZone);

    return true;
}
