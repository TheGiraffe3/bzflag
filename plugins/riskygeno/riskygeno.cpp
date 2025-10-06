// cpp : Defines the entry point for the DLL application.

#include <map>
#include <queue>
#include <string>

#include "bzfsAPI.h"

class riskygenoHandler : public bz_Plugin
{
public:

  virtual const char* Name (){return "Risky Geno";}
  virtual void Init (const char* config);
  virtual void Event(bz_EventData *eventData);
  virtual void Cleanup ();
  void doWarn(int playerID);

  bool friendlyFire;
  bool includeRogues;
  bool includeServer;
  bool warnPlayers;

  struct GenoStatus
  {
    bool hasGeno;
    bool warn;
  };

  std::map<int, GenoStatus> genoInfo;

  // We need to delay the setting of hasGeno because clients drop flags
  // immediately before dying.
  struct DropTime
  {
    int playerID;
    double time;
  };

  std::queue<DropTime> dropTimes;

  std::string flag; //use this string repeatedly instead of making them.

};

BZ_PLUGIN(riskygenoHandler);

void riskygenoHandler::Init(const char* commandLine)
{
  friendlyFire = false;
  includeRogues = false;
  includeServer = false;
  warnPlayers = false;

  for(int i = 0; commandLine != NULL && commandLine[i] != '\0'; i++)
  {
    if(commandLine[i] == 'f' || commandLine[i] == 'F')
    {
      friendlyFire = true;
    }
    else if (commandLine[i] == 'r' || commandLine[i] == 'R')
    {
      includeRogues = true;
    }
    else if (commandLine[i] == 's' || commandLine[i] == 'S')
    {
      includeServer = true;
    }
    else if (commandLine[i] == 'w' || commandLine[i] == 'W')
    {
      warnPlayers = true;
    }
  }

  bz_APIIntList* pl = bz_newIntList();
  bz_getPlayerIndexList(pl);

  for (unsigned int x = 0; x < pl->size(); x++)
  {
    bz_BasePlayerRecord* pr = bz_getPlayerByIndex(pl->get(x));

    if (pr != NULL)
    {
      genoInfo[pr->playerID].hasGeno = (pr->currentFlag == "Genocide");
      genoInfo[pr->playerID].warn = warnPlayers;

      bz_freePlayerRecord(pr);
    }
  }

  bz_deleteIntList(pl);

  Register(bz_ePlayerDieEvent);
  Register(bz_ePlayerJoinEvent);
  Register(bz_ePlayerPartEvent);
  Register(bz_eFlagTransferredEvent);
  Register(bz_eFlagGrabbedEvent);
  Register(bz_eFlagDroppedEvent);
  Register(bz_eTickEvent);

  bz_debugMessage(4,"riskygeno plugin loaded");
}

void riskygenoHandler::Event(bz_EventData *eventData)
{
  switch (eventData->eventType)
  {
    case bz_ePlayerDieEvent:
    {

      bz_PlayerDieEventData_V1* diedata = (bz_PlayerDieEventData_V1*)eventData;

      if(genoInfo[diedata->playerID].hasGeno &&
	(friendlyFire || diedata->team != diedata->killerTeam) &&
	(includeRogues || diedata->team != eRogueTeam) &&
	(includeServer || diedata->killerID != BZ_SERVER) &&
	(diedata->team != eRogueTeam || diedata->killerTeam != eRogueTeam || (friendlyFire && includeRogues)) &&
	diedata->flagKilledWith != "G") // Don't override normal geno behavior
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

	msg += "killed while holding Genocide";

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
	genoInfo[diedata->playerID].hasGeno = false;
      }
    }
    break;

    case bz_ePlayerJoinEvent:
    case bz_ePlayerPartEvent:
    {
      bz_PlayerJoinPartEventData_V1* joinpartdata = (bz_PlayerJoinPartEventData_V1*)eventData;

      genoInfo[joinpartdata->playerID].hasGeno = false;
      genoInfo[joinpartdata->playerID].warn = warnPlayers;
    }
    break;

    case bz_eFlagTransferredEvent:
    {
      bz_FlagTransferredEventData_V1* flagtransferdata = (bz_FlagTransferredEventData_V1*)eventData;

      flag = flagtransferdata->flagType;
      if(flag == "G")
      {
	genoInfo[flagtransferdata->fromPlayerID].hasGeno = false;
	genoInfo[flagtransferdata->toPlayerID].hasGeno = true;
	doWarn(flagtransferdata->toPlayerID);
      }
    }
    break;

    case bz_eFlagGrabbedEvent:
    {
      bz_FlagGrabbedEventData_V1* flaggrabdata = (bz_FlagGrabbedEventData_V1*)eventData;

      flag = flaggrabdata->flagType;
      if(flag == "G")
      {
	genoInfo[flaggrabdata->playerID].hasGeno = true;
	doWarn(flaggrabdata->playerID);
      }
    }
    break;

    case bz_eFlagDroppedEvent:
    {
      bz_FlagDroppedEventData_V1* flagdropdata = (bz_FlagDroppedEventData_V1*)eventData;

      flag = flagdropdata->flagType;
      if(flag == "G")
      {
	// They're not in the clear for 1s.
	DropTime dt;
	dt.playerID = flagdropdata->playerID;
	dt.time = bz_getCurrentTime() + 1.0;
	dropTimes.push(dt);
      }
    }
    break;

    case bz_eTickEvent:
    {
      if(!dropTimes.empty() && dropTimes.front().time < bz_getCurrentTime())
      {
	genoInfo[dropTimes.front().playerID].hasGeno = false;
	dropTimes.pop();
      }
    }
  default:
    break;
  }
}

void riskygenoHandler::doWarn(int playerID)
{
  if(genoInfo[playerID].warn)
  {
    bz_sendTextMessage(BZ_SERVER, playerID, "Be careful! If you die while holding Genocide, your entire team will die!");
    genoInfo[playerID].warn = false;
  }
}

void riskygenoHandler::Cleanup()
{
  // Add any clean-up/shutdown stuff here.

  Flush();

  bz_debugMessage(4,"riskygeno plugin unloaded");

}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
