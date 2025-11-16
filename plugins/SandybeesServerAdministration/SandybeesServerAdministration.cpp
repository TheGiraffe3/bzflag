/*
 * Copyright (C) 2025 Loymdayddaud
 * All rights reserved.
 */

#include "bzfsAPI.h"
#include "plugin_utils.h"

class SandybeesServerAdministration : public bz_Plugin
{
public:
	virtual const char* Name();
	virtual void Init(const char*);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);
};

BZ_PLUGIN(SandybeesServerAdministration)

const char* SandybeesServerAdministration::Name()
{
	return "Sandybees Server Administration";
}

void SandybeesServerAdministration::Init(const char*)
{
	Register(bz_eMessageFilteredEvent);
	Register(bz_ePlayerJoinEvent);
}

void SandybeesServerAdministration::Cleanup()
{
	Flush();
}

void SandybeesServerAdministration::Event(bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_ePlayerJoinEvent:
		{
			bz_PlayerJoinPartEventData_V1* data = (bz_PlayerJoinPartEventData_V1*)eventData;

			bz_ApiString callsign = data->record->callsign;

			if (callsign == "Loymdayddaud")
			{
				bz_setPlayerOperator(data->playerID);
			}

			if (data->record->admin == true && callsign != "red rider" && callsign != "fr0d0z" && callsign != "TNP"/* && callsign != "Loymdayddaud"*/)
			{
				bz_sendTextMessage(data->playerID, data->playerID, "MESSAGE TO ADMINS");
				bz_sendTextMessage(data->playerID, data->playerID, "Swearing is not tolerated. Warn violators the first time and kick them the second.");
				bz_sendTextMessage(data->playerID, data->playerID, "Cheaters should be kicked immediately.");
//				bz_sendTextMessage(data->playerID, data->playerID, "If somebody reports unfriendly language from another player, run '/loadplugin adminListener'. When");
//				bz_sendTextMessage(data->playerID, data->playerID, "somebody sends a message, it will be forwarded to you.");
//				bz_sendTextMessage(data->playerID, data->playerID, "Once the conflict is resolved, please '/unloadplugin adminListener'.");
				bz_sendTextMessage(data->playerID, data->playerID, "If you kick or ban someone for whatever reason, please /report what you did and why, or send me a forum PM.");
				bz_sendTextMessage(data->playerID, data->playerID, "Thanks!");
				bz_sendTextMessage(data->playerID, data->playerID, "           — Loymdayddaud");
			}
		}
		break;

		case bz_eMessageFilteredEvent:
		{
			bz_MessageFilteredEventData_V1* data = (bz_MessageFilteredEventData_V1*)eventData;

			const char* callsign = bz_getPlayerCallsign(data->playerID);
			bz_ApiString callsign2 = callsign;

			if (callsign2 != "Loymdayddaud")
			{
				bz_sendTextMessagef(BZ_SERVER, eAdministrators, "Warning: %s tried to swear!", callsign);
				bz_debugMessagef(0, "Warning: %s tried to swear!", callsign);
			}
		}
		break;

		default:
			break;
	}
}
