/*
UselessRampage
    Copyright (C) 2014 Vladimir "allejo" Jimenez

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstring>

#include "bzfsAPI.h"
#include "plugin_utils.h"

const int MAX_KILLS_POINTS = 20;

class UselessRampage : public bz_Plugin
{
public:
    virtual const char* Name () {return "UselessRampage";}
    virtual void Init (const char* config);
    virtual void Event (bz_EventData *eventData);
    virtual void Cleanup (void);

    int consecutiveKillsWithUS[256];  // Keep count of how many consecutive kills a player has made with useless
    int consecutiveKills[256];        // Keep count of how many consecutive kills a player has made
};

BZ_PLUGIN(UselessRampage)

void UselessRampage::Init (const char* commandLine)
{
    // Register our events with Register()
    Register(bz_eFlagDroppedEvent);
    Register(bz_ePlayerDieEvent);
    Register(bz_ePlayerJoinEvent);
}

void UselessRampage::Cleanup (void)
{
    Flush(); // Clean up all the events
}

void UselessRampage::Event (bz_EventData *eventData)
{
    switch (eventData->eventType)
    {
        case bz_eFlagDroppedEvent: // This event is called each time a flag is dropped by a player.
        {
            bz_FlagDroppedEventData_V1* flagDropData = (bz_FlagDroppedEventData_V1*)eventData;

            if (strcmp(bz_getName(flagDropData->flagID).c_str(), "US") == 0) // If a player dropped the Useless flag by choice or death...
            {
                consecutiveKillsWithUS[flagDropData->playerID] = 0; // Reset their counter because their spree is ruined
            }
        }
        break;

        case bz_ePlayerDieEvent: // This event is called each time a tank is killed.
        {
            bz_PlayerDieEventData_V1* dieData = (bz_PlayerDieEventData_V1*)eventData;

            // Variables for easier reference
            int killerID = dieData->killerID;
            int victimID = dieData->playerID;

            // Always increment the number of consecutive kills the killer has made
            consecutiveKills[killerID]++;

            if (dieData->flagKilledWith == "US") // A player was killed with the Useless flag
            {
                // Increment the counter for consecutive kills with useless
                consecutiveKillsWithUS[killerID]++;

                // Get a player's score for easy access
                int killerScore = bz_getPlayerWins(killerID);

                // Check if a player has gotten the maximum allowed points per kill, if not then we'll get their consecutive
                // minus one. We subtract one because the server automatically grants the player a point so we don't want to
                // give the player an extra point when their consecutive kill count is one.
                killerScore += (consecutiveKills[killerID] < MAX_KILLS_POINTS) ? consecutiveKills[killerID] - 1 : MAX_KILLS_POINTS;

                bz_setPlayerWins(killerID, killerScore);

                // Special messages for when players reach milestones in their spree
                if (consecutiveKillsWithUS[killerID] == 10)
                {
                    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "We finally found a use for %s. 10 kills with useless",
                        bz_getPlayerByIndex(killerID)->callsign.c_str());
                }
                else if (consecutiveKillsWithUS[killerID] == 20)
                {
                    bz_sendTextMessagef(BZ_SERVER, BZ_ALLUSERS, "Holy cow! %s is pwning with the useless flag",
                        bz_getPlayerByIndex(killerID)->callsign.c_str());
                }
            }

            // Reset the victim's counter since their spree has ended
            consecutiveKills[victimID] = 0;
            consecutiveKillsWithUS[victimID] = 0;
        }
        break;

        case bz_ePlayerJoinEvent: // This event is called each time a player joins the game
        {
            bz_PlayerJoinPartEventData_V1* joinData = (bz_PlayerJoinPartEventData_V1*)eventData;

            // Set the player's counters to 0 in case they have not been set or they were set by a previous player
            consecutiveKills[joinData->playerID] = 0;
            consecutiveKillsWithUS[joinData->playerID] = 0;
        }
        break;

        default: break;
    }
}
