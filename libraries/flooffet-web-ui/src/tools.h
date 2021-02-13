#ifndef UI_TOOLS_GUARD
#define UI_TOOLS_GUARD

#include "Arduino.h"
#include <LittleFS.h>
#include <ESP8266WiFi.h>

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

template <typename object, typename member>
size_t class_index(object &array, member &address)
{
	DEBUG_MSG("array addr: %ul, member addr: %ul, sizeof object: %ul, result: %ul\n", (uintptr_t)&address, (uintptr_t)&array, sizeof(object), ((uintptr_t)&address - (uintptr_t)&array) / sizeof(object));
	return ((uintptr_t)&address - (uintptr_t)&array) / sizeof(object);
}

extern bool save_settings(const char *file_name, const char *setting_name, void *ptr, size_t sz);
extern bool load_settings(const char *file_name, const char *setting_name, void *ptr, size_t sz);

#endif
