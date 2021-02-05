#include "electrobox.h"




//////////////////////////////////////////////////////////////////////////////////
/*				PROTOTYPES					*/
//////////////////////////////////////////////////////////////////////////////////
extern struct sync_time sync_time;
extern struct language_selector language_selector;
extern struct factory_reset factory_reset;
extern struct network_settings network_settings;
extern struct sta_settings sta_settings;
void rebuild_electrobox_ui();




//////////////////////////////////////////////////////////////////////////////////
/*				SETTINGS SECTION				*/
//////////////////////////////////////////////////////////////////////////////////
struct settings &settings::defaults() {
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
	DEBUG_MSG("Saving electrobox settings...\n");
	File f = LittleFS.open("/electrobox.bin", "w");
	if (f) {
		f.write((uint8_t *)this, sizeof(struct settings));
		f.close();
		
		committed();
		DEBUG_MSG("\tSave succeed!\n");
	} else {
		DEBUG_MSG("\tSave failed!\n");
	}
	//maybe error here
}
			
void settings::load()
{
	DEBUG_MSG("Loading electrobox settings...\n");
	File f = LittleFS.open("/electrobox.bin", "r");
	if (f) {
		f.read((uint8_t *)this, sizeof(struct settings));
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




//////////////////////////////////////////////////////////////////////////////////
/*				LANGUAGE SECTION				*/
//////////////////////////////////////////////////////////////////////////////////
const char* languages[NUMBER_OF_SUPPORTED_LANGUAGES][LANGUAGE_ITEMS] PROGMEM = {
	{"Головна", "Налаштування", "Вибір мови", "Українська", "Встановити дату та час", "Рік", "Години", "Місяць", "Хвилини", "День", "Секунди", "Тижневий розклад", "Дні тижня", "Старт зарядки", "Стоп зарядки", "Увімкнути", "Понеділок", "Вівторок", "Середа", "Четвер", "П'ятниця", "Субота", "Неділя", "Статус зарядки", "Пістолет не вставлено", "Триває зарядка", "Автомобіль заряджено", "Перевірте заземлення", "Спрацював захист ПЗВ", "Спрацював захист по напрузі (більше 270V)", "Спрацював захист по току (більше 50А)", "Очікує зарядки по розкладу", "кВт", "Час та кВт за сесію", "Напруга, струм та потужність", "Регулювання струму", "Перевірка заземлення", "Заряджати по розкладу", "Адаптивний режим", "Обмежити кількість кВт за сесію", "Таймери", "Регулювання яскравості дисплею", "Обмежити тривалість заряду в годинах", "Час до вимкнення дисплею в хвилинах", "Статистика", "Лог змін статусу", "Ввімкнути зарядку", "Синхронізувати час з браузерним", "Очистка логу", "Скинути налаштування", "Налаштування мережі", "Зберегти зміни", "Точка доступу", "Станція", "SSID\\:", "Пароль\\:", "IP адрес\\:", "Cтанція ввімкнена\\:", "Статус підключення\\:", "Доступні мережі\\:", "Сканувати мережу", "Підключення відсутнє", "Підключено до\\: ", "Спроба підключення до\\: ", "Неможливо підключитися до\\: ", "Статистика за період", "За минулу(ий)", "За теперішню(ній)", "Годину\\:", "День\\:", "Тиждень\\:", "Місяць\\:", "Рік\\:", "За весь час\\:", "Рахувати з теперешнього моменту", "Споживано за сесію", "Зараз", "При підключенні\\:", "Ви дійсно хочете скинути налаштування?", "Так", "Ні"},
	{"Главная", "Настройки", "Выбор языка", "Русский", "Установить дату и время", "Год", "Часы", "Месяц", "Минуты", "День", "Секунды", "Еженедельное расписание", "Дни недели", "Старт заряда", "Стоп зарада", "Включить", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье", "Статус зарядки", "Пистолет не вставлен", "Продолжается зарядка", "Автомобиль заряжен", "Проверьте заземление", "Сработала защита УЗО", "Сработала защита по напряжению (более 270V)", "Сработала защита по току (более 50A)", "Ожидание зарядки по расписанию", "кВт", "Время и кВт за сессию", "Напряжение ток и мощность", "Регулировка тока", "Проверка заземления", "Заряжать по расписанию", "Адаптивний режим", "Ограничить кол-во кВт за сессию", "Таймеры", "Регулировка яркости дисплея", "Ограничить время заряда в часах", "Время до отключения дисплея в минутах", "Статистика", "Лог смен статуса", "Включить зарядку", "Синхронизировать время с браузерным", "Очистить лог", "Сбросить настройки", "Настройки сети", "Сохраниить изменения", "Точка доступа", "Станция", "SSID\\:", "Пароль\\:", "IP адрес\\:", "Станция включена\\:", "Статус подключения\\:", "Доступные сети\\:", "Сканировать сети", "Подключение отсутствует", "Подключено к\\: ", "Попытка подключения к\\: ", "Невозможно подключиться к\\: ", "Статистика за период", "За предыдущий(ую))", "За Текущий(ую)", "Час\\:", "День\\:", "Неделю\\:", "Месяц\\:", "Год\\:", "За всё время\\:", "Считать с текущего момента", "Употреблено за сессию", "Сейчас", "При подключении\\:", "Вы действительно хотите сбросить настройки?", "Да", "Нет"},
	{"Home", "Settings", "Select language", "English", "Setup date and time", "Year", "Hours", "Month", "Minutes", "Day", "Seconds", "Weekly sсhedule", "Days of the week", "Start charging", "Stop charging", "Enable", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday", "Charger status", "Pistol not inserted", "Charging continues", "Vehicle is charged", "Check ground", "RCD protection blocking", "Voltage protection blocking (above 270V)", "Current protection blocking (above 50A)", "Waiting for charging by chedule", "kWt", "Time and kWt for session", "Voltage, current and kWt", "Current regulator", "Ground check", "Charge by schedule", "Adaptive mode", "Limit kWt for session", "Timers", "Display brightness regulator", "Limit charge time by hours", "Time to turn off the display in minutes", "Statistics", "Status change log", "Enable charger", "Synchronize time with browser time", "Clear log", "Factory reset", "Network settings", "Save settings", "Access point", "Station", "SSID\\:", "Password\\:", "IP\\:", "Station enabled\\:", "Connection status\\:", "Available networks\\:", "Scan networks", "No connection", "Connected to\\: ", "Attempting to connect to\\: ", "Failed to connect to\\: ", "Statistics for a period", "Previous", "Current", "Hour\\:", "Day\\:", "Week\\:", "Month\\:", "Year\\:", "For all time\\:", "Count from current time", "Consumed for session", "Now", "On connect\\:", "Do you really want to reset?", "Yes", "No"},
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
		
		C_UNKNOWN
	};
	
	enum status value = C_UNSET;
	
	struct box box;
	struct text text;
	
	void update(int val)
	{
		if (value == C_CHARGING && val != C_CHARGING) {
			//TODO update statistics here
		}
		
		value = (status)val;
		//TODO put value into log here
		text.pack(l_str(CHARGER_STATUS + val)).send_all();
	}
	 
	packet build()
	{
		return box.pack(navigation_panel.home, (attr::text = l_str(CHARGER_STATUS_TITLE), attr::direction = DIR_H)) + text.pack(box, (attr::text = l_str(CHARGER_STATUS + (int)value)));
	}
} charge_status;

struct time_and_kwt_for_session {	
	struct box box;
	struct text text;
	
	String get()
	{
		return hm_to_str(settings.statistics.time_elapsed_for_session) + " - " + settings.statistics.kwt_for_session + " " + l_str(KWT);
	}
	
	void update(int h, int m, float k)
	{
		settings.changed().statistics.time_elapsed_for_session = convert_to_hm(h, m);
		settings.changed().statistics.kwt_for_session = k;
		text.pack(get()).send_all();
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
		text.pack(get()).send_all();
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
		return box.pack(navigation_panel.home, (attr::text = l_str(GROUND_CHECK_TITLE), attr::direction = DIR_H)) + switcher.pack(box, switcher.get(settings.ground_check));
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
	struct date_field date;
	struct time_field time;
	
	template <class T>
	static void field_callback(T &id, String value, client_id_t sender)
	{
		time_t t = id.parse(value);
		setTime(t);
		id.pack(attr::value = id.formatted(t)).send_all();
	}
	
	void update()
	{
		time_t t = now();
		(date.pack(attr::value = date.formatted(t)) + time.pack(attr::value = time.formatted(t))).send_all();
	}
	
	date_time() : date(field_callback<struct date_field>), time(field_callback<struct time_field>) {};
	packet build()
	{
		time_t t = now();
		return box.pack(navigation_panel.settings, (attr::text = l_str(DATE_TIME_TITLE))) + date.pack(box, attr::value = date.formatted(t)) + time.pack(box, attr::value = time.formatted(t));
	}
} date_time;

struct sync_time {	
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
		//statistics::update(t, 0.0); //we do not put kwt's here(just time update) //TODO
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
		return  box.pack(navigation_panel.settings, (attr::text = l_str(SYNC_TIME_TITLE), attr::direction = DIR_V)) +
			button.pack(box, (attr::text = l_str(SYNC_TIME_NOW), attr::background = "orange")) +
			
			wrapper.pack(box, (attr::direction = DIR_H)) +
			text.pack(wrapper, (attr::text = l_str(SYNC_TIME_ON_CONNECT))) + switcher.pack(wrapper, switcher.get(settings.sync_time_on_connect)) +
			
			(settings.sync_time_on_connect ? browser_time.pack() : packet());
	}
} sync_time;

struct language_selector {
	struct box box;
	struct language_radio_item : radio { //wrapper inheritance. Not sure if there is no overhead guaranteed (WARNING)
		static void item_callback(radio &id, uint32_t sender) {
			struct radio &old_item = ::language_selector.item[settings.language];
			
			if (&id != &old_item) {
				settings.changed().language = (enum language)(&id - static_cast<radio*>(&::language_selector.item[0]));
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
	struct box wrapper; //not very bright solution, may change in future because dialogs suck for now, really
	struct button yes, no;
	
	static void yes_callback(struct button &id, uint32_t sender)
	{
		::factory_reset.dialog.pack(attr::display = false).send(sender);
		network::settings.defaults().save();
		settings.defaults().save();
		
		//tab_navigation::selected = &tab_navigation::settings;
		rebuild_electrobox_ui();
	}
	
	static void no_callback(struct button &id, uint32_t sender)
	{
		::factory_reset.dialog.pack(attr::display = false).send(sender);
	}
	
	static void applier_callback(struct applier &id, uint32_t sender)
	{
		::factory_reset.dialog.pack(attr::display = true).send(sender);
	}

	factory_reset() : yes(yes_callback), no(no_callback), applier(applier_callback) {};
	packet build()
	{
		return
		box.pack(navigation_panel.settings, (attr::text = l_str(FACTORY_RESET_TITLE), attr::direction = DIR_H)) +
		applier.pack(box, (attr::background = "orange", applier.get())) +
		
		dialog.pack(root, (attr::display = false)) + wrapper.pack(dialog, (attr::text = l_str(ARE_YOU_SURE_RESET), attr::direction = DIR_H)) +
		yes.pack(wrapper, (attr::text = l_str(RESET_YES))) + no.pack(wrapper, (attr::text = l_str(RESET_NO)));
	}
} factory_reset;

//Maybeh divide network settings onto two parts? TODO
#define MAX_STATIONS_SHOWN_AT_ONCE 8
struct network_settings {
	struct box box;
	
	struct tab ap_tab;
	struct tab sta_tab;
	
	struct applier apply_settings;
	
	static void applier_callback(struct applier &id, uint32_t sender)
	{
		network::end();
		network::settings.save();
		DEBUG_MSG("Network settings applied, restarting...\n");
		delay(1000);
		ESP.restart();
	}
	
	network_settings() : apply_settings(applier_callback) {};
	packet build()
	{
		return
		box.pack(navigation_panel.settings, (attr::text = l_str(NETWORK_SETTINGS_TITLE), attr::fill = true)) +
		ap_tab.pack(box, (attr::text = l_str(AP_TAB), attr::panel = "net", attr::direction = DIR_H, attr::selected = true, attr::wrap = true)) +
		sta_tab.pack(box, (attr::text = l_str(STA_TAB), attr::panel = "net", attr::direction = DIR_V, attr::wrap = true)) +
		apply_settings.pack(box, (attr::text = l_str(SAVE_NETWORK_SETTINGS), attr::background = "orange", apply_settings.get()));
	}
} network_settings;

struct ap_settings {
	struct box ap_text_col, ap_field_col;
	struct text ap_ssid_text, ap_pass_text, ap_ip_text;
	struct field ap_ssid_field, ap_pass_field, ap_ip_field;

	static void ap_ssid_field_callback(struct field &id, String value, client_id_t sender)
	{
		value.toCharArray(network::settings.changed().ap_ssid, 80); //TODO check ssid maybeh?
		id.pack(attr::value = String(network::settings.ap_ssid)).send_all();
	}
	
	static void ap_pass_field_callback(struct field &id, String value, client_id_t sender)
	{
		if (network::passphrase_is_valid(value))
			value.toCharArray(network::settings.changed().ap_pass, 80);
		id.pack(attr::value = String(network::settings.ap_pass)).send_all();
	}
	
	static void ap_ip_field_callback(struct field &id, String value, client_id_t sender)
	{
		IPAddress ip;
		ip.fromString(value);
		network::settings.changed().ap_ip = network::settings.ap_gateway = (uint32_t)ip;
		id.pack(attr::value = IPAddress(network::settings.ap_ip).toString()).send_all();
	}

	ap_settings() : ap_ssid_field(ap_ssid_field_callback), ap_pass_field(ap_pass_field_callback), ap_ip_field(ap_ip_field_callback) {}
	packet build()
	{
		return
		ap_text_col.pack(network_settings.ap_tab, (attr::direction = DIR_V, attr::width = "content")) +
		ap_field_col.pack(network_settings.ap_tab, (attr::direction = DIR_V)) +
		ap_ssid_text.pack(ap_text_col, (attr::text = l_str(AP_SSID))) + ap_pass_text.pack(ap_text_col, (attr::text = l_str(AP_PASS))) + ap_ip_text.pack(ap_text_col, (attr::text = l_str(AP_IP))) +
		ap_ssid_field.pack(ap_field_col, (attr::value = String(network::settings.ap_ssid))) + ap_pass_field.pack(ap_field_col, (attr::value = String(network::settings.ap_pass))) + 
			ap_ip_field.pack(ap_field_col, (attr::value = IPAddress(network::settings.ap_ip).toString()));
	}
} ap_settings;

struct sta_settings {
	enum network::sta_status old_status;
	
	/*struct box sta_enabled_wrapper;
	struct text sta_enabled_text;
	struct switcher sta_enabled;*/
	struct box connection_status_wrapper;
	struct text connection_status;
	struct box avail_stations_wrapper;
	struct applier scan_sta;
	
	struct avail_stations : radio {
		static void select_station_callback(struct radio &id, uint32_t sender)
		{
			int idx = (&id - static_cast<radio*>(&::sta_settings.avail_stations[0]));
			if (idx != 0) {
				network::change_sta_to(idx - 1, "0987w456321s");
				::sta_settings.list_avail_stations().send_all();
			}
		}
		 
		 avail_stations() : radio(select_station_callback) {}
	} avail_stations[MAX_STATIONS_SHOWN_AT_ONCE]/*, *selected_sta = &avail_stations[0]*/; //Not very dynamic way to do it, but OK yet. TODO

	/*String status_str()
	{
		switch (network::sta_status) {
		case network::STA_DISCONNECTED:
			return l_str(STA_DISCONNECTED);
		
		default:
			return l_str(STA_STATUS + network::sta_status) + network::get_station_name(-1);
		*//*case network::STA_CONNECTED:
			return l_str(STA_CONNECTED) + network::get_station_name(i);
		
		case network::STA_CONNECTING:
			return l_str(STA_CONNECTING)
		
		case network::STA_TIMEOUT:
			return l_str(STA_TIMEOUT)*/
	/*	}
	}*/

	packet list_avail_stations()
	{
		packet buf;
		if (network::sta_status != STA_DISCONNECTED)
			avail_stations[0].pack(network_settings.sta_tab, (attr::text = network::get_station_name(-1), attr::display = true, attr::background = "green")); //zero slot is reserved for connected station
		
		for (int i = 1; i < network::avail_networks + 1; i++) {
			if ((network::sta_status != STA_DISCONNECTED) && (network::get_station_name(-1) == network::get_station_name(i)))
				continue; //skip same station as from zero slot;
			buf += avail_stations[i].pack(network_settings.sta_tab, (attr::text = network::get_station_name(i), attr::display = true));
		}
		
		//do not display entries further
		for (int i = network::avail_networks + 1; i < MAX_STATIONS_SHOWN_AT_ONCE; i++)
			buf += avail_stations[i].pack(network_settings.sta_tab, (attr::display = false));
		
		return buf;
	}
	
	/*static void sta_enabled_callback(struct switcher &id, uint32_t sender)
	{
		id.turn(network::settings.changed().sta_enabled).send_all(); //TODO deferred changing
	}*/
	
	static void scan_sta_callback(struct applier &id, uint32_t sender)
	{
		packet buf = id.turn(false);
		for (int i = 0; i < MAX_STATIONS_SHOWN_AT_ONCE; i++)
			buf += ::sta_settings.avail_stations[i].pack(network_settings.sta_tab, (attr::display = false));
		buf.send_all();
		network::begin_scan();
	}
	
	sta_settings() : scan_sta(scan_sta_callback), /*sta_enabled(sta_enabled_callback),*/ old_status(network::STA_DISCONNECTED) {}
	packet build()
	{
		return
		/*sta_enabled_wrapper.pack(network_settings.sta_tab, (attr::direction = DIR_H)) +
		sta_enabled_text.pack(sta_enabled_wrapper, (attr::text = l_str(STA_ENABLED), attr::width = "content")) + sta_enabled.pack(sta_enabled_wrapper, (sta_enabled.get(network::settings.sta_enabled))) +*/
		/*connection_status_wrapper.pack(network_settings.sta_tab) + connection_status_wrapper.pack((attr::text = l_str(STA_CONNECTION_STATUS), attr::height = 10)) + //lil hack, TODO (put something else here) //TODO margin attribute
		connection_status.pack(connection_status_wrapper, (attr::text = status_str(), attr::wrap = true, attr::height = "content")) +*/
		avail_stations_wrapper.pack(network_settings.sta_tab) + avail_stations_wrapper.pack(attr::text = l_str(STA_AVAIL_NETWORKS)) + //lil hack, TODO (put something else here)
		list_avail_stations() +
		scan_sta.pack(network_settings.sta_tab, (attr::text = l_str(STA_SCAN_NETWORK), attr::background = "cornflowerblue", attributes(scan_sta.turn(true).buffer))); //TODO better applier setter
	}
} sta_settings;



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
		ADD_TO_INTERFACE(4, current_regulator);
		ADD_TO_INTERFACE(5, ground_check);
		ADD_TO_INTERFACE(6, adaptive_mode);
		ADD_TO_INTERFACE(7, display_brightness_regulator);
		ADD_TO_INTERFACE(8, display_off_time);
		ADD_TO_INTERFACE(9, date_time);
		ADD_TO_INTERFACE(10, sync_time);
		ADD_TO_INTERFACE(11, language_selector);
		ADD_TO_INTERFACE(12, factory_reset);
		ADD_TO_INTERFACE(13, network_settings);
		ADD_TO_INTERFACE(14, ap_settings);
		ADD_TO_INTERFACE(15, sta_settings);
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
	
	settings.load();
}

void electrobox_loop()
{
	static unsigned long old_millis = 0;
	static unsigned time_counter = 0;
	if (millis() > old_millis + 1000) {
		packet buf;
		
		if (time_counter >= 60) {
			date_time.update();
			time_counter = 0;
		} else {
			time_counter++;
		}
		
		if (settings.need_commit) {
			if (!factory_reset.applier.avail)
				buf += factory_reset.applier.turn(true);
			settings.save();
		}
		
		//don't mess network_settings with network::settings - they are different
		if (network::settings.need_commit) {
			if (!network_settings.apply_settings.avail)
				buf += network_settings.apply_settings.turn(true);
			//do not save settings here, better do this inside applier.
		}
		
		/*if (network::sta_status != sta_settings.old_status) { //if network status changed
			sta_settings.old_status = network::sta_status;
			
			DEBUG_MSG("Status = %i\n", network::sta_status);
			//search network inside available networks list
			//sta_settings.search();
			
			//buf += sta_settings.connection_status.pack(attr::text = sta_settings.status_str());
		}*/
		
		if (network::scan && network::avail_networks > 0)
		{
			buf += sta_settings.scan_sta.turn(true) + sta_settings.list_avail_stations();
			network::end_scan();
		}
		
		if (buf.buffer)
			buf.send_all();
		
		old_millis += 1000;
	}
	interface_loop();
}
