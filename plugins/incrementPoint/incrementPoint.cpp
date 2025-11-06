#include "bzfsAPI.h"

class incrementPoint : public bz_Plugin, public bz_CustomSlashCommandHandler
{
	virtual const char* Name ();
	virtual void Init (const char*);
	virtual void Cleanup ();
	virtual bool SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);
};

BZ_PLUGIN(incrementPoint)

const char* incrementPoint::Name ()
{
	return "Increment Point Plugin";
}

void incrementPoint::Init (const char*)
{
	bz_registerCustomSlashCommand("incrementPoint", this);
}

void incrementPoint::Cleanup ()
{
	Flush();

	bz_removeCustomSlashCommand("incrementPoint");
}

bool incrementPoint::SlashCommand (int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
	if (command == "incrementPoint")
	{
		bz_BasePlayerRecord *fromPlayer = bz_getPlayerByIndex(playerID);
		if (params->size() != 1)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "/incrementPoint <player ID|callsign>");
			return true;
		}

		bz_BasePlayerRecord* receiverID;
		receiverID = bz_getPlayerBySlotOrCallsign(params->get(0).c_str());

		if (!receiverID)
		{
			bz_sendTextMessagef(BZ_SERVER, playerID, "player \"%s\" not found", receiverID);
		}
		else
		{
			bz_incrementPlayerLosses(receiverID->playerID, -1);
			bz_sendTextMessagef(BZ_SERVER, fromPlayer->playerID, "Decreased %s's losses by one.", bz_getPlayerCallsign(receiverID->playerID));
		}
		bz_freePlayerRecord(receiverID);
		bz_freePlayerRecord(fromPlayer);

		return true;
	}
	if (command == "decrementPoint")
	{
		bz_BasePlayerRecord *fromPlayer = bz_getPlayerByIndex(playerID);
		if (params->size() != 1)
		{
			bz_sendTextMessage(BZ_SERVER, playerID, "/decrementPoint <player ID|callsign>");
			return true;
		}

		bz_BasePlayerRecord* receiverID;
		receiverID = bz_getPlayerBySlotOrCallsign(params->get(0).c_str());

		if (!receiverID)
		{
			bz_sendTextMessagef(BZ_SERVER, playerID, "player \"%s\" not found", receiverID);
		}
		else
		{
			bz_incrementPlayerWins(receiverID->playerID, -1);
			bz_sendTextMessagef(BZ_SERVER, fromPlayer->playerID, "Decreased %s's wins by one.", bz_getPlayerCallsign(receiverID->playerID));
		}
		bz_freePlayerRecord(receiverID);
		bz_freePlayerRecord(fromPlayer);

		return true;
	}
	return false;
}
