/*
Bounty Hunter
    Copyright (C) 2015 Vladimir "allejo" Jimenez

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

#include "bzfsAPI.h"
#include "plugin_utils.h"

class BountyHunter : public bz_Plugin
{
public:
    virtual const char* Name () {return "Bounty Hunter";}
    virtual void Init (const char* config);
    virtual void Event (bz_EventData *eventData);
    virtual void Cleanup (void);

    double      flagCarryTime;          // The time (in server seconds) a team flag was dropped

    int         consecutiveKills[253],  // The array to keep the number of consecutive kills a player has made
                lastFlagCarrier;        // The player ID of the last person who dropped a team flag

    std::string teamFlagDropped;        // The abbreviation of the team flag that was dropped
};

BZ_PLUGIN(BountyHunter)

void BountyHunter::Init (const char* commandLine)
{
    // Register our events with Register()
    Register(bz_eFlagDroppedEvent);
    Register(bz_ePlayerDieEvent);
    Register(bz_ePlayerJoinEvent);

    // Setup default variables
    lastFlagCarrier = -1;
    flagCarryTime = -1;
    teamFlagDropped = "";
}

void BountyHunter::Cleanup (void)
{
    Flush(); // Clean up all the events
}

void BountyHunter::Event (bz_EventData *eventData)
{
    switch (eventData->eventType)
    {
        case bz_eFlagDroppedEvent: // This event is called each time a flag is dropped by a player.
        {
            bz_FlagDroppedEventData_V1* flagDropData = (bz_FlagDroppedEventData_V1*)eventData;

            // Get the abbreviation of the flag that was dropped to check if it was a team flag
            std::string flag = bz_getName(flagDropData->flagID).c_str();

            // If the dropped flag is a team flag, keep track of who dropped it and when
            if (flag == "R*" || flag == "G*" || flag == "B*" || flag == "P*")
            {
                lastFlagCarrier = flagDropData->playerID;
                teamFlagDropped = flag;
                flagCarryTime   = bz_getCurrentTime();
            }
        }
        break;

        case bz_ePlayerDieEvent: // This event is called each time a tank is killed.
        {
            bz_PlayerDieEventData_V1* dieData = (bz_PlayerDieEventData_V1*)eventData;
            int victimID = dieData->playerID;
            int killerID = dieData->killerID;

            // If the player did not kill themselves, then calculate the bounty points
            if (victimID != killerID)
            {
                // Increment the consecutive kills of the killer if the player is not the server (player ID 253)
                if (killerID != 253)
                {
                    consecutiveKills[killerID]++;
                }

                // If the person killed had more than 0 kills, calculate the bounty points
                if (consecutiveKills[victimID] > 0)
                {
                    // This value will keep track of how many levels of a rampage a player has gotten where each level
                    // is an increment of 6 kills
                    int rampageMultiplier = consecutiveKills[victimID] / 6;

                    // The rampage multiplier times two will be the amount of bounty points granted
                    int bountyPoints = rampageMultiplier * 2;

                    // Only reward bounty points if it's greater than 0
                    if (bountyPoints > 0)
                    {
                        // Set the player's new points and notify them
                        if (dieData->team == dieData->killerTeam && dieData->team != eRogueTeam)
                        {
                            int penalty = bountyPoints * 2;

                            bz_incrementPlayerLosses(killerID, penalty);
                            bz_sendTextMessagef(BZ_SERVER, killerID, "Stopping your teammate's rampage is in poor taste. You have received a penalty of %i points", bountyPoints);
                        }
                        else
                        {
                            bz_incrementPlayerWins(killerID, bountyPoints);
                            bz_sendTextMessagef(BZ_SERVER, killerID, "Stopping %s's rampage earned you %i bounty points.", bz_getPlayerByIndex(victimID)->callsign.c_str(), bountyPoints);
                        }
                    }
                }

                // If the person killed was carrying a team flag less than 0.2 seconds ago, then reward the killer
                if (lastFlagCarrier == victimID && flagCarryTime + 0.2 > bz_getCurrentTime())
                {
                    // Store the team color
                    std::string teamColor = "";

                    // Set the respective color judging by the team flag abbreviation
                    if      (teamFlagDropped == "R*") { teamColor = "red"; }
                    else if (teamFlagDropped == "G*") { teamColor = "green"; }
                    else if (teamFlagDropped == "B*") { teamColor = "blue"; }
                    else if (teamFlagDropped == "P*") { teamColor = "purple"; }

                    // Set the player's new points and notify them
                    if (dieData->team == dieData->killerTeam && dieData->killerTeam != eRogueTeam)
                    {
                        bz_incrementPlayerLosses(killerID, 4);
                        bz_sendTextMessagef(BZ_SERVER, killerID, "Killing your teammate is in poor taste. You have received a penalty of 4 points", teamColor.c_str());
                    }
                    else
                    {
                        bz_incrementPlayerWins(killerID, 2);
                        bz_sendTextMessagef(BZ_SERVER, killerID, "Shooting the %s team flag carrier has earned you 2 bounty points", teamColor.c_str());
                    }
                }
            }

            // Reset the consecutive kills made by the player who just got killed
            consecutiveKills[victimID] = 0;
        }
        break;

        case bz_ePlayerJoinEvent: // This event is called each time a player joins the game
        {
            bz_PlayerJoinPartEventData_V1* joinData = (bz_PlayerJoinPartEventData_V1*)eventData;

            // Set a player's consecutive kills to 0 when they join in case the array has a null value
            consecutiveKills[joinData->playerID] = 0;
        }
        break;

        default: break;
    }
}
