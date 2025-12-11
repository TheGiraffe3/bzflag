#include "bzfsAPI.h"

// Takes a position and xmin, etc. Returns true if pos is within the xmin xmax ymin etc vars.
bool pointIn(float pos[3], float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
{
	if ( pos[0] > xmax || pos[0] < xmin )
		return false;
	if ( pos[1] > ymax || pos[1] < ymin )
		return false;
	if ( pos[2] > zmax || pos[2] < zmin )
		return false;
	return true;
}

// If they go in a powerup thing, this function checks if they already have a flag or not.
bool checkflag(int playerID)
{
	bz_BasePlayerRecord *updateData;//get data
	updateData = bz_getPlayerByIndex(playerID);
	if(!updateData || updateData->currentFlag=="CLoaking (+CL)" || updateData->currentFlag=="STealth (+ST)" || updateData->currentFlag=="Guided Missile (+GM)" || updateData->currentFlag=="SteamRoller (+SR)" || updateData->currentFlag=="Shock Wave (+SW)" || updateData->currentFlag=="Tiny (+T)" || !updateData->spawned)
	{
		bz_freePlayerRecord(updateData);
		return 0;
	}
	// If they are already holding a flag, don't give them a powerup.
	bz_freePlayerRecord(updateData);
	return 1;
}

int givepowerup(int playerID)
{
	int random = rand()%7;
	switch(random)
	{
		case 0:
			bz_givePlayerFlag(playerID, "CL", 0);
			bz_sendTextMessage(BZ_SERVER, playerID, "Invisibility Acquired");
			break;
		case 1:
			bz_givePlayerFlag(playerID, "ST", 0);
			bz_sendTextMessage(BZ_SERVER, playerID, "Hacker Mode Enabled");
			break;
		case 2:
			bz_givePlayerFlag(playerID, "GM", 0);
			bz_sendTextMessage(BZ_SERVER, playerID, "Super Missiles Acquired");
			break;
		case 3:
			bz_givePlayerFlag(playerID, "SR", 0);
			bz_sendTextMessage(BZ_SERVER, playerID, "Death Ball Acquired");
			break;
		case 4:
			bz_givePlayerFlag(playerID, "SW", 0);
			bz_sendTextMessage(BZ_SERVER, playerID, "Power Bombs Acquired");
			break;
		case 5:
			bz_givePlayerFlag(playerID, "T", 0);
			bz_sendTextMessage(BZ_SERVER, playerID, "Invulnerability Acquired");
			break;
		case 6:
			bz_givePlayerFlag(playerID, "CL", 0);
			bz_sendTextMessage(BZ_SERVER, playerID, "Invisibility Acquired");
			break;
	}
	return random;
}

void removeFlagOnMatch(int player)
{
	// Check that the player is still here - don't try to yank a flag from someone who left. It causes segfaults.
	bz_BasePlayerRecord *updateData;
	int i = player;
	updateData = bz_getPlayerByIndex(i);
	// Pull the flag, send a message depending on what flag it is
	if(updateData)
	{
		if(updateData->currentFlag == "CLoaking (+CL)")
		{
			bz_removePlayerFlag(i);
			bz_sendTextMessage(BZ_SERVER, i, "Invisibility Lost");
		}
		else if(updateData->currentFlag == "STealth (+ST)")
		{
			bz_removePlayerFlag(i);
			bz_sendTextMessage(BZ_SERVER, i, "Hacker Mode Disabled");
		}
		else if(updateData->currentFlag == "Guided Missiles (+GM)")
		{
			bz_removePlayerFlag(i);
			bz_sendTextMessage(BZ_SERVER, i, "Super Missiles Lost");
		}
		else if(updateData->currentFlag == "SteamRoller (+SR)")
		{
			bz_removePlayerFlag(i);
			bz_sendTextMessage(BZ_SERVER, i, "Death Ball Lost");
		}
		else if(updateData->currentFlag == "ShockWave (+SW)")
		{
			bz_removePlayerFlag(i);
			bz_sendTextMessage(BZ_SERVER, i, "Power Bombs Lost");
		}
		else if(updateData->currentFlag == "Tiny (+T)")
		{
			bz_removePlayerFlag(i);
			bz_sendTextMessage(BZ_SERVER, i, "Invulnerability Lost");
		}
	}
	bz_freePlayerRecord(updateData);
}

bool playerSpawned(int player) {
	bool spawned = false;
	bz_BasePlayerRecord *pr = bz_getPlayerByIndex(player);
	if (pr && pr->spawned)
		spawned = true;
	bz_freePlayerRecord(pr);
	return spawned;
}

std::string getCallsign(int player) {
	std::string callsign = std::string("UNKNOWN"); // By default, set to UNKNOWN.
	bz_BasePlayerRecord *updateData = bz_getPlayerByIndex(player);
	if(updateData)
		// If they have a callsign, use that instead of UNKNOWN.
		callsign = updateData->callsign.c_str();
	bz_freePlayerRecord(updateData);
	return callsign;
}

class Turret : public bz_Plugin
{
public:
	virtual const char* Name();
	virtual void Init (const char*);
	virtual void Cleanup ();
	virtual void Event(bz_EventData *eventData);

	int poweruprechargetimers[6] = {-1};

	// Sets how long a player can have a powerup for. If set, it will remove their flag when it reaches 0.
	int playerpoweruptimers[256] = {-1};

	int playerinturret = -1;

	int alreadysentmessage[256] = {0};

	// The position from which to fire the turret.
	float firepos[3] = {-335, 0, 46.57};
};

BZ_PLUGIN(Turret)

const char* Turret::Name()
{
	return "Shooting Gallery";
}

void Turret::Init(const char*)
{
	Register(bz_ePlayerUpdateEvent);
	Register(bz_eTickEvent);
	Register(bz_ePlayerDieEvent);
	bz_registerCustomBZDBDouble("_powerupRechargeTime", 100);
}

void Turret::Cleanup()
{
	Flush();
}

void Turret::Event(bz_EventData *eventData ){
	switch (eventData->eventType)
	{
		case bz_ePlayerUpdateEvent:
		{
			bz_PlayerUpdateEventData_V1* data = (bz_PlayerUpdateEventData_V1*)eventData;

			float pos[3] = {0};
			float ang=0;

			for (int i=0;i<=2;i++)
				pos[i] = data->lastState.pos[i];
			ang = data->lastState.rotation;

			int playerID = data->playerID;

			if (pointIn(pos, -365, -335, -7, 7, 44, 50)) // If they are in the turret.
			{
				// Fire the turret! Get direction from tank position and angle.
				bz_fireWorldWep ( "L", (float)bz_getBZDBDouble("_reloadTime")*2, BZ_SERVER, firepos, (-(pos[0]+350))/10, ang, 0 , 0.0f );
				// If the last player in the turret was not ourselves, or no one, then two people are inside the turret.
				if(playerinturret != -1 && playerinturret != playerID)
				{
					// Make sure the person currently in the turret is not NR or dead.
					if (playerSpawned(playerinturret))
					{
						bz_killPlayer(playerID, 0, BZ_SERVER); // Kill them! Only one person in the turret!
						bz_sendTextMessage(BZ_SERVER, playerID, "Only one tank can go inside the turret!");
					}
					 // Otherwise, if the player in the turret is not alive, or is NR, then we set the playerID to ourself.
					else
						playerinturret = playerID;
					bz_sendTextMessage(BZ_SERVER, playerID, "Change the turret's vertical direction by driving forwards and backwards.");
					bz_sendTextMessage(BZ_SERVER, playerID, "Aim the turret by turning your tank.");
					bz_sendTextMessage(BZ_SERVER, playerID, "Warning! If someone gets behind the turret on the caution pad, you will be killed!");
				}
				else playerinturret = playerID; // same - duplicate else.
			}

			// If the last player in the turret was us, but now we are not inside the turret, set the playerinturret to no one (-1)
			else if (playerinturret == playerID)
				playerinturret = -1;

			if (pointIn(pos, -400, -385, -7, 7, 25, 35) && playerinturret != -1) // If they are on the eject button.
			{
				int killer = playerID;

				// By default, the callsigns should be UNKNOWN.
				std::string callsign = std::string("UNKNOWN");
				std::string callsign2 = std::string("UNKNOWN");

				if (playerSpawned(killer) == false)
					killer = BZ_SERVER;
				else
					callsign = getCallsign(killer); // Get callsign of killer.

				if (playerinturret)
					callsign2 = getCallsign(playerinturret);

				std::string message = std::string("You were ejected from the turret by ")+callsign;
				std::string message2 = std::string("You ejected ") + callsign2 + " from the turret.";

				if (playerSpawned(playerinturret))
				{
					bz_sendTextMessage(BZ_SERVER, playerinturret, message.c_str());
					bz_killPlayer(playerinturret, 0, killer);
					bz_sendTextMessage(BZ_SERVER, playerID, message2.c_str());
				}
				playerinturret = -1; // Reset the player in the turret - they've been ejected.
			}

			int powerup = -1;
			if (pointIn(pos, 215, 225, -5, 5, 30, 49))
				powerup = 0;
			else if (pointIn(pos, 255, 275, 115, 135, 0, 49))
				powerup = 1;
			else if (pointIn(pos, 255, 275, -135, -115, 0, 49))
				powerup = 2;
			else if (pointIn(pos, -80, -70, -5, 5, 0, 25))
				powerup = 3;
			else if (pointIn(pos, -155, -145, -70, -60, 0, 25))
				powerup = 4;
			else if (pointIn(pos, -155, -145, 60, 70, 0, 25))
				powerup = 5;
			else
				alreadysentmessage[playerID] = 0; // If they are not inside a powerup thing, reset the message-already-sent-variable.

			if (powerup != -1)
			{
				if (checkflag(playerID))
				{
					if(poweruprechargetimers[powerup] == -1)
					{
						int power = givepowerup(playerID);
						if (power != 2 && power != 4)
							playerpoweruptimers[playerID]=12000;// GM and SW have shot limits, but no timers.
						poweruprechargetimers[powerup] = bz_getBZDBDouble("_powerupRechargeTime");
					}
					else if(alreadysentmessage[playerID] == 0)
					{
						bz_sendTextMessage(playerID, playerID, "No powerups available. Recharging...");
						alreadysentmessage[playerID] = 1;
					}
				}
			}
		}
		break;

		case bz_eTickEvent:
		{
			for(int a=0;a<=5;a++)
			{
				if(poweruprechargetimers[a]>-1)
					poweruprechargetimers[a]-=1;
			}

			for(int i=0;i<256;i++)
			{
				if(playerpoweruptimers[i] > -1)
				{
					playerpoweruptimers[i] -= 1;
					if(playerpoweruptimers[i] == 0)
					{
						playerpoweruptimers[i] = -1;
						removeFlagOnMatch(i);
					}
				}
			}
		}
		break;

		case bz_ePlayerDieEvent:
		{
			bz_PlayerDieEventData_V2* dieData = (bz_PlayerDieEventData_V2*)eventData;

			if (dieData->flagKilledWith == "L" && playerinturret != -1 && dieData->killerID != playerinturret)
				// If a player was killed by laser, change the killerID so the player in the turret gets a point.
				dieData->killerID = playerinturret;
		}
		break;

		default:
			break;
	}
}
