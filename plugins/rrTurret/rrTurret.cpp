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
  virtual const char* Name(){return "Turret-red-rider";}
  virtual void Init (const char* commandLine) {
    Register(bz_ePlayerUpdateEvent);
    Register(bz_eTickEvent);
    Register(bz_ePlayerDieEvent);
    bz_debugMessage(4,"turret plugin loaded");
  }
  virtual void Event(bz_EventData *eventData );
  virtual void Cleanup (void) {
    Flush();
    bz_debugMessage(4,"turret plugin unloaded");
  }
  int playerinturret=-1;//the player in the turret. -1 for no one
  float firepos[3]={0,0,23};//the position from which to fire the turret
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
      if(pointIn(pos,10,-10,10,-10,10,23))
      {//They are in the turret.
        bz_fireWorldWep ( "L", (float)bz_getBZDBDouble("_reloadTime")*2, BZ_SERVER,firepos, (-(pos[0]+350))/10, ang, 0 , 0.0f, eBlueTeam );
        //fire the turret! Get direction from tank position and angle
        if(playerinturret != -1 && playerinturret != playerID)
          {//If the last player in the turret was not ourselves, or no one. That means that two people are inside the turret
            if (playerSpawned(playerinturret)) {//make sure the person still in the turret is not NR or dead
              bz_killPlayer(playerID,0,BZ_SERVER);//Kill them! only one person in the turret!
              bz_sendTextMessage(BZ_SERVER,playerID,"Only one tank can go inside the turret!");
            }
            else playerinturret=playerID;//otherwise - if the player in the turret is not alive, or nr, then we set the playerID to ourself
          }
          else playerinturret=playerID;//same - duplicate else.
      }
      else if(playerinturret==playerID) {playerinturret=-1;}
      //If the last player in the turret was us, but now we are not inside the turret, set the playerinturret to no one (-1)

      //This is the eject button
      if(pointIn(pos,-10,10,-10,10,26,32) && playerinturret != -1) {
        int killer=playerID;//the killer
        std::string callsign=std::string("UNKNOWN");//If they have no callsign, set it to UNKNOWN
        std::string message;
        if (playerSpawned(killer)==false) {killer=BZ_SERVER;}
        else { callsign = getCallsign(killer);} //Get callsign of killer.
        message = std::string("You were ejected from the turret by ")+callsign;//Make a message to send to the ejected player.
        
        //Make sure that the player to be killed is not already dead.
        if (playerSpawned(playerinturret)) {//If they are not dead, eject from the turret
          bz_killPlayer(playerinturret,0,killer);//Kill player in the turret
          bz_sendTextMessage(BZ_SERVER,playerinturret,message.c_str());//Send them a message
          bz_sendTextMessage(BZ_SERVER,playerID,"You ejected the tank out of the turret.");//Tell the ejector about it.
        }
        playerinturret=-1;//reset the player in the turret - they've been ejected.
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
