// cpp : Defines the entry point for the DLL application.
//

#include <list>
#include <map>
#include <string>

#include "bzfsAPI.h"

class genostatsHandler : public bz_Plugin, public bz_CustomSlashCommandHandler
{
public:

  struct GenoStat
  {
    int kills;
    int deaths;
  };

  struct Node // Used for sorting.
  {
    Node(int playerID, int nscore);
    ~Node();

    void add(Node* n);

    int playerID;
    int kills;
    int deaths;
    int score;

    Node* l;
    Node* r;
  };

  virtual const char* Name (){return "Geno Stats";}
  virtual void Init ( const char* config);
  virtual void Event(bz_EventData *eventData);
  virtual bool SlashCommand(int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList *params);
  virtual void Cleanup ();

  void updateInfo();
  void printNode(Node* n);

  std::map<int, GenoStat> stats;
  std::list<std::string> info;
  std::list<std::string> recent;

  GenoStat blank;

};

BZ_PLUGIN(genostatsHandler);

void genostatsHandler::Init(const char* /*commandLine*/)
{
  blank.kills = 0;
  blank.deaths = 0;

  bz_APIIntList* pl = bz_newIntList();
  bz_getPlayerIndexList(pl);

  for (unsigned int x = 0; x < pl->size(); x++)
  {
    stats[pl->get(x)] = blank;
  }

  bz_deleteIntList(pl);

  updateInfo();

  Register(bz_ePlayerDieEvent);
  Register(bz_ePlayerJoinEvent);
  Register(bz_ePlayerPartEvent);

  bz_registerCustomSlashCommand ("genostats", this);

  bz_debugMessage(4,"genostats plugin loaded");
}

void genostatsHandler::Event(bz_EventData *eventData)
{
  switch (eventData->eventType)
  {
    case bz_ePlayerDieEvent:
    {
      bz_PlayerDieEventData_V1* diedata = (bz_PlayerDieEventData_V1*)eventData;

      if(diedata->flagKilledWith == "G")
      {
	// Update stats
	stats[diedata->playerID].deaths++;
	stats[diedata->killerID].kills++;
	updateInfo();

	// Update last five
	bz_BasePlayerRecord* v = bz_getPlayerByIndex(diedata->playerID);
	bz_BasePlayerRecord* k = bz_getPlayerByIndex(diedata->killerID);
	if(v != NULL && k != NULL)
	{
	  std::string msg = k->callsign.c_str();
	  msg += " -> ";
	  msg += v->callsign.c_str();

	  recent.push_front(msg);

	  if(recent.size() > 5)
	  {
	    recent.pop_back();
	  }
	}
	if(v != NULL)
	{
	  bz_freePlayerRecord(v);
	}
	if(k != NULL)
	{
	  bz_freePlayerRecord(k);
	}
      }

    }
    break;

    case bz_ePlayerJoinEvent:
    {
      bz_PlayerJoinPartEventData_V1* joindata = (bz_PlayerJoinPartEventData_V1*)eventData;
      bz_BasePlayerRecord *joinData = bz_getPlayerByIndex(joinData->playerID);

      if(joinData->team != eObservers)
      {
	stats[joindata->playerID] = blank;
	updateInfo();
      }
		bz_freePlayerRecord(joinData);
    }
    break;

    case bz_ePlayerPartEvent:
    {
      bz_PlayerJoinPartEventData_V1* partdata = (bz_PlayerJoinPartEventData_V1*)eventData;
      bz_BasePlayerRecord *partData = bz_getPlayerByIndex(partData->playerID);

      if(partData->team != eObservers)
      {
	stats.erase(partdata->playerID);
	updateInfo();
      }
		bz_freePlayerRecord(partData);
    }
    break;

  default:
    break;
  }
}

bool genostatsHandler::SlashCommand(int playerID, bz_ApiString /* command */, bz_ApiString /* message */, bz_APIStringList *params)
{
  // "Recent" command
  if(params->size() >= 1 && params->get(0) == "recent")
  {
    bz_sendTextMessage(playerID, playerID, "Recent Genocides:");

    int n = 1;
    for(std::list<std::string>::iterator i = recent.begin(); i != recent.end(); i++)
    {
      bz_sendTextMessage(playerID, playerID, bz_format("%i. %s", n, i->c_str()));
      n++;
    }
  }
  else
  {
  for(std::list<std::string>::iterator i = info.begin(); i != info.end(); i++)
    {
      bz_sendTextMessage(playerID, playerID, i->c_str());
    }
  }
  return true;
}

void genostatsHandler::Cleanup(void)
{
  Flush();

  bz_removeCustomSlashCommand ("genostats");

  bz_debugMessage(4,"genostats plugin unloaded");
}

void genostatsHandler::updateInfo()
{
  // Remove the old stats.
  info.clear();

  // Add a header
  info.push_back(std::string("Genocide statistics:"));
  info.push_back(std::string("Kills   Deaths  Callsign"));

  if(stats.size() < 1)
  {
    return;
  }

  // Build a tree
  std::map<int, GenoStat>::iterator i = stats.begin();
  Node* tree = new Node(i->first, (i->second.kills - i->second.deaths));

  i++;
  while(i != stats.end())
  {
    Node* n = new Node(i->first, (i->second.kills - i->second.deaths));
    tree->add(n);

    i++;
  }

  // Spit out the data
  printNode(tree);

  // Clean up
  delete tree;

}

void genostatsHandler::printNode(Node* n)
{
  if(n == NULL)
  {
    return;
  }

  if(n->l != NULL)
  {
    printNode(n->l);
  }

  bz_BasePlayerRecord* p = bz_getPlayerByIndex(n->playerID);
  if(p != NULL)
  {
    std::string msg = bz_format("%i", stats[n->playerID].kills);
    while(msg.size() < 8)
    {
      msg += ' ';
    }

    msg += bz_format("%i", stats[n->playerID].deaths);
    while(msg.size() < 16)
    {
      msg += ' ';
    }

    msg += p->callsign.c_str();

    info.push_back(msg);

    bz_freePlayerRecord(p);
  }

  if(n->r != NULL)
  {
    printNode(n->r);
  }
}

genostatsHandler::Node::Node(int nplayerID, int nscore)
{
  playerID = nplayerID;
  score = nscore;
  l = NULL;
  r = NULL;
}

genostatsHandler::Node::~Node()
{
  if(l != NULL)
  {
    delete l;
  }
  if(r != NULL)
  {
    delete r;
  }
}

void genostatsHandler::Node::add(Node* n)
{
  if(n != NULL)
  {
    if(n->score > score)
    {
      if(l != NULL)
      {
	l->add(n);
      }
      else
      {
	l = n;
      }
    }
    else
    {
      if(r != NULL)
      {
	r->add(n);
      }
      else
      {
	r = n;
      }
    }
  }
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
