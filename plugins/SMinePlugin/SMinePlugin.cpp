#include <bzfsAPI.h>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

struct SMine {
    float pos[3];
    double spawnTime; 
    int playerID; 
    bool isActive; 
    bool effectTriggered; 
};

std::map<int, SMine> activeSMines;
int nextMineID = 0; 

void applyShockwaveEffect(SMine& mine); 

class FMSlashCommand : public bz_CustomSlashCommandHandler
{
public:
    virtual bool SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList* /*params*/) override
    {
        if (command != "fm") {
            return false;
        }
        bz_BasePlayerRecord* player = bz_getPlayerByIndex(playerID);
        if (!player) {
            bz_sendTextMessage(BZ_SERVER, playerID, "Error: Player not found.");
            return true;
        }
        player->update();
        
        bz_debugMessagef(1, "SMinePlugin: Giocatore %d (%s) ha la bandiera (al momento di /fm): '%s'",
                         playerID, player->callsign.c_str(), player->currentFlag.c_str());
        bool hasSMFlag = false;
        std::string currentFlagStr = player->currentFlag.c_str();
        if (!currentFlagStr.empty() && currentFlagStr.find("SM") != std::string::npos) {
            hasSMFlag = true;
        }
        bz_debugMessagef(1, "SMinePlugin: Controllo bandiera SM per /fm: currentFlag='%s', find('SM') result=%zu, hasSMFlag=%s",
                         currentFlagStr.c_str(), currentFlagStr.find("SM"), hasSMFlag ? "true" : "false");
        if (!hasSMFlag) {
            bz_sendTextMessage(BZ_SERVER, playerID, "You must have the SM (Mine) flag to place a Mine.");
            bz_freePlayerRecord(player);
            return true;
        }
        const bz_PlayerUpdateState& state = player->lastKnownState;
        SMine newMine;
        newMine.pos[0] = state.pos[0];
        newMine.pos[1] = state.pos[1];
        newMine.pos[2] = state.pos[2];
        newMine.spawnTime = bz_getCurrentTime();
        newMine.playerID = playerID;
        newMine.isActive = false;
        newMine.effectTriggered = false;
        int currentMineID = nextMineID++;
        activeSMines[currentMineID] = newMine;
        bz_sendTextMessage(BZ_SERVER, playerID, "You've placed a Mine. You have 3 seconds to move away!");
        bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS,
                            "%s has placed a mine!", player->callsign.c_str());
        bz_removePlayerFlag(playerID);
        bz_freePlayerRecord(player);
        return true;
    }
};
FMSlashCommand fmCommandHandler;
class SMinePlugin : public bz_Plugin
{
public:
    const char* Name() override { return "SMinePlugin"; }
    void Init(const char* /*config*/) override
    {
        bz_debugMessage(1, "SMinePlugin: Inizializzazione avviata.");
        if (bz_RegisterCustomFlag("SM", "Mine: Write /fm", "A flag to place a powerful mine.", (bz_eShotType)0, eGoodFlag)) {
            bz_debugMessage(1, "SMinePlugin: Bandiera 'SM' personalizzata registrata con successo.");
        } else {
            bz_debugMessage(1, "SMinePlugin: Fallita la registrazione della bandiera 'SM'. Potrebbe essere già esistente o il nome è invalido.");
        }
        bz_registerCustomSlashCommand("fm", &fmCommandHandler);
        bz_debugMessage(1, "SMinePlugin: Tentativo di registrazione /fm.");
        Register(bz_eTickEvent);
        bz_debugMessage(1, "SMinePlugin: Registrato bz_eTickEvent.");
        bz_debugMessage(1, "SMinePlugin: Inizializzazione completata.");
    }
    void Cleanup() override
    {
        bz_debugMessage(1, "SMinePlugin: Cleanup avviato.");
        bz_removeCustomSlashCommand("fm");
        bz_debugMessage(1, "SMinePlugin: Deregistrato /fm.");
        Remove(bz_eTickEvent);
        activeSMines.clear();
        bz_debugMessage(1, "SMinePlugin: Cleanup completato.");
    }
    void Event(bz_EventData *eventData) override
    {
        switch (eventData->eventType)
        {
            case bz_eTickEvent:
            {
                double currentTime = bz_getCurrentTime();
                std::vector<int> minesMarkedForRemoval;
                std::vector<int> currentMineIDs;
                for (auto const& [mineID, mine] : activeSMines) {
                    currentMineIDs.push_back(mineID);
                }
                for (int mineID : currentMineIDs) {
                    if (activeSMines.count(mineID) == 0) {
                        continue;
                    }
                    SMine& mine = activeSMines.at(mineID);
                    if (!mine.isActive && (currentTime >= mine.spawnTime + 3.0)) {
                        mine.isActive = true;
                        bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS,
                                            "A mine placed by %s is now active!", bz_getPlayerCallsign(mine.playerID));
                        bz_debugMessagef(1, "SMinePlugin: Mina ID %d attivata a %.1f, %.1f, %.1f",
                                         mineID, mine.pos[0], mine.pos[1], mine.pos[2]);
                    }
                    if (mine.isActive && !mine.effectTriggered) {
                        applyShockwaveEffect(mine);
                        if (mine.effectTriggered) {
                            minesMarkedForRemoval.push_back(mineID);
                            bz_debugMessagef(1, "SMinePlugin: Mina ID %d segnata per rimozione (esplosa).", mineID);
                        }
                    }
                }
                for (int mineID : minesMarkedForRemoval) {
                    auto it = activeSMines.find(mineID);
                    if (it != activeSMines.end()) {
                        activeSMines.erase(it);
                        bz_debugMessagef(1, "SMinePlugin: Mina ID %d rimossa dalla mappa.", mineID);
                    }
                }
            }
            break;
            default:
                break;
        }
    }
};
void applyShockwaveEffect(SMine& mine)
{
    const float SHOCKWAVE_RADIUS = 20.0f;
    bool someoneWasHit = false;
    bz_debugMessagef(1, "SMinePlugin: applyShockwaveEffect chiamata per Mina a %.1f, %.1f, %.1f. isActive: %s, effectTriggered: %s",
                     mine.pos[0], mine.pos[1], mine.pos[2], mine.isActive ? "true" : "false", mine.effectTriggered ? "true" : "false");
    bz_APIIntList* playerList = bz_getPlayerIndexList();
    if (!playerList) {
        bz_debugMessage(1, "SMinePlugin: Errore: bz_getPlayerIndexList ha restituito NULL.");
        return;
    }
    bz_debugMessagef(1, "SMinePlugin: Trovati %zu giocatori attivi.", playerList->size());
    for (size_t i = 0; i < playerList->size(); ++i) {
        int targetPlayerID = (*playerList)[i];
        bz_BasePlayerRecord* targetPlayer = bz_getPlayerByIndex(targetPlayerID);
        if (!targetPlayer) {
            bz_debugMessagef(1, "SMinePlugin: applyShockwaveEffect: Giocatore target ID %d non trovato.", targetPlayerID);
            continue;
        }
        targetPlayer->update();
        bz_debugMessagef(1, "SMinePlugin: Controllo Giocatore %s (ID %d), Stato: %d, Bandiera: '%s'",
                         targetPlayer->callsign.c_str(), targetPlayerID, targetPlayer->lastKnownState.status, targetPlayer->currentFlag.c_str());
        if (targetPlayer->lastKnownState.status != eAlive) {
            bz_debugMessagef(1, "SMinePlugin: applyShockwaveEffect: Giocatore %s (ID %d) non è vivo. Status: %d",
                             targetPlayer->callsign.c_str(), targetPlayerID, targetPlayer->lastKnownState.status);
            bz_freePlayerRecord(targetPlayer);
            continue;
        }
        const bz_PlayerUpdateState& targetState = targetPlayer->lastKnownState;
        float dx = targetState.pos[0] - mine.pos[0];
        float dy = targetState.pos[1] - mine.pos[1];
        float dz = targetState.pos[2] - mine.pos[2];
        float distanceSq = (dx * dx) + (dy * dy) + (dz * dz);
        bz_debugMessagef(1, "SMinePlugin: Giocatore %s (ID %d) - Posizione: (%.1f, %.1f, %.1f), DistanzaSq dalla Mina: %.2f, RaggioSq: %.2f",
                         targetPlayer->callsign.c_str(), targetPlayerID, targetState.pos[0], targetState.pos[1], targetState.pos[2], distanceSq, (SHOCKWAVE_RADIUS * SHOCKWAVE_RADIUS));
        if (distanceSq <= (SHOCKWAVE_RADIUS * SHOCKWAVE_RADIUS)) {
            bz_debugMessagef(1, "SMinePlugin: Giocatore %s (ID %d) RIENTRA NEL RAGGIO (distSq: %.2f <= %.2f)",
                             targetPlayer->callsign.c_str(), targetPlayerID, distanceSq, (SHOCKWAVE_RADIUS * SHOCKWAVE_RADIUS));
            
            bool targetIsImmune = false;
            for (size_t j = 0; j < targetPlayer->flagHistory.size(); ++j) {
                if (!targetPlayer->flagHistory[j].empty() && strcmp(targetPlayer->flagHistory[j].c_str(), "IM") == 0) {
                    targetIsImmune = true;
                    bz_debugMessagef(1, "SMinePlugin: Giocatore %s (ID %d) ha bandiera IM e NON sarà colpito.", targetPlayer->callsign.c_str(), targetPlayerID);
                    break;
                }
            }
            if (targetIsImmune) {
                bz_sendTextMessagef(BZ_SERVER, targetPlayerID, "You were hit by a mine but protected by IM!");
                bz_freePlayerRecord(targetPlayer);
                continue;
            }
            
            std::string currentFlagStr = targetPlayer->currentFlag.c_str();
            bool hasSHFlag = (!currentFlagStr.empty() && currentFlagStr.find("SH") != std::string::npos);
            bz_debugMessagef(1, "SMinePlugin: Controllo bandiera SH per Giocatore %s (ID %d): currentFlag='%s', find('SH') result=%zu, hasSHFlag=%s",
                             targetPlayer->callsign.c_str(), targetPlayerID, currentFlagStr.c_str(), currentFlagStr.find("SH"), hasSHFlag ? "true" : "false");
            if (hasSHFlag) {
                bz_debugMessagef(1, "SMinePlugin: Giocatore %s (ID %d) ha SH, proteggo dalla mina.", targetPlayer->callsign.c_str(), targetPlayerID);
                bz_sendTextMessagef(BZ_SERVER, targetPlayerID, "Your SH flag protected you from a mine!");
                
                bz_removePlayerFlag(targetPlayerID);
                someoneWasHit = true; 
            } else {
                
                bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS,
                                    "The mine at (%.1f, %.1f, %.1f) exploded and hit %s!",
                                    mine.pos[0], mine.pos[1], mine.pos[2], targetPlayer->callsign.c_str());
                bz_killPlayer(targetPlayerID, true, mine.playerID, "SM");
                bz_debugMessagef(1, "SMinePlugin: Giocatore %s (ID %d) è stato ucciso da una Mina (Piazzata da %s, ID %d).",
                                 targetPlayer->callsign.c_str(), targetPlayerID, bz_getPlayerCallsign(mine.playerID), mine.playerID);
                someoneWasHit = true;
            }
        } else {
            bz_debugMessagef(1, "SMinePlugin: Giocatore %s (ID %d) FUORI DAL RAGGIO (distSq: %.2f > %.2f)",
                             targetPlayer->callsign.c_str(), targetPlayerID, distanceSq, (SHOCKWAVE_RADIUS * SHOCKWAVE_RADIUS));
        }
        bz_freePlayerRecord(targetPlayer);
    }
    bz_deleteIntList(playerList);
    if (someoneWasHit) {
        mine.effectTriggered = true;
        bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS,
                            "*** Mine exploded (%.1f, %.1f, %.1f)!",
                            mine.pos[0], mine.pos[1], mine.pos[2]);
        bz_debugMessage(1, "SMinePlugin: Mina ha colpito, verrà rimossa nel prossimo tick.");
    }
}
BZ_PLUGIN(SMinePlugin)
