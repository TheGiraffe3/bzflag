#include "bzfsAPI.h"
#include <vector>

static bool genocideInProgress = false;

class SimpleRogueG : public bz_Plugin
{
public:
    virtual const char* Name() { return "SimpleRogueG"; }
    virtual void Init(const char* config);
    virtual void Event(bz_EventData* eventData);
    virtual void Cleanup(void);
    
private:
    void performGenocide(int killerID, int victimID);
};

BZ_PLUGIN(SimpleRogueG)

void SimpleRogueG::Init(const char* config) {
    (void)config;
    Register(bz_ePlayerDieEvent);
    bz_debugMessage(4, "SimpleRogueG: Plugin loaded successfully.");
}

void SimpleRogueG::Cleanup(void) {
    Flush();
    genocideInProgress = false;
}

void SimpleRogueG::performGenocide(int killerID, int victimID) {
    if (genocideInProgress) return;
    
    genocideInProgress = true;
    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "*** Rogue Genocide successful! ***");
    
    std::vector<int> roguesToKill;
    
    // Metodo standard per ottenere la lista dei giocatori
    bz_APIIntList* playerList = bz_newIntList();
    bz_getPlayerIndexList(playerList);
    
    if (playerList && playerList->size() > 0) {
        for (unsigned int i = 0; i < playerList->size(); i++) {
            int currentPlayerID = (*playerList)[i];
            
            // Salta il giocatore che ha innescato il genocidio (la vittima originale)
            if (currentPlayerID == victimID) continue;
            
            bz_BasePlayerRecord* targetRecord = bz_getPlayerByIndex(currentPlayerID);
            
            if (targetRecord) {
                // Se il giocatore è Rogue, aggiungilo alla lista da uccidere
                if (targetRecord->team == eRogueTeam) {
                    roguesToKill.push_back(currentPlayerID);
                }
                bz_freePlayerRecord(targetRecord);
            }
        }
        bz_deleteIntList(playerList);
    }
    
    // Uccidi tutti i Rogue rimanenti DOPO aver liberato i record
    for (unsigned int i = 0; i < roguesToKill.size(); i++) {
        bz_killPlayer(roguesToKill[i], false, killerID, "G");
    }
    
    genocideInProgress = false;
}

void SimpleRogueG::Event(bz_EventData* eventData) {
    // Evita esecuzione durante un genocidio già in corso
    if (genocideInProgress) return;
    
    // Gestisci solo eventi di morte
    if (eventData->eventType != bz_ePlayerDieEvent) return;
    
    bz_PlayerDieEventData_V1* data = (bz_PlayerDieEventData_V1*)eventData;
    
    // Verifica che la vittima sia stata uccisa con la bandiera Genocidio
    if (data->flagKilledWith != "G") return;
    
    // Ottieni il record del giocatore ucciso
    bz_BasePlayerRecord* victimRecord = bz_getPlayerByIndex(data->playerID);
    
    if (!victimRecord) return;
    
    // Verifica se la vittima era un Rogue
    bool victimIsRogue = (victimRecord->team == eRogueTeam);
    bz_freePlayerRecord(victimRecord);
    
    // Se non era Rogue, il genocidio non avviene
    if (!victimIsRogue) return;
    
    // Esegui il genocidio di tutti gli altri Rogue
    performGenocide(data->killerID, data->playerID);
}
