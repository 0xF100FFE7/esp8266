#include "interface.h"

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
	AP_SSID = 54,
	AP_PASS = 55,
	AP_IP = 56,
	STA_ENABLED = 57,
	STA_CONNECTION_STATUS = 58,
	STA_AVAIL_NETWORKS = 59,
	STA_SCAN_NETWORK = 60,
	
	STA_STATUS = 61,
	STA_DISCONNECTED = 61,
	STA_CONNECTED = 62,
	STA_CONNECTING = 63,
	STA_TIMEOUT = 64,
	
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

struct settings {
	static bool need_commit;
	settings &changed();
	void committed();
	
	struct statistics {
		hm_t time_elapsed_for_session = 0; //1337 = 13:37
		double kwt_for_session = 0.0;
		
		double total_kwt;

		struct kwt_range {
			time_t from;
			time_t offset;
			
			double delta;
			double kwt;

			//kwt_range_statistics(time_t f, time_t o) : from(f), offset(o) {};
		} ranges[5] = {
			{0, 60 * 60, 0, 0}, //hour
			{0, 60 * 60 * 24, 0, 0}, //day
			{0, 60 * 60 * 24 * 7, 0, 0}, //week
			{0, 60 * 60 * 24 * 7 * 4, 0, 0}, //month
			{0, 60 * 60 * 24 * 7 * 4 * 12, 0, 0}, //year
		};
	};

	struct day_schedule {
		int begin = 2300; //23:00
		int end = 700; //07:00
		bool enabled = false;
	};

	struct weekly_schedule {
		bool enabled = false;
		day_schedule days[7];
	};

	uint64_t version;
	
	enum language language;
	struct weekly_schedule weekly_schedule;
	struct statistics statistics;
	
	bool sync_time_on_connect;
	
	int current_regulator;
	bool ground_check;
	bool adaptive_mode;
	bool kwt_limit_enabled;
	int kwt_limit;
	
	bool time_limit_enabled;
	int time_limit;
	
	bool display_off_enabled;
	int display_off_time;
	int display_brightness;
	
	struct settings &defaults();
	void save();		
	void load();
	
	settings();
} extern settings;

extern String l_str(unsigned i);

extern void electrobox_setup();
extern void electrobox_loop();
