#include "bzfsAPI.h"
#include "plugin_utils.h"

class ActuallyUselessFlag : public bz_Plugin
{
public:
    virtual const char* Name();
    virtual void Init(const char*);
    virtual void Cleanup();
    virtual void Event(bz_EventData* eventData);
};

BZ_PLUGIN(ActuallyUselessFlag)

const char* ActuallyUselessFlag::Name()
{
    return "ActuallyUselessFlag";
}

void ActuallyUselessFlag::Init(const char*)
{
    Register(bz_eFlagGrabbedEvent);

    bz_RegisterCustomFlag("AU", "Actually Useless", "The Useless flag places mines. The Actually Useless flag does not.", 0, eGoodFlag);
}

void ActuallyUselessFlag::Cleanup()
{
    Flush();
}

void ActuallyUselessFlag::Event(bz_EventData* eventData)
{
    switch (eventData->eventType)
    {
        case bz_eFlagGrabbedEvent:
        {
            // This event is called each time a flag is grabbed by a player
            bz_FlagGrabbedEventData_V1* data = (bz_FlagGrabbedEventData_V1*)eventData;
            // bz_sendTextMessage(BZ_SERVER, data->playerID, "Looks like someone grabbed the AU flag.");
            // Data
            // ----
            // (int)         playerID  - The player that grabbed the flag
            // (int)         flagID    - The flag ID that was grabbed
            // (const char*) flagType  - The flag abbreviation of the flag that was grabbed
            // (float[3])    pos       - The position at which the flag was grabbed
            // (double)      eventTime - This value is the local server time of the event.
        }
        break;

        default:
            break;
    }
}
