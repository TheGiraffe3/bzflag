#include "bzfsAPI.h"
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <cctype>

// --- Message Constants ---
const char* GLOBAL_PRIMARY_COLOR = "\033[1;35m";    // Purple
const char* GLOBAL_HIGHLIGHT_COLOR = "\033[1;33m"; // Bright Yellow
const char* GLOBAL_GOOD_COLOR = "\033[1;32m";      // Bright Green
const char* GLOBAL_RED = "\033[1;31m";             // Bright Red

// --- Scoreboard Data Structure ---
struct PlayerGlobalStats {
    int kills;
    int deaths;
    
    PlayerGlobalStats() : kills(0), deaths(0) {}
    
    // Calcola il punteggio netto (Kills - Deaths)
    int getNetScore() const {
        return kills - deaths;
    }
};

// --- GLOBAL VARIABLES ---
std::map<std::string, PlayerGlobalStats> globalPlayerStats; 
const std::string SCOREBOARD_FILE = "bzflag-score.txt"; // Percorso relativo universale
const int MAX_SCOREBOARD_SIZE = 20; 

std::chrono::steady_clock::time_point lastGlobalSaveTime;
const int GLOBAL_SAVE_INTERVAL_SECONDS = 480; // Save every 8 minutes

// --- Helper: Get Top N scores sorted by Net Score ---
std::vector<std::pair<std::string, PlayerGlobalStats>> getTopScores(int maxCount = MAX_SCOREBOARD_SIZE) {
    std::vector<std::pair<std::string, PlayerGlobalStats>> sortedScores(globalPlayerStats.begin(), globalPlayerStats.end());
    
    // Ordina per Punteggio Netto decrescente. In caso di parità, vince chi ha più kill.
    std::sort(sortedScores.begin(), sortedScores.end(), [](const auto& a, const auto& b) {
        int netA = a.second.getNetScore();
        int netB = b.second.getNetScore();
        if (netA != netB) {
            return netA > netB; 
        }
        return a.second.kills > b.second.kills; 
    });
    
    if (static_cast<int>(sortedScores.size()) > maxCount) {
        sortedScores.resize(maxCount);
    }
    return sortedScores;
}

// --- Load Global Scores from File ---
void loadGlobalScores() {
    globalPlayerStats.clear();
    std::ifstream file(SCOREBOARD_FILE);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string callsign;
            int k, d;
            // Legge Callsign, Kills e Deaths separatamente per massima sicurezza
            if (ss >> callsign >> k >> d) {
                globalPlayerStats[callsign].kills = k;
                globalPlayerStats[callsign].deaths = d;
            }
        }
        file.close();
    }
}

// --- Save Global Scores to File ---
void saveGlobalScores() {
    // Protezione critica: Se la mappa è vuota, NON toccare il file
    // Questo impedisce di perdere lo storico quando la RAM viene svuotata (solo observer/server vuoto)
    if (globalPlayerStats.empty()) {
        return; 
    }
    
    std::ofstream file(SCOREBOARD_FILE);
    if (file.is_open()) {
        for (const auto& entry : globalPlayerStats) {
            file << entry.first << " " << entry.second.kills << " " << entry.second.deaths << "\n";
        }
        file.close();
    }
}

// --- Display Global Scoreboard ---
void displayGlobalScoreboard(int targetPlayerID) {
    auto topScores = getTopScores();
    if (topScores.empty()) {
        bz_sendTextMessage(BZ_SERVER, targetPlayerID, "Classifica vuota. Combatti per entrare nella Top 20!");
        return;
    }

    bz_sendTextMessage(BZ_SERVER, targetPlayerID, (std::string(GLOBAL_PRIMARY_COLOR) + "--- Classifica Globale (Top 20) ---").c_str());
    
    int count = 0;
    for (const auto& entry : topScores) {
        std::stringstream ss;
        int netScore = entry.second.getNetScore();
        
        // Colora il totale effettivo: Verde se >= 0, Rosso se negativo
        const char* scoreColor = (netScore >= 0) ? GLOBAL_GOOD_COLOR : GLOBAL_RED;
        
        ss << GLOBAL_HIGHLIGHT_COLOR << (count + 1) << ". "
           << GLOBAL_GOOD_COLOR << entry.first << ": "
           << GLOBAL_PRIMARY_COLOR << "Kills: " << entry.second.kills
           << ", Deaths: " << entry.second.deaths
           << " | Totale: " << scoreColor << netScore;
        
        bz_sendTextMessage(BZ_SERVER, targetPlayerID, ss.str().c_str());
        count++;
    }
    
    bz_sendTextMessage(BZ_SERVER, targetPlayerID, (std::string(GLOBAL_PRIMARY_COLOR) + "-----------------------------------").c_str());
}

class GlobalScoreboard : public bz_Plugin {
public:
    virtual const char* Name() override { return "GlobalScoreboard"; }
    
    virtual void Init(const char*) override {
        Register(bz_ePlayerDieEvent);
        Register(bz_eRawChatMessageEvent);
        Register(bz_eTickEvent);
        Register(bz_ePlayerPartEvent);
        Register(bz_ePlayerJoinEvent); 
        
        loadGlobalScores();
        lastGlobalSaveTime = std::chrono::steady_clock::now();
    }
    
    virtual void Cleanup() override {
        Remove(bz_ePlayerDieEvent);
        Remove(bz_eRawChatMessageEvent);
        Remove(bz_eTickEvent);
        Remove(bz_ePlayerPartEvent);
        Remove(bz_ePlayerJoinEvent); 
        saveGlobalScores();
        globalPlayerStats.clear();
    }
    
    virtual void Event(bz_EventData* eventData) override {
        switch (eventData->eventType) {

                        case bz_ePlayerDieEvent: {
                bz_PlayerDieEventData_V1* dieEventData = (bz_PlayerDieEventData_V1*)eventData;
                int killerID = dieEventData->killerID;
                int victimID = dieEventData->playerID;
                
                // --- SICUREZZA KILLER ---
                if (killerID != BZ_SERVER && killerID != victimID) {
                    const char* killerName = bz_getPlayerCallsign(killerID);
                    // Controllo critico: evita il crash se l'API ritorna nullptr durante i countdown
                    if (killerName != nullptr) {
                        std::string killerCallsign(killerName);
                        if (!killerCallsign.empty() && killerCallsign != "anonymous") {
                            globalPlayerStats[killerCallsign].kills++;
                        }
                    }
                }
                
                // --- SICUREZZA VITTIMA ---
                if (victimID != BZ_SERVER) {
                    const char* victimName = bz_getPlayerCallsign(victimID);
                    // Controllo critico: evita il crash se l'API ritorna nullptr durante i countdown
                    if (victimName != nullptr) {
                        std::string victimCallsign(victimName);
                        if (!victimCallsign.empty() && victimCallsign != "anonymous") {
                            globalPlayerStats[victimCallsign].deaths++;
                        }
                    }
                }
                break;
            }
            
            case bz_eRawChatMessageEvent: {
                bz_ChatEventData_V1* chatData = (bz_ChatEventData_V1*)eventData;
                std::string receivedMessage = chatData->message.c_str();
                std::transform(receivedMessage.begin(), receivedMessage.end(), receivedMessage.begin(), ::tolower);
                
                // Accetta SOLO il comando -top
                if (receivedMessage == "-top") {
                    displayGlobalScoreboard(chatData->from);
                    chatData->message = ""; // Consuma il messaggio per non mostrarlo in chat pubblica
                }
                break;
            }
            
            case bz_eTickEvent: {
                auto now = std::chrono::steady_clock::now();
                
                // --- CONTROLLO GIOCATORI ATTIVI (Gestione Memoria) ---
                int totalPlayers = bz_getPlayerCount();
                bool hasActivePlayers = false;
                
                for (int i = 0; i < totalPlayers; i++) {
                    bz_BasePlayerRecord* record = bz_getPlayerByIndex(i);
                    if (record) {
                        // Se c'è almeno un giocatore che NON è Observer, il server è attivo
                        if (record->team != eObservers) {
                            hasActivePlayers = true;
                        }
                        bz_freePlayerRecord(record); // Fondamentale per evitare memory leak dell'API
                        
                        if (hasActivePlayers) break; 
                    }
                }
                
                if (hasActivePlayers) {
                    // Se ci sono giocatori attivi ma la RAM è vuota, ricarico i dati dal disco
                    if (globalPlayerStats.empty()) {
                        loadGlobalScores();
                    }
                } else {
                    // Non ci sono giocatori attivi (solo observer o server vuoto) -> Pulisco la RAM
                    if (!globalPlayerStats.empty()) {
                        globalPlayerStats.clear();
                    }
                }
                
                // --- SALVATAGGIO PERIODICO ---
                auto elapsed_save_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - lastGlobalSaveTime).count();
                if (elapsed_save_seconds >= GLOBAL_SAVE_INTERVAL_SECONDS) {
                    saveGlobalScores(); 
                    lastGlobalSaveTime = now;
                }
                break;
            }
            
            case bz_ePlayerPartEvent: {
                saveGlobalScores(); 
                break;
            }
            
            case bz_ePlayerJoinEvent: {
                bz_PlayerJoinPartEventData_V1* joinEventData = (bz_PlayerJoinPartEventData_V1*)eventData;
                int playerID = joinEventData->playerID;
                std::string callsign = bz_getPlayerCallsign(playerID);
                
                std::stringstream welcomeMsg;
                welcomeMsg << GLOBAL_GOOD_COLOR << "Ciao " << callsign << ", "
                           << GLOBAL_PRIMARY_COLOR << "digita "
                           << GLOBAL_HIGHLIGHT_COLOR << "-top" << GLOBAL_PRIMARY_COLOR 
                           << " nella chat per vedere la classifica globale!";
                           
                bz_sendTextMessage(BZ_SERVER, playerID, welcomeMsg.str().c_str());
                break;
            }
            
            default:
                break;
        }
    }
};

BZ_PLUGIN(GlobalScoreboard)
