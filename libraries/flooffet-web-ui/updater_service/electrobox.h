#ifndef ELECTROBOX_GUARD
#define ELECTROBOX_GUARD

#include <EEPROM.h>
//#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>
//#include <ArduinoOTA.h>
#include <TimeLib.h>
#include <flame_ui.h>

struct frame {
	ui::element element;
	
	frame(void *callback);
};

#define MAX_ELBOX_CLIENTS DEFAULT_MAX_WS_CLIENTS

struct elbox_client {
	bool present; //if client exists
	uint32_t id = 0;
	
	bool interface_loaded;
	bool waiting_for_frame_confirmation;
	int frame_number;
	
	ui::messages messages;
	frame current_frame;
	
	static void confirm_frame(void *);
	void build_interface();
	void cleanup();
	
	elbox_client();
} extern elbox_clients[MAX_ELBOX_CLIENTS];

elbox_client *find_elbox_client(uint32_t id);

enum language {
	LANG_UA,
	LANG_RU,
	LANG_EN,
	NUMBER_OF_SUPPORTED_LANGUAGES
} extern current_language;

enum language_item {
	HOME_TAB = 0,
	SETTINGS_TAB = 1,
	LANGUAGE_SELECTOR_TITLE = 2,
	LANGUAGE_NAME = 3,
	DATE_TIME_TITLE = 4,
	DATE_TIME_COL_NAMES = 5,
	WEEKLY_SCHEDULE_TITLE = 11,
	WEEKLY_SCHEDULE_COL_NAMES = 12,
	WEEKLY_SCHEDULE_DAY_NAMES = 16,
	CHARGER_STATUS_TITLE = 23,
	CHARGER_STATUS = 24,
	KWT = 32,
	TIME_KWT_TITLE = 33,
	CONSUMPTION_TITLE = 34,
	CURRENT_REGULATOR_TITLE = 35,
	GROUND_CHECK_TITLE = 36,
	SCHEDULE_ENABLER_TITLE = 37,
	ADAPTIVE_MODE_TITLE = 38,
	LIMIT_KWT_TITLE = 39,
	TIMERS_TAB = 40,
	BRIGHTNESS_REGULATOR_TITLE = 41,
	LIMIT_BY_TIME_TITLE = 42,
	DISPLAY_OFF_TIME = 43,
	LOGS_TAB = 44,
	LOGS_TITLE = 45,
	CHARGE_SWITCH_TITLE = 46,
	SYNC_TIME_TITLE = 47,
	CLEAR_LOG_TITLE = 48,
	FACTORY_RESET_TITLE = 49,
	NETWORK_SETTINGS_TITLE = 50,
	SAVE_NETWORK_SETTINGS = 51,
	AP_TAB = 52,
	STA_TAB = 53,
	NETWORK_SETTINGS = 54,
	STATS_TITLE = 65,
	STATS_PREV_TITLE = 66,
	STATS_CUR_TITLE = 67,
	STATS_TEXT = 68,
	STATS_ALL_TIME = 73,
	STATS_RESET = 74,
	KWT_FOR_SESSION = 75,
	
	SYNC_TIME_NOW = 76,
	SYNC_TIME_ON_CONNECT = 77,
	ARE_YOU_SURE_RESET = 78,
	RESET_YES = 79,
	RESET_NO = 80,
	
	LANGUAGE_ITEMS = 81,
	//LANGUAGE_ITEMS = 64,
};

extern const char* l_str(unsigned i);

void send_log(String text);

struct ifield {
	ui::element element;
	int val;
	
	ui::messages update(String v);
	
	ifield(void *callback);
};

namespace charge_switch {
	extern bool enabled;
}

namespace charge_status {
	enum status {
		C_UNSET,
		C_CHARGING,
		C_CHARGED,
		C_CHECK_GROUND,
		C_RCD_TRIGGERED,
		C_VOLTAGE_PROTECTION_TRIGGERED,
		C_CURRENT_PROTECTION_TRIGGERED,
		C_TIMER_ENABLED,
		
		C_UNKNOWN
	} extern value;
	
	extern void update(int);
}

namespace date_time {
	struct ifield : ::ifield {
		static void callback(ifield&, String);
		ifield();
	};
	extern bool changed;
	extern ifield date[3];
	extern ifield time[3];
};

namespace weekly_schedule {
	extern bool enabled;
	extern bool blocking;
	extern void trigger();
}

namespace consumption {
	extern float voltage;
	extern float current;
	extern float kwt;

	extern void update(float, float, float);
}

namespace time_and_kwt_for_session {
	extern int hours, minutes;
	extern float kwt;
	extern float previous_kwt;
	extern void update(int, int, float);
}

namespace current_regulator {
	extern int value;
	extern void update(int);
}

namespace ground_check {
	extern bool enabled;
}

namespace adaptive_mode {
	extern bool enabled;
}

namespace limit_kwt_for_session {
	extern bool enabled;
	extern int value;
}

namespace limit_by_time {
	extern bool enabled;
	extern int value;
}

namespace display_off_time {
	extern bool enabled;
	extern int value;
}

namespace display_brightness_regulator {
	extern int value;
}

namespace statistics {
	struct kwt_range {
		time_t from;
		time_t offset;
		
		double delta;
		double kwt;

		//kwt_range_statistics(time_t f, time_t o) : from(f), offset(o) {};
	};
	
	extern double total_kwt;
	extern kwt_range ranges[5];

	extern void update(time_t now, double kwt); 
}

extern void rebuild_electrobox_ui();
extern void electrobox_ui_setup();
extern void electrobox_ui_update();

#endif
