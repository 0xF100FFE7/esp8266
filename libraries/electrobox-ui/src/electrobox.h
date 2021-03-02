#include "tools.h"
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
	KWT = 33,
	TIME_KWT_TITLE = 34,
	CONSUMPTION_TITLE = 35,
	CURRENT_REGULATOR_TITLE = 36,
	GROUND_CHECK_TITLE = 37,
	SCHEDULE_ENABLER_TITLE = 38,
	ADAPTIVE_MODE_TITLE = 39,
	LIMIT_KWT_TITLE = 40,
	TIMERS_TAB = 41,
	BRIGHTNESS_REGULATOR_TITLE = 42,
	LIMIT_BY_TIME_TITLE = 43,
	DISPLAY_OFF_TIME = 44,
	LOGS_TAB = 45,
	LOGS_TITLE = 46,
	CHARGE_SWITCH_TITLE = 47,
	SYNC_TIME_TITLE = 48,
	CLEAR_LOG_TITLE = 49,
	FACTORY_RESET_TITLE = 50,
	NETWORK_SETTINGS_TITLE = 51,
	SAVE_NETWORK_SETTINGS = 52,
	AP_TAB = 53,
	STA_TAB = 54,
	NETWORK_SETTINGS = 55,
	AP_SSID = 55,
	AP_PASS = 56,
	AP_IP = 57,
	STA_ENABLED = 58,
	STA_CONNECTION_STATUS = 59,
	STA_AVAIL_NETWORKS = 60,
	STA_SCAN_NETWORK = 61,
	
	STA_DISCONNECTED = 62,
	STA_CONNECTED = 63,
	STA_CONNECTING = 64,
	STA_TIMEOUT = 65,
	
	STATS_TITLE = 66,
	STATS_PREV_TITLE = 67,
	STATS_CUR_TITLE = 68,
	STATS_TEXT = 69,
	STATS_ALL_TIME = 74,
	STATS_RESET = 75,
	KWT_FOR_SESSION = 76,
	
	SYNC_TIME_NOW = 77,
	SYNC_TIME_ON_CONNECT = 78,
	ARE_YOU_SURE_RESET = 79,
	ARE_YOU_SURE_CHANGE_AP = 80,
	YES = 81,
	NO = 82,
	
	//NETWORK_DIALOG = 82,
	DHCP_ENABLED = 83,
	STA_IP = 84,
	CONNECT = 85,
	DISCONNECT = 86,
	APPLY = 87,
	CLOSE = 88,
	MANUAL_STOP = 89,
	MANUAL_START = 90,
	STA_STATUS = 91,
	STA_RSSI = 92,
	STA_SUBNET = 93,
	STA_GATEWAY = 94,
	DEVICE_IP = 95,
	
	LANGUAGE_ITEMS = 96,
	//LANGUAGE_ITEMS = 64,
};

struct settings {
	static bool need_commit;
	settings &changed();
	void committed();
	bool modified;
	
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
		size_t begin = 23 * SECS_PER_HOUR; //23:00
		size_t end = 7 * SECS_PER_HOUR; //07:00
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

//#include "electrobox.h"




//////////////////////////////////////////////////////////////////////////////////
/*				PROTOTYPES					*/
//////////////////////////////////////////////////////////////////////////////////
extern struct date_time date_time;
//extern struct sync_time sync_time;
extern struct language_selector language_selector;
extern struct factory_reset factory_reset;
extern struct network_settings network_settings;
extern struct ap_settings ap_settings;
extern struct sta_settings sta_settings;
extern struct weekly_schedule weekly_schedule;
extern struct statistics statistics;
extern struct sta_dialog sta_dialog;
extern struct manual_start_stop manual_start_stop;
void rebuild_electrobox_ui();




//////////////////////////////////////////////////////////////////////////////////
/*				SETTINGS SECTION				*/
//////////////////////////////////////////////////////////////////////////////////
struct settings &settings::defaults() {
	modified = false;
	
	version = 75;
		
	language = LANG_UA;
		
	sync_time_on_connect = false;
	
	current_regulator = 6;
	ground_check = false;
	adaptive_mode = false;
	kwt_limit_enabled = false;
	kwt_limit = 0;
	
	time_limit_enabled = false;
	time_limit = 0;
	
	display_off_enabled = false;
	display_off_time = 10;
	display_brightness = 100;
	
	return *this;
}

void settings::save()
{
	modified = true;
	if (save_settings("/electrobox.bin", "electrobox", this, sizeof(struct settings)))
		committed();
}
		
void settings::load()
{
	if (!load_settings("/electrobox.bin", "electrobox", this, sizeof(struct settings)))
		defaults();
}

struct settings &settings::changed()
{
	need_commit = true;
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




//////////////////////////////////////////////////////////////////////////////////
/*				LANGUAGE SECTION				*/
//////////////////////////////////////////////////////////////////////////////////
const char* languages[NUMBER_OF_SUPPORTED_LANGUAGES][LANGUAGE_ITEMS] PROGMEM = {
	{"Головна", "Налаштування", "Вибір мови", "Українська", "Встановити дату та час", "Рік", "Години", "Місяць", "Хвилини", "День", "Секунди", "Тижневий розклад", "Дні тижня", "Старт зарядки", "Стоп зарядки", "Увімкнути", "Понеділок", "Вівторок", "Середа", "Четвер", "П'ятниця", "Субота", "Неділя", "Статус зарядки", "Пістолет не вставлено", "Триває зарядка", "Автомобіль заряджено", "Перевірте заземлення", "Спрацював захист ПЗВ", "Спрацював захист по напрузі (більше 270V)", "Спрацював захист по току (більше 50А)", "Очікує зарядки по розкладу", "Ручна зупинка", "кВт", "Час та кВт за сесію", "Напруга, струм та потужність", "Регулювання струму", "Перевірка заземлення", "Заряджати по розкладу", "Адаптивний режим", "Обмежити кількість кВт за сесію", "Таймери", "Регулювання яскравості дисплею", "Обмежити тривалість заряду в годинах", "Час до вимкнення дисплею в хвилинах", "Статистика", "Лог змін статусу", "Ввімкнути зарядку", "Синхронізувати час з браузерним", "Очистка логу", "Скинути налаштування", "Налаштування мережі", "Зберегти зміни", "Точка доступу", "Станція", "SSID:", "Пароль:", "IP адрес:", "Cтанція ввімкнена:", "Статус підключення:", "Доступні мережі:", "Сканувати мережу", "Підключення відсутнє", "Підключено", "Спроба підключення", "Неможливо підключитися", "Статистика за період", "За минулу(ий)", "За теперішню(ній)", "Годину:", "День:", "Тиждень:", "Місяць:", "Рік:", "За весь час:", "Обнулити статистику", "Споживано за сесію", "Автоматично", "При підключенні:", "Ви дійсно хочете скинути налаштування?", "Зміна налаштувань точки доступу призведе до перезавантаження пристрою. Ви дійсно бажаєте продовжити?", "Так", "Ні", "DHCP: ", "Статичний ip: ", "Підключитися", "Відключитися", "Застосувати", "Закрити", "Ручна зупинка", "Ручний старт", "Статус:", "Рівень сигналу:", "Маска підмережі: ", "Шлюз: ", "IP зарядки:"},
	{"Главная", "Настройки", "Выбор языка", "Русский", "Установить дату и время", "Год", "Часы", "Месяц", "Минуты", "День", "Секунды", "Еженедельное расписание", "Дни недели", "Старт заряда", "Стоп зарада", "Включить", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье", "Статус зарядки", "Пистолет не вставлен", "Продолжается зарядка", "Автомобиль заряжен", "Проверьте заземление", "Сработала защита УЗО", "Сработала защита по напряжению (более 270V)", "Сработала защита по току (более 50A)", "Ожидание зарядки по расписанию", "Ручная остановка", "кВт", "Время и кВт за сессию", "Напряжение ток и мощность", "Регулировка тока", "Проверка заземления", "Заряжать по расписанию", "Адаптивний режим", "Ограничить кол-во кВт за сессию", "Таймеры", "Регулировка яркости дисплея", "Ограничить время заряда в часах", "Время до отключения дисплея в минутах", "Статистика", "Лог смен статуса", "Включить зарядку", "Синхронизировать время с браузерным", "Очистить лог", "Сбросить настройки", "Настройки сети", "Сохраниить изменения", "Точка доступа", "Станция", "SSID:", "Пароль:", "IP адрес:", "Станция включена:", "Статус подключения:", "Доступные сети:", "Сканировать сети", "Подключение отсутствует", "Подключено к: ", "Попытка подключения к: ", "Невозможно подключиться к: ", "Статистика за период", "За предыдущий(ую))", "За Текущий(ую)", "Час:", "День:", "Неделю:", "Месяц:", "Год:", "За всё время:", "Обнулить статистику", "Употреблено за сессию", "Автоматически", "При подключении:", "Вы действительно хотите сбросить настройки?", "Изменение настроек точки доступа приведет к перезагрузке устройства. Вы действительно хотите продолжить?", "Да", "Нет", "DHCP: ", "Статический ip: ", "Подключиться", "Отключиться", "Применить", "Закрыть", "Ручная остановка", "Ручной старт", "Статус:", "Уровень сигнала:", "Маска подсети: ", "Шлюз", "IP зарядки:"},
	{"Home", "Settings", "Select language", "English", "Setup date and time", "Year", "Hours", "Month", "Minutes", "Day", "Seconds", "Weekly sсhedule", "Days of the week", "Start charging", "Stop charging", "Enable", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday", "Charger status", "Pistol not inserted", "Charging continues", "Vehicle is charged", "Check ground", "RCD protection blocking", "Voltage protection blocking (above 270V)", "Current protection blocking (above 50A)", "Waiting for charging by chedule", "Manual stop", "kWt", "Time and kWt for session", "Voltage, current and kWt", "Current regulator", "Ground check", "Charge by schedule", "Adaptive mode", "Limit kWt for session", "Timers", "Display brightness regulator", "Limit charge time by hours", "Time to turn off the display in minutes", "Statistics", "Status change log", "Enable charger", "Synchronize time with browser time", "Clear log", "Factory reset", "Network settings", "Save settings", "Access point", "Station", "SSID:", "Password:", "IP:", "Station enabled:", "Connection status:", "Available networks:", "Scan networks", "No connection", "Connected to: ", "Attempting to connect to: ", "Failed to connect to: ", "Statistics for a period", "Previous", "Current", "Hour:", "Day:", "Week:", "Month:", "Year:", "For all time:", "Reset statistics", "Consumed for session", "Automatically", "On connect:", "Do you really want to reset?", "Changing the access point settings will reboot the device. Do you really want to continue?", "Yes", "No", "DHCP: ", "Static ip: ", "Connect", "Disconnect", "Apply", "Close", "Manual stop", "Manual start", "Status:", "Signal level:", "Subnetwork mask: ", "Gateway: ", "Charger IP:"},
};

//Localized string
String l_str(unsigned i)
{
	return String(languages[settings.language][i]);
}




//////////////////////////////////////////////////////////////////////////////////
/*				NAVIGATION PANEL WIDGETS			*/
//////////////////////////////////////////////////////////////////////////////////
struct tab_navigation_widget {
	tab home;
	tab settings;
	tab timers;
	tab logs;
	
	tab *selected = &home; //User specified, does not change its state via websockets
	
	packet build()
	{
		return	
			home.pack(root, (attr::panel = "nav", (attr::text = l_str(HOME_TAB), selected == &home ? attr::selected = true : none), attr::direction = DIR_H, attr::wrap = true)) +
			settings.pack(root, (attr::panel = "nav", (attr::text = l_str(SETTINGS_TAB), selected == &settings ? attr::selected = true : none), attr::direction = DIR_H, attr::wrap = true)) +
			timers.pack(root, (attr::panel = "nav", (attr::text = l_str(TIMERS_TAB), selected == &timers ? attr::selected = true : none), attr::direction = DIR_H, attr::wrap = true)) +
			logs.pack(root, (attr::panel = "nav", (attr::text = l_str(LOGS_TAB), selected == &logs ? attr::selected = true : none), attr::tab_align = ALIGN_RIGHT, attr::direction = DIR_H, attr::wrap = true));
	}
} navigation_panel;




//////////////////////////////////////////////////////////////////////////////////
/*				STATISTICS WIDGETS				*/
//////////////////////////////////////////////////////////////////////////////////

struct statistics {
	struct box box;	
	struct text total_kwt_text;
	struct tab prev_stats_tab, cur_stats_tab;
	struct text prev_text[5], cur_text[5];
	
	struct button reset;
 
	double prev_kwt = 0.0;
	bool need_update = false;

	packet update_stat(int i)
	{
		return
		prev_text[i].pack(attr::text = l_str(STATS_TEXT + i) + " " + settings.statistics.ranges[i].delta + l_str(KWT)) +
		cur_text[i].pack(attr::text = l_str(STATS_TEXT + i) + " " + settings.statistics.ranges[i].kwt + l_str(KWT));
	}
	
	//TODO make statistics carrier protection
	void update(time_t now, double kwt)
	{
		packet buf;
		double delta = kwt - prev_kwt;
		prev_kwt = kwt;
		
		settings.statistics.total_kwt += delta;
		
		for (int i = 0; i < 5; i++) {
			DEBUG_MSG("Statistics for %i: carry: %s, now: %lu, from: %lu, offset: %lu.\n", i, ((now > settings.statistics.ranges[i].from + settings.statistics.ranges[i].offset) ? "true" : "false"), now, settings.statistics.ranges[i].from, settings.statistics.ranges[i].offset);
			if (now > settings.statistics.ranges[i].from + settings.statistics.ranges[i].offset) {
				settings.statistics.ranges[i].from = now;
				settings.statistics.ranges[i].delta = settings.statistics.ranges[i].kwt + delta;
				settings.statistics.ranges[i].kwt = 0;
			} else {
				settings.statistics.ranges[i].kwt += delta;
			}
			buf += update_stat(i);
		}
		settings.changed();
		
		buf += total_kwt_text.pack(attr::text = l_str(STATS_ALL_TIME) + " " + settings.statistics.total_kwt + l_str(KWT));
		buf.send_all();
	}
	
	static void button_callback(struct button& id, client_id_t sender)
	{
		packet buf;
		time_t t = now();
		for (int i = 0; i < 5; i++) {
			settings.statistics.ranges[i].from = t;
			settings.statistics.ranges[i].kwt = settings.statistics.ranges[i].delta = 0;
			buf += ::statistics.update_stat(i);
		} //total kwt not changed
		settings.changed();
		
		buf.send_all();
	}

	statistics() : reset(button_callback) {}
	packet build()
	{
		packet buf = box.pack(navigation_panel.logs, (attr::text = l_str(STATS_TITLE), attr::fill = true)) + total_kwt_text.pack(box, (attr::text = l_str(STATS_ALL_TIME) + " " + settings.statistics.total_kwt + l_str(KWT))) +
			prev_stats_tab.pack(box, (attr::text = l_str(STATS_PREV_TITLE), attr::panel = "stats", attr::selected = true)) + cur_stats_tab.pack(box, (attr::text = l_str(STATS_CUR_TITLE), attr::panel = "stats"));
			for (int i = 0; i < 5; i++) {
				buf +=
					prev_text[i].pack(prev_stats_tab, (attr::text = l_str(STATS_TEXT + i) + " " + settings.statistics.ranges[i].delta + l_str(KWT), attr::align = ALIGN_LEFT)) +
					cur_text[i].pack(cur_stats_tab, (attr::text = l_str(STATS_TEXT + i) + " " + settings.statistics.ranges[i].kwt + l_str(KWT), attr::align = ALIGN_LEFT));
			}
			
		buf += reset.pack(box, (attr::text = l_str(STATS_RESET)));
		return buf;
	}
} statistics;




//////////////////////////////////////////////////////////////////////////////////
/*				HOME PANEL WIDGETS				*/
//////////////////////////////////////////////////////////////////////////////////
struct charge_status_widget {
	enum status {
		C_UNSET,
		C_CHARGING,
		C_CHARGED,
		C_CHECK_GROUND,
		C_RCD_TRIGGERED,
		C_VOLTAGE_PROTECTION_TRIGGERED,
		C_CURRENT_PROTECTION_TRIGGERED,
		C_TIMER_ENABLED,
		C_MANUAL_STOP,
		
		C_UNKNOWN
	};
	
	enum status value = C_UNSET;
	
	struct box box;
	struct text text;
	
	void update(int);
	packet build();
} extern charge_status;

struct manual_start_stop {
	bool trigger = false;
	
	struct box box;
	struct button button;
	
	static void button_callback(struct button &id, client_id_t sender)
	{
		::manual_start_stop.trigger = true; //Change to false somewhere else
	}
	
	manual_start_stop() : button(button_callback) {}
	packet build()
	{
		bool charging = charge_status.value == charge_status_widget::C_CHARGING;
		return
		box.pack(navigation_panel.home, (attr::text = charging ? l_str(MANUAL_STOP) : l_str(MANUAL_START), attr::fill = true)) +
		button.pack(box, (attr::background = charging ? "red" : "green"));
	}
} manual_start_stop;

void charge_status_widget::update(int val)
{
	if (val == C_CHARGING && value != C_CHARGING) {
		statistics.need_update = true;
	} else if (val != C_CHARGING && value == C_CHARGING) {
		statistics.update(now(), settings.statistics.kwt_for_session);
		statistics.need_update = false;
		statistics.prev_kwt = 0.0;
	}
		
	value = (status)val;
	//TODO put value into log here
	(manual_start_stop.build() + text.pack(attr::text = l_str(CHARGER_STATUS + val))).send_all();
}

packet charge_status_widget::build()
{
	return box.pack(navigation_panel.home, (attr::text = l_str(CHARGER_STATUS_TITLE), attr::direction = DIR_H)) + text.pack(box, (attr::text = l_str(CHARGER_STATUS + (int)value)));
}

struct charge_status_widget charge_status;

struct time_and_kwt_for_session {	
	struct box box;
	struct text text;
	
	String get()
	{
		return hm_to_str(settings.statistics.time_elapsed_for_session) + " - " + settings.statistics.kwt_for_session + " " + l_str(KWT);
	}
	
	void update(int h, int m, float k)
	{
		settings.statistics.time_elapsed_for_session = convert_to_hm(h, m);
		settings.statistics.kwt_for_session = k;
		text.pack(attr::text = get()).send_all();
	}
	
	packet build()
	{
		return box.pack(navigation_panel.home, (attr::text = l_str(TIME_KWT_TITLE), attr::direction = DIR_H)) + text.pack(box, (attr::text = get()));
	}
} time_and_kwt_for_session;

struct consumption {
	float voltage = 0;
	float current = 0;
	float kwt = 0;
	
	struct box box;
	struct text text;
	
	String get()
	{
		return String(voltage) + "V  " + current + "A  " + kwt + "kWt";
	}
	
	void update(float v, float c, float k)
	{
		voltage = v, current = c, kwt = k;
		text.pack(attr::text = get()).send_all();
	}
	
	packet build()
	{
		return box.pack(navigation_panel.home, (attr::text = l_str(CONSUMPTION_TITLE), attr::direction = DIR_H)) + text.pack(box, (attr::text = get()));
	}
} consumption;

struct current_regulator {
	struct box box;
	struct range range;
	
	void update(int value)
	{
		range.slide(settings.changed().current_regulator, value).send_all();
	}
	
	static void range_callback(struct range &id, int value, client_id_t sender)
	{
		id.slide(settings.changed().current_regulator, value).send_all(); //TODO link update function here
	}
	
	current_regulator() : range(range_callback) {};
	packet build()
	{
		return
			box.pack(navigation_panel.home, (attr::text = l_str(CURRENT_REGULATOR_TITLE), attr::direction = DIR_H, attr::fill = true)) + 
			range.pack(box, (attr::min = 6, attr::max = 48, attr::value = settings.current_regulator));
	}
} current_regulator;

struct ground_check {
	struct box box;
	struct switcher switcher;
	
	static void switcher_callback(struct switcher &id, client_id_t sender)
	{
		id.turn(settings.changed().ground_check).send_all();
	}
	
	ground_check() : switcher(switcher_callback) {};
	packet build()
	{
		return box.pack(navigation_panel.home, (attr::text = l_str(GROUND_CHECK_TITLE), attr::direction = DIR_H)) + switcher.pack(box, switcher.get(settings.ground_check));
	}
} ground_check;

struct adaptive_mode {
	struct box box;
	struct switcher switcher;
	
	static void switcher_callback(struct switcher &id, client_id_t sender)
	{
		id.turn(settings.changed().adaptive_mode).send_all();
	}
	
	adaptive_mode() : switcher(switcher_callback) {};
	packet build()
	{
		return box.pack(navigation_panel.home, (attr::text = l_str(ADAPTIVE_MODE_TITLE), attr::direction = DIR_H)) + switcher.pack(box, switcher.get(settings.ground_check));
	}
} adaptive_mode;




//////////////////////////////////////////////////////////////////////////////////
/*				SETTINGS PANEL WIDGETS				*/
//////////////////////////////////////////////////////////////////////////////////
struct display_brightness_regulator {
	struct box box;
	struct range range;
	
	static void range_callback(struct range &id, int value, client_id_t sender)
	{
		id.slide(settings.changed().display_brightness, value).send_all();
	}
	
	display_brightness_regulator() : range(range_callback) {};
	packet build()
	{
		return box.pack(navigation_panel.settings, (attr::text = l_str(BRIGHTNESS_REGULATOR_TITLE), attr::direction = DIR_H, attr::fill = true, attr::height = "content")) +
		range.pack(box, (attr::min = 0, attr::max = 100, attr::value = settings.display_brightness));
	}
} display_brightness_regulator;

struct display_off_time {
	struct box box;
	struct switcher switcher;
	struct range range;
	
	static void switcher_callback(struct switcher &id, client_id_t sender)
	{
		id.turn(settings.changed().display_off_enabled).send_all();
	}
	
	static void range_callback(struct range &id, int value, client_id_t sender)
	{
		id.slide(settings.changed().display_off_time, value).send_all();
	}
	
	display_off_time() : switcher(switcher_callback), range(range_callback) {};
	packet build()
	{
		return box.pack(navigation_panel.settings, (attr::text = l_str(DISPLAY_OFF_TIME), attr::direction = DIR_H, attr::fill = true)) +
		switcher.pack(box, (switcher.get(settings.display_off_enabled), attr::width = 5)) + range.pack(box, (attr::min = 1, attr::max = 60, attr::value = settings.display_off_time));
	}
} display_off_time;

struct date_time {
	bool changed = false; //if it was changed manually

	struct box box;
	struct box wrapper;
	struct date_field date;
	struct time_field time;
	struct time browser_time;
	struct button button;
		
	static void browser_time_callback(struct time &id, time_t t) //get time
	{
		::date_time.changed = true;
		setTime(t);
		::date_time.update();
		
		//we have to check and update statistics if time changed;
		statistics.update(t, 0.0); //we do not put kwt's here(just time update) //TODO
	}

	static void button_callback(struct button &id, client_id_t sender)
	{
		::date_time.browser_time.pack().send(sender);
	}
	
	template <class T>
	static void field_callback(T &id, String value, client_id_t sender)
	{
		time_t t = id.parse(value);
		setTime(t);
		id.pack(attr::value = id.formatted(t)).send_all();
		
		//we have to check and update statistics if time changed;
		statistics.update(t, 0.0); //we do not put kwt's here(just time update) //TODO
	}
	
	void update()
	{
		time_t t = now();
		(date.pack(attr::value = date.formatted(t)) + time.pack(attr::value = time.formatted(t))).send_all();
	}
	
	date_time() : date(field_callback<struct date_field>), time(field_callback<struct time_field>), browser_time(browser_time_callback), button(button_callback) {};
	packet build()
	{
		time_t t = now();
		return box.pack(navigation_panel.settings, (attr::text = l_str(DATE_TIME_TITLE))) +
		wrapper.pack(box, (attr::direction = DIR_H)) + date.pack(wrapper, attr::value = date.formatted(t)) + time.pack(wrapper, attr::value = time.formatted(t)) +
		button.pack(box, (attr::text = l_str(SYNC_TIME_NOW), attr::background = "orange"));
	}
} date_time;

/*struct sync_time {	
	struct time browser_time;
	
	struct box box;
	struct button button;
	
	struct box wrapper;
	struct text text;
	struct switcher switcher;
	
	static void browser_time_callback(struct time &id, time_t t) //get time
	{
		date_time.changed = true;
		setTime(t);
		date_time.update();
		
		//we have to check and update statistics if time changed;
		statistics.update(t, 0.0); //we do not put kwt's here(just time update) //TODO
	}

	static void button_callback(struct button &id, client_id_t sender)
	{
		::sync_time.browser_time.pack().send(sender);
	}
	
	static void switcher_callback(struct switcher &id, client_id_t sender)
	{
		id.turn(settings.changed().sync_time_on_connect).send_all();
	}
	
	sync_time() : browser_time(browser_time_callback), button(button_callback), switcher(switcher_callback) {};
	packet build()
	{		
		return box.pack(navigation_panel.settings, (attr::text = l_str(SYNC_TIME_TITLE), attr::direction = DIR_V)) +
			button.pack(box, (attr::text = l_str(SYNC_TIME_NOW), attr::background = "orange")) +
			
			wrapper.pack(box, (attr::direction = DIR_H)) +
			text.pack(wrapper, (attr::text = l_str(SYNC_TIME_ON_CONNECT))) + switcher.pack(wrapper, switcher.get(settings.sync_time_on_connect)) +
			
			(settings.sync_time_on_connect ? browser_time.pack() : packet());
	}
} sync_time;*/

struct language_selector {
	struct box box;
	struct language_radio_item : radio { //wrapper inheritance. Not sure if there is no overhead guaranteed (WARNING)
		static void item_callback(radio &id, client_id_t sender) {
			struct radio &old_item = ::language_selector.item[settings.language];
			
			if (&id != &old_item) {
				settings.changed().language = (enum language)class_index(::language_selector.item[0], id);
				//tab_navigation::selected = &tab_navigation::settings;
				rebuild_electrobox_ui();
			}
		}
		
		language_radio_item() : radio(item_callback) {};
	} item[NUMBER_OF_SUPPORTED_LANGUAGES];
	
	packet build()
	{		
		packet buf;
		buf = box.pack(navigation_panel.settings, (attr::text = l_str(LANGUAGE_SELECTOR_TITLE), attr::height = "content"));
		for (int i = 0; i < NUMBER_OF_SUPPORTED_LANGUAGES; i++)
			buf += item[i].pack(box, (attr::text = languages[i][LANGUAGE_NAME], item[i].get(i == (int)settings.language ? true : false)));
		return buf;
	}
} language_selector;

struct factory_reset {	
	struct box box;
	struct applier applier;
	
	struct dialog dialog;
	struct box wrapper; //not very bright solution, may change in future because dialogs suck for now, really. TODO
	struct button yes, no;
	
	static void yes_callback(struct button &id, client_id_t sender)
	{
		::factory_reset.dialog.pack(attr::display = false).send(sender);
		network::ap.defaults().save();
		network::sta.defaults().save();
		settings.defaults().save();
		
		//tab_navigation::selected = &tab_navigation::settings;
		rebuild_electrobox_ui();
	}
	
	static void no_callback(struct button &id, client_id_t sender)
	{
		::factory_reset.dialog.pack(attr::display = false).send(sender);
	}
	
	static void applier_callback(struct applier &id, client_id_t sender)
	{
		::factory_reset.dialog.pack(attr::display = true).send(sender);
	}

	factory_reset() : yes(yes_callback), no(no_callback), applier(applier_callback) {};
	packet build()
	{
		return
		box.pack(navigation_panel.settings, (/*attr::text = l_str(FACTORY_RESET_TITLE), */attr::fill = true)) +
		applier.pack(box, (attr::text = l_str(FACTORY_RESET_TITLE), attr::background = "orange", applier.get(settings.modified))) +
		//sync_time.build();
		
		dialog.pack(root, (attr::display = false)) + wrapper.pack(dialog, (attr::text = l_str(ARE_YOU_SURE_RESET), attr::direction = DIR_H)) +
		yes.pack(wrapper, (attr::text = l_str(YES))) + no.pack(wrapper, (attr::text = l_str(NO)));
	}
} factory_reset;

//Maybeh divide network settings onto two parts? TODO
#define MAX_STATIONS_SHOWN_AT_ONCE 8
struct network_settings {
	struct box box;
	
	struct tab ap_tab;
	struct tab sta_tab;
	
	packet build()
	{
		return
		box.pack(navigation_panel.settings, (attr::text = l_str(NETWORK_SETTINGS_TITLE), attr::fill = true)) +
		ap_tab.pack(box, (attr::text = l_str(AP_TAB), attr::panel = "net", attr::direction = DIR_V, attr::selected = true, attr::wrap = true)) +
		sta_tab.pack(box, (attr::text = l_str(STA_TAB), attr::panel = "net", attr::direction = DIR_V, attr::wrap = true));
	}
} network_settings;

struct ap_settings {
	struct box ap_text_col, ap_field_col;
	struct box tab_wrapper;
	struct text ap_ssid_text, ap_pass_text, ap_ip_text;
	struct field ap_ssid_field, ap_pass_field, ap_ip_field;
	struct applier apply_settings;
	bool applier_enabled = false;
	
	struct dialog dialog;
	struct box wrapper; //TODO
	struct button yes, no;

	static void ap_ssid_field_callback(struct field &id, String value, client_id_t sender)
	{
		value.toCharArray(network::ap.changed().ssid, 80); //TODO check ssid maybeh?
		id.pack(attr::value = String(network::ap.ssid)).send_all();
	}
	
	static void ap_pass_field_callback(struct field &id, String value, client_id_t sender)
	{
		if (network::passphrase_is_valid(value))
			value.toCharArray(network::ap.changed().pass, 80);
		id.pack(attr::value = String(network::ap.pass)).send_all();
	}
	
	static void ap_ip_field_callback(struct field &id, String value, client_id_t sender)
	{
		IPAddress ip;
		ip.fromString(value);
		network::ap.changed().ip = network::ap.gateway = (uint32_t)ip;
		id.pack(attr::value = IPAddress(network::ap.ip).toString()).send_all();
	}

	static void applier_callback(struct applier &id, client_id_t sender)
	{
		::ap_settings.dialog.pack(attr::display = true).send(sender);
	}

	static void yes_callback(struct button &id, client_id_t sender)
	{
		::ap_settings.dialog.pack(attr::display = false).send(sender);
		network::end();
		network::ap.save();
		DEBUG_MSG("Ap settings applied, restarting...\n");
		delay(1000);
		ESP.restart();
	}
	
	static void no_callback(struct button &id, client_id_t sender)
	{
		::ap_settings.dialog.pack(attr::display = false).send(sender);
	}

	ap_settings() : ap_ssid_field(ap_ssid_field_callback), ap_pass_field(ap_pass_field_callback), ap_ip_field(ap_ip_field_callback), yes(yes_callback), no(no_callback), apply_settings(applier_callback) {}
	packet build()
	{
		return
		tab_wrapper.pack(network_settings.ap_tab, (attr::direction = DIR_H)) +
		ap_text_col.pack(tab_wrapper, (attr::direction = DIR_V, attr::width = "content")) +
		ap_field_col.pack(tab_wrapper, (attr::direction = DIR_V)) +
		ap_ssid_text.pack(ap_text_col, (attr::text = l_str(AP_SSID))) + ap_pass_text.pack(ap_text_col, (attr::text = l_str(AP_PASS))) + ap_ip_text.pack(ap_text_col, (attr::text = l_str(AP_IP))) +
		ap_ssid_field.pack(ap_field_col, (attr::value = String(network::ap.ssid))) + ap_pass_field.pack(ap_field_col, (attr::value = String(network::ap.pass))) + 
			ap_ip_field.pack(ap_field_col, (attr::value = IPAddress(network::ap.ip).toString())) +
		apply_settings.pack(network_settings.ap_tab, (attr::text = l_str(SAVE_NETWORK_SETTINGS), attr::background = "orange", apply_settings.get(applier_enabled))) +
		
		dialog.pack(root, (attr::display = false)) + wrapper.pack(dialog, (attr::text = l_str(ARE_YOU_SURE_CHANGE_AP), attr::direction = DIR_H)) +
		yes.pack(wrapper, (attr::text = l_str(YES))) + no.pack(wrapper, (attr::text = l_str(NO)));
	}
} ap_settings;

struct sta_dialog {
	char pass[80];
	uint32_t ip;
	uint32_t gateway;
	uint32_t subnet;
	int sta_id = -1;
	bool secure = false;
	bool dhcp;
	
	struct dialog dialog;
	struct box box;
	
	struct text status;
	
	struct box dhcp_wrapper;
	struct text dhcp_text;
	struct switcher dhcp_enabler;
	
	struct box manual_wrapper;
	struct box addr_text_wrapper, addr_field_wrapper;
	struct text ip_text;
	struct field ip_field;
	struct text subnet_text;
	struct field subnet_field;
	struct text gateway_text;
	struct field gateway_field;
	
	struct box pass_wrapper;
	struct text pass_text;
	struct field pass_field;
	
	struct box button_wrapper;
	//struct applier apply;
	struct button connect;
	struct button disconnect;
	struct button close;
	
	void init()
	{
		dhcp = network::sta.dhcp;
		ip = network::sta.ip;
		subnet = network::sta.subnet;
		gateway = network::sta.gateway;
		String(network::sta.pass).toCharArray(pass, 80);
	}
	
	static void switcher_callback(struct switcher &id, client_id_t sender)
	{
		(id.turn(::sta_dialog.dhcp) + ::sta_dialog.manual_wrapper.pack(attr::display = !::sta_dialog.dhcp)).send_all();
	}
	
	static void ip_field_callback(struct field &id, String value, client_id_t sender)
	{
		IPAddress ip;
		ip.fromString(value);
		::sta_dialog.ip = (uint32_t)ip;
		id.pack(attr::value = ip.toString()).send_all();
	}
	
	static void subnet_field_callback(struct field &id, String value, client_id_t sender)
	{
		IPAddress ip;
		ip.fromString(value);
		::sta_dialog.subnet = (uint32_t)ip;
		id.pack(attr::value = ip.toString()).send_all();
	}
	
	static void gateway_ip_field_callback(struct field &id, String value, client_id_t sender)
	{
		IPAddress ip;
		ip.fromString(value);
		::sta_dialog.gateway = (uint32_t)ip;
		id.pack(attr::value = ip.toString()).send_all();
	}
	
	static void pass_field_callback(struct field &id, String value, client_id_t sender)
	{
		if (network::passphrase_is_valid(value))
			value.toCharArray(::sta_dialog.pass, 80);
		id.pack(attr::value = ::sta_dialog.pass).send_all();
	}

	void apply_sta(client_id_t sender)
	{
		//dialog.pack(attr::display = false).send(sender); //Do not close window
		if (secure)
			String(pass).toCharArray(network::sta.pass, 80);
		else
			network::sta.pass[0] = '\0';

		network::sta.secure = secure;
		network::sta.dhcp = dhcp;
		network::sta.ip = ip;
		network::sta.subnet = subnet;
		network::sta.gateway = gateway;
		network::refresh_sta();
	}
	
	/*static void apply_callback(struct applier &id, client_id_t sender)
	{
		::sta_dialog.apply_sta(sender);
	}*/

	static void connect_callback(struct button &id, client_id_t sender)
	{
		network::get_station_name(::sta_dialog.sta_id).toCharArray(network::sta.ssid, 80);
		::sta_dialog.apply_sta(sender);
	}
	
	static void disconnect_callback(struct button &id, client_id_t sender)
	{
		::sta_dialog.dialog.pack(attr::display = false).send(sender);
		network::disconnect_sta();
	}
	
	static void close_callback(struct button &id, client_id_t sender)
	{
		::sta_dialog.dialog.pack(attr::display = false).send(sender);
		
	}
	
	String status_to_str()
	{
		switch (network::sta_status) {
		case network::STA_BEGIN_CONNECTION:
		case network::STA_ATTEMPT_TO_CONNECT:
		case network::STA_SWITCH:
			return l_str(STA_CONNECTING);
		case network::STA_CONNECTED:
			return l_str(STA_CONNECTED);
		case network::STA_LOST_CONNECTION:
		case network::STA_TIMEOUT_TO_RECONNECT:
			return l_str(STA_TIMEOUT);
		case network::STA_DISCONNECTED:
			return l_str(STA_DISCONNECTED);
		default:
			return "ERROR";
		}
	}
	
	packet update(int idx)
	{
			sta_id = idx;
			secure = network::connection_is_secure(idx);
			
			return
			status.pack(attr::text = 
				l_str(AP_SSID) + " " + network::get_station_name(idx) + "\n" +
				l_str(DEVICE_IP) + " " + network::get_connected_station_ip().toString() + "\n" +
				l_str(STA_RSSI) + " " + network::get_station_rssi_in_percents(idx) + "%\n" +
				l_str(STA_STATUS) + " " + (idx == -1 ? status_to_str() : l_str(STA_DISCONNECTED))
			) +
			dhcp_wrapper.pack(attr::display = idx != -1) +
			manual_wrapper.pack(attr::display = (idx != -1 && !dhcp)) +
			pass_wrapper.pack(attr::display = (idx != -1 && secure)) +
			disconnect.pack(attr::display = idx == -1) +
			connect.pack(attr::display = idx != -1)/* + apply.pack(attr::display = idx == -1))*/;
	}
	
	sta_dialog() : dhcp_enabler(switcher_callback), ip_field(ip_field_callback), subnet_field(subnet_field_callback), gateway_field(gateway_ip_field_callback), pass_field(pass_field_callback), /*apply(apply_callback),*/ connect(connect_callback), disconnect(disconnect_callback), close(close_callback) {}
	packet build()
	{
		return
		dialog.pack(root, (attr::display = false)) + box.pack(dialog, (attr::text = l_str(NETWORK_SETTINGS))) + 
		status.pack(box, (attr::wrap = true)) +
		dhcp_wrapper.pack(box, (attr::direction = DIR_H)) + dhcp_text.pack(dhcp_wrapper, (attr::text = l_str(DHCP_ENABLED))) + dhcp_enabler.pack(dhcp_wrapper, dhcp_enabler.get(dhcp)) +
		manual_wrapper.pack(box, (attr::direction = DIR_H, attr::disabled = dhcp)) + addr_text_wrapper.pack(manual_wrapper, (attr::direction = DIR_V, attr::width = "content")) + addr_field_wrapper.pack(manual_wrapper, (attr::direction = DIR_V)) + 
			ip_text.pack(addr_text_wrapper, (attr::text = l_str(AP_IP))) + subnet_text.pack(addr_text_wrapper, (attr::text = l_str(STA_SUBNET))) + gateway_text.pack(addr_text_wrapper, (attr::text = l_str(STA_GATEWAY))) +
			ip_field.pack(addr_field_wrapper, (attr::value = IPAddress(ip).toString())) + subnet_field.pack(addr_field_wrapper, (attr::value = IPAddress(subnet).toString())) + gateway_field.pack(addr_field_wrapper, (attr::value = IPAddress(gateway).toString())) +
		pass_wrapper.pack(box, (attr::direction = DIR_H)) + pass_text.pack(pass_wrapper, (attr::text = l_str(AP_PASS))) + pass_field.pack(pass_wrapper, (attr::value = String(pass))) +
		button_wrapper.pack(box, (attr::direction = DIR_H)) + /*apply.pack(button_wrapper, (attr::text = l_str(APPLY))) +*/ connect.pack(button_wrapper, (attr::text = l_str(CONNECT))) + disconnect.pack(button_wrapper, (attr::text = l_str(DISCONNECT))) + 
		close.pack(button_wrapper, (attr::text = l_str(CLOSE)));
	}
} sta_dialog;

struct sta_settings {
	enum network::sta_status old_status;

	struct box connection_status_wrapper;
	struct text connection_status;
	struct box avail_stations_wrapper;
	struct applier scan_sta;
	
	struct avail_stations : radio {
		static void select_station_callback(struct radio &id, client_id_t sender)
		{
			int idx = class_index(::sta_settings.avail_stations[0], id);
			idx = idx ? idx - 1 : -1;
			
			(sta_dialog.update(idx) + sta_dialog.dialog.pack(attr::display = true)).send_all();
		}
		 
		 avail_stations() : radio(select_station_callback) {}
	} avail_stations[MAX_STATIONS_SHOWN_AT_ONCE]; //Not very dynamic way to do it, but OK yet. TODO

	String short_sta_info(int i)
	{
		return String((network::connection_is_secure(i) ? " &#128274;" : "&#128275;")) + " (&#128246;" + network::get_station_rssi_in_percents(i) + "%)";
	}

	packet list_avail_stations()
	{
		packet buf;// = sta_dialog.update(-1); //same as: int random() {return 4;}
		DEBUG_MSG("Total AP's found: %i\n", network::avail_networks);
		DEBUG_MSG("Network status: %i\n", network::sta_status);

		String color;
		switch (network::sta_status) {
		case network::STA_CONNECTED:
		case network::STA_DISCONNECTED:
			color = "green";
			break;
		case network::STA_BEGIN_CONNECTION:
		case network::STA_ATTEMPT_TO_CONNECT:
		case network::STA_SWITCH:
			color = "orange";
			break;
		default:
			color = "red";
			break;
		}

		if (network::sta_status != network::STA_DISCONNECTED)
			buf += avail_stations[0].pack(network_settings.sta_tab, (attr::text = network::get_station_name(-1) + short_sta_info(-1), attr::display = true, attr::background = color)); //zero slot is reserved for running station
		else
			buf += avail_stations[0].pack(network_settings.sta_tab, (attr::display = false));
		
		for (int i = 0; i < network::avail_networks; i++) {
			DEBUG_MSG("\t%i: %s\n", i + 1, network::get_station_name(i).c_str());
			if (network::get_station_name(-1) == network::get_station_name(i))
				buf += avail_stations[i + 1].pack(network_settings.sta_tab, (attr::display = false)); //skip same station as from zero slot;
			else
				buf += avail_stations[i + 1].pack(network_settings.sta_tab, (attr::text = network::get_station_name(i) + short_sta_info(i), attr::display = true));
		}
		
		//do not display entries further
		for (int i = network::avail_networks; i < MAX_STATIONS_SHOWN_AT_ONCE - 1; i++)
			buf += avail_stations[i + 1].pack(network_settings.sta_tab, (attr::display = false));
		
		return buf;
	}
	
	static void scan_sta_callback(struct applier &id, client_id_t sender)
	{
		packet buf = id.pack(attr::disabled = false);
		for (int i = 0; i < MAX_STATIONS_SHOWN_AT_ONCE; i++)
			buf += ::sta_settings.avail_stations[i].pack(network_settings.sta_tab, (attr::display = false));
		buf.send_all();
		network::begin_scan();
	}
	
	sta_settings() : scan_sta(scan_sta_callback), old_status(network::STA_DISCONNECTED) {}
	packet build()
	{
		return
		avail_stations_wrapper.pack(network_settings.sta_tab) + avail_stations_wrapper.pack(attr::text = l_str(STA_AVAIL_NETWORKS)) + //lil hack, TODO (put something else here)
		list_avail_stations() +
		scan_sta.pack(network_settings.sta_tab, (attr::text = l_str(STA_SCAN_NETWORK), attr::background = "cornflowerblue", scan_sta.get(!network::scan))); //TODO better applier setter
	}
} sta_settings;




//////////////////////////////////////////////////////////////////////////////////
/*				TIMER WIDGETS					*/
//////////////////////////////////////////////////////////////////////////////////

struct weekly_schedule {
	struct box wrapper;	
	struct box box;
	struct box cols[4];
	struct text col_names[4];
	struct box enabler_wrapper;
	struct text enabler_text;
	struct switcher enabled;
	bool blocking = false;
	
	struct day_schedule {
		struct text name;
		struct time_field begin;
		struct time_field end;
		struct switcher enabled;
		
		static void begin_callback(struct time_field &id, String value, client_id_t sender)
		{
			time_t t = id.parse(value);
			settings.changed().weekly_schedule.days[class_index(::weekly_schedule.days[0], id)].begin = elapsedSecsToday(t);
			id.pack(attr::value = id.formatted(t)).send_all();
		}
		
		static void end_callback(struct time_field &id, String value, client_id_t sender)
		{
			time_t t = id.parse(value);
			settings.changed().weekly_schedule.days[class_index(::weekly_schedule.days[0], id)].end = elapsedSecsToday(t);
			id.pack(attr::value = id.formatted(t)).send_all();
		}
		
		static void switcher_callback(struct switcher &id, client_id_t sender)
		{
			id.turn(settings.weekly_schedule.days[class_index(::weekly_schedule.days[0], id)].enabled).send_all();
		}
		
		day_schedule() : begin(begin_callback), end(end_callback), enabled(switcher_callback) {}
	} days[7];
	
	static void switcher_callback(switcher &id, client_id_t sender)
	{
		id.turn(settings.changed().weekly_schedule.enabled).send_all();
	}
	
	void trigger()
	{
		time_t t = now();
		time_t cur_time = elapsedSecsToday(t);	
		int cur_day = ((weekday(t) - 2) + 7) % 7;
	
		if (!settings.weekly_schedule.enabled || !settings.weekly_schedule.days[cur_day].enabled) {
			blocking = false;
			return;
		}
		
		int cur_day_begin_time = settings.weekly_schedule.days[cur_day].begin;
		int cur_day_end_time = settings.weekly_schedule.days[cur_day].end;
		
		int prev_day = ((cur_day - 1) + 7) % 7;
		int prev_day_begin_time = settings.weekly_schedule.days[prev_day].begin;
		int prev_day_end_time = settings.weekly_schedule.days[prev_day].end;
			
		if (settings.weekly_schedule.days[prev_day].enabled && prev_day_begin_time >= prev_day_end_time && cur_time < prev_day_end_time)
			blocking = false;
		else if (cur_day_begin_time < cur_day_end_time)
			blocking = (cur_time >= cur_day_begin_time && cur_time < cur_day_end_time) ? false : true;
		else
			blocking = (cur_time >= cur_day_begin_time) ? false : true;
	}
	
	weekly_schedule() : enabled(switcher_callback) {}
	packet build()
	{
		packet buf = box.pack(navigation_panel.timers, (attr::text = l_str(WEEKLY_SCHEDULE_TITLE), attr::direction = DIR_V)) + wrapper.pack(box, (attr::direction = DIR_H)) +
		enabler_wrapper.pack(box, (attr::direction = DIR_H)) + enabler_text.pack(enabler_wrapper, (attr::text = l_str(SCHEDULE_ENABLER_TITLE))) + enabled.pack(enabler_wrapper, (enabled.get(settings.weekly_schedule.enabled)));
		for (int i = 0; i < 4; i++)
			buf += cols[i].pack(wrapper) + col_names[i].pack(cols[i], (attr::text = l_str(WEEKLY_SCHEDULE_COL_NAMES + i)));
			
		for (int i = 0; i < 7; i++)
			buf += days[i].name.pack(cols[0], (attr::text = l_str(WEEKLY_SCHEDULE_DAY_NAMES + i))) + days[i].begin.pack(cols[1], attr::value = days[i].begin.formatted(settings.weekly_schedule.days[i].begin)) +
			days[i].end.pack(cols[2], attr::value = days[i].end.formatted(settings.weekly_schedule.days[i].end)) + days[i].enabled.pack(cols[3], days[i].enabled.get(settings.weekly_schedule.days[i].enabled));
		return buf;
	}
} weekly_schedule;




//////////////////////////////////////////////////////////////////////////////////
/*				INTERFACE BUILDER				*/
//////////////////////////////////////////////////////////////////////////////////
#define BEGIN_ADD_TO_INTERFACE switch(idx)
#define ADD_TO_INTERFACE(idx, stuff) case idx: cl.packet += stuff.build(); return false
#define END_ADD_TO_INTERFACE default: return true
	
bool ui::interface(client &cl, int idx) //implementation of interface builder is user specified
{
	BEGIN_ADD_TO_INTERFACE {
		ADD_TO_INTERFACE(0, navigation_panel);
		ADD_TO_INTERFACE(1, charge_status);
		ADD_TO_INTERFACE(2, time_and_kwt_for_session);
		ADD_TO_INTERFACE(3, consumption);
		ADD_TO_INTERFACE(4, manual_start_stop);
		ADD_TO_INTERFACE(5, current_regulator);
		ADD_TO_INTERFACE(6, ground_check);
		ADD_TO_INTERFACE(7, adaptive_mode);
		ADD_TO_INTERFACE(8, network_settings);
		ADD_TO_INTERFACE(9, date_time);
		//ADD_TO_INTERFACE(12, sync_time);
		ADD_TO_INTERFACE(10, language_selector);
		ADD_TO_INTERFACE(11, display_brightness_regulator);
		ADD_TO_INTERFACE(12, display_off_time);
		ADD_TO_INTERFACE(13, factory_reset);
		ADD_TO_INTERFACE(14, ap_settings);
		ADD_TO_INTERFACE(15, sta_dialog);
		ADD_TO_INTERFACE(16, sta_settings);
		ADD_TO_INTERFACE(17, weekly_schedule);
		ADD_TO_INTERFACE(18, statistics);
		END_ADD_TO_INTERFACE;
	}
}




//////////////////////////////////////////////////////////////////////////////////
/*				CORE FUNCTIONS					*/
//////////////////////////////////////////////////////////////////////////////////
void rebuild_electrobox_ui()
{
	for (int i = 0; i < MAX_UI_CLIENTS; i++) {
		number_of_interface_loaders++;
		clients[i].cleanup();
	}
}

void electrobox_setup()
{
	interface_setup();
	sta_dialog.init();
	
	settings.load();
	settings.ground_check = true;
}

void electrobox_loop()
{
	static unsigned long old_millis = 0;
	static unsigned time_counter = 0;
	static int statistics_update_counter = 0; //5 minutes
	
	if (millis() > old_millis + 1000) {
		packet buf;
		
		weekly_schedule.trigger();
		
		//time_and_kwt_for_session.update(13, 37, millis() / 1234.0);
		if (time_counter >= 60) {
			date_time.update();
			//charge_status.update(1);
			
			if (statistics_update_counter >= 5)
			{
				statistics.update(now(), settings.statistics.kwt_for_session);
				statistics_update_counter = 0;
			} else if (statistics.need_update) {
				statistics_update_counter++;
			}
			
			time_counter = 0;
		} else {
			time_counter++;
		}
		
		if (settings.need_commit) {
			if (!settings.modified)
				buf += factory_reset.applier.pack(attr::disabled = false);
			settings.save();
		}
		
		if (network::ap.need_commit) {
			if (!ap_settings.applier_enabled)
				buf += ap_settings.apply_settings.pack(attr::disabled = false);
			//do not save settings here, better do this inside applier.
		}
		
		if (network::sta_status != sta_settings.old_status) { //if network status changed
			//if (network::sta.dhcp)
			//	buf += sta_dialog.ip_field.pack(attr::value = network::get_connected_station_ip().toString());
			sta_settings.old_status = network::sta_status;
			
			//DEBUG_MSG("Status = %i\n", network::sta_status);
			//search network inside available networks list
			//sta_settings.search();
			
			//buf += sta_settings.connection_status.pack(attr::text = sta_settings.status_str());
			//buf += sta_settings.list_avail_stations();
			buf += sta_settings.list_avail_stations() + sta_dialog.update(-1);
		}
		
		if (network::scan && network::avail_networks > 0)
		{
			buf += sta_settings.scan_sta.pack(attr::disabled = false) + sta_settings.list_avail_stations();
			network::end_scan();
		}
		
		if (buf.buffer)
			buf.send_all();
		
		old_millis += 1000;
	}
	interface_loop();
}
