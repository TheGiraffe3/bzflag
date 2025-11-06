#include "bzfsAPI.h"
#include "plugin_utils.h"

class ActuallyUselessFlag : public bz_Plugin
{
public:
    virtual const char* Name();
    virtual void Init(const char*);
    virtual void Cleanup();
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
