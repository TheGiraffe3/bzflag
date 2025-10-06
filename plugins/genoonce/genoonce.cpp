// genoonce.cpp : Defines the entry point for the DLL application.
//

#include "bzfsAPI.h"

class genoonceHandler : public bz_Plugin
{
public:
  virtual const char* Name (){return "Geno Once";}
  virtual void Init ( const char* config);
  virtual void Event(bz_EventData *eventData);
  virtual void Cleanup ();
};

BZ_PLUGIN(genoonceHandler);

void genoonceHandler::Init(const char* /*commandLine*/)
{
  Register(bz_ePlayerDieEvent);

  bz_debugMessage(4,"genoonce plugin loaded");
}

void genoonceHandler::Event(bz_EventData *eventData)
{
  bz_PlayerDieEventData_V1* diedata = (bz_PlayerDieEventData_V1*)eventData;

  if(diedata->flagKilledWith == "G")
  {
    // Make sure they're still holding Genocide.
    const char* f = bz_getPlayerFlag(diedata->killerID);
    if(f != NULL)
    {
      std::string flag = f;
      if(flag == "G")
      {
	bz_removePlayerFlag(diedata->killerID);
      }
    }
  }
}


void genoonceHandler::Cleanup()
{
  Flush();

  bz_debugMessage(4,"genoonce plugin unloaded");

}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
