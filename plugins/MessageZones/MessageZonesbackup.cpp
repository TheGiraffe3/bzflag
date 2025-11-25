#include "bzfsAPI.h"
#include "plugin_utils.h"

struct MessageInfo {
	int zoneID;
	int playerID;
	bool messageSent{false};
};

class MessageZone : public bz_CustomZoneObject
{
public:
	MessageZone() : bz_CustomZoneObject()
	{
	}

	std::string message = "If you see this, /report because the map author forgot something.";
	std::string flag = "CL";
	int zoneID = 0;
};

class MessageZones : public bz_Plugin, bz_CustomMapObjectHandler
{
public:
	virtual const char* Name ()
	{
		return "Message Zones";
	}
	virtual void Init ( const char* );
	virtual void Event ( bz_EventData *eventData );
	virtual void Cleanup ( void );
	virtual bool MapObject (bz_ApiString object, bz_CustomMapObjectInfo *data);

	std::vector<MessageZone> messageZones;
	std::vector<std::pair<int, int>> playersWhoGotMessage;

	struct SentRecord {
		int playerID;
		int zoneID;
	};
	std::vector<SentRecord> alreadySent;
};

BZ_PLUGIN(MessageZones)

void MessageZones::Init (const char*)
{
	Register(bz_ePlayerUpdateEvent);
	Register(bz_ePlayerPartEvent);

	bz_registerCustomMapObject("MESSAGEZONE", this);
}

void MessageZones::Cleanup (void)
{
	Flush();

	bz_removeCustomMapObject("MESSAGEZONE");
}

bool MessageZones::MapObject (bz_ApiString object, bz_CustomMapObjectInfo *data)
{
	if (object != "MESSAGEZONE" || !data)
		return false;

	MessageZone newZone;
	newZone.handleDefaultOptions(data);

	for (unsigned int i = 0; i < data->data.size(); i++)
	{
		std::string line = data->data.get(i).c_str();

		bz_APIStringList *nubs = bz_newStringList();

		nubs->tokenize(line.c_str(), " ", 0, true);

		if (nubs->size() > 0)
		{
			std::string key = bz_toupper(nubs->get(0).c_str());

			if (key == "MESSAGE" && nubs->size() > 1)
				newZone.message = nubs->get(1).c_str();
			else if (key == "FLAG" && nubs->size() > 1)
				newZone.flag = nubs->get(1).c_str();
		}
		bz_deleteStringList(nubs);
	}

	newZone.zoneID = (int)messageZones.size();
	messageZones.push_back(newZone);

	return true;
}
void MessageZones::Event (bz_EventData *eventData)
{
	switch (eventData->eventType)
	{
		case bz_ePlayerUpdateEvent: // This event is called each time a player sends an update to the server
		{
			bz_PlayerUpdateEventData_V1* data = (bz_PlayerUpdateEventData_V1*)eventData;
			int playerID = update->playerID;

			for (size_t i = 0; i < messageZones.size(); ++i)
			{
				const MessageZone& zone = messageZones[i];

				if (!zone.pointInZone(update->state.pos))
					continue;

				if (bz_getPlayerFlagID(playerID) < 0)
					continue;

				if (strcmp(bz_getPlayerFlag(playerID), zone.flag.c_str()) != 0)
					continue;

				bool alreadyGotIt = false;
				for (const auto& record : playersWhoGotMessage) {
					if (record.first == playerID && record.second == (int)i) {
						alreadyGotIt = true;
						break;
					}
				}
			}

			if (!alreadyGotIt)
			{
				bz_sendTextMessage(BZ_SERVER, playerID, zone.message.c_str());
				playersWhoGotMessage.push_back({playerID, (int)i});
			}

		}
		break;

		case bz_ePlayerPartEvent:
		{
			bz_PlayerJoinPartEventData_V1* data = (bz_PlayerJoinPartEventData_V1*)eventData;

			playersWhoGotMessage.erase(
			std::remove_if(playersWhoGotMessage.begin(), playersWhoGotMessage.end(),
				[part](const auto& p) { return p.first == part->playerID; }),
			playersWhoGotMessage.end());
		}
		break;

		default:
			break;
	}
}
