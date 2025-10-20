#include "bzfsAPI.h"

class SneakPlugin : public bz_Plugin, public bz_CustomSlashCommandHandler
{
    virtual const char* Name ();
    virtual void Init (const char*);
    virtual void Cleanup ();
    virtual void Event (bz_EventData* eventData);
    virtual bool SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);

    int SneakPluginAttack[256];
};

BZ_PLUGIN(SneakPlugin)

const char* SneakPlugin::Name ()
{
    return "Sneak Plugin";
}

void SneakPlugin::Init (const char*)
{
    Register(bz_eGetPlayerSpawnPosEvent);
    Register(bz_ePlayerPartEvent);

    bz_registerCustomSlashCommand("sneak", this);

    // Range-based for loop syntax; a C++11 feature
    for (int &i : SneakPluginAttack)
    {
        i = -1;
    }
}

void SneakPlugin::Cleanup ()
{
    Flush();

    bz_removeCustomSlashCommand("sneak");
}

void SneakPlugin::Event (bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_eGetPlayerSpawnPosEvent:
        {
            bz_GetPlayerSpawnPosEventData_V1 *data = (bz_GetPlayerSpawnPosEventData_V1*)eventData;

            if (SneakPluginAttack[data->playerID] >= 0)
            {
                bz_BasePlayerRecord *pr = bz_getPlayerByIndex(SneakPluginAttack[data->playerID]);

                if (!pr)
                {
                    bz_sendTextMessage(BZ_SERVER, data->playerID, "Your sneak attack failed! Victim not found.");
                    return;
                }

                data->handled = true;
                data->pos[0] = pr->lastKnownState.pos[0];
                data->pos[1] = pr->lastKnownState.pos[1];
                data->pos[2] = pr->lastKnownState.pos[2];
                data->rot = pr->lastKnownState.rotation;

                bz_freePlayerRecord(pr);

                SneakPluginAttack[data->playerID] = -1;
            }
        }
        break;

        case bz_ePlayerPartEvent:
        {
            bz_PlayerJoinPartEventData_V1 *data = (bz_PlayerJoinPartEventData_V1*)eventData;

            SneakPluginAttack[data->playerID] = -1;
        }
        break;

        default: break;
    }
}

bool SneakPlugin::SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
    if (command == "sneakplugin")
    {
        if (params->size() != 1)
        {
            bz_sendTextMessage(BZ_SERVER, playerID, "/sneakplugin <player ID|callsign>");
            return true;
        }

        const char* victimID = params->get(0).c_str();

        bz_BasePlayerRecord *pr = bz_getPlayerBySlotOrCallsign(victimID);

        if (!pr)
        {
            bz_sendTextMessagef(BZ_SERVER, playerID, "player \"%s\" not found", victimID);
        }
        else
        {
            SneakPluginAttack[playerID] = pr->playerID;
            bz_freePlayerRecord(pr);
        }

        return true;
    }

    return false;
}
