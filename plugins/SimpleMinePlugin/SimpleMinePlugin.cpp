#include "bzfsAPI.h"
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#ifndef BZ_API_MAX_PLAYERS
#define BZ_API_MAX_PLAYERS 255
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct MineInfo {
    float pos[3];
    int placerID;
    double activationTime;
    unsigned int mineArrayIndex;
    bz_eTeamType team;
};

class SimpleMinePlugin : public bz_Plugin, public bz_CustomSlashCommandHandlerV2
{
public:
    virtual const char* Name() { return "SimpleMinePlugin"; }
    virtual void Init(const char* config);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);
    virtual bool SlashCommand(int playerID, int sourceChannel, bz_ApiString command, bz_ApiString message, bz_APIStringList* params);

private:
    std::vector<MineInfo> activeMines;
    double mineActivationDelay;
    double mineLifetime;
    float mineRadius;
    bool allowTeamMines;
    static const unsigned int MAX_MINES = 255;
    bool mineActive[MAX_MINES];
    signed int minePlacerID[MAX_MINES];
    float minePositions[MAX_MINES][3];
    bz_eTeamType mineTeam[MAX_MINES];
    double safetyTime[BZ_API_MAX_PLAYERS];
    bool hasUSFlag[BZ_API_MAX_PLAYERS];

    std::string safeBzApiStringToString(const bz_ApiString& apiString, const std::string& defaultVal = "Unknown") {
        const char* cstr = apiString.c_str();
        if (cstr == nullptr || apiString.empty()) {
            return defaultVal;
        }
        return std::string(cstr);
    }

    void placeMine(int playerID, const float pos[3]);
    void detonateMine(int instigatorID, const float explosionPos[3]);
    void checkMines();
    unsigned int getFreeMineIndex();
    void removeMineFromArrays(unsigned int index);
    void removeAllPlayerMines(int playerID);
};

BZ_PLUGIN(SimpleMinePlugin)

void SimpleMinePlugin::Init(const char* config)
{
    (void)config;
    Register(bz_eTickEvent);
    Register(bz_eFlagGrabbedEvent);
    Register(bz_ePlayerPartEvent);
    Register(bz_ePlayerUpdateEvent);
    Register(bz_ePlayerDieEvent);
    Register(bz_ePlayerSpawnEvent);
    Register(bz_eFlagDroppedEvent);
    bz_registerCustomSlashCommand("fm", this);

    mineActivationDelay = bz_getBZDBDouble("simplemine_activation_delay");
    if (mineActivationDelay <= 0 || !std::isfinite(mineActivationDelay)) {
        mineActivationDelay = 3.0;
    }
    mineLifetime = bz_getBZDBDouble("simplemine_lifetime");
    if (mineLifetime < 0 || !std::isfinite(mineLifetime)) {
        mineLifetime = 240.0; // 4 minutes
    }
    mineRadius = bz_getBZDBDouble("simplemine_mine_radius");
    if (mineRadius < 5.0f || !std::isfinite(mineRadius)) {
        mineRadius = 15.0f; // Radius 15 units
    }
    allowTeamMines = bz_getBZDBBool("simplemine_allow_team_damage");

    bz_registerCustomBZDBDouble("simplemine_activation_delay", mineActivationDelay, 0, true);
    bz_registerCustomBZDBDouble("simplemine_lifetime", mineLifetime, 0, true);
    bz_registerCustomBZDBBool("simplemine_allow_team_damage", allowTeamMines, 0, true);
    bz_registerCustomBZDBDouble("simplemine_mine_radius", mineRadius, 0, true);

    for (unsigned int i = 0; i < MAX_MINES; i++) {
        mineActive[i] = false;
        minePlacerID[i] = -1;
        minePositions[i][0] = 0;
        minePositions[i][1] = 0;
        minePositions[i][2] = 0;
        mineTeam[i] = eNoTeam;
    }
    for (int i = 0; i < BZ_API_MAX_PLAYERS; i++) {
        safetyTime[i] = 0.0;
        hasUSFlag[i] = false;
    }
}

void SimpleMinePlugin::Cleanup()
{
    bz_removeCustomSlashCommand("fm");
    activeMines.clear();
    for (unsigned int i = 0; i < MAX_MINES; i++) {
        mineActive[i] = false;
        minePlacerID[i] = -1;
        minePositions[i][0] = 0;
        minePositions[i][1] = 0;
        minePositions[i][2] = 0;
        mineTeam[i] = eNoTeam;
    }
    for (int i = 0; i < BZ_API_MAX_PLAYERS; i++) {
        safetyTime[i] = 0.0;
        hasUSFlag[i] = false;
    }
}

unsigned int SimpleMinePlugin::getFreeMineIndex()
{
    for (unsigned int i = 0; i < MAX_MINES; i++) {
        if (!mineActive[i]) {
            return i;
        }
    }
    return MAX_MINES; // Indica nessun indice libero
}

void SimpleMinePlugin::removeMineFromArrays(unsigned int index)
{
    if (index < MAX_MINES) {
        mineActive[index] = false;
        minePlacerID[index] = -1;
        minePositions[index][0] = 0;
        minePositions[index][1] = 0;
        minePositions[index][2] = 0;
        mineTeam[index] = eNoTeam;
    }
}

void SimpleMinePlugin::removeAllPlayerMines(int playerID)
{
    for (unsigned int i = 0; i < MAX_MINES; i++) {
        if (mineActive[i] && minePlacerID[i] == playerID) {
            removeMineFromArrays(i);
        }
    }
    activeMines.erase(std::remove_if(activeMines.begin(), activeMines.end(),
                                     [playerID](const MineInfo& mine) {
                                         return mine.placerID == playerID;
                                     }),
                      activeMines.end());
}

void SimpleMinePlugin::Event(bz_EventData* eventData)
{
    int playerID = -1;
    bz_BasePlayerRecord *pr = nullptr;
    switch (eventData->eventType) {
        case bz_eTickEvent:
            checkMines();
            break;
        case bz_eFlagGrabbedEvent: {
            bz_FlagGrabbedEventData_V1* grabData = (bz_FlagGrabbedEventData_V1*)eventData;
            playerID = grabData->playerID;
            if (playerID < 0 || playerID >= BZ_API_MAX_PLAYERS) {
                break;
            }
            pr = bz_getPlayerByIndex(playerID);
            if (!pr) {
                break;
            }
            std::string flagType = safeBzApiStringToString(grabData->flagType);
            if (flagType.find("US") != std::string::npos) {
                hasUSFlag[playerID] = true;
                bz_sendTextMessage(BZ_SERVER, playerID, "Type /fm to place a mine. You are immune to its explosion");
            }
            bz_freePlayerRecord(pr);
            break;
        }
        case bz_eFlagDroppedEvent: {
            bz_FlagDroppedEventData_V1* dropData = (bz_FlagDroppedEventData_V1*)eventData;
            playerID = dropData->playerID;
            if (playerID >= 0 && playerID < BZ_API_MAX_PLAYERS) {
                hasUSFlag[playerID] = false;
            }
            break;
        }
        case bz_ePlayerPartEvent: {
            playerID = ((bz_PlayerJoinPartEventData_V1*)eventData)->playerID;
            if (playerID >= 0 && playerID < BZ_API_MAX_PLAYERS) {
                removeAllPlayerMines(playerID);
                hasUSFlag[playerID] = false;
            }
            break;
        }
        case bz_ePlayerUpdateEvent: {
            bz_PlayerUpdateEventData_V1* updateData = (bz_PlayerUpdateEventData_V1*)eventData;
            playerID = updateData->playerID;
            if (playerID < 0 || playerID >= BZ_API_MAX_PLAYERS) {
                break;
            }
            pr = bz_getPlayerByIndex(playerID);
            if (pr && pr->spawned) {
                float playerPos[3];
                memcpy(playerPos, pr->lastKnownState.pos, sizeof(float) * 3);
                bz_eTeamType playerTeam = pr->team;
                bool playerSafetyTimePassed = safetyTime[playerID] <= bz_getCurrentTime();
                
                // Ottimizzazione: crea una copia per evitare modifiche durante l'iterazione
                std::vector<unsigned int> minesToRemove;
                std::vector<MineInfo> activeMinesCopy = activeMines;
                
                for (size_t j = 0; j < activeMinesCopy.size(); ++j) {
                    const MineInfo& currentMine = activeMinesCopy[j];
                    unsigned int arrayIndex = currentMine.mineArrayIndex;
                    
                    if (arrayIndex >= MAX_MINES || !mineActive[arrayIndex]) {
                        minesToRemove.push_back(arrayIndex);
                        continue;
                    }
                    
                    if (playerID == currentMine.placerID) {
                        continue;
                    }
                    
                    if (!allowTeamMines && playerTeam == mineTeam[arrayIndex] && playerTeam != eRogueTeam) {
                        continue;
                    }
                    
                    float dx = playerPos[0] - minePositions[arrayIndex][0];
                    float dy = playerPos[1] - minePositions[arrayIndex][1];
                    float distanceSq = (dx * dx) + (dy * dy);
                    bool inRange = distanceSq <= (mineRadius * mineRadius);
                    bool mineActivatedReady = bz_getCurrentTime() >= currentMine.activationTime;
                    
                    if (inRange && playerSafetyTimePassed && mineActivatedReady) {
                        detonateMine(currentMine.placerID, minePositions[arrayIndex]);
                        minesToRemove.push_back(arrayIndex);
                        break; // Solo una mina per frame
                    }
                }
                
                // Rimuovi le mine dopo l'iterazione
                for (unsigned int index : minesToRemove) {
                    removeMineFromArrays(index);
                    activeMines.erase(std::remove_if(activeMines.begin(), activeMines.end(),
                                                   [index](const MineInfo& mine) {
                                                       return mine.mineArrayIndex == index;
                                                   }),
                                    activeMines.end());
                }
                
                bz_freePlayerRecord(pr);
            }
            break;
        }
        case bz_ePlayerDieEvent: {
            bz_PlayerDieEventData_V1* dieData = (bz_PlayerDieEventData_V1*)eventData;
            playerID = dieData->playerID;
            if (playerID < 0 || playerID >= BZ_API_MAX_PLAYERS) {
                break;
            }
            float victimPos[3];
            pr = bz_getPlayerByIndex(playerID);
            if (pr) {
                memcpy(victimPos, pr->lastKnownState.pos, sizeof(float) * 3);
                bz_freePlayerRecord(pr);
            } else {
                break;
            }
            const float messageCorrelationRadius = 15.0f;
            for (size_t i = 0; i < activeMines.size(); ++i) {
                MineInfo& mine = activeMines[i];
                float dx = victimPos[0] - mine.pos[0];
                float dy = victimPos[1] - mine.pos[1];
                float distanceSq = (dx * dx) + (dy * dy);
                if (dieData->killerID == mine.placerID && distanceSq <= (messageCorrelationRadius * messageCorrelationRadius)) {
                    int actualKillerID = mine.placerID;
                    std::string victimCallsignStr = safeBzApiStringToString(bz_getPlayerCallsign(playerID));
                    std::string killerCallsignStr = safeBzApiStringToString(bz_getPlayerCallsign(actualKillerID));
                    std::string killMsg;
                    switch (rand() % 6) {
                        case 0: killMsg = victimCallsignStr + " was killed by " + killerCallsignStr + "'s mine."; break;
                        case 1: killMsg = victimCallsignStr + " was destroyed by " + killerCallsignStr + "'s mine."; break;
                        case 2: killMsg = killerCallsignStr + "'s mine disintegrated " + victimCallsignStr + "'s tank."; break;
                        case 3: killMsg = victimCallsignStr + " was eliminated by " + killerCallsignStr + "'s mine."; break;
                        case 4: killMsg = victimCallsignStr + " exploded due to " + killerCallsignStr + "'s mine."; break;
                        case 5: killMsg = "A mine from " + killerCallsignStr + " neutralized " + victimCallsignStr + "."; break;
                    }
                    bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, killMsg.c_str());
                    break;
                }
            }
            break;
        }
        case bz_ePlayerSpawnEvent: {
            playerID = ((bz_PlayerSpawnEventData_V1*)eventData)->playerID;
            if (playerID >= 0 && playerID < BZ_API_MAX_PLAYERS) {
                safetyTime[playerID] = bz_getCurrentTime() + mineActivationDelay;
                hasUSFlag[playerID] = false;
            }
            break;
        }
        default:
            break;
    }
}

bool SimpleMinePlugin::SlashCommand(int playerID, int sourceChannel, bz_ApiString command, bz_ApiString message, bz_APIStringList* params)
{
    (void)sourceChannel; (void)message; (void)params;
    if (command == "fm") {
        if (playerID < 0 || playerID >= BZ_API_MAX_PLAYERS) {
            return true;
        }
        bz_BasePlayerRecord* player = bz_getPlayerByIndex(playerID);
        if (!player) {
            bz_sendTextMessage(BZ_SERVER, playerID, "Could not place mine: Player record missing.");
            return true;
        }
        if (!player->spawned) {
            bz_sendTextMessage(BZ_SERVER, playerID, "You must be spawned to place a Mine.");
            bz_freePlayerRecord(player);
            return true;
        }
        if (!hasUSFlag[playerID]) {
            bz_sendTextMessage(BZ_SERVER, playerID, "You need the 'Useless' flag to place a Mine.");
            bz_freePlayerRecord(player);
            return true;
        }
        float minePos[3];
        memcpy(minePos, player->lastKnownState.pos, sizeof(float) * 3);
        minePos[2] = 0.0f;
        std::string playerCallsign = safeBzApiStringToString(player->callsign);
        
        // CORRETTO: Rimossa la duplicazione del messaggio
        bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "%s has placed a Mine!", playerCallsign.c_str());
        
        bz_removePlayerFlag(playerID);
        bz_sendTextMessage(BZ_SERVER, playerID, "Your 'Useless' flag has been removed");
        hasUSFlag[playerID] = false;
        placeMine(playerID, minePos);
        bz_freePlayerRecord(player);
        return true;
    }
    return false;
}

void SimpleMinePlugin::placeMine(int playerID, const float pos[3])
{
    unsigned int freeIndex = getFreeMineIndex();
    if (freeIndex == MAX_MINES) {
        bz_sendTextMessage(BZ_SERVER, playerID, "Cannot place Mine: Maximum number of mines reached");
        return;
    }
    MineInfo newMine;
    newMine.placerID = playerID;
    memcpy(newMine.pos, pos, sizeof(float) * 3);
    newMine.activationTime = bz_getCurrentTime() + mineActivationDelay;
    newMine.mineArrayIndex = freeIndex;
    newMine.team = eNoTeam;
    mineActive[freeIndex] = true;
    minePlacerID[freeIndex] = playerID;
    memcpy(minePositions[freeIndex], pos, sizeof(float) * 3);
    bz_BasePlayerRecord *pr = bz_getPlayerByIndex(playerID);
    if (pr) {
        newMine.team = pr->team;
        mineTeam[freeIndex] = pr->team;
        bz_freePlayerRecord(pr);
    }
    activeMines.push_back(newMine);
}

void SimpleMinePlugin::detonateMine(int instigatorID, const float explosionPos[3])
{
    const float localExplosionKillRadius = 15.0f;
    int playersKilled = 0;
    std::string instigatorCallsign = safeBzApiStringToString(bz_getPlayerCallsign(instigatorID));
    bz_eTeamType instigatorTeam = eRogueTeam;
    bz_BasePlayerRecord *instigatorPr = bz_getPlayerByIndex(instigatorID);
    if (instigatorPr) {
        instigatorTeam = instigatorPr->team;
        bz_freePlayerRecord(instigatorPr);
    }

    for (int targetID = 0; targetID < BZ_API_MAX_PLAYERS; ++targetID) {
        bz_BasePlayerRecord *targetRecord = bz_getPlayerByIndex(targetID);
        if (!targetRecord) {
            continue;
        }
        
        bool shouldKill = false;
        if (targetRecord->spawned) {
            float dx = targetRecord->lastKnownState.pos[0] - explosionPos[0];
            float dy = targetRecord->lastKnownState.pos[1] - explosionPos[1];
            float distanceSq = (dx * dx) + (dy * dy);
            if (distanceSq <= (localExplosionKillRadius * localExplosionKillRadius)) {
                if (targetID == instigatorID) {
                    bz_freePlayerRecord(targetRecord);
                    continue;
                }
                if (safetyTime[targetID] > bz_getCurrentTime()) {
                    bz_freePlayerRecord(targetRecord);
                    continue;
                }
                bz_eTeamType minePlacerTeam = instigatorTeam;
                bool isRogueInstigator = (minePlacerTeam == eRogueTeam);
                bool isRogueTarget = (targetRecord->team == eRogueTeam);
                bool isSameTeam = (targetRecord->team == minePlacerTeam && !isRogueInstigator && !isRogueTarget);
                if (!allowTeamMines && isSameTeam) {
                    bz_freePlayerRecord(targetRecord);
                    continue;
                }
                shouldKill = true;
            }
        }
        
        if (shouldKill) {
            bz_killPlayer(targetID, instigatorID, 0);
            bz_incrementPlayerWins(instigatorID, 1);
            playersKilled++;
        }
        bz_freePlayerRecord(targetRecord); // CORRETTO: Sempre liberato
    }
    
    if (playersKilled > 0) {
        std::string detonationMsg = instigatorCallsign + " has unleashed a mine explosion, affecting " +
                                   std::to_string(playersKilled) + " nearby tank(s)!";
        bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, detonationMsg.c_str());
    }
}

void SimpleMinePlugin::checkMines()
{
    double currentTime = bz_getCurrentTime();
    for (size_t i = activeMines.size(); i-- > 0; ) {
        MineInfo& mine = activeMines[i];
        unsigned int currentMineArrayIndex = mine.mineArrayIndex;
        if (currentMineArrayIndex >= MAX_MINES || !mineActive[currentMineArrayIndex] || minePlacerID[currentMineArrayIndex] != mine.placerID) {
            activeMines.erase(activeMines.begin() + i);
            removeMineFromArrays(currentMineArrayIndex);
            continue;
        }
        if (mineLifetime > 0 && (currentTime - mine.activationTime) > mineLifetime) {
            std::string placerCallsign = safeBzApiStringToString(bz_getPlayerCallsign(mine.placerID));
            // CORRETTO: Usa mineLifetime invece del valore hardcodato
            int lifetimeMinutes = static_cast<int>(mineLifetime / 60);
            std::string expiryMsg = placerCallsign + "'s Mine at (" +
                                    std::to_string((int)mine.pos[0]) + ", " +
                                    std::to_string((int)mine.pos[1]) + ") has expired after " +
                                    std::to_string(lifetimeMinutes) + " minutes";
            bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, expiryMsg.c_str());
            removeMineFromArrays(currentMineArrayIndex);
            activeMines.erase(activeMines.begin() + i);
            continue;
        }
    }
}
