/* Double score plugin - extra 1 point for kill, including selfkill
 */
 
#include "bzfsAPI.h"

using namespace std;

class Double : public bz_Plugin {

  virtual const char* Name() {
    return "Double Point";
  }
  
  virtual void Init(const char */*config*/);

  virtual void Event(bz_EventData */*eventData*/);
  
  ~Double();
  
  virtual void Cleanup(void) {
    Flush();
  }
};

BZ_PLUGIN(Double)

void Double::Init(const char* arg) {
  bz_RegisterCustomFlag("DP", "Double Point", "Score an extra point for a kill, including suicide.", 0, eGoodFlag);
  Register(bz_ePlayerDieEvent);
}

Double::~Double() {
}

void Double::Event(bz_EventData *ed) {
  if (ed->eventType == bz_ePlayerDieEvent) {
    bz_PlayerDieEventData_V1 *d = static_cast<bz_PlayerDieEventData_V1 *>(ed);
    if (d->flagKilledWith == "DP") {
      int k = d->killerID;
      // Use the number of points the flag should deliver - minus one
      // Special behaviour for selfkill to do with order of score change
      if (k == d->playerID)
        bz_setPlayerWins(k, bz_getPlayerWins(k) - 1);
      else
        bz_setPlayerWins(k, bz_getPlayerWins(k) + 1);
    }
  }
}