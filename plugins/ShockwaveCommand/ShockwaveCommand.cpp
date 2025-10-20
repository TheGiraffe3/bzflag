
/*
 * ShockwaveCommand
 *   Copyright (C) 2025 Loymdayddaud
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bzfsAPI.h"
#include "plugin_utils.h"

class ShockwaveCommand : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:
	virtual const char* Name();
	virtual void Init(const char*);
	virtual void Cleanup();
	virtual bool SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params);
};

BZ_PLUGIN(ShockwaveCommand)

const char* ShockwaveCommand::Name()
{
	return "Shockwave Command";
}

void ShockwaveCommand::Init(const char*)
{
	bz_registerCustomSlashCommand("shockwave", this);
}

void ShockwaveCommand::Cleanup()
{
	Flush();

	bz_removeCustomSlashCommand("shockwave");
}

bool ShockwaveCommand::SlashCommand(int playerID, bz_ApiString command, bz_ApiString /*message*/, bz_APIStringList *params)
{
	if (command == "shockwave")
	{
		bz_BasePlayerRecord* playerRecord = bz_getPlayerByIndex(playerID);
		float pos[3];
		float vel[3];
		pos[0] = playerRecord->lastKnownState.pos[0];
		pos[1] = playerRecord->lastKnownState.pos[1];
		pos[2] = playerRecord->lastKnownState.pos[2] + bz_getBZDBDouble("_muzzleHeight");
		vel[0] = 0;
		vel[1] = 0;
		vel[2] = 0;
		bz_fireServerShot("SW", pos, vel, bz_getPlayerTeam(playerID));
		bz_incrementPlayerLosses(playerID, -1);
		bz_freePlayerRecord(playerRecord);
		return true;
	}

	return false;
}

