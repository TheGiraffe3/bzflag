/*
 * Copyright (C) 2021 Vladimir "allejo" Jimenez, with changes from "GEP"
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

#include "bzfsAPI.h"

class CascadeFlag : public bz_Plugin
{
    virtual const char* Name();
    virtual void Init(const char* config);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);
};

BZ_PLUGIN(CascadeFlag)

const char* CascadeFlag::Name()
{
    return "CascadeFlag";
}

void CascadeFlag::Init(const char* config)
{
    Register(bz_ePlayerDieEvent);

    bz_RegisterCustomFlag("CA", "Cascade", "All of your victims will trigger a shockwave at their death location", 0, eGoodFlag);
}

void CascadeFlag::Cleanup()
{
    Flush();
}

void CascadeFlag::Event(bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
    case bz_ePlayerDieEvent:
    {
        bz_PlayerDieEventData_V1 *data = (bz_PlayerDieEventData_V1*)eventData;

        if (data->flagKilledWith == "CA")
        {
            float vector[3] = { 0, 0, 0 };
            uint32_t shotGUID = bz_fireServerShot("SW", data->state.pos, vector, data->killerTeam);

            bz_setShotMetaData(shotGUID, "type", data->flagKilledWith.c_str());
            bz_setShotMetaData(shotGUID, "owner", data->killerID);

            return;
        }

        uint32_t shotGUID = bz_getShotGUID(data->killerID, data->shotID);

        if (bz_shotHasMetaData(shotGUID, "type") && bz_shotHasMetaData(shotGUID, "owner"))
        {
            std::string flagType = bz_getShotMetaDataS(shotGUID, "type");

            if (flagType == "CA")
            {
                data->killerID = bz_getShotMetaDataI(shotGUID, "owner");

                float vector[3] = { 0, 0, 0 };//Portion below added by GEP
                uint32_t shotsGUID = bz_fireServerShot("SW", data->state.pos, vector, bz_getPlayerTeam(bz_getShotMetaDataI(shotGUID, "owner")));

                bz_setShotMetaData(shotsGUID, "type", bz_getShotMetaDataS(shotGUID, "type"));
                bz_setShotMetaData(shotsGUID, "owner", bz_getShotMetaDataI(shotGUID, "owner"));
            }
        }
    }
    break;

    default: break;
    }
}
