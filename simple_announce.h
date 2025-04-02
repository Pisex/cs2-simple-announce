#ifndef _INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_
#define _INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_

#include <ISmmPlugin.h>
#include <sh_vector.h>
#include "utlvector.h"
#include <iserver.h>
#include <entity2/entitysystem.h>
#include "igameevents.h"
#include <deque>
#include <functional>
class CCSGameRules;
#include "include/admin.h"
#include "include/menus.h"

class simple_announce final : public ISmmPlugin, public IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late);
	bool Unload(char* error, size_t maxlen);
	void AllPluginsLoaded();
private:
	const char* GetAuthor();
	const char* GetName();
	const char* GetDescription();
	const char* GetURL();
	const char* GetLicense();
	const char* GetVersion();
	const char* GetDate();
	const char* GetLogTag();

private:
	bool FireEvent(IGameEvent* pEvent, bool bDontBroadcast);
};

#endif //_INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_
