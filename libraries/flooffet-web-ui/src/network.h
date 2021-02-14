#ifndef UI_NETWORK_GUARD
#define UI_NETWORK_GUARD

#include "tools.h"

//#define SCAN_PERIOD 8000
//extern long unsigned int last_scan_millis;

namespace ui {
	namespace network {
		enum sta_status {
			STA_BEGIN_CONNECTION,
			STA_ATTEMPT_TO_CONNECT,
			STA_SWITCH,
			STA_CONNECTED,
			STA_LOST_CONNECTION,
			STA_TIMEOUT_TO_RECONNECT,
			STA_DISCONNECTED,
		} extern sta_status;
		
		extern bool sta_connected;
		extern int avail_networks;
		extern bool scan;
		
		struct ap {
			static bool need_commit;
			ap &changed();
			void committed();
			bool modified;
			
			char ssid[80];
			char pass[80];
			uint32_t ip;
			uint32_t gateway;
			uint32_t subnet;
			
			struct ap &defaults();
			void save();		
			void load();
			
			ap();
		} extern ap;
		
		struct sta {
			static bool need_commit;
			sta &changed();
			void committed();
			bool modified;
			
			bool dhcp;
			char ssid[80];
			char pass[80];
			uint32_t ip;
			uint32_t gateway;
			uint32_t subnet;
			
			struct sta &defaults();
			void save();		
			void load();
			
			sta();
		} extern sta;
		
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
		extern bool refresh_sta();
		extern bool connection_is_secure(int i);
		extern bool disconnect_sta();
		extern IPAddress get_connected_station_ip();
		//extern String get_connected_station_name();
		//extern String get_connected_station_mac();
		extern void process_sta();
		extern void loop();
	}
}

#endif
