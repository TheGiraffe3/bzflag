#include "bzfsAPI.h"
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <cctype>
#include <algorithm>

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
    bool isAssigningFlag;
    void loadConfig(const std::string& filename);
    void safeAssignFlagToPlayer(int playerID, bool fromEvent = false);
    void levelUp(int playerID);
    void levelDown(int playerID);
    void awardBonus(int playerID);
    std::string trim(const std::string& str);
    bool isValidFlag(const std::string& flag);
    int countActivePlayers();
    bool isPlayerValid(int playerID);
    void giveFlagSafely(int playerID, const std::string& flag);
    int getMaxLevel() { return levelFlags.size(); }
};

bool GunGamePlugin::isPlayerValid(int playerID)
{
    if (playerID < 0)
        return false;
  
    bz_BasePlayerRecord* player = bz_getPlayerByIndex(playerID);
    if (!player)
        return false;
  
    bool valid = player->spawned;
    bz_freePlayerRecord(player);
  
    return valid;
}

std::string GunGamePlugin::trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return "";
  
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

bool GunGamePlugin::isValidFlag(const std::string& flag)
{
    const char* validFlags[] = {"SB", "CL", "L", "GM", "SW", "V", "SR", "F", "ST", "T",
                               "IB", "N", "QT", "BU", "MG", "ID", "WG", "SH", "A",
                               "OO", "PZ", "MQ", "JM", "NJ"};
  
    for (size_t i = 0; i < sizeof(validFlags)/sizeof(validFlags[0]); ++i)
    {
        if (flag == validFlags[i])
            return true;
    }
    return false;
}

int GunGamePlugin::countActivePlayers()
{
    int count = 0;
    bz_APIIntList* playerList = bz_getPlayerIndexList();
  
    if (!playerList)
        return 0;
  
    for (unsigned int i = 0; i < playerList->size(); i++)
    {
        int playerID = (*playerList)[i];
        if (isPlayerValid(playerID))
            count++;
    }
  
    bz_deleteIntList(playerList);
    return count;
}

void GunGamePlugin::loadConfig(const std::string& filename)
{
    levelFlags.clear();
  
    std::ifstream file(filename.c_str());
    if (!file.is_open())
    {
        bz_debugMessage(0, "GunGame4: usando configurazione di default");
        levelFlags.push_back({"SB"});
        levelFlags.push_back({"CL"});
        levelFlags.push_back({"L"});
        levelFlags.push_back({"GM"});
        levelFlags.push_back({"SW"});
        levelFlags.push_back({"V"});
        levelFlags.push_back({"SR"});
        return;
    }
    std::string line;
    while (std::getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;
        std::vector<std::string> flags;
        std::stringstream ss(line);
        std::string flag;
      
        while (std::getline(ss, flag, ','))
        {
            flag = trim(flag);
            if (!flag.empty() && isValidFlag(flag))
                flags.push_back(flag);
        }
      
        if (!flags.empty())
            levelFlags.push_back(flags);
    }
  
    file.close();
  
    if (levelFlags.empty())
    {
        bz_debugMessage(0, "GunGame4: configurazione di default (file vuoto o non valido)");
        levelFlags.push_back({"SB"});
        levelFlags.push_back({"CL"});
        levelFlags.push_back({"L"});
        levelFlags.push_back({"GM"});
        levelFlags.push_back({"SW"});
        levelFlags.push_back({"V"});
        levelFlags.push_back({"SR"});
    }
  
    bz_debugMessagef(0, "GunGame4: caricati %d livelli", (int)levelFlags.size());
}

void GunGamePlugin::Init(const char* commandLine)
{
    Register(bz_ePlayerJoinEvent);
    Register(bz_ePlayerPartEvent);
    Register(bz_ePlayerDieEvent);
    Register(bz_ePlayerSpawnEvent);
    Register(bz_eFlagGrabbedEvent);
    Register(bz_eFlagDroppedEvent);
    std::string configFile = "GunGame4.cfg";
    if (commandLine && *commandLine)
        configFile = commandLine;
  
    loadConfig(configFile);
    bz_setShotMismatch(false); // Disabilita il controllo anti-cheat per i colpi
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
}

void GunGamePlugin::awardBonus(int playerID)
{
    static int callCounter = 0; // Contatore per tracciare le chiamate
    bz_debugMessagef(0, "GunGame4: Entrato in awardBonus per player %d (chiamata #%d)", playerID, ++callCounter);
   
    if (bonusAwardedPerPlayer[playerID])
    {
        bz_debugMessagef(0, "GunGame4: Bonus già assegnato per player %d", playerID);
        return;
    }
      
    int activePlayers = countActivePlayers();
    int bonusPoints = activePlayers + 1;
  
    bz_BasePlayerRecord* player = bz_getPlayerByIndex(playerID);
    if (!player)
    {
        bz_debugMessagef(0, "GunGame4: Impossibile assegnare bonus, player %d non trovato", playerID);
        return;
    }
  
    bz_incrementPlayerWins(playerID, bonusPoints);
   
    bonusAwardedPerPlayer[playerID] = true;
    playerLevels[playerID] = 1;
    safeAssignFlagToPlayer(playerID, false);
    bonusAwardedPerPlayer[playerID] = false; // Resetta per consentire bonus futuri
   
    bz_debugMessagef(0, "GunGame4: Tentativo invio messaggio pubblico per player %d (callsign: %s, punti: %d)",
                     playerID, player->callsign.c_str(), bonusPoints);
    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS,
        "%s has reached the final level and receives %d bonus points!",
        player->callsign.c_str(), bonusPoints);
    bz_debugMessagef(0, "GunGame4: Messaggio pubblico di bonus inviato per player %d", playerID);
      
    bz_freePlayerRecord(player);
    bz_debugMessagef(0, "GunGame4: Bonus assegnato a player %d, livello resettato a 1", playerID);
}

void GunGamePlugin::giveFlagSafely(int playerID, const std::string& flag)
{
    if (!isPlayerValid(playerID))
        return;
  
    isAssigningFlag = true;
    bz_removePlayerFlag(playerID);
    bz_givePlayerFlag(playerID, flag.c_str(), true);
    isAssigningFlag = false;
}

void GunGamePlugin::safeAssignFlagToPlayer(int playerID, bool fromEvent)
{
    if (!isPlayerValid(playerID) || (isAssigningFlag && !fromEvent))
    {
        bz_debugMessagef(0, "GunGame4: Impossibile assegnare bandiera a player %d (non valido o assegnazione in corso)", playerID);
        return;
    }
  
    isAssigningFlag = true;
  
    if (playerLevels.find(playerID) == playerLevels.end())
        playerLevels[playerID] = 1;
  
    int level = playerLevels[playerID];
    int maxLevel = getMaxLevel();
  
    bz_debugMessagef(0, "GunGame4: Player %d at level %d/%d", playerID, level, maxLevel);
  
    if (level > maxLevel)
    {
        bz_debugMessagef(0, "GunGame4: Player %d exceeded max level, calling awardBonus", playerID);
        awardBonus(playerID);
        isAssigningFlag = false;
        return;
    }
  
    if (level == 0)
    {
        bz_removePlayerFlag(playerID);
        playerFlags[playerID] = "";
        bz_sendTextMessage(BZ_SERVER, playerID,
            "Level 0: No flag!");
        isAssigningFlag = false;
        return;
    }
  
    if (level >= 1 && level <= maxLevel)
    {
        if (level - 1 < 0 || level - 1 >= (int)levelFlags.size() || levelFlags.empty())
        {
            bz_debugMessagef(0, "GunGame4: errore livello %d per player %d", level, playerID);
            isAssigningFlag = false;
            return;
        }
      
        const auto& availableFlags = levelFlags[level - 1];
        if (availableFlags.empty())
        {
            bz_sendTextMessage(BZ_SERVER, playerID, "No flags available");
            isAssigningFlag = false;
            return;
        }
      
        int randomIndex = rand() % availableFlags.size();
        std::string selectedFlag = availableFlags[randomIndex];
      
        playerFlags[playerID] = selectedFlag;
        giveFlagSafely(playerID, selectedFlag);
      
        bz_sendTextMessagef(BZ_SERVER, playerID,
            "Level %d/%d: %s", level, maxLevel, selectedFlag.c_str());
    }
  
    isAssigningFlag = false;
}

void GunGamePlugin::levelUp(int playerID)
{
    if (playerLevels.find(playerID) == playerLevels.end())
        playerLevels[playerID] = 1;
  
    int oldLevel = playerLevels[playerID];
    int maxLevel = getMaxLevel();
   
    if (oldLevel >= maxLevel)
    {
        bz_debugMessagef(0, "GunGame4: Player %d reached max level %d, calling awardBonus", playerID, maxLevel);
        awardBonus(playerID);
        return;
    }
   
    playerLevels[playerID]++;
   
    bz_debugMessagef(0, "GunGame4: Player %d leveled up to %d/%d", playerID, playerLevels[playerID], maxLevel);
   
    if (oldLevel == 0)
    {
        bz_sendTextMessagef(BZ_SERVER, playerID,
            "You've reached level 1! You can now use flags");
    }
    else
    {
        bz_sendTextMessagef(BZ_SERVER, playerID,
            "You've reached level %d/%d!", playerLevels[playerID], maxLevel);
    }
  
    safeAssignFlagToPlayer(playerID, false);
}

void GunGamePlugin::levelDown(int playerID)
{
    if (playerLevels.find(playerID) == playerLevels.end())
        playerLevels[playerID] = 1;
  
    int maxLevel = getMaxLevel();
    int oldLevel = playerLevels[playerID];
  
    if (playerLevels[playerID] > 0)
    {
        playerLevels[playerID]--;
      
        if (playerLevels[playerID] == 0)
        {
            bz_sendTextMessage(BZ_SERVER, playerID,
                "You've dropped to level 0! No flags!");
        }
        else
        {
            bz_sendTextMessagef(BZ_SERVER, playerID,
                "You have come down to the level %d/%d!", playerLevels[playerID], maxLevel);
        }
      
        bz_debugMessagef(0, "GunGame4: Player %d level down %d -> %d",
                        playerID, oldLevel, playerLevels[playerID]);
    }
    else
    {
        bz_sendTextMessage(BZ_SERVER, playerID,
            "You remained at level 0 (livello minimo)");
    }
  
    safeAssignFlagToPlayer(playerID, false);
}

void GunGamePlugin::Event(bz_EventData* eventData)
{
    if (!eventData)
        return;
    switch (eventData->eventType)
    {
    case bz_ePlayerJoinEvent:
    {
        bz_PlayerJoinPartEventData_V1* ev = (bz_PlayerJoinPartEventData_V1*)eventData;
        playerLevels[ev->playerID] = 1;
        playerFlags[ev->playerID] = "SB";
        bonusAwardedPerPlayer[ev->playerID] = false;
        bz_debugMessagef(0, "GunGame4: Player %d joined", ev->playerID);
        break;
    }
    case bz_ePlayerPartEvent:
    {
        bz_PlayerJoinPartEventData_V1* ev = (bz_PlayerJoinPartEventData_V1*)eventData;
        playerLevels.erase(ev->playerID);
        playerFlags.erase(ev->playerID);
        bonusAwardedPerPlayer.erase(ev->playerID);
        bz_debugMessagef(0, "GunGame4: Player %d left", ev->playerID);
        break;
    }
    case bz_ePlayerSpawnEvent:
    {
        bz_PlayerSpawnEventData_V1* ev = (bz_PlayerSpawnEventData_V1*)eventData;
        safeAssignFlagToPlayer(ev->playerID, false);
        bz_debugMessagef(0, "GunGame4: Player %d spawned", ev->playerID);
        break;
    }
    case bz_ePlayerDieEvent:
    {
        bz_PlayerDieEventData_V1* ev = (bz_PlayerDieEventData_V1*)eventData;
      
        bz_debugMessagef(0, "GunGame4: PlayerDieEvent - player: %d, killer: %d",
                        ev->playerID, ev->killerID);
      
        // Killer guadagna un livello (solo se non è suicide e il killer esiste)
        if (ev->killerID >= 0 && ev->killerID != ev->playerID)
        {
            bz_BasePlayerRecord* killer = bz_getPlayerByIndex(ev->killerID);
            if (killer && killer->spawned)
            {
                bz_debugMessagef(0, "GunGame4: Killer %d level up", ev->killerID);
                levelUp(ev->killerID);
            }
            if (killer) bz_freePlayerRecord(killer);
        }
      
        // Vittima perde un livello (sempre, tranne forse per suicide)
        bz_BasePlayerRecord* victim = bz_getPlayerByIndex(ev->playerID);
        if (victim)
        {
            bz_debugMessagef(0, "GunGame4: Victim %d spawned status: %d", ev->playerID, victim->spawned);
            bz_debugMessagef(0, "GunGame4: Victim %d level down", ev->playerID);
            levelDown(ev->playerID);
        }
        if (victim) bz_freePlayerRecord(victim);
      
        break;
    }
    case bz_eFlagGrabbedEvent:
    {
        bz_FlagGrabbedEventData_V1* ev = (bz_FlagGrabbedEventData_V1*)eventData;
       
        if (isAssigningFlag || !isPlayerValid(ev->playerID))
            break;
           
        if (playerLevels[ev->playerID] == 0)
        {
            bz_removePlayerFlag(ev->playerID);
            bz_sendTextMessage(BZ_SERVER, ev->playerID,
                "At level 0 you can't take flags!");
            break;
        }
       
        if (playerLevels[ev->playerID] > 1)
        {
            bz_removePlayerFlag(ev->playerID);
           
            if (playerFlags.find(ev->playerID) != playerFlags.end())
            {
                giveFlagSafely(ev->playerID, playerFlags[ev->playerID]);
            }
           
            bz_sendTextMessage(BZ_SERVER, ev->playerID, "You can only use your assigned flag!");
        }
        break;
    }
    case bz_eFlagDroppedEvent:
    {
        bz_FlagDroppedEventData_V1* ev = (bz_FlagDroppedEventData_V1*)eventData;
      
        if (!isPlayerValid(ev->playerID))
            break;
          
        if (playerLevels[ev->playerID] == 0)
            break;
          
        if (playerLevels[ev->playerID] > 1)
        {
            if (playerFlags.find(ev->playerID) != playerFlags.end())
            {
                giveFlagSafely(ev->playerID, playerFlags[ev->playerID]);
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
