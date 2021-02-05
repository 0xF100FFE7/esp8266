#ifndef UI_SERVER_GUARD
#define UI_SERVER_GUARD

#include "Arduino.h"

#define private public        //This is the hack to access priviate variables of any class 
#define protected public     // This is the hack to access protected variables of any class  
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

#include "web_javascript.h"
#include "web_styles.h"
#include "web_page.h"

#include "widgets.h" //server must know about widgets to parse or send them;

#define WS_AUTHENTICATION false

namespace ui {
	extern const char* basicAuthUsername;
	extern const char* basicAuthPassword;
	extern bool basicAuth;
	
	extern DNSServer dns_server;
	extern AsyncWebServer* server;
	extern AsyncWebSocket* ws;
	
	extern void web_event(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType, void *, uint8_t *, size_t);
	extern void start_web_server(const char *, const char *);
}

#endif
