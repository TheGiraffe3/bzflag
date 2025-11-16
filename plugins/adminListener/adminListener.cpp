#include "bzfsAPI.h"
#include <string>

class AdminListener : public bz_Plugin {
public:
	virtual const char* Name (){return "Admin Listener";}
	virtual void Init (const char* config);
	virtual void Event(bz_EventData *eventData );
	virtual void Cleanup ();
};

BZ_PLUGIN(AdminListener);

void AdminListener::Init(const char* commandline) {
	bz_debugMessage(4,"adminListener plugin loaded");
	Register(bz_eRawChatMessageEvent);
}

void AdminListener::Cleanup( void ) {
	bz_debugMessage(4,"adminListener plugin unloaded");
	Flush();
}

void AdminListener::Event( bz_EventData *eventData ) {
	bz_ChatEventData_V1 *messageEvent = (bz_ChatEventData_V1*) eventData;

	if( messageEvent->team == eRogueTeam ||
				messageEvent->team == eRedTeam ||
				messageEvent->team == eGreenTeam ||
				messageEvent->team == eBlueTeam ||
				messageEvent->team == ePurpleTeam ||
				messageEvent->team == eHunterTeam ) {

		bz_BasePlayerRecord *sender =
				bz_getPlayerByIndex( messageEvent->from );
		if( ! sender ) return;
		std::string senderCallsign = (sender->callsign).c_str();
		bz_freePlayerRecord( sender );

		bz_APIIntList *playerList = bz_newIntList();
		bz_getPlayerIndexList( playerList );

		std::string message =
		std::string( "[Player to team] " ) +
		senderCallsign +
		std::string( ": " ) +
		std::string( (messageEvent->message).c_str() );

		for( unsigned int i = 0; i < playerList->size(); i++ ) {
			bz_BasePlayerRecord *player = bz_getPlayerByIndex( i );

			if( ! player ) return;
			if( player->admin &&
					player->team == eObservers ) {
				bz_sendTextMessage( i, i, message.c_str() );
			}

			bz_freePlayerRecord( player );
		}
	} else if( messageEvent->to >= 0 && messageEvent->from >= 0) {
		// Send to admins if it is a PM sent from a non-admin
		// to a non-admin
		bz_BasePlayerRecord *sender =
				bz_getPlayerByIndex( messageEvent->from );
		bz_BasePlayerRecord *receiver =
				bz_getPlayerByIndex( messageEvent->to );

		if( ! sender || ! receiver ) return;

		if( sender->admin || receiver->admin ) return;

		std::string messageString =
				std::string("[") +
						sender->callsign.c_str() +
						"->" +
						receiver->callsign.c_str() +
						"] " +
						messageEvent->message.c_str();

		bz_freePlayerRecord( sender );
		bz_freePlayerRecord( receiver );

		bz_APIIntList *playerList = bz_newIntList();
		bz_getPlayerIndexList( playerList );

		for( unsigned int i = 0; i < playerList->size(); i++ ) {
			bz_BasePlayerRecord *player = bz_getPlayerByIndex( i );

			if( !player ) return;

			if( player->admin ) {//&&
//					player->team == eObservers ) {
				bz_sendTextMessage( i, i,
						messageString.c_str() );
			}

			bz_freePlayerRecord( player );
		}
	}
}
