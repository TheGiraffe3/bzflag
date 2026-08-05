// Plugin UG (Ultimate Genocide) per BZFlag 2.4.30

#include "bzfsAPI.h"
#include <vector>
#include <cstring>  // For strcmp

class UGPlugin : public bz_Plugin
{
public:
    virtual const char* Name() { return "Ultimate Genocide"; }
    virtual void Init(const char* config);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);

private:
    void executeGenocide(int killerID);
    int ugFlagHolderID; // Stores the playerID of the UG flag holder, -1 if no one
    int ugFlagID;       // Stores the ID of the custom UG flag
    bool isGenocideActive; // Flag to prevent re-triggering during genocide execution
};

BZ_PLUGIN(UGPlugin)

void UGPlugin::Init(const char* config)
{
    ugFlagHolderID = -1; // Inizializza a -1 (nessun giocatore)
    isGenocideActive = false; // Inizializza il flag
    // Register custom flag "UG"
    ugFlagID = bz_RegisterCustomFlag("UG", "Ultimate Genocide", "Eliminates all other tanks (enemies and rogues) on the map and grants points for each kill after the holder makes a kill", 0, eGoodFlag);
    if (ugFlagID == -1) {
        bz_debugMessage(0, "UG Plugin: Failed to register custom flag 'UG'!");
    } else {
        bz_debugMessagef(4, "UG Plugin: Custom flag 'UG' registered with ID %d.", ugFlagID);
    }

    Register(bz_eFlagGrabbedEvent);
    Register(bz_ePlayerDieEvent); 
    Register(bz_eFlagDroppedEvent); 

    bz_debugMessage(4, "UG Plugin loaded successfully!");
    (void)config; // Suppress unused parameter warning
}

void UGPlugin::Cleanup()
{
    bz_debugMessage(4, "UG Plugin unloaded successfully!");
}

void UGPlugin::Event(bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_eFlagGrabbedEvent:
        {
            bz_FlagGrabbedEventData_V1* flagData = static_cast<bz_FlagGrabbedEventData_V1*>(eventData);
            if (strcmp(flagData->flagType, "UG") == 0)
            {
                ugFlagHolderID = flagData->playerID;
                bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, 
                    "ATTENTION: %s has grabbed the ULTIMATE GENOCIDE flag! Make a kill to unleash its power!", 
                    bz_getPlayerCallsign(flagData->playerID));
                bz_debugMessagef(4, "UG Plugin: Player %s (ID %d) grabbed UG flag.", bz_getPlayerCallsign(flagData->playerID), flagData->playerID);
            }
        }
        break;

        case bz_ePlayerDieEvent: 
        {
            bz_PlayerDieEventData_V1* dieData = static_cast<bz_PlayerDieEventData_V1*>(eventData);

            // Caso 1: Possessore della bandiera UG si suicida o viene ucciso da un non-player (es. dal mondo)
            if (ugFlagHolderID != -1 && dieData->playerID == ugFlagHolderID && (dieData->playerID == dieData->killerID || dieData->killerID == -1))
            {
                ugFlagHolderID = -1; // Reset UG holder
                bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS,
                    "%s has lost the ULTIMATE GENOCIDE power due to self-elimination!",
                    bz_getPlayerCallsign(dieData->playerID));
                bz_debugMessagef(4, "UG Plugin: Player %s (ID %d) lost UG power (suicide/non-killer).", bz_getPlayerCallsign(dieData->playerID), dieData->playerID);
                return; // Esce dalla funzione per non processare altri casi
            }

            // Caso 2: Il possessore della bandiera UG è stato ucciso da qualcun altro
            if (ugFlagHolderID != -1 && dieData->playerID == ugFlagHolderID)
            {
                ugFlagHolderID = -1; // Reset UG holder
                bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS,
                    "%s has lost the ULTIMATE GENOCIDE power!",
                    bz_getPlayerCallsign(dieData->playerID));
                bz_debugMessagef(4, "UG Plugin: Player %s (ID %d) lost UG power (killed by %s).", bz_getPlayerCallsign(dieData->playerID), dieData->playerID, dieData->killerID != -1 ? bz_getPlayerCallsign(dieData->killerID) : "unknown");
            }

            // Caso 3: Il possessore della bandiera UG ha fatto una kill (su un altro giocatore)
            // Aggiunta condizione !isGenocideActive per prevenire re-trigger durante il genocidio
            if (ugFlagHolderID != -1 && dieData->killerID == ugFlagHolderID && dieData->playerID != dieData->killerID && !isGenocideActive)
            {
                bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, 
                    "%s's kill has unleashed the ULTIMATE GENOCIDE!", 
                    bz_getPlayerCallsign(dieData->killerID));
                bz_debugMessagef(4, "UG Plugin: Player %s (ID %d) triggered genocide by killing %s (ID %d).", 
                                 bz_getPlayerCallsign(dieData->killerID), dieData->killerID, bz_getPlayerCallsign(dieData->playerID), dieData->playerID);
                
                ugFlagHolderID = -1; // Reset UG holder ID *prima* della chiamata a executeGenocide
                isGenocideActive = true; // Imposta il flag: genocidio in corso
                executeGenocide(dieData->killerID);
                isGenocideActive = false; // Reset del flag: genocidio completato

                bz_removePlayerFlag(dieData->killerID); 
            }
        }
        break;

        case bz_eFlagDroppedEvent: 
        {
            bz_FlagDroppedEventData_V1* dropData = static_cast<bz_FlagDroppedEventData_V1*>(eventData);
            
            bz_debugMessagef(4, "UG Plugin: Flag dropped event. Flag ID: %d, Player ID: %d, Flag Type: %s, UG Flag ID: %d, UG Holder: %d",
                                        dropData->flagID, dropData->playerID, dropData->flagType, ugFlagID, ugFlagHolderID);
            
            // --- MODIFICA QUI ---
            // Se la bandiera rilasciata è la UG, resetta ugFlagHolderID indipendentemente da chi l'ha rilasciata.
            if (ugFlagID != -1 && strcmp(dropData->flagType, "UG") == 0) // Controlla il tipo di bandiera e l'ID registrato
            {
                if (ugFlagHolderID != -1) { // Solo se c'era un holder tracciato
                    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, 
                        "%s has dropped the ULTIMATE GENOCIDE flag!", 
                        bz_getPlayerCallsign(ugFlagHolderID)); // Usa l'holder tracciato per il messaggio
                    bz_debugMessagef(4, "UG Plugin: UG flag holder reset due to explicit drop by %s (ID %d).", bz_getPlayerCallsign(ugFlagHolderID), ugFlagHolderID);
                } else {
                    // Caso in cui la bandiera UG è stata droppata, ma non avevamo un holder tracciato.
                    // Questo può succedere se la bandiera viene generata e cade subito, o altri scenari.
                    bz_debugMessagef(4, "UG Plugin: UG flag (ID %d) was dropped, but no active holder was tracked.", ugFlagID);
                }
                ugFlagHolderID = -1; // Reset SEMPRE l'holder a -1
            }
            // --- FINE MODIFICA ---
        }
        break;

        default:
            break;
    }
}

void UGPlugin::executeGenocide(int killerID)
{
    std::vector<int> playersToKill;
    bz_APIIntList* playerList = bz_getPlayerIndexList();
    if (!playerList) {
        bz_debugMessage(0, "UG Plugin: bz_getPlayerIndexList returned null. Aborting genocide.");
        return;
    }

    bool isFFA = (bz_getGameType() == static_cast<bz_eGameType>(eNoTeam)); 
    bz_eTeamType killerTeam = bz_getPlayerTeam(killerID);

    if (killerID == -1 || (killerTeam == eNoTeam && !isFFA)) { 
        bz_debugMessagef(2, "UG Plugin: Killer %s (ID %d) has eNoTeam in non-FFA game or is invalid. Treating as Rogue.", bz_getPlayerCallsign(killerID), killerID);
    }

    for (unsigned int i = 0; i < playerList->size(); i++)
    {
        int playerID = playerList->get(i);
        if (playerID == killerID || bz_getPlayerTeam(playerID) == eObservers) {
            continue; 
        }

        bz_eTeamType victimTeam = bz_getPlayerTeam(playerID);

        if (isFFA)
        {
            playersToKill.push_back(playerID);
        }
        else 
        {
            if (victimTeam != killerTeam || victimTeam == eRogueTeam)
            {
                playersToKill.push_back(playerID);
            }
        }
    }

    bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, 
        "*** ULTIMATE GENOCIDE IN PROGRESS ***");

    int killCount = 0;
    for (std::vector<int>::iterator it = playersToKill.begin(); it != playersToKill.end(); ++it)
    {
        int victimID = *it;
        const char* victimFlag = bz_getPlayerFlag(victimID); 
        bool protectedBySH = false;
        if (victimFlag && strcmp(victimFlag, "SH") == 0) {
            protectedBySH = true;
        }

        if (protectedBySH) {
            bz_removePlayerFlag(victimID); 
            bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS,
                "%s's SHIELD protected them from the ULTIMATE GENOCIDE!", 
                bz_getPlayerCallsign(victimID));
            bz_debugMessagef(4, "UG Plugin: Victim %s (ID %d) protected by SH.", bz_getPlayerCallsign(victimID), victimID);
        } else {
            // bz_killPlayer assegna automaticamente 1 punto al killer.
            // Non c'è bisogno di bz_incrementPlayerWins qui.
            bz_killPlayer(victimID, false, killerID);
            killCount++; 
            bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS,
                "%s has been eliminated by the ultimate genocide!", 
                bz_getPlayerCallsign(victimID));
            bz_debugMessagef(4, "UG Plugin: Victim %s (ID %d) eliminated by genocide.", bz_getPlayerCallsign(victimID), victimID);
        }
    }

    if (killCount > 0)
    {
        bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS,
            "GENOCIDE COMPLETE! %s eliminated %d tanks and gained %d points!", 
            bz_getPlayerCallsign(killerID), killCount, killCount);
        
        // Non assegniamo punti qui, dato che bz_killPlayer lo fa automaticamente.
        bz_debugMessagef(4, "UG Plugin: Killer %s (ID %d) effectively gained %d points from genocide (via bz_killPlayer auto-scoring).", bz_getPlayerCallsign(killerID), killerID, killCount);
    }
    else
    {
        bz_sendTextMessage(BZ_SERVER, killerID,
            "No other players to eliminate!");
        bz_debugMessagef(4, "UG Plugin: No players eliminated by genocide for killer %s (ID %d).", bz_getPlayerCallsign(killerID), killerID);
    }
}
