#include "network.h"

//long unsigned int last_scan_millis = 0;

namespace ui {
	namespace network {
		enum sta_status sta_status = STA_DISCONNECTED;
		int avail_networks = 0;
		bool scan = false;
				
		struct settings &settings::defaults() {
			(String("ap_generic") /*+ millis()*/).toCharArray(ap_ssid, 80);
			ap_pass[0] = '\0';
			ap_ip = IPAddress(7, 7, 7, 7);
			ap_gateway = IPAddress(7, 7, 7, 7);
			ap_subnet = IPAddress(255, 255, 255, 0);
			
			dhcp_enabled = false;
			(String("soni2") /*+ millis()*/).toCharArray(sta_ssid, 80);
			sta_pass[0] = '\0';
			sta_ip = IPAddress(192, 168, 1, 227);
			sta_gateway = IPAddress(192, 168, 1, 1);
			sta_subnet = IPAddress(255, 255, 255, 0);
			
			return *this;
		}
			
		void settings::save()
		{
			DEBUG_MSG("Saving network settings...\n");
			File f = LittleFS.open("/network.bin", "w");
			if (f) {
				f.write((uint8_t *)this, sizeof(settings));
				f.close();
				
				committed();
				
				DEBUG_MSG("\tSave succeed!\n");
			} else {
				DEBUG_MSG("\tSave failed!\n");
			}
		}
			
		void settings::load()
		{
			DEBUG_MSG("Loading network settings...\n");
			File f = LittleFS.open("/network.bin", "r");
			if (f) {
				f.read((uint8_t *)this, sizeof(settings));
				f.close();
				
				DEBUG_MSG("\tLoad succeed!\n");
			} else {
				DEBUG_MSG("\tLoad failed!\n");
				defaults();
			}
		}
		
		struct settings &settings::changed()
		{
			settings::need_commit = true;
			return *this;
		}

		void settings::committed()
		{
			settings::need_commit = false;
		}
		
		settings::settings()
		{
			defaults();
		}
		
		bool settings::need_commit = false;
		struct settings settings;
		
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
			if (!settings.dhcp_enabled)
				WiFi.config(settings.sta_ip, settings.sta_gateway, settings.sta_subnet);
			WiFi.begin(settings.sta_ssid, settings.sta_pass);
		}
		
		void connect_ap()
		{
			WiFi.softAPConfig(settings.ap_ip, settings.ap_gateway, settings.ap_subnet);
			WiFi.softAP(settings.ap_ssid, settings.ap_pass);
			/*if (!is_valid_mac(mac_addr))
				WiFi.softAPmacAddress(mac_addr);*/
		}
		
		void begin() //bool sta_enabled
		{
			settings.load();
			//WiFi.persistent(false);
			WiFi.mode(settings.sta_ssid[0] ? WIFI_AP_STA : WIFI_AP);
			//WiFi.mode(WIFI_AP);
			connect_ap();
			if (settings.sta_ssid[0])
				connect_sta();
		};
		
		void end()
		{
			WiFi.softAPdisconnect();
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
			String probably_incorrect_output = i > 0 ? WiFi.SSID(i) : String(settings.sta_ssid);
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
			return String(settings.sta_ssid);
		}*/
		
		/*String get_connected_station_mac()
		{
			return WiFi.BSSIDstr();
		}*/
		
		bool change_sta_to(int i, String passwd)
		{
			if (avail_networks < i || i < 0)
				return false; //invalid station
				
			WiFi.SSID(i).toCharArray(settings.changed().sta_ssid, 80);
			(passphrase_is_valid(passwd) ? passwd : "").toCharArray(settings.changed().sta_pass, 80);
			settings.save();
			end();
			DEBUG_MSG("Network settings applied, restarting...\n");
			delay(1000);
			ESP.restart();
			return true;
			//connect_sta();
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
					DEBUG_MSG("Connecting to %s (%s) (%i)...\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), attempts);
					attempts--;
				} else {
					DEBUG_MSG("Can't connect to sta\n");
					sta_status = STA_LOST_CONNECTION;
					attempts = 5;
				}
				
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
