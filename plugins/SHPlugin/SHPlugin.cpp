// SHPlugin v.1 2025 - Autore tankatek
// Rilasciato sotto licenza LGPL
// LGPL: https://www.gnu.org/licenses/lgpl-3.0.txt
// DISCALMER: https://drive.google.com/file/d/1lS3ovFUy_bUqHBtNLuwVhuCuXOy9WPfM/view

#include "bzfsAPI.h"
#include <string>
#include <map>
#include <set>
#include <mutex>
#include <cstring>
#include <cstdlib>

const std::string NOME_BANDIERA_SH = "SH";

class SHPlugin : public bz_Plugin
{
public:
    const char* Name() override { return "SHPlugin"; }

    void Init(const char* /*config*/) override
    {
        Register(bz_eFlagGrabbedEvent);
        Register(bz_eFlagDroppedEvent);
        Register(bz_ePlayerDieEvent);
        Register(bz_ePlayerPartEvent);
        Register(bz_ePlayerSpawnEvent);
        Register(bz_ePlayerJoinEvent);
    }

    void Cleanup() override
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        hasReceivedSecondSH.clear();
        activePlayers.clear();
        givingInProgress.clear();
    }

    void Event(bz_EventData* eventData) override
    {
        if (!eventData)
            return;

        switch (eventData->eventType)
        {
            case bz_eFlagGrabbedEvent:
                HandleFlagGrabbed(static_cast<bz_FlagGrabbedEventData_V1*>(eventData));
                break;
            case bz_eFlagDroppedEvent:
                HandleFlagDropped(static_cast<bz_FlagDroppedEventData_V1*>(eventData));
                break;
            case bz_ePlayerDieEvent:
                HandlePlayerDie(static_cast<bz_PlayerDieEventData_V1*>(eventData));
                break;
            case bz_ePlayerPartEvent:
                HandlePlayerPart(static_cast<bz_PlayerJoinPartEventData_V1*>(eventData));
                break;
            case bz_ePlayerSpawnEvent:
                HandlePlayerSpawn(static_cast<bz_PlayerSpawnEventData_V1*>(eventData));
                break;
            case bz_ePlayerJoinEvent:
                HandlePlayerJoin(static_cast<bz_PlayerJoinPartEventData_V1*>(eventData));
                break;
            default:
                break;
        }
    }

private:
    std::map<int, bool> hasReceivedSecondSH;
    std::set<int> activePlayers;
    std::set<int> givingInProgress;
    std::mutex dataMutex;

    bool IsPlayerActive(int playerID)
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        return (activePlayers.find(playerID) != activePlayers.end());
    }

    void TryGiveSecondSH(int playerID)
    {
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            if (givingInProgress.count(playerID) || hasReceivedSecondSH[playerID])
                return;
            givingInProgress.insert(playerID);
        }

        bz_BasePlayerRecord* rec = bz_getPlayerByIndex(playerID);
        if (!rec)
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            givingInProgress.erase(playerID);
            return;
        }
        bz_freePlayerRecord(rec);

        const char* currentFlag = bz_getPlayerFlag(playerID);
        bool hasOtherFlag = (currentFlag != nullptr);
        if (currentFlag)
            free((void*)currentFlag);

        if (hasOtherFlag)
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            givingInProgress.erase(playerID);
            return;
        }

        bool success = bz_givePlayerFlag(playerID, NOME_BANDIERA_SH.c_str(), true);

        {
            std::lock_guard<std::mutex> lock(dataMutex);
            if (success)
                hasReceivedSecondSH[playerID] = true;
            givingInProgress.erase(playerID);
        }
    }

    void HandleFlagGrabbed(bz_FlagGrabbedEventData_V1* data)
    {
        if (!data || !IsPlayerActive(data->playerID))
            return;

        if (strcmp(data->flagType, NOME_BANDIERA_SH.c_str()) == 0)
        {
            {
                std::lock_guard<std::mutex> lock(dataMutex);
                hasReceivedSecondSH[data->playerID] = false;
            }
            bz_sendTextMessage(BZ_SERVER, data->playerID, "Drop SH to get second shield");
        }
    }

    void HandleFlagDropped(bz_FlagDroppedEventData_V1* data)
    {
        if (!data || !IsPlayerActive(data->playerID))
            return;

        if (strcmp(data->flagType, NOME_BANDIERA_SH.c_str()) == 0)
        {
            bool needsSecond = false;
            {
                std::lock_guard<std::mutex> lock(dataMutex);
                needsSecond = !hasReceivedSecondSH[data->playerID];
            }

            if (needsSecond)
                TryGiveSecondSH(data->playerID);
        }
    }

    void HandlePlayerDie(bz_PlayerDieEventData_V1* data)
    {
        if (!data) return;
        std::lock_guard<std::mutex> lock(dataMutex);
        activePlayers.erase(data->playerID);
        hasReceivedSecondSH.erase(data->playerID);
        givingInProgress.erase(data->playerID);
    }

    void HandlePlayerPart(bz_PlayerJoinPartEventData_V1* data)
    {
        if (!data) return;
        std::lock_guard<std::mutex> lock(dataMutex);
        activePlayers.erase(data->playerID);
        hasReceivedSecondSH.erase(data->playerID);
        givingInProgress.erase(data->playerID);
    }

    void HandlePlayerSpawn(bz_PlayerSpawnEventData_V1* data)
    {
        if (!data) return;
        std::lock_guard<std::mutex> lock(dataMutex);
        activePlayers.insert(data->playerID);
        hasReceivedSecondSH.erase(data->playerID);
        givingInProgress.erase(data->playerID);
    }

    void HandlePlayerJoin(bz_PlayerJoinPartEventData_V1* data)
    {
        if (!data) return;
        std::lock_guard<std::mutex> lock(dataMutex);
        activePlayers.insert(data->playerID);
        hasReceivedSecondSH.erase(data->playerID);
        givingInProgress.erase(data->playerID);
    }
};

BZ_PLUGIN(SHPlugin)
