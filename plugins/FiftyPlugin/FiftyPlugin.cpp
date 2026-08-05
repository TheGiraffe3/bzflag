#include "bzfsAPI.h"
#include <map>
#include <string>
#include <sstream>
#include <cstdio>
#include <algorithm>
#include <memory>

class PlayerRecordGuard {
public:
    explicit PlayerRecordGuard(int id) : rec(bz_getPlayerByIndex(id)) {}
    ~PlayerRecordGuard() { if (rec) bz_freePlayerRecord(rec); }
    PlayerRecordGuard(const PlayerRecordGuard&) = delete;
    PlayerRecordGuard& operator=(const PlayerRecordGuard&) = delete;
    PlayerRecordGuard(PlayerRecordGuard&& o) noexcept : rec(o.rec) { o.rec = nullptr; }
    bz_BasePlayerRecord* get() const { return rec; }
    bz_BasePlayerRecord* operator->() const { return rec; }
    explicit operator bool() const { return rec != nullptr; }
private:
    bz_BasePlayerRecord* rec;
};

class PlayerListGuard {
public:
    PlayerListGuard() : list(bz_getPlayerIndexList()) {}
    ~PlayerListGuard() { if (list) bz_deleteIntList(list); }
    PlayerListGuard(const PlayerListGuard&) = delete;
    PlayerListGuard& operator=(const PlayerListGuard&) = delete;
    PlayerListGuard(PlayerListGuard&& o) noexcept : list(o.list) { o.list = nullptr; }
    bz_APIIntList* get() const { return list; }
    explicit operator bool() const { return list != nullptr; }
private:
    bz_APIIntList* list;
};

class FileGuard {
public:
    explicit FileGuard(FILE* f) : file(f) {}
    ~FileGuard() { if (file) fclose(file); }
    FileGuard(const FileGuard&) = delete;
    FileGuard& operator=(const FileGuard&) = delete;
    FILE* get() const { return file; }
    explicit operator bool() const { return file != nullptr; }
private:
    FILE* file;
};

class FiftyPlugin : public bz_Plugin {
public:
    virtual const char* Name() { return "FiftyPlugin"; }
    virtual void Init(const char* config);
    virtual void Cleanup();
    virtual void Event(bz_EventData *eventData);

private:
    std::map<int, std::string> playerDeservedFlag; 
    std::map<int, std::string> playerActiveFlag;   
    std::map<int, int>         playerScore;
    std::map<int, bz_eTeamType> playerTeams;
    std::map<int, bool>        playerAlive;

    bz_eTeamType lastWinningTeam;
    bz_eTeamType currentWinningTeam;
    int countdownStep;
    bool isResetting; 

    const int TEAM_SCORE_CAP      = 50;
    const int WINNER_BONUS_POINTS = 2;
    const int SH_THRESHOLD        = 5;   // SH a 6 punti
    const int T_THRESHOLD         = 14;  // T a 15 punti
    const int ST_THRESHOLD        = 29;  // ST a 30 punti
    const std::string T_FLAG_NAME = "T";

    void updatePlayerState(int playerID, bool force = false);
    void syncFlag(int playerID);
    std::string getDeservedFlag(int score) const;
    std::string getTeamName(bz_eTeamType team) const;
    void sendRulesToPlayer(int playerID);
    void checkTeamVictory();
    int  getTeamScore(bz_eTeamType team);
    void resetAllScores();
    void restartGame();
    void writeBonusFile(bz_eTeamType winningTeam);
    void startCountdown(bz_eTeamType winningTeam, int teamScore);
    void processCountdownStep();
    void applyWinnerBonus();
};

BZ_PLUGIN(FiftyPlugin)

void FiftyPlugin::Init(const char* /*config*/) {
    lastWinningTeam = eNoTeam;
    currentWinningTeam = eNoTeam;
    countdownStep = 0;
    isResetting = false;

    Register(bz_ePlayerDieEvent);
    Register(bz_ePlayerSpawnEvent);
    Register(bz_ePlayerJoinEvent);
    Register(bz_ePlayerPartEvent);
    Register(bz_eFlagDroppedEvent);
    Register(bz_eTickEvent);

    bz_debugMessage(4, "FiftyPlugin: Initialized");
    bz_debugMessagef(4, "FiftyPlugin: SH@%d T@%d ST@%d Cap@%d Bonus@%d",
                     SH_THRESHOLD, T_THRESHOLD, ST_THRESHOLD,
                     TEAM_SCORE_CAP, WINNER_BONUS_POINTS);
}

void FiftyPlugin::Cleanup() {
    Flush();
    playerDeservedFlag.clear();
    playerActiveFlag.clear();
    playerScore.clear();
    playerTeams.clear();
    playerAlive.clear();
    lastWinningTeam = eNoTeam;
    currentWinningTeam = eNoTeam;
    countdownStep = 0;
    isResetting = false;
    bz_debugMessage(4, "FiftyPlugin: Cleaned up");
}

void FiftyPlugin::sendRulesToPlayer(int playerID) {
    const char* rules[] = {
        "=== FIFTY PLUGIN - QUICK RULES ===",
        "VICTORY: Team reaches 50 points (sum of positive scores)",
        "BONUS: Winners start next round with +2 points",
        "FLAGS (automatic based on personal score):",
        " 6+ pts  -> SH (Shield: protects from 1 hit)",
        "15+ pts  -> T  (Tiny: smaller tank)",
        "30+ pts  -> ST (Stealth: invisible to radar)",
        "NOTE: Loss of SH = Vulnerability until respawn.",
        "      You immediately gain SH if you destroy a tank",
        "========================================="
    };
    for (int r = 0; r < 10; r++) {
        bz_sendTextMessage(BZ_SERVER, playerID, rules[r]);
    }
}

std::string FiftyPlugin::getDeservedFlag(int score) const {
    if (score >= ST_THRESHOLD) return "ST";
    if (score >= T_THRESHOLD)  return T_FLAG_NAME;
    if (score >= SH_THRESHOLD) return "SH";
    return "";
}

void FiftyPlugin::syncFlag(int playerID) {
    if (!playerAlive[playerID]) return;

    const std::string& deserved = playerDeservedFlag[playerID];
    const std::string& active   = playerActiveFlag[playerID];

    if (active == deserved) return;

    if (!active.empty()) {
        PlayerRecordGuard p(playerID);
        if (p && p->currentFlag == active) {
            bz_removePlayerFlag(playerID);
            bz_debugMessagef(4, "FiftyPlugin: Removed %s from %d", active.c_str(), playerID);
        }
        playerActiveFlag[playerID] = "";
    }

    if (!deserved.empty()) {
        if (bz_givePlayerFlag(playerID, deserved.c_str(), true)) {
            playerActiveFlag[playerID] = deserved;
            bz_debugMessagef(4, "FiftyPlugin: Assigned %s to %d", deserved.c_str(), playerID);

            if (deserved == "SH")
                bz_sendTextMessage(BZ_SERVER, playerID, "Shield activated! Protects from 1 hit.");
            else if (deserved == T_FLAG_NAME)
                bz_sendTextMessage(BZ_SERVER, playerID, "Tiny activated! Smaller target.");
            else if (deserved == "ST")
                bz_sendTextMessage(BZ_SERVER, playerID, "Stealth activated! Invisible to radar.");
        } else {
            bz_debugMessagef(2, "FiftyPlugin: ERROR assigning %s to %d!", deserved.c_str(), playerID);
        }
    }
}

void FiftyPlugin::updatePlayerState(int playerID, bool force) {
    PlayerRecordGuard p(playerID);
    if (!p) return;

    int newScore = p->wins - p->losses;

    if (playerScore[playerID] != newScore || force) {
        
        playerScore[playerID] = newScore;

        std::string oldDeserved = playerDeservedFlag[playerID];
        std::string newDeserved = getDeservedFlag(newScore);
        playerDeservedFlag[playerID] = newDeserved;

        if (newDeserved != oldDeserved && !force) {
            std::stringstream msg;
            if (newDeserved.empty()) {
                msg << "Score " << newScore << ": Flag lost (below threshold).";
            } else if (oldDeserved.empty()) {
                msg << "Score " << newScore << ": " << newDeserved << " unlocked!";
            } else {
                msg << "Score " << newScore << ": " << oldDeserved << " -> " << newDeserved;
            }
            bz_sendTextMessage(BZ_SERVER, playerID, msg.str().c_str());
        }

        syncFlag(playerID);

        bz_debugMessagef(4, "FiftyPlugin: P%d score=%d deserved=%s active=%s alive=%d",
                         playerID, newScore, newDeserved.c_str(),
                         playerActiveFlag[playerID].c_str(),
                         playerAlive[playerID]);
    }
}

void FiftyPlugin::startCountdown(bz_eTeamType winningTeam, int teamScore) {
    currentWinningTeam = winningTeam;
    countdownStep = 5;
    lastWinningTeam = winningTeam;

    std::string teamName = getTeamName(winningTeam);

    // Annuncio pubblico della vittoria (senza soglie tecniche)
    std::stringstream victoryMsg;
    victoryMsg << "*** THE " << teamName << " TEAM WON! *** Score: "
               << teamScore << "/" << TEAM_SCORE_CAP;
    bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, victoryMsg.str().c_str());

    // NOTA: NON chiamare processCountdownStep() qui.
    // Il primo messaggio "New round in 5..." verrà stampato dal tick event
    // al prossimo secondo, evitando il doppio countdown.

    bz_debugMessage(1, victoryMsg.str().c_str());
}

void FiftyPlugin::processCountdownStep() {
    if (countdownStep <= 0) return;
    countdownStep--;

    if (countdownStep > 0) {
        std::stringstream msg;
        msg << "New round in " << countdownStep << "...";
        bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, msg.str().c_str());
    } else {
        bz_sendTextMessage(BZ_SERVER, BZ_ALLUSERS, "=== NEW ROUND STARTING NOW! ===");
        bz_setTeamWins(currentWinningTeam, bz_getTeamWins(currentWinningTeam) + 1);

        isResetting = true;
        resetAllScores();
        applyWinnerBonus();
        restartGame();
        isResetting = false;

        currentWinningTeam = eNoTeam;
    }
}

void FiftyPlugin::applyWinnerBonus() {
    PlayerListGuard list;
    if (!list) return;

    for (unsigned int i = 0; i < list.get()->size(); ++i) {
        int pid = (*list.get())[i];
        PlayerRecordGuard p(pid);
        if (!p || p->team != lastWinningTeam) continue;

        bz_setPlayerWins(pid, WINNER_BONUS_POINTS);
        bz_setPlayerLosses(pid, 0);
        playerDeservedFlag[pid] = "";
        playerActiveFlag[pid] = "";

        if (p->currentFlag == "SH" || p->currentFlag == T_FLAG_NAME || p->currentFlag == "ST")
            bz_removePlayerFlag(pid);

        std::stringstream privateMsg;
        privateMsg << "Victory bonus: +" << WINNER_BONUS_POINTS << " points!";
        bz_sendTextMessage(BZ_SERVER, pid, privateMsg.str().c_str());
    }
}

void FiftyPlugin::Event(bz_EventData *eventData) {
    if (!eventData) return;

    switch (eventData->eventType) {

        case bz_eTickEvent: {
            static double lastTime = 0;
            double now = bz_getCurrentTime();
            if (countdownStep > 0 && (now - lastTime) >= 1.0) {
                lastTime = now;
                processCountdownStep();
            }
        } break;

        case bz_ePlayerJoinEvent: {
            auto* d = (bz_PlayerJoinPartEventData_V1*)eventData;
            int pid = d->playerID;
            bz_eTeamType team = d->record->team;
            if (team < eRedTeam || team > ePurpleTeam) break;

            playerDeservedFlag[pid] = "";
            playerActiveFlag[pid]   = "";
            playerScore[pid]        = 0;
            playerTeams[pid]        = team;
            playerAlive[pid]        = false;

            updatePlayerState(pid);

            if (team == lastWinningTeam) {
                bz_setPlayerWins(pid, WINNER_BONUS_POINTS);
                bz_setPlayerLosses(pid, 0);
                updatePlayerState(pid, true);

                std::stringstream msg;
                msg << "Team victory bonus: " << WINNER_BONUS_POINTS << " points! "
                    << "SH@" << SH_THRESHOLD << ", T@" << T_THRESHOLD << ", ST@" << ST_THRESHOLD << ".";
                bz_sendTextMessage(BZ_SERVER, pid, msg.str().c_str());
            }

            sendRulesToPlayer(pid);
        } break;

        case bz_ePlayerPartEvent: {
            auto* d = (bz_PlayerJoinPartEventData_V1*)eventData;
            int pid = d->playerID;
            playerDeservedFlag.erase(pid);
            playerActiveFlag.erase(pid);
            playerScore.erase(pid);
            playerTeams.erase(pid);
            playerAlive.erase(pid);
        } break;

        case bz_ePlayerSpawnEvent: {
            auto* d = (bz_PlayerSpawnEventData_V1*)eventData;
            int pid = d->playerID;

            if (countdownStep > 0) {
                bz_killPlayer(pid, false);
                break;
            }
            if (playerDeservedFlag.find(pid) == playerDeservedFlag.end()) break;

            playerAlive[pid] = true;
            playerActiveFlag[pid] = "";

            {
                PlayerRecordGuard p(pid);
                if (p && p->team == lastWinningTeam &&
                    (p->wins - p->losses) < WINNER_BONUS_POINTS) {
                    bz_setPlayerWins(pid, WINNER_BONUS_POINTS);
                    bz_setPlayerLosses(pid, 0);
                }
            }

            updatePlayerState(pid, true);
        } break;

        case bz_ePlayerDieEvent: {
            
            if (countdownStep > 0) break;
            if (isResetting) break;

            auto* d = (bz_PlayerDieEventData_V1*)eventData;
            int pid      = d->playerID;
            int killerID = d->killerID;

            if (playerDeservedFlag.find(pid) == playerDeservedFlag.end()) break;

            playerAlive[pid] = false;
            playerActiveFlag[pid] = "";

            updatePlayerState(pid);

            if (killerID != BZ_SERVER && killerID != pid &&
                playerTeams.find(killerID) != playerTeams.end() &&
                playerTeams[killerID] != playerTeams[pid] &&
                playerTeams[killerID] != eNoTeam) {
                updatePlayerState(killerID);
            }

            checkTeamVictory();
        } break;

        case bz_eFlagDroppedEvent: {
            if (countdownStep > 0) break;

            auto* d = (bz_FlagDroppedEventData_V1*)eventData;
            int pid = d->playerID;

            if (playerDeservedFlag.find(pid) == playerDeservedFlag.end()) break;

            std::string dropped = d->flagType;
            if ((dropped == "SH" || dropped == T_FLAG_NAME || dropped == "ST") &&
                playerActiveFlag[pid] == dropped) {

                playerActiveFlag[pid] = "";
                bz_debugMessagef(4, "FiftyPlugin: %s dropped by %d, reassign at next spawn only",
                                 dropped.c_str(), pid);
            }
        } break;

        default: break;
    }
}

void FiftyPlugin::checkTeamVictory() {
    if (countdownStep > 0) return;
    if (isResetting) return; 

    bz_eTeamType teams[] = {eRedTeam, eGreenTeam, eBlueTeam, ePurpleTeam};
    for (int i = 0; i < 4; i++) {
        int score = getTeamScore(teams[i]);
        if (score >= TEAM_SCORE_CAP) {
            writeBonusFile(teams[i]);
            startCountdown(teams[i], score);
            return;
        }
    }
}

int FiftyPlugin::getTeamScore(bz_eTeamType team) {
    int sum = 0;
    PlayerListGuard list;
    if (!list) return 0;

    for (unsigned int i = 0; i < list.get()->size(); ++i) {
        int pid = (*list.get())[i];
        PlayerRecordGuard p(pid);
        if (p && p->team == team)
            sum += std::max(0, p->wins - p->losses);
    }
    return sum;
}

void FiftyPlugin::resetAllScores() {
    PlayerListGuard list;
    if (!list) return;

    for (unsigned int i = 0; i < list.get()->size(); ++i) {
        int pid = (*list.get())[i];
        PlayerRecordGuard p(pid);
        if (!p || p->team == lastWinningTeam) continue;

        bz_setPlayerWins(pid, 0);
        bz_setPlayerLosses(pid, 0);
        playerDeservedFlag[pid] = "";
        playerActiveFlag[pid] = "";

        if (p->currentFlag == "SH" || p->currentFlag == T_FLAG_NAME || p->currentFlag == "ST")
            bz_removePlayerFlag(pid);
    }
}

void FiftyPlugin::restartGame() {
    PlayerListGuard list;
    if (list) {
        for (unsigned int i = 0; i < list.get()->size(); ++i)
            bz_killPlayer((*list.get())[i], true);
    }
}

void FiftyPlugin::writeBonusFile(bz_eTeamType winningTeam) {
    FILE* file = fopen("FiftyBONUS.txt", "w");
    if (!file) return;
    FileGuard fg(file);

    fprintf(file, "=== TEAM VICTORY %s ===\n", getTeamName(winningTeam).c_str());
    fprintf(file, "Timestamp: %lf\n", bz_getCurrentTime());
    fprintf(file, "Total score: %d/%d\n", getTeamScore(winningTeam), TEAM_SCORE_CAP);
    fprintf(file, "Players:\n");

    PlayerListGuard list;
    if (list) {
        for (unsigned int i = 0; i < list.get()->size(); ++i) {
            int pid = (*list.get())[i];
            PlayerRecordGuard p(pid);
            if (p && p->team == winningTeam) {
                int s = p->wins - p->losses;
                fprintf(file, "- %s (W:%d L:%d Score:%d Flag:%s)\n",
                        p->callsign.c_str(), p->wins, p->losses, s,
                        getDeservedFlag(s).c_str());
            }
        }
    }
    fprintf(file, "=== END ===\n");
}

std::string FiftyPlugin::getTeamName(bz_eTeamType team) const {
    switch (team) {
        case eRedTeam:    return "Red";
        case eGreenTeam:  return "Green";
        case eBlueTeam:   return "Blue";
        case ePurpleTeam: return "Purple";
        default:          return "Unknown";
    }
}
