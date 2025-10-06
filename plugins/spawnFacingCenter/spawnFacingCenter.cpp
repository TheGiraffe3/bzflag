// spawnFacingCenter.cpp : Defines the entry point for the DLL application.
//
#define _USE_MATH_DEFINES

#include <math.h>

#include "bzfsAPI.h"

class spawnFacingCenter : public bz_Plugin
{
  virtual const char* Name (){return "Spawn Facing the Center";}
  virtual void Init (const char* config);

  virtual void Event (bz_EventData* eventData);
};

BZ_PLUGIN(spawnFacingCenter)

void spawnFacingCenter::Init (const char* /*commandLine*/)
{
  Register(bz_eGetPlayerSpawnPosEvent);
  
  bz_debugMessage(4, "spawnFacingCenter plugin loaded");
}

void spawnFacingCenter::Event(bz_EventData* eventData)
{
  if (eventData->eventType == bz_eGetPlayerSpawnPosEvent)
  {
    bz_GetPlayerSpawnPosEventData_V1* spawnPosData = (bz_GetPlayerSpawnPosEventData_V1*)eventData;
    
    if (spawnPosData->pos[0] != 0.0 || spawnPosData->pos[1] != 0.0) // Don't do anything if the spawn is dead center.
    {
      spawnPosData->rot = atan2(-spawnPosData->pos[1], -spawnPosData->pos[0]);
    }
  }
}
// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

