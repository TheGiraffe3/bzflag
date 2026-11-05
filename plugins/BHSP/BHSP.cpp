#include "bzfsAPI.h"
#include "plugin_utils.h"

#include <string>

class BHSP : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:
	virtual const char* Name();
	virtual void Init(const char* config);
	virtual void Cleanup();
	virtual void Event(bz_EventData* eventData);
	virtual bool SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);
};

BZ_PLUGIN(BHSP)

bool isInGroup1(int playerID) {
	bz_BasePlayerRecord* player = bz_getPlayerByIndex(playerID);
	if (!player)
		return false;
	std::string callsign = player->callsign;
	bz_freePlayerRecord(player);

	if (callsign == "gabongaming2000" || callsign == "bb")
		return true;

	return false;
}

void getPlayerName(int playerID, std::string& callsignString) {
	bz_BasePlayerRecord* player = bz_getPlayerByIndex(playerID);
	if (!player) {
		callsignString = "none";
		return;
	}
	callsignString = player->callsign;
	bz_freePlayerRecord(player);
	return;
}

const char* BHSP::Name()
{
	return "BHS Plugin";
}

bool zeroPoints = true;
int rrKills = 0;
int loyKills = 0;

void BHSP::Init(const char* config)
{
	Register(bz_ePlayerDieEvent);
	Register(bz_ePlayerScoreChanged);

	bz_registerCustomSlashCommand("sw", this);
	bz_registerCustomSlashCommand("wg", this);
	bz_registerCustomSlashCommand("sr", this);

	bz_registerCustomSlashCommand("zp", this);
}

void BHSP::Cleanup()
{
	Flush();

	bz_removeCustomSlashCommand("sw");
	bz_removeCustomSlashCommand("wg");
	bz_removeCustomSlashCommand("sr");

	bz_removeCustomSlashCommand("zp");
}

void BHSP::Event(bz_EventData* eventData)
{
	switch (eventData->eventType)
	{
		case bz_ePlayerDieEvent:
		{
			bz_PlayerDieEventData_V2* data = (bz_PlayerDieEventData_V2*)eventData;

			std::string callsign;

			getPlayerName(data->playerID, callsign);
			if (callsign == "red rider" || callsign == "Aeonovyli")
				rrKills = 0;
			else if (callsign == "Loymdayddaud")
				loyKills = 0;

			getPlayerName(data->killerID, callsign);

			if (callsign == "red rider" || callsign == "Aeonovyli") {
				rrKills++;

				if (rrKills >= 2) {
					rrKills = 0;
					bz_sendTextMessage(BZ_SERVER, data->killerID, "You've made enough kills with that flag. Find another.");
					bz_removePlayerFlag(data-> killerID);
				}
			}
			else if (callsign == "Loymdayddaud") {
				loyKills++;

				if (loyKills >= 2) {
					loyKills = 0;
					bz_sendTextMessage(BZ_SERVER, data-> killerID, "You've made enough kills with that flag. Find another.");
					bz_removePlayerFlag(data-> killerID);
				}
			}
		}
		break;

		case bz_ePlayerScoreChanged:
		{
			bz_PlayerScoreChangeEventData_V1* data = (bz_PlayerScoreChangeEventData_V1*)eventData;

			if (zeroPoints == true && data->thisValue != 0)
				bz_resetPlayerScore(data->playerID);
		}
		break;

		default:
			break;
	}
}

bool BHSP::SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
	if (command == "sw" || command == "wg" || command == "sr")
	{
		if (!isInGroup1(playerID)) {
			bz_sendTextMessage(BZ_SERVER, playerID, "You do not qualify for this command!");
			return true;
		}			

		bz_sendTextMessage(BZ_SERVER, playerID, "Enjoy your new flag!");

		if (command == "sw") {
			bz_givePlayerFlag(playerID, "SW", false);
			return true;
		}
		else if (command == "wg") {
			bz_givePlayerFlag(playerID, "WG", false);
			return true;
		}
		else if (command == "sr") {
			bz_givePlayerFlag(playerID, "SR", false);
			return true;
		}
	}
	else if (command == "zp") {
		if (!bz_getAdmin(playerID)) {
			bz_sendTextMessage(BZ_SERVER, playerID, "You do not have permission to run this command!");
			return true;
		}

		zeroPoints = !zeroPoints;
		if (zeroPoints)
			bz_sendTextMessage(BZ_SERVER, playerID, "Switched Zero Points on!");
		else
			bz_sendTextMessage(BZ_SERVER, playerID, "Switched Zero Points off!");

		return true;
	}

	return false;
}
