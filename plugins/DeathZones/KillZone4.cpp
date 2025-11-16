#include "bzfsAPI.h"
#include <vector>
#include <string>

class DeathZone : public bz_CustomZoneObject {
public:
    int team;               // 0 = rogue, 1 = red, 2 = green, 3 = blue, 4 = purple
    std::string message;

    DeathZone() : bz_CustomZoneObject(), team(-1) {}
    bool pointInZone(const bz_Vector3 &pos) const {
        return this->contains(pos);
    }
};

class DeathZonePlugin : public bz_Plugin, public bz_CustomMapObjectHandler {
    std::vector<DeathZone*> zones;

public:
    virtual const char* Name() { return "DeathZoneMap"; }
    virtual void Init(const char* /*config*/) {
        Register(bz_ePlayerUpdateEvent);
        bz_registerCustomMapObject("deathzone", this);
    }
    virtual void Cleanup() {
        Flush();
        for (auto z : zones) delete z;
        zones.clear();
        bz_removeCustomMapObject("deathzone");
    }

    virtual bool MapObject(bz_ApiString object, bz_CustomMapObjectInfo *data) {
        if (object != "DEATHZONE" || !data) return false;

        DeathZone *z = new DeathZone();
        z->handleDefaultOptions(data);

        for (unsigned i = 0; i < data->data.size(); ++i) {
            std::string line = data->data.get(i).c_str();
            std::string key;
            std::string rest;
            {
                size_t pos = line.find(' ');
                if (pos != std::string::npos) {
                    key = line.substr(0, pos);
                    rest = line.substr(pos + 1);
                } else {
                    key = line;
                    rest = "";
                }
            }
            for (auto &c : key) c = toupper(c);

            if (key == "TEAM") {
                z->team = atoi(rest.c_str());
            } else if (key == "MESSAGE") {
                z->message = rest;
            }
        }

        zones.push_back(z);
        return true;
    }

    virtual void Event(bz_EventData *e) {
        if (e->eventType != bz_ePlayerUpdateEvent) return;
        bz_PlayerUpdateEventData_V1 *ud = (bz_PlayerUpdateEventData_V1*)e;
        int pid = ud->playerID;
        bz_BasePlayerRecord *pr = bz_getPlayerByIndex(pid);
        if (!pr) return;

        int pteam = pr->team;
        bz_Vector3 pos = ud->state.pos;

        for (auto z : zones) {
            if (z->team == pteam && z.pointInZone(data->pos)) {
                bz_killPlayer(pid, false, BZ_SERVER);
                bz_sendTextMessagef(BZ_SERVER, pid, "You died: %s", z->message.c_str());
                break;
            }
        }

        bz_freePlayerRecord(pr);
    }
};

BZ_PLUGIN(DeathZonePlugin)
