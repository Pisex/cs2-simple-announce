#include <stdio.h>
#include <stdint.h>
#include "simple_announce.h"
#include "metamod_oslink.h"
#include "networksystem/inetworksystem.h"
#include <utlstring.h>
#include <KeyValues.h>
#include <cstring>
#include <map>
#include <string>
#include <sstream>
#include <future>

IVEngineServer2* engine = nullptr;
IGameEventManager2* gameeventmanager = nullptr;

IUtilsApi* g_pUtils;
IAdminApi* g_pAdminApi;
IPlayersApi* g_pPlayersApi;
simple_announce g_simple_announce;

PLUGIN_EXPOSE(simple_announce, g_simple_announce);

SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent*, bool);

std::map<std::string, std::string> g_vecPhrases;

bool g_bHide[64];

void OnAdminAction(int iSlot, const char* szAction, const char* szParam)
{
	if(!strcmp(szAction, "hide_on"))
	{
		g_bHide[iSlot] = true;
	}
	else if(!strcmp(szAction, "hide_off"))
	{
		g_bHide[iSlot] = false;
	}
}

bool simple_announce::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	ConVar_Register(FCVAR_RELEASE | FCVAR_GAMEDLL);
	return true;
}

bool simple_announce::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK(IGameEventManager2, FireEvent, gameeventmanager, SH_MEMBER(this, &simple_announce::FireEvent), false);
	ConVar_Unregister();
	
	return true;
}

bool simple_announce::FireEvent(IGameEvent* pEvent, bool bDontBroadcast)
{
    if (!pEvent) {
        RETURN_META_VALUE(MRES_IGNORED, false);
    }
	const char* szName = pEvent->GetName();
	bool bNewParams = false;
	if(!strcmp(szName, "player_team"))
	{
		int iSlot = pEvent->GetInt("userid");
		if(!g_bHide[iSlot]) {
			int iTeam = pEvent->GetInt("team");
			if(iTeam > 0) {
				static const char* szPhrase[] = {"", "ChangeTeam_SPEC", "ChangeTeam_T", "ChangeTeam_CT"};
				if(g_vecPhrases[szPhrase[iTeam]].size()) g_pUtils->PrintToChatAll(g_vecPhrases[szPhrase[iTeam]].c_str(), g_pPlayersApi->GetPlayerName(iSlot));
			}
		}
		pEvent->SetBool("silent", true);
		bNewParams = true;
	}
	else if(!strcmp(szName, "player_disconnect"))
	{
		int iSlot = pEvent->GetInt("userid");
		if(pEvent->GetInt("reason") != 54)
		{
			if(g_vecPhrases["User_Leave"].size()) g_pUtils->PrintToChatAll(g_vecPhrases["User_Leave"].c_str(), g_pPlayersApi->GetPlayerName(iSlot));
		}
		bNewParams = true;
	}
	else if(!strcmp(szName, "player_connect"))
	{
		if(g_vecPhrases["User_Connect"].size()) g_pUtils->PrintToChatAll(g_vecPhrases["User_Connect"].c_str(), pEvent->GetString("name"));
		bNewParams = true;
	}
	if(bNewParams) RETURN_META_VALUE_NEWPARAMS(MRES_IGNORED, true, &IGameEventManager2::FireEvent, (pEvent, true));
    else RETURN_META_VALUE(MRES_IGNORED, true);
}

void simple_announce::AllPluginsLoaded()
{
	char error[64] = { 0 };
	int ret;
	g_pUtils = (IUtilsApi *)g_SMAPI->MetaFactory(Utils_INTERFACE, &ret, NULL);
	if (ret == META_IFACE_FAILED)
	{
		V_strncpy(error, "Missing Utils system plugin", 64);
		ConColorMsg(Color(255, 0, 0, 255), "[%s] %s\n", GetLogTag(), error);
		std::string sBuffer = "meta unload "+std::to_string(g_PLID);
		engine->ServerCommand(sBuffer.c_str());
		return;
	}
	g_pPlayersApi = (IPlayersApi *)g_SMAPI->MetaFactory(PLAYERS_INTERFACE, &ret, NULL);
	if (ret == META_IFACE_FAILED)
	{
		g_pUtils->ErrorLog("[%s] Missing Players system plugin", GetLogTag());
		std::string sBuffer = "meta unload "+std::to_string(g_PLID);
		engine->ServerCommand(sBuffer.c_str());
		return;
	}
	g_pAdminApi = (IAdminApi *)g_SMAPI->MetaFactory(Admin_INTERFACE, &ret, NULL);
	if (ret != META_IFACE_FAILED) g_pAdminApi->OnAction(g_PLID, OnAdminAction);

	KeyValues::AutoDelete g_kvPhrases("Phrases");
	const char *pszPath = "addons/translations/simple_annonce.phrases.txt";
	if (!g_kvPhrases->LoadFromFile(g_pFullFileSystem, pszPath))
	{
		g_pUtils->ErrorLog("[%s] Failed to load phrases file: %s", GetLogTag(), pszPath);
		return;
	}
	const char* pszLanguage = g_pUtils->GetLanguage();
	for (KeyValues *pKey = g_kvPhrases->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey())
		g_vecPhrases[std::string(pKey->GetName())] = std::string(pKey->GetString(pszLanguage));

	gameeventmanager = g_pUtils->GetGameEventManager();
	SH_ADD_HOOK(IGameEventManager2, FireEvent, gameeventmanager, SH_MEMBER(this, &simple_announce::FireEvent), false);
}

///////////////////////////////////////
const char* simple_announce::GetLicense()
{
	return "";
}

const char* simple_announce::GetVersion()
{
	return "1.0";
}

const char* simple_announce::GetDate()
{
	return __DATE__;
}

const char *simple_announce::GetLogTag()
{
	return "Simple Announce";
}

const char* simple_announce::GetAuthor()
{
	return "Pisex";
}

const char* simple_announce::GetDescription()
{
	return "Simple Announce";
}

const char* simple_announce::GetName()
{
	return "Simple Announce";
}

const char* simple_announce::GetURL()
{
	return "";
}
