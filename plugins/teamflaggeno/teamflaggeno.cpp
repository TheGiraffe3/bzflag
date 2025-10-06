// teamflaggeno.cpp : Defines the entry point for the DLL application.

#include <map>
#include <queue>
#include <string>

#include "bzfsAPI.h"

//BZ_GET_PLUGIN_VERSION

class TeamflagGenoHandler;

class TeamflagGenoHandler : public bz_Plugin//bz_EventHandler
{
  bool friendlyFire;
  bool includeRogues;

public:
  TeamflagGenoHandler():friendlyFire(false),includeRogues(false){}
  virtual void Init ( const char* config){

    /*    std::string nice
    for(int i = 0; commandLine != NULL && commandLine[i] != '\0'; i++)
      {
	if(commandLine[i] == 'f' || commandLine[i] == 'F')
	  {
	    teamflaggeno.friendlyFire = true;
	  }
	else if (commandLine[i] == 'r' || commandLine[i] == 'R')
	  {
	    teamflaggeno.includeRogues = true;
	  }
	  }*/
    
    //    bz_registerEvent(bz_ePlayerDieEvent, &teamflaggeno);
    Register( bz_ePlayerDieEvent );    

    //bz_debugMessage(4,"teamflaggeno plugin loaded");
  }
  
  virtual const char* Name (){
    return "teamflaggeno";
  }

  virtual void Event( bz_EventData *eventData );

  void Cleanup(){
    bz_debugMessage(4,"teamflaggeno plugin cleaned up");
  }
};




void TeamflagGenoHandler::Event(bz_EventData *eventData)
{
  if (eventData->eventType != bz_ePlayerDieEvent)
    return;

  bz_PlayerDieEventData_V1* diedata = (bz_PlayerDieEventData_V1*)eventData;

      if(((diedata->flagKilledWith == "R*" && diedata->killerTeam == eRedTeam) || 
	  (diedata->flagKilledWith == "G*" && diedata->killerTeam == eGreenTeam) || 
	  (diedata->flagKilledWith == "B*" && diedata->killerTeam == eBlueTeam) || 
	  (diedata->flagKilledWith == "P*" && diedata->killerTeam == ePurpleTeam)) &&
	(friendlyFire || diedata->team != diedata->killerTeam) &&
	(includeRogues || diedata->team != eRogueTeam))
      {
	std::string msg = "Teammate ";
	bz_BasePlayerRecord* victim = bz_getPlayerByIndex(diedata->playerID);
	if(victim != NULL)
	{
	  if(victim->callsign != "")
	  {
	    msg += victim->callsign.c_str();
	    msg += ' ';
	  }
	  bz_freePlayerRecord(victim);
	}

	msg += "shot with ";
	msg += diedata->flagKilledWith.c_str();

	bz_BasePlayerRecord* killer = bz_getPlayerByIndex(diedata->killerID);
	if(killer != NULL)
	{
	  if(killer->callsign != "")
	  {
	    msg += " by ";
	    msg += killer->callsign.c_str();
	  }
	  bz_freePlayerRecord(killer);
	}

	bz_APIIntList* pl = bz_newIntList();
	bz_getPlayerIndexList(pl);

	for (unsigned int x = 0; x < pl->size(); x++)
	{
	  bz_BasePlayerRecord* pr = bz_getPlayerByIndex(pl->get(x));

	  if (pr != NULL)
	  {
	    if(pr->team == diedata->team &&  pr->spawned)
	    {
	      bz_sendTextMessage(pr->playerID, pr->playerID, msg.c_str());
	      bz_killPlayer(pr->playerID, false, diedata->killerID, "");
	    }
	    bz_freePlayerRecord(pr);
	  }
	}

	bz_deleteIntList(pl);
      }
}

BZ_PLUGIN(TeamflagGenoHandler)
