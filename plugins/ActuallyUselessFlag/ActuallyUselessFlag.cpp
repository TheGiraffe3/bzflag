#include "bzfsAPI.h"
#include "plugin_utils.h"

class ActuallyUselessFlag : public bz_Plugin
{
public:
	virtual const char* Name();
	virtual void Init(const char* config);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);
};

BZ_PLUGIN(ActuallyUselessFlag)

const char* ActuallyUselessFlag::Name()
{
	return "ActuallyUselessFlag";
}

void ActuallyUselessFlag::Init(const char* config)
{
	bz_RegisterCustomFlag("AU", "Actually Useless", "The Useless flag places mines. The Actually Useless flag does not.", 0, eGoodFlag);
}

void ActuallyUselessFlag::Cleanup()
{
	Flush();
}
