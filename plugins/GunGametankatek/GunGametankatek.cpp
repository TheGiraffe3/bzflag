#include "bzfsAPI.h"
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <ctime>
#include <cstring>

class GunGamePlugin : public bz_Plugin
{
public:
    virtual const char* Name() { return "GunGame4"; }
    virtual void Init(const char* commandLine);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);

private:
    std::map<int, int> playerLevels;
    std::map<int, std::string> playerFlags;
    std::vector<std::vector<std::string>> levelFlags;
    std::map<int, bool> bonusAwardedPerPlayer;
    std::map<int, bool> pendingLevelDownMessage;
    bool isAssigningFlag;

    void loadConfig(const std::string& filename);
    void safeAssignFlagToPlayer(int playerID);
    void levelUp(int playerID);
    void levelDown(int playerID);
    void awardBonus(int playerID);
    void sendQuickGuide(int playerID); 
    std::string trim(const std::string& str);
    bool isValidFlag(const std::string& flag);
    int countActivePlayers();
    bool isPlayerValid(int playerID);
    void giveFlagSafely(int playerID, const std::string& flag);
    int getMaxLevel() { return (int)levelFlags.size(); }
};

bool GunGamePlugin::isPlayerValid(int playerID)
{
    if (playerID < 0) return false;
    bz_BasePlayerRecord* player = bz_getPlayerByIndex(playerID);
    if (!player) return false;
    bool valid = player->spawned;
    bz_freePlayerRecord(player);
    return valid;
}

std::string GunGamePlugin::trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

bool GunGamePlugin::isValidFlag(const std::string& flag)
{
    const char* validFlags[] = {"SB", "CL", "L", "GM", "SW", "V", "SR", "F", "ST", "T",
                                "IB", "N", "QT", "BU", "MG", "ID", "WG", "SH", "A",
                                "OO", "PZ", "MQ", "JM", "NJ"};
    for (size_t i = 0; i < sizeof(validFlags)/sizeof(validFlags[0]); ++i) {
        if (flag == validFlags[i]) return true;
    }
    return false;
}

int GunGamePlugin::countActivePlayers()
{
    int count = 0;
    bz_APIIntList* playerList = bz_getPlayerIndexList();
    if (!playerList) return 0;
    for (unsigned int i = 0; i < playerList->size(); i++) {
        if (isPlayerValid((*playerList)[i])) count++;
    }
    bz_deleteIntList(playerList);
    return count;
}

void GunGamePlugin::sendQuickGuide(int playerID)
{
    const char* guide[] = {
        ">>>>>>>>>>>>>>>> Quick Guide in the console >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",
        ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",
        "GUNGAME4.4 Plugin - Authors: tankatek & Bepi - LGpl License",
        ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",
        "QUICK GUIDE",
        "Tier System",
        "-     Level 0: No flags (penalty).",
        "-  Levels 1-7: Progressive flags assigned automatically.",
        "- Final Bonus: Reaching level 7 earns you bonus points and",
        "               returns you to level 1.",
        ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",
        "-    Level 1: SH or BU are assigned and you have the option to change flags.",
        "- Levels 2-7: Specific flags are assigned automatically.",
        "-    Level 0: No flags are assigned. You cannot take flags.",
        ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",
        "- Protection: Assigned flags cannot be abandoned. Only in Level 1 can you abandon",
        "              the assigned flag and take one of the flags located in certain",
        "              areas of the map.",
        ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",
        "Bonus Point Calculation: The bonus score may vary depending on the number of",
        "                         tanks still alive just as the plugin calculates",
        "                         bonus points.",
        ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>",
        " >>>>>>>>        Quick Guide in the console         >>>>>>>>                       "
    };

    for (const char* line : guide) {
        bz_sendTextMessage(BZ_SERVER, playerID, line);
    }
}

void GunGamePlugin::loadConfig(const std::string& filename)
{
    levelFlags.clear();
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        bz_debugMessage(0, "GunGame4: File non trovato, uso default");
        levelFlags.push_back({"SB"}); levelFlags.push_back({"CL"}); levelFlags.push_back({"L"});
        levelFlags.push_back({"GM"}); levelFlags.push_back({"SW"}); levelFlags.push_back({"V"}); levelFlags.push_back({"SR"});
        return;
    }

    std::string line;
    int lineNumber = 0;
    while (std::getline(file, line)) {
        lineNumber++;
        size_t hashPos = line.find('#');
        if (hashPos != std::string::npos) line = line.substr(0, hashPos);
        
        line = trim(line);
        if (line.empty()) continue;

        std::vector<std::string> flags;
        std::stringstream ss(line);
        std::string flag;
        while (std::getline(ss, flag, ',')) {
            flag = trim(flag);
            std::transform(flag.begin(), flag.end(), flag.begin(), ::toupper);
            if (!flag.empty()) {
                if (!isValidFlag(flag)) {
                    bz_debugMessagef(0, "GunGame4: Bandiera '%s' alla riga %d non valida!", flag.c_str(), lineNumber);
                } else {
                    flags.push_back(flag);
                }
            }
        }
        if (!flags.empty()) levelFlags.push_back(flags);
    }
    file.close();

    if (levelFlags.empty()) {
        bz_debugMessage(0, "GunGame4: Config vuota, uso default");
        levelFlags.push_back({"SB"}); levelFlags.push_back({"CL"}); levelFlags.push_back({"L"});
        levelFlags.push_back({"GM"}); levelFlags.push_back({"SW"}); levelFlags.push_back({"V"}); levelFlags.push_back({"SR"});
    }
    bz_debugMessagef(0, "GunGame4: Caricati %d livelli.", (int)levelFlags.size());
}

void GunGamePlugin::Init(const char* commandLine)
{
    Register(bz_ePlayerJoinEvent);
    Register(bz_ePlayerPartEvent);
    Register(bz_ePlayerDieEvent);
    Register(bz_ePlayerSpawnEvent);
    Register(bz_eFlagGrabbedEvent);
    Register(bz_eFlagDroppedEvent);
    Register(bz_eAllowFlagGrab);

    srand((unsigned int)time(NULL));

    std::string configFile = "GunGame4.cfg";
    if (commandLine && *commandLine) configFile = commandLine;

    loadConfig(configFile);
    bz_setShotMismatch(false);
    isAssigningFlag = false;
    bz_debugMessagef(0, "GunGame4 plugin caricato. Max livello: %d", getMaxLevel());
}

void GunGamePlugin::Cleanup()
{
    Flush();
    playerLevels.clear();
    playerFlags.clear();
    levelFlags.clear();
    bonusAwardedPerPlayer.clear();
    pendingLevelDownMessage.clear();
}

void GunGamePlugin::awardBonus(int playerID)
{
    if (bonusAwardedPerPlayer[playerID]) return;

    int activePlayers = countActivePlayers();
    int bonusPoints = activePlayers + 1;

    bz_BasePlayerRecord* player = bz_getPlayerByIndex(playerID);
    if (!player) return;

    bz_incrementPlayerWins(playerID, bonusPoints);
    bonusAwardedPerPlayer[playerID] = true;
    playerLevels[playerID] = 1;
    safeAssignFlagToPlayer(playerID);
    bonusAwardedPerPlayer[playerID] = false;

    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS,
                        "%s has reached the final level and receives %d bonus points!",
                        player->callsign.c_str(), bonusPoints);
    bz_freePlayerRecord(player);
}

void GunGamePlugin::giveFlagSafely(int playerID, const std::string& flag)
{
    if (!isPlayerValid(playerID)) return;
    isAssigningFlag = true;
    bz_givePlayerFlag(playerID, flag.c_str(), true);
    isAssigningFlag = false;
}

void GunGamePlugin::safeAssignFlagToPlayer(int playerID)
{
    if (!isPlayerValid(playerID) || isAssigningFlag) return;

    isAssigningFlag = true;
    if (playerLevels.find(playerID) == playerLevels.end()) playerLevels[playerID] = 1;

    int level = playerLevels[playerID];
    int maxLevel = getMaxLevel();

    if (level > maxLevel) {
        awardBonus(playerID);
        isAssigningFlag = false;
        return;
    }

    if (level == 0) {
        bz_removePlayerFlag(playerID);
        playerFlags[playerID] = "";
        isAssigningFlag = false;
        return;
    }

    if (level >= 1 && level <= maxLevel) {
        if (level - 1 < 0 || level - 1 >= (int)levelFlags.size() || levelFlags.empty()) {
            isAssigningFlag = false;
            return;
        }
        const auto& availableFlags = levelFlags[level - 1];
        if (availableFlags.empty()) {
            isAssigningFlag = false;
            return;
        }
        int randomIndex = rand() % availableFlags.size();
        std::string selectedFlag = availableFlags[randomIndex];
        playerFlags[playerID] = selectedFlag;
        giveFlagSafely(playerID, selectedFlag);
        
        bz_sendTextMessagef(BZ_SERVER, playerID, "Level %d/%d: %s", level, maxLevel, selectedFlag.c_str());
    }
    isAssigningFlag = false;
}

void GunGamePlugin::levelUp(int playerID)
{
    if (playerLevels.find(playerID) == playerLevels.end()) playerLevels[playerID] = 1;

    int oldLevel = playerLevels[playerID];
    int maxLevel = getMaxLevel();

    if (oldLevel >= maxLevel) {
        awardBonus(playerID);
        return;
    }

    playerLevels[playerID]++;

    if (oldLevel == 0) {
        bz_sendTextMessagef(BZ_SERVER, playerID, "You've reached level 1! You can now use flags");
    } else {
        bz_sendTextMessagef(BZ_SERVER, playerID, "You've reached level %d/%d!", playerLevels[playerID], maxLevel);
    }

    safeAssignFlagToPlayer(playerID);
}

void GunGamePlugin::levelDown(int playerID)
{
    if (playerLevels.find(playerID) == playerLevels.end()) playerLevels[playerID] = 1;

    if (playerLevels[playerID] > 0) {
        playerLevels[playerID]--;
        pendingLevelDownMessage[playerID] = true;
        bz_debugMessagef(0, "GunGame4: Player %d level down %d -> %d",
                         playerID, playerLevels[playerID] + 1, playerLevels[playerID]);
    } else {
        bz_debugMessagef(0, "GunGame4: Player %d remained at level 0", playerID);
    }
}

void GunGamePlugin::Event(bz_EventData* eventData)
{
    if (!eventData) return;

    switch (eventData->eventType)
    {
        case bz_ePlayerJoinEvent:
        {
            bz_PlayerJoinPartEventData_V1* ev = (bz_PlayerJoinPartEventData_V1*)eventData;
            playerLevels[ev->playerID] = 1;
            playerFlags[ev->playerID] = "SB";
            bonusAwardedPerPlayer[ev->playerID] = false;
            pendingLevelDownMessage[ev->playerID] = false;
            
            sendQuickGuide(ev->playerID);
            break;
        }
        case bz_ePlayerPartEvent:
        {
            bz_PlayerJoinPartEventData_V1* ev = (bz_PlayerJoinPartEventData_V1*)eventData;
            playerLevels.erase(ev->playerID);
            playerFlags.erase(ev->playerID);
            bonusAwardedPerPlayer.erase(ev->playerID);
            pendingLevelDownMessage.erase(ev->playerID);
            break;
        }
        case bz_ePlayerSpawnEvent:
        {
            bz_PlayerSpawnEventData_V1* ev = (bz_PlayerSpawnEventData_V1*)eventData;
            int level = playerLevels[ev->playerID];
            int maxLevel = getMaxLevel();

            if (pendingLevelDownMessage[ev->playerID]) {
                if (level == 0) {
                    bz_sendTextMessage(BZ_SERVER, ev->playerID, "You've dropped to level 0! No flags!");
                } else {
                    bz_sendTextMessagef(BZ_SERVER, ev->playerID, "You have come down to the level %d/%d!", level, maxLevel);
                }
                pendingLevelDownMessage[ev->playerID] = false;
            } else if (level == 0) {
                bz_sendTextMessage(BZ_SERVER, ev->playerID, "Level 0: Kill an enemy to get a flag!");
            }

            safeAssignFlagToPlayer(ev->playerID);

            if (level == 1) {
                bz_sendTextMessage(BZ_SERVER, ev->playerID, "Level 1: You CAN drop your flag and pick another from the map.");
            }
            break;
        }
        case bz_ePlayerDieEvent:
        {
            bz_PlayerDieEventData_V1* ev = (bz_PlayerDieEventData_V1*)eventData;

            if (ev->killerID >= 0 && ev->killerID != ev->playerID) {
                bz_BasePlayerRecord* killer = bz_getPlayerByIndex(ev->killerID);
                if (killer && killer->spawned) {
                    levelUp(ev->killerID);
                }
                if (killer) bz_freePlayerRecord(killer);
            }

            bz_BasePlayerRecord* victim = bz_getPlayerByIndex(ev->playerID);
            if (victim) {
                levelDown(ev->playerID);
            }
            if (victim) bz_freePlayerRecord(victim);
            break;
        }
        case bz_eAllowFlagGrab:
        {
            bz_AllowFlagGrabData_V1* ev = (bz_AllowFlagGrabData_V1*)eventData;
            if (isAssigningFlag) {
                ev->allow = true;
                break;
            }

            int level = playerLevels[ev->playerID];

            if (level == 0) {
                ev->allow = false;
            } 
            else if (level > 1) {
                if (playerFlags.find(ev->playerID) != playerFlags.end()) {
                    std::string assigned = playerFlags[ev->playerID];
                    if (ev->flagType && assigned == ev->flagType) {
                        ev->allow = true;
                    } else {
                        ev->allow = false;
                    }
                } else {
                    ev->allow = false;
                }
            }
            break;
        }
        case bz_eFlagGrabbedEvent:
        {
            bz_FlagGrabbedEventData_V1* ev = (bz_FlagGrabbedEventData_V1*)eventData;
            if (isAssigningFlag || !isPlayerValid(ev->playerID)) break;

            int level = playerLevels[ev->playerID];
            
            if (level == 0) {
                bz_removePlayerFlag(ev->playerID);
                bz_sendTextMessage(BZ_SERVER, ev->playerID, "At level 0 you can't take flags!");
            }
            else if (level > 1) {
                if (playerFlags.find(ev->playerID) != playerFlags.end()) {
                    if (ev->flagType && playerFlags[ev->playerID] != ev->flagType) {
                        bz_removePlayerFlag(ev->playerID);
                        giveFlagSafely(ev->playerID, playerFlags[ev->playerID]);
                        bz_sendTextMessage(BZ_SERVER, ev->playerID, "You can only use your assigned flag!");
                    }
                }
            }
            break;
        }
        case bz_eFlagDroppedEvent:
        {
            bz_FlagDroppedEventData_V1* ev = (bz_FlagDroppedEventData_V1*)eventData;
            
            if (playerLevels.find(ev->playerID) != playerLevels.end() && 
                playerLevels[ev->playerID] > 1) 
            {
                if (isPlayerValid(ev->playerID) && playerFlags.find(ev->playerID) != playerFlags.end()) 
                {
                    bz_givePlayerFlag(ev->playerID, playerFlags[ev->playerID].c_str(), true);
                    bz_sendTextMessage(BZ_SERVER, ev->playerID, "You cannot drop the assigned flag!");
                }
            }
            break;
        }
        default:
            break;
    }
}

BZ_PLUGIN(GunGamePlugin)
