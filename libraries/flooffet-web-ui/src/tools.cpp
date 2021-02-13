#include "tools.h"

bool save_settings(const char *file_name, const char *setting_name, void* ptr, size_t sz)
{
	DEBUG_MSG("Saving %s settings...\n", setting_name);
	File f = LittleFS.open(file_name, "w");
	if (f) {
		f.write((uint8_t *)ptr, sz);
		f.close();			
		DEBUG_MSG("\tSave succeed!\n");
		return true;
	} else {
		DEBUG_MSG("\tSave failed!\n");
		return false;
	}
}

bool load_settings(const char *file_name, const char *setting_name, void* ptr, size_t sz)
{
	DEBUG_MSG("Loading %s settings...\n", setting_name);
	File f = LittleFS.open(file_name, "r");
	if (f) {
		f.read((uint8_t *)ptr, sz);
		f.close();
		DEBUG_MSG("\tLoad succeed!\n");
		return true;
	} else {
		DEBUG_MSG("\tLoad failed!\n");
		return false;
	}
}
