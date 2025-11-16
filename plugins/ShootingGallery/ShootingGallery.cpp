// Turret.cpp

#include "bzfsAPI.h"

bool pointIn(float pos[3], float xmin, float xmax, float ymin, float ymax, float zmin, float zmax)
{//Takes a position and xmin, etc. Returns true if pos is within the xmin xmax ymin etc vars
  //return false if they are the wrong place on any axis
  if ( pos[0] > xmax || pos[0] < xmin )
    return false;
  if ( pos[1] > ymax || pos[1] < ymin )
    return false;
  if ( pos[2] > zmax || pos[2] < zmin )
    return false;
  //if it passes all the tests, return true
  return true;
}

bool checkflag(int playerID)
{//if they go in a powerup thing, it checks if they already have a flag or not.
  bz_BasePlayerRecord  *updateData;//get data
  updateData  = bz_getPlayerByIndex(playerID);
  if(!updateData || updateData->currentFlag=="CLoaking (+CL)" || updateData->currentFlag=="STealth (+ST)" || updateData->currentFlag=="Guided Missile (+GM)" || updateData->currentFlag=="SteamRoller (+SR)" || updateData->currentFlag=="Shock Wave (+SW)" || updateData->currentFlag=="Tiny (+T)" || !updateData->spawned){bz_freePlayerRecord(updateData);return 0;}//Give them no powerup if they are already holding a flag.
  bz_freePlayerRecord(updateData);
  return 1;//if they have no flag, return true
}

int givepowerup(int playerID) {
  int random=rand()%7;//pick a random number
  switch(random) {//and convert it to a flag
    //give different flags, send a message with the name of the flag. Some flags have time limits, if so, set them.
    case 0:bz_givePlayerFlag(playerID,"CL",0);bz_sendTextMessage(BZ_SERVER,playerID,"Invisibility Acquired");break;
    case 1:bz_givePlayerFlag(playerID,"ST",0);bz_sendTextMessage(BZ_SERVER,playerID,"Hacker Mode Enabled");break;
    case 2:bz_givePlayerFlag(playerID,"GM",0);bz_sendTextMessage(BZ_SERVER,playerID,"Super Missiles Acquired");break;
    case 3:bz_givePlayerFlag(playerID,"SR",0);bz_sendTextMessage(BZ_SERVER,playerID,"Death Ball Acquired");break;
    case 4:bz_givePlayerFlag(playerID,"SW",0);bz_sendTextMessage(BZ_SERVER,playerID,"Power Bombs Acquired");break;
    case 5:bz_givePlayerFlag(playerID,"T",0);bz_sendTextMessage(BZ_SERVER,playerID,"Invulnerability Acquired");break;
    case 6:bz_givePlayerFlag(playerID,"CL",0);bz_sendTextMessage(BZ_SERVER,playerID,"Invisibility Acquired");break;
  }
  return random;
}
//
void removeFlagOnMatch(int player) {
  bz_BasePlayerRecord  *updateData;//Check that the player is still here - don't yank a flag from someone who left. It causes segfaults
  int i = player;
  updateData = bz_getPlayerByIndex(i);
  if(updateData) {//Pull the flag, send a message depending on what flag it is
    if(updateData->currentFlag=="CLoaking (+CL)"){bz_removePlayerFlag(i);bz_sendTextMessage(BZ_SERVER,i,"Invisibility Lost");}
    else if(updateData->currentFlag=="STealth (+ST)"){bz_removePlayerFlag(i);bz_sendTextMessage(BZ_SERVER,i,"Hacker Mode Disabled");}
    else if(updateData->currentFlag=="Guided Missiles (+GM)"){bz_removePlayerFlag(i);bz_sendTextMessage(BZ_SERVER,i,"Super Missiles Lost");}
    else if(updateData->currentFlag=="SteamRoller (+SR)"){bz_removePlayerFlag(i);bz_sendTextMessage(BZ_SERVER,i,"Death Ball Lost");}
    else if(updateData->currentFlag=="ShockWave (+SW)"){bz_removePlayerFlag(i);bz_sendTextMessage(BZ_SERVER,i,"Power Bombs Lost");}
    else if(updateData->currentFlag=="Tiny (+T)"){bz_removePlayerFlag(i);bz_sendTextMessage(BZ_SERVER,i,"Invulnerability Lost");}
  }
  bz_freePlayerRecord(updateData);
}
//
bool playerSpawned(int player) {
  bool spawned = false;
  bz_BasePlayerRecord *pr;
  pr  = bz_getPlayerByIndex(player);
  if(pr && pr->spawned) {spawned=true;}
  bz_freePlayerRecord(pr);
  return spawned;
}
//
std::string getCallsign(int player) {
  std::string callsign = std::string("UNKNOWN");// Set it by default to UNKNOWN
  bz_BasePlayerRecord  *updateData;
  updateData  = bz_getPlayerByIndex(player);
  if(updateData) {callsign = updateData->callsign.c_str();}//get the callsign. Replaces UNKNOWN if they have a callsign
  bz_freePlayerRecord(updateData);//Free
  return callsign;
}
//
class Turret : public bz_Plugin
{
public:
  virtual const char* Name(){return "Turret";}
  virtual void Init (const char* commandLine) {
    Register(bz_ePlayerUpdateEvent);
    Register(bz_eTickEvent);
    Register(bz_ePlayerDieEvent);
    bz_debugMessage(4,"turret plugin loaded");
    bz_registerCustomBZDBDouble("_powerupRechargeTime", 100);
  }
  virtual void Event(bz_EventData *eventData );
  virtual void Cleanup (void) {
    Flush();
    bz_debugMessage(4,"turret plugin unloaded");
  }
  int poweruptimers[6]={-1,-1,-1,-1,-1,-1};//power up timers for each of the three power up thingys
  int playerpoweruptimers[256]={-1};//times how long a player can have a powerup. If set, it will remove their flag when it reaches 0
  int playerinturret=-1;//the player in the turret. -1 for no one
  int alreadysentmessage[256]={0};//if each player has already received a "power up charging" message.
  float firepos[3]={-335,0,46.57};//the position from which to fire the turret
};

BZ_PLUGIN(Turret)

void Turret::Event(bz_EventData *eventData ){
  switch (eventData->eventType) {
    case bz_ePlayerUpdateEvent: {
      //Player update event. Check positions
      float pos[3] = {0};//position (x,y,z)
      float ang=0;//tank angle
      int playerID = -1;
      //get the position, playerID
      for (int i=0;i<=2;i++) { pos[i] = ((bz_PlayerUpdateEventData_V1*)eventData)->lastState.pos[i]; }
      ang = ((bz_PlayerUpdateEventData_V1*)eventData)->lastState.rotation;
      playerID = ((bz_PlayerUpdateEventData_V1*)eventData)->playerID;
      //get position and playerID

      //in turret
      if(pointIn(pos,-365,-335,-7,7,44,50))
      {//They are in the turret.
        bz_fireWorldWep ( "L", (float)bz_getBZDBDouble("_reloadTime")*2, BZ_SERVER,firepos, (-(pos[0]+350))/10, ang, 0 , 0.0f );
        //fire the turret! Get direction from tank position and angle
        if(playerinturret != -1 && playerinturret != playerID)
          {//If the last player in the turret was not ourselves, or no one. That means that two people are inside the turret
            if (playerSpawned(playerinturret)) {//make sure the person still in the turret is not NR or dead
              bz_killPlayer(playerID,0,BZ_SERVER);//Kill them! only one person in the turret!
              bz_sendTextMessage(BZ_SERVER,playerID,"Only one tank can go inside the turret!");
            }
            else playerinturret=playerID;//otherwise - if the player in the turret is not alive, or nr, then we set the playerID to ourself
            bz_sendTextMessage(BZ_SERVER,playerID,"Change the turret's vertical direction by driving forwards and backwards.");
            bz_sendTextMessage(BZ_SERVER,playerID,"Aim the turret by turning your tank.");
            bz_sendTextMessage(BZ_SERVER,playerID,"Warning! If someone gets behind the turret on the caution pad, you will be killed!");
          }
          else playerinturret=playerID;//same - duplicate else.
      }
      else if(playerinturret==playerID) {playerinturret=-1;}
      //If the last player in the turret was us, but now we are not inside the turret, set the playerinturret to no one (-1)

      //This is the eject button
      if(pointIn(pos,-400,-385,-7,7,25,35) && playerinturret != -1) {
        int killer=playerID;//the killer
        std::string callsign=std::string("UNKNOWN");//If they have no callsign, set it to UNKNOWN
        std::string callsign2=std::string("UNKNOWN");
        std::string message;
        std::string message2;
        if (playerSpawned(killer)==false) {killer=BZ_SERVER;}
        else { callsign = getCallsign(killer);} //Get callsign of killer.
        if (playerinturret) {callsign2 = getCallsign(playerinturret);}
        message = std::string("You were ejected from the turret by ")+callsign;//Make a message to send to the ejected player.
        message2 = std::string("You ejected "+callsign2+" from the turret.");

        //Make sure that the player to be killed is not already dead.
        if (playerSpawned(playerinturret)) {//If they are not dead, eject from the turret
          bz_killPlayer(playerinturret,0,killer);//Kill player in the turret
          bz_sendTextMessage(BZ_SERVER,playerinturret,message.c_str());//Send them a message
          bz_sendTextMessage(BZ_SERVER,playerID,message2.c_str());//Tell the ejector about it.
        }
        playerinturret=-1;//reset the player in the turret - they've been ejected.
      }

      //in a power up thing
      int powerup = -1;
      if (pointIn(pos,215,225,-5,5,30,49)) { powerup=0;}
      else if (pointIn(pos,255,275,115,135,0,49)) { powerup=1;}
      else if (pointIn(pos,255,275,-135,-115,0,49)) { powerup=2;}
      else if (pointIn(pos,-80,-70,-5,5,0,25)) { powerup=3;}
      else if (pointIn(pos,-155,-145,-70,-60,0,25)) { powerup=4;}
      else if (pointIn(pos,-155,-145,60,70,0,25)) { powerup=5;}
      else { alreadysentmessage[playerID]=0;} //if they are not inside a powerup thing, reset the message-already-sent-variable

      if (powerup != -1) {//If they are in a power up thing
        if (checkflag(playerID)) {
          if(poweruptimers[powerup]==-1) {//and it has recharged fully
            int power = givepowerup(playerID);//then give them  a flag
            if (power != 2 && power != 4) {playerpoweruptimers[playerID]=12000;}// GM and SW have shot limits, but no timers.
            poweruptimers[powerup]=bz_getBZDBDouble("_powerupRechargeTime");//set the recharge timer for the powerup thingy
          }
          else if(alreadysentmessage[playerID]==0) {
            //if not recharged (and they have not already been sent this message) tell them its recharging
            bz_sendTextMessage(playerID,playerID,"No powerups available. Recharging...");
            alreadysentmessage[playerID]=1;//make sure they dont receive the message again
          }
        }
      }
    }break;

    case bz_eTickEvent: { //Tick event. Used to count down timers
      for(int a=0;a<=2;a++) { if(poweruptimers[a]>-1) poweruptimers[a]-=1;}//Count down powerup timers

      for(int i=0;i<256;i++) {//Count down the flag timers for each player
        if(playerpoweruptimers[i] > -1) {// -1 means off. If it's above -1 then count down
          playerpoweruptimers[i]-=1;//counting down
          if(playerpoweruptimers[i]==0) {//It hit 0, so pull their flag
            playerpoweruptimers[i]=-1;//Set to -1 (turn timer off)
            removeFlagOnMatch(i);
          }
        }
      }
    }break;

    case bz_ePlayerDieEvent: {
      bz_PlayerDieEventData_V2* dieData = (bz_PlayerDieEventData_V2*)eventData;
      if(dieData->flagKilledWith=="L" && playerinturret != -1 && dieData->killerID != playerinturret)
      {//If a player was killed by laser, change the killerID so the player in the turret gets a point.
        dieData->killerID=playerinturret;
      }
   }break;

    default:{
    }break;
  }
}

