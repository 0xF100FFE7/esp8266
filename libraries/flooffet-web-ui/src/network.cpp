#include "network.h"

//long unsigned int last_scan_millis = 0;

namespace ui {
	namespace network {
		enum sta_status sta_status = STA_DISCONNECTED;
		int avail_networks = 0;
		bool scan = false;
				
		struct ap &ap::defaults() {
			(String("ap_") + ESP.getChipId() /*+ millis()*/).toCharArray(ssid, 80);
			pass[0] = '\0';
			ip = IPAddress(7, 7, 7, 7);
			gateway = IPAddress(7, 7, 7, 7);
			subnet = IPAddress(255, 255, 255, 0);
		}
		
		struct sta &sta::defaults() {
			dhcp = false;
			ssid[0] = '\0';
			pass[0] = '\0';
			ip = IPAddress(192, 168, 1, 227);
			gateway = IPAddress(192, 168, 1, 1);
			subnet = IPAddress(255, 255, 255, 0);
			
			return *this;
		}
		
		void ap::save()
		{
			if (save_settings("/ap.bin", "acess point", this, sizeof(struct ap)))
				committed();
		}
		
		void ap::load()
		{
			if (!load_settings("/ap.bin", "acess point", this, sizeof(struct ap)))
				defaults();
		}
		
		struct ap &ap::changed()
		{
			ap::need_commit = true;
			return *this;
		}

		void ap::committed()
		{
			ap::need_commit = false;
		}
		
		ap::ap()
		{
			defaults();
		}
		
		bool ap::need_commit = false;
		struct ap ap;
		
		void sta::save()
		{
			if (save_settings("/sta.bin", "station", this, sizeof(struct sta)))
				committed();
		}
		
		void sta::load()
		{
			if (!load_settings("/sta.bin", "station", this, sizeof(struct sta)))
				defaults();
		}
		
		struct sta &sta::changed()
		{
			sta::need_commit = true;
			return *this;
		}

		void sta::committed()
		{
			sta::need_commit = false;
		}
		
		sta::sta()
		{
			defaults();
		}		
		
		bool sta::need_commit = false;
		struct sta sta;
		
		bool passphrase_is_valid(String val)
		{
			if (val.length() && (val.length() > 40 || val.length() < 8))
				return false;
			else
				return true;
		}
		
		void connect_sta()
		{
			sta_status = STA_BEGIN_CONNECTION;
			/*if (is_valid_mac(mac_addr))
				wifi_set_macaddr(SOFTAP_IF, &mac_addr[0]);*/
			if (!sta.dhcp)
				WiFi.config(sta.ip, sta.gateway, sta.subnet);
			WiFi.begin(sta.ssid, sta.pass);
		}
		
		void connect_ap()
		{
			WiFi.persistent(true);
			WiFi.softAPConfig(ap.ip, ap.gateway, ap.subnet);
			WiFi.softAP(ap.ssid, ap.pass);
			/*if (!is_valid_mac(mac_addr))
				WiFi.softAPmacAddress(mac_addr);*/
		}
		
		void begin() //bool sta_enabled
		{
			ap.load();
			sta.load();
			
			//WiFi.persistent(false);
			WiFi.mode(WIFI_AP_STA);
			//WiFi.mode(WIFI_AP);
			connect_ap();
			if (sta.ssid[0])
				connect_sta();
		};
		
		void end()
		{
			//WiFi.softAPdisconnect();
			WiFi.disconnect();
		}
		
		void begin_scan()
		{
			//last_scan_millis = millis();
			WiFi.scanNetworks(true);
			avail_networks = 0;
			scan = true;
		}
		
		void end_scan()
		{
			scan = false;
		}
		
		String get_station_name(int i)
		{
			String probably_incorrect_output = i > 0 ? WiFi.SSID(i) : String(sta.ssid);
			String correct_output;
			for (int i = 0; i < probably_incorrect_output.length(); i++)
			{
				char c = probably_incorrect_output[i];
				correct_output += isPrintable(c) ? c : '*';
			}
			return correct_output;
			//return WiFi.SSID(i);
		}
		
		//String correct_station_ssid
		
		/*String get_station_mac(int i)
		{
			return WiFi.BSSIDstr(i);
		}*/
		
		/*String get_connected_station_name()
		{
			//return WiFi.SSID();
			return String(sta.ssid);
		}*/
		
		/*String get_connected_station_mac()
		{
			return WiFi.BSSIDstr();
		}*/
		
		bool change_sta_to(int i, String passwd)
		{
			if (avail_networks < i || i < 0)
				return false; //invalid station
				
			WiFi.SSID(i).toCharArray(sta.changed().ssid, 80);
			(passphrase_is_valid(passwd) ? passwd : "").toCharArray(sta.changed().pass, 80);
			sta.save();
			end();
			DEBUG_MSG("Network settings applied, switching...\n");
			sta_status = STA_SWITCH;
			//delay(1000);
			//ESP.restart();
			return true;
			//connect_sta();
		}
		
		bool disconnect_sta()
		{
			sta_status = STA_DISCONNECTED;
			WiFi.persistent(false);
			WiFi.disconnect(); //quit ESP sta reconnect limbo
			WiFi.persistent(true);
		}
		
		/*void disconnect_sta()
		{
			sta_status = STA_DISCONNECTED;
			WiFi.disconnect();
		}*/
		
		void process_sta()
		{
			static unsigned long old_millis = 0;

			if (millis() > old_millis + 1000)
				old_millis += 1000;
			else
				return;

			//TODO less complex automaton
			switch (sta_status) {
			case STA_BEGIN_CONNECTION:
				sta_status = STA_ATTEMPT_TO_CONNECT;
				WiFi.begin();
				break;
				
			case STA_ATTEMPT_TO_CONNECT:
				static int attempts = 5;
				
				if (WiFi.status() == WL_CONNECTED) {
					DEBUG_MSG("Connected to sta: %s (%s)\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
					sta_status = STA_CONNECTED;
					attempts = 5;
					break;
				}

				if (attempts > 0) {
					DEBUG_MSG("Connecting to %s (%i)...\n", sta.ssid, attempts);
					attempts--;
				} else {
					DEBUG_MSG("Can't connect to sta\n");
					sta_status = STA_LOST_CONNECTION;
					attempts = 5;
				}
				
				break;
			
			case STA_SWITCH:
				static int wait = 5; //delay before switch
				
				if (wait > 0) {
					DEBUG_MSG("Switching to %s in %i...\n", sta.ssid, wait);
					wait--;
					break;
				} else {
					sta_status = STA_ATTEMPT_TO_CONNECT;
					wait = 5;
				}
				
				if (!sta.dhcp)
					WiFi.config(sta.ip, sta.gateway, sta.subnet);
						WiFi.begin(sta.ssid, sta.pass);
				break;
				
			case STA_CONNECTED:
				if (WiFi.status() != WL_CONNECTED) {
					DEBUG_MSG("Connection to sta is lost\n");
					sta_status = STA_LOST_CONNECTION;
					break;
				}
				break;
				
			case STA_LOST_CONNECTION:
				WiFi.persistent(false);
				WiFi.disconnect(); //quit ESP sta reconnect limbo
				WiFi.persistent(true);
				sta_status = STA_TIMEOUT_TO_RECONNECT;
				break;
				
			case STA_TIMEOUT_TO_RECONNECT:
				static int timeout = 60;
				
				if (timeout > 0) {
					if (!(timeout % 10))
						DEBUG_MSG("Timeout to reconnect sta: %i...\n", timeout--);
					timeout--;
				} else {
					DEBUG_MSG("Sta been disconnected too long, reconnecting...\n");
					sta_status = STA_BEGIN_CONNECTION;
					timeout = 60;
				}
				
				break;
			
			case STA_DISCONNECTED:
				break;
			}
		}
		
		void loop()
		{	
			if (scan)
				avail_networks = WiFi.scanComplete();
			process_sta();
		}
	}
}
