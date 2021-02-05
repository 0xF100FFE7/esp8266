#ifndef UI_NETWORK_GUARD
#define UI_NETWORK_GUARD

#include "Arduino.h"
#include <LittleFS.h>
#include <ESP8266WiFi.h>

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

//#define SCAN_PERIOD 8000
//extern long unsigned int last_scan_millis;

namespace ui {
	namespace network {
		enum sta_status {
			STA_BEGIN_CONNECTION,
			STA_ATTEMPT_TO_CONNECT,
			STA_CONNECTED,
			STA_LOST_CONNECTION,
			STA_TIMEOUT_TO_RECONNECT,
			STA_DISCONNECTED,
		} extern sta_status;
		
		extern bool sta_connected;
		extern int avail_networks;
		extern bool scan;
		
		struct settings {
			static bool need_commit;
			settings &changed();
			void committed();
		
			bool dhcp_enabled;
			
			char ap_ssid[80];
			char ap_pass[80];
			uint32_t ap_ip;
			uint32_t ap_gateway;
			uint32_t ap_subnet;
				
			char sta_ssid[80];
			char sta_pass[80];
			uint32_t sta_ip;
			uint32_t sta_gateway;
			uint32_t sta_subnet;
			
			struct settings &defaults();
			void save();		
			void load();
			
			settings();
		};
		
		extern settings settings;
		
		extern bool passphrase_is_valid(String);
		extern void connect_sta();
		extern void connect_ap();
		extern void begin();
		extern void end();
		extern void begin_scan();
		extern void end_scan();
		extern String get_station_name(int i);
		//extern String get_station_mac(int i);
		extern bool change_sta_to(int i, String passwd);
		//extern String get_connected_station_name();
		//extern String get_connected_station_mac();
		extern void process_sta();
		extern void loop();
	}
}

#endif
