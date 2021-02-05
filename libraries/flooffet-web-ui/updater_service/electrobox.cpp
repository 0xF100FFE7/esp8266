//TODO
//Fix web server client response https://github.com/me-no-dev/ESPAsyncWebServer/issues/870
//Move localization to JS
//create :put: and :get: methods to create flexible labels (example: :put:abc i am some value:, and :get:abc: )
//create setparent method which will recognize if it's connect message or not
//create setstatic method which will recognize if it's connect message or not
//create setdynamic method whi...
//or set flags parameter to build function
//also set parent inside build function, not in connect message function
//set address size to small integer type
#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

#include "electrobox.h"

//todo move localization to javascript

//User specified ui helpers(wrappers)
String hours_and_minutes_to_str(int hours, int minutes) {
	return (hours < 10 ? String("0") + hours : String(hours)) + "\\:" + (minutes < 10 ? String("0") + minutes : String(minutes));
}

void send_log(String text) {
	(ui::messages("log:") << ui::escape(text) << ":").flush();
};

struct tab {
	ui::element element;
	
	ui::messages update(String text)
	{
		return ui::messages("id:") << this << ":text:" << text << ":";
	}
	
	tab() : element(ui::E_TAB) {};
};

struct text {
	ui::element element;
	
	ui::messages update(String text)
	{
		return ui::messages("id:") << this << ":text:" << text << ":";
	}
	
	text() : element(ui::E_TEXT) {};
};

struct box {
	ui::element element;
	
	ui::messages update(String text)
	{
		return ui::messages("id:") << this << ":text:" << text << ":";
	}
	
	box() : element(ui::E_BOX) {};
};

struct field {
	ui::element element;
	String val;
	
	ui::messages update(String v)
	{
		val = v;
		return ui::messages("id:") << this << ":value:" << val << ":";
	}
	
	field(void *callback) : element(ui::E_FIELD, callback) {};
};

/*struct ifield {
	ui::element element;
	int val;
	
	ui::messages update(String v)
	{
		val = v.toInt();
		return ui::messages("id:") << this << ":value:" << v << ":";
	}
	
	ifield(void *callback) : element(ui::E_FIELD, callback) {};
};*/

ui::messages ifield::update(String v)
{
	val = v.toInt();
	return ui::messages("id:") << this << ":value:" << v << ":";
}

ifield::ifield(void *callback) : element(ui::E_FIELD, callback) {};

struct tfield {
	ui::element element;
	int hours, minutes;
		
	ui::messages attributes()
	{
		return ui::messages("value:") << hours_and_minutes_to_str(hours, minutes) << ":";
	}
	
	ui::messages update(String val)
	{	
		//Both are good - old is less complex, but new is much flexible :)
		
		//old parser (unresistant to bad format/errors)
		//hours = val.toInt() % 24;
		//minutes = val.substring(val.indexOf(":") + 1).toInt() % 60;
		
		//new parser (resistant to bad format/errors)
		int i = 0;
		int skip_digits = 2;
		
		for (; i < val.length(); i++) //search hours digits
			if (isDigit(val[i])) {
				hours = val.substring(i).toInt() % 24;
				break;
			}
		
		for (; i < val.length() && skip_digits && isDigit(val[i]); i++, skip_digits--); //skip hour digits

		for (; i < val.length(); i++) //search for minutes digits
			if (isDigit(val[i])) {
				minutes = val.substring(i).toInt() % 60;
				break;
			}
		
		return ui::messages("id:") << this << ":" << attributes();
	}
	
	tfield(void *callback) : element(ui::E_TIME_FIELD, callback) {};
};

struct radio {
	ui::element element;
	
	ui::messages attributes(bool enabled)
	{
		return enabled ? ui::messages("backcolor:green:") : ui::messages("backcolor:#666:");
	}
	
	ui::messages turn(void *old_id)
	{
		return ui::messages("id:") << old_id << ":" << attributes(false) //disable old radio button
			<< "id:" << this << ":" << attributes(true);
	}
	
	radio(void *callback) : element(ui::E_BUTTON, callback) {};
};

struct button {
	ui::element element;
	
	ui::messages disable(bool &enabled)
	{
		enabled = false;
		return ui::messages("id:") << this << ":disabled:true:";
	}
	
	ui::messages enable(bool &enabled)
	{
		enabled = true;
		return ui::messages("id:") << this << ":disabled:false:";	
	}
	
	button(void *callback) : element(ui::E_BUTTON, callback) {};
};

struct switcher {
	ui::element element;
	
	ui::messages attributes(bool enabled)
	{
		return enabled ? ui::messages("backcolor:green:text:&#10003;:") : ui::messages("backcolor:red:text:&#10008;:");
	}
	
	ui::messages turn(bool &enabled)
	{
		enabled = !enabled;
		return ui::messages("id:") << this << ":" << attributes(enabled);
	}
	
	switcher(void *callback) : element(ui::E_BUTTON, callback) {};
};

struct range {
	ui::element element;
	
	ui::messages slide(int &val, int new_val)
	{
		val = new_val;
		return ui::messages("id:") << this << ":value:" << val << ":";
	}
	
	range(void *callback) : element(ui::E_RANGE, callback) {};
};

struct time {
	ui::element element;
	
	time(void *callback) : element(ui::GET_TIME, callback) {};
};

struct dialog {
	ui::element element;
	
	dialog() : element(ui::E_DIALOG) {};
};

frame::frame(void *callback) : element(ui::FRAME, callback) {};

//Connect specific messages
//ui::messages persist_messages;	//Always stays the same until build_all is true
//ui::messages dynamic_messages;	//Always rebuilds at connection, required for synchronizing with new UI state.
//ui::messages client->messages;

//Runtime specific massages
ui::messages runtime_messages;

ui::messages electrobox_ui();
void rebuild_electrobox_ui();
void factory_init(bool); //Factory initialization, basically on first run or factory reset
void save_settings(); //save all settings





language current_language;

const char* languages[NUMBER_OF_SUPPORTED_LANGUAGES][LANGUAGE_ITEMS] PROGMEM = {
	{"Головна", "Налаштування", "Вибір мови", "Українська", "Встановити дату та час", "Рік", "Години", "Місяць", "Хвилини", "День", "Секунди", "Тижневий розклад", "Дні тижня", "Старт зарядки", "Стоп зарядки", "Увімкнути", "Понеділок", "Вівторок", "Середа", "Четвер", "П'ятниця", "Субота", "Неділя", "Статус зарядки", "Пістолет не вставлено", "Триває зарядка", "Автомобіль заряджено", "Перевірте заземлення", "Спрацював захист ПЗВ", "Спрацював захист по напрузі (більше 270V)", "Спрацював захист по току (більше 50А)", "Очікує зарядки по розкладу", "кВт", "Час та кВт за сесію", "Напруга, струм та потужність", "Регулювання струму", "Перевірка заземлення", "Заряджати по розкладу", "Адаптивний режим", "Обмежити кількість кВт за сесію", "Таймери", "Регулювання яскравості дисплею", "Обмежити тривалість заряду в годинах", "Час до вимкнення дисплею в хвилинах", "Статистика", "Лог змін статусу", "Ввімкнути зарядку", "Синхронізувати час з браузерним", "Очистка логу", "Скинути налаштування", "Налаштування мережі", "Зберегти зміни", "Точка доступу", "Станція", "SSID\\:", "Пароль\\:", "IP адрес\\:", "Шлюз\\:", "Маска підмережі\\:", "Ввімкнена\\:", "SSID\\:", "Пароль\\:", "Статичний IP адрес\\:", "Шлюз\\:", "Маска підмережі\\:", "Статистика за період", "За минулу(ий)", "За теперішню(ній)", "Годину\\:", "День\\:", "Тиждень\\:", "Місяць\\:", "Рік\\:", "За весь час\\:", "Рахувати з теперешнього моменту", "Споживано за сесію", "Зараз", "При підключенні\\:", "Ви дійсно хочете скинути налаштування?", "Так", "Ні"},
	{"Главная", "Настройки", "Выбор языка", "Русский", "Установить дату и время", "Год", "Часы", "Месяц", "Минуты", "День", "Секунды", "Еженедельное расписание", "Дни недели", "Старт заряда", "Стоп зарада", "Включить", "Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье", "Статус зарядки", "Пистолет не вставлен", "Продолжается зарядка", "Автомобиль заряжен", "Проверьте заземление", "Сработала защита УЗО", "Сработала защита по напряжению (более 270V)", "Сработала защита по току (более 50A)", "Ожидание зарядки по расписанию", "кВт", "Время и кВт за сессию", "Напряжение ток и мощность", "Регулировка тока", "Проверка заземления", "Заряжать по расписанию", "Адаптивний режим", "Ограничить кол-во кВт за сессию", "Таймеры", "Регулировка яркости дисплея", "Ограничить время заряда в часах", "Время до отключения дисплея в минутах", "Статистика", "Лог смен статуса", "Включить зарядку", "Синхронизировать время с браузерным", "Очистить лог", "Сбросить настройки", "Настройки сети", "Сохраниить изменения", "Точка доступа", "Станция", "SSID\\:", "Пароль\\:", "IP адрес\\:", "Шлюз\\:", "Маска подсети\\:", "Включена\\:", "SSID\\:", "Пароль\\:", "Статический IP адрес\\:", "Шлюз\\:", "Маска подсети\\:", "Статистика за период", "За предыдущий(ую))", "За Текущий(ую)", "Час\\:", "День\\:", "Неделю\\:", "Месяц\\:", "Год\\:", "За всё время\\:", "Считать с текущего момента", "Употреблено за сессию", "Сейчас", "При подключении\\:", "Вы действительно хотите сбросить настройки?", "Да", "Нет"},
	{"Home", "Settings", "Select language", "English", "Setup date and time", "Year", "Hours", "Month", "Minutes", "Day", "Seconds", "Weekly sсhedule", "Days of the week", "Start charging", "Stop charging", "Enable", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday", "Charger status", "Pistol not inserted", "Charging continues", "Vehicle is charged", "Check ground", "RCD protection blocking", "Voltage protection blocking (above 270V)", "Current protection blocking (above 50A)", "Waiting for charging by chedule", "kWt", "Time and kWt for session", "Voltage, current and kWt", "Current regulator", "Ground check", "Charge by schedule", "Adaptive mode", "Limit kWt for session", "Timers", "Display brightness regulator", "Limit charge time by hours", "Time to turn off the display in minutes", "Statistics", "Status change log", "Enable charger", "Synchronize time with browser time", "Clear log", "Factory reset", "Network settings", "Save settings", "Access point", "Station", "SSID\\:", "Password\\:", "IP\\:", "Gate\\:", "Subnet mask\\:", "Enabled\\:", "SSID\\:", "Password\\:", "Static ip\\:", "Gate\\:", "Subnet mask\\:", "Statistics for a period", "Previous", "Current", "Hour\\:", "Day\\:", "Week\\:", "Month\\:", "Year\\:", "For all time\\:", "Count from current time", "Consumed for session", "Now", "On connect\\:", "Do you really want to reset?", "Yes", "No"},
};

//Localized string
const char* l_str(unsigned i)
{
	return languages[current_language][i];
}




struct sstatistics {
	int time_elapsed_for_session = 0;
	double kwt_for_session = 0.0;
	
	double total_kwt;

	statistics::kwt_range ranges[5] = {
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

struct sweekly_schedule {
	bool enabled = false;
	day_schedule days[7];
};

struct settings {
	uint64_t version;
	
	language slanguage;
	sweekly_schedule ssweekly_schedule;
	sstatistics ssstatistics;
	
	bool sync_time_on_connect;
	
	int scurrent_regulator;
	bool sground_check;
	bool sadaptive_mode;
	bool kwt_limit_enabled;
	int kwt_limit;
	
	bool time_limit_enabled;
	int time_limit;
	
	bool display_off_enabled;
	int sdisplay_off_time;
	int display_brightness;
	
	char ap_ssid[80];
	uint32_t ap_ip;
	uint32_t ap_gateway;
	uint32_t ap_subnet;
		
	char sta_ssid[80];
	uint32_t sta_ip;
	uint32_t sta_gateway;
	uint32_t sta_subnet;
	
	void defaults() {
		version = 75;
		
		slanguage = LANG_UA;
		
		sync_time_on_connect = false;
		
		scurrent_regulator = 6;
		sground_check = false;
		sadaptive_mode = false;
		kwt_limit_enabled = false;
		kwt_limit = 0;
		
		time_limit_enabled = false;
		time_limit = 0;
		
		display_off_enabled = false;
		sdisplay_off_time = 10;
		display_brightness = 100;
		
		ap_ssid[0] = '\0';
		ap_ip = IPAddress(7, 7, 7, 7);
		ap_gateway = IPAddress(7, 7, 7, 7);
		ap_subnet = IPAddress(255, 255, 255, 0);
		
		sta_ssid[0] = '\0';
		sta_ip = IPAddress(192, 168, 1, 227);
		sta_gateway = IPAddress(192, 168, 1, 1);
		sta_subnet = IPAddress(255, 255, 255, 0);
	}
} settings;

//all variables and arrays that will be stored in flash memory
//TODO make struct of this shit
enum flash_variable {
	FV_HEADER_MAGIC,
	FV_LANGUAGE = FV_HEADER_MAGIC + sizeof(uint64_t),
	FV_SCHEDULE_ENABLED = FV_LANGUAGE + sizeof(language),
	FV_SCHEDULE_BEGINS = FV_SCHEDULE_ENABLED + sizeof(bool),
	FV_SCHEDULE_ENDS = FV_SCHEDULE_BEGINS + sizeof(int) * 2 * 7,
	FV_SCHEDULE_SWITCHERS = FV_SCHEDULE_ENDS + sizeof(int) * 2 * 7,
	FV_HOURS_FOR_SESSION = FV_SCHEDULE_SWITCHERS + sizeof(bool) * 7,
	FV_MINUTES_FOR_SESSION = FV_HOURS_FOR_SESSION + sizeof(int),
	FV_KWT_FOR_SESSION = FV_MINUTES_FOR_SESSION + sizeof(int),
	FV_CURRENT_REGULATOR = FV_KWT_FOR_SESSION + sizeof(float),
	FV_GROUND_CHECK = FV_CURRENT_REGULATOR + sizeof(int),
	FV_ADAPTIVE_MODE = FV_GROUND_CHECK + sizeof(bool),
	FV_LIMIT_KWT_ENABLED = FV_ADAPTIVE_MODE + sizeof(bool),
	FV_LIMIT_KWT_VALUE = FV_LIMIT_KWT_ENABLED + sizeof(bool),
	FV_LIMIT_BY_TIME_ENABLED = FV_LIMIT_KWT_VALUE + sizeof(int),
	FV_LIMIT_BY_TIME_VALUE = FV_LIMIT_BY_TIME_ENABLED + sizeof(bool),
	FV_DISPLAY_OFF_ENABLED = FV_LIMIT_BY_TIME_VALUE + sizeof(int),
	FV_DISPLAY_OFF_VALUE = FV_DISPLAY_OFF_ENABLED + sizeof(bool),
	FV_DISPLAY_BRIGHTNESS = FV_DISPLAY_OFF_VALUE + sizeof(int),
	
	FV_AP_SSID = FV_DISPLAY_BRIGHTNESS + sizeof(int),
	FV_AP_PASS = FV_AP_SSID + sizeof(char) * 40,
	FV_AP_IP = FV_AP_PASS + sizeof(char) * 40,
	FV_AP_GATEWAY = FV_AP_IP + sizeof(uint32_t),
	FV_AP_SUBNET = FV_AP_GATEWAY + sizeof(uint32_t),

	FV_STA_ENABLED = FV_AP_SUBNET + sizeof(uint32_t),
	FV_STA_SSID = FV_STA_ENABLED + sizeof(bool),
	FV_STA_PASS = FV_STA_SSID + sizeof(char) * 40,
	FV_STA_IP = FV_STA_PASS + sizeof(char) * 40,
	FV_STA_GATEWAY = FV_STA_IP + sizeof(uint32_t),
	FV_STA_SUBNET = FV_STA_GATEWAY + sizeof(uint32_t),
	
	FV_STATS_TOTAL = FV_STA_SUBNET + sizeof(uint32_t),
	FV_STATS_RANGES = FV_STATS_TOTAL + sizeof(double),
	
	FV_SYNC_TIME_ON_CONNECT = FV_STATS_RANGES + sizeof(statistics::kwt_range) * 5,
	
	FV_SIZE = FV_SYNC_TIME_ON_CONNECT + sizeof(bool),
	FV_VERSION = 74  //Set new value all the time if you want to reset flash memory
};

uint64_t flash_header_magic = 0x1337C0DE154AD0BE + FV_VERSION;

bool need_commit = false;


//Static UI componets
namespace tab_navigation {
	tab home;
	tab settings;
	tab timers;
	tab logs;
	
	tab *selected = &home; //User specified, does not change its state via websockets
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_TAB << ":id:" << &home << ":parent:main:panel:nav:" << (selected == &home ? "selected:true:" : "") << "dir:h:wrap:true:text:" << l_str(HOME_TAB) << ":"
			<< "type:" << ui::E_TAB << ":id:" << &settings << ":parent:main:panel:nav:" << (selected == &settings ? "selected:true:" : "") << "dir:h:wrap:true:text:" << l_str(SETTINGS_TAB) << ":"
			<< "type:" << ui::E_TAB << ":id:" << &timers << ":parent:main:panel:nav:" << (selected == &timers ? "selected:true:" : "") << "dir:h:wrap:true:text:" << l_str(TIMERS_TAB) << ":"
			<< "type:" << ui::E_TAB << ":id:" << &logs << ":parent:main:panel:nav:tab_align:right:" << (selected == &logs ? "selected:true:" : "") << "dir:h:wrap:true:text:" << l_str(LOGS_TAB) << ":";
	}
}

namespace sync_time {	
	bool on_connect = false;
	
	struct time : ::time {
		static void callback(time_t);
		time() : ::time((void *)callback) {};
	};
	
	time browser_time;
	
	ui::messages time_request()
	{
		return (ui::messages("type:") << ui::GET_TIME << ":id:" << &browser_time << ":"); 
	}
	
	void sync_now_callback(button &id, uint32_t sender)
	{
		time_request().flush(); //send time request
	}
	
	void sync_on_connect_callback(switcher &id, uint32_t sender)
	{
		id.turn(on_connect).flush();	
		need_commit = true;
	}
	
	box widget;
	
	button sync_now((void*)sync_now_callback);
	
	box wrapper;
	text text;
	switcher sync_on_connect((void*)sync_on_connect_callback);
	
	void time::callback(time_t t) //get time
	{
		date_time::changed = true;
		setTime(t);
		
		//we have to check and update statistics if time changed;
		statistics::update(t, 0.0); //we do not put kwt's here(just time update)
	}
	
	void save()
	{
		EEPROM.put(FV_SYNC_TIME_ON_CONNECT, on_connect);
	}

	void load()
	{
		EEPROM.get(FV_SYNC_TIME_ON_CONNECT, on_connect);
	}
	
	void build(elbox_client *client)
	{
		if (on_connect)
			client->messages << time_request();
			
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::settings << ":dir:v:text:" << l_str(SYNC_TIME_TITLE) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &sync_now << ":parent:" << &widget << ":text:" << l_str(SYNC_TIME_NOW) << ":backcolor:orange:"
			
			<< "type:" << ui::E_BOX << ":id:" << &wrapper << ":parent:" << &widget << ":dir:h:"
			<< "type:" << ui::E_TEXT << ":id:" << &text << ":parent:" << &wrapper << ":text:" << l_str(SYNC_TIME_ON_CONNECT) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &sync_on_connect << ":parent:" << &wrapper << ":" << sync_on_connect.attributes(on_connect);
	}
}

namespace date_time {
	bool changed = false;
	ifield::ifield() : ::ifield((void *)callback) {};

	box widget;
	box cols[3];

	text date_names[3];
	text time_names[3];
	
	//UNSAFE, order is not guaranteed
	ifield date[3]; //Year, month, day
	ifield time[3]; //Hour, minute, second
	
	void ifield::callback(ifield &id, String val)
	{
		changed = true; //set to false somewhere
		id.val = val.toInt();
		setTime(time[0].val, time[1].val, time[2].val, date[2].val, date[1].val, date[0].val);
		
		//we have to check and update statistics if time changed;
		statistics::update(now(), 0.0); //we do not put kwt's here(just time update)
	}
	
	 //TODO send less data
	void get_time()
	{
		time_t t = now();
		date[0].val = year(t), date[1].val = month(t), date[2].val = day(t);
		time[0].val = hour(t), time[1].val = minute(t), time[2].val = second(t);
	}
	
	void update()
	{
		get_time();
		for (int i = 0; i < 3; i++)
			runtime_messages
				<< "id:" << &date[i] << ":value:" << date[i].val << ":"
				<< "id:" << &time[i] << ":value:" << time[i].val << ":";
	}
	
	void build(elbox_client *client)
	{
		client->messages << "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::settings << ":dir:h:align:up:text:" << l_str(DATE_TIME_TITLE) << ":";
		for (int i = 0; i < 3; i++)
			client->messages
				<< "type:" << ui::E_BOX << ":id:" << &cols[i] << ":parent:" << &widget << ":"
				<< "type:" << ui::E_TEXT << ":id:" << &date_names[i] << ":parent:" << &cols[i] << ":text:" << l_str(DATE_TIME_COL_NAMES + i * 2) << ":"
				<< "type:" << ui::E_FIELD << ":id:" << &date[i] << ":parent:" << &cols[i] << ":value:" << date[i].val << ":"
				<< "type:" << ui::E_TEXT << ":id:" << &time_names[i] << ":parent:" << &cols[i] << ":text:" << l_str(DATE_TIME_COL_NAMES + i * 2 + 1) << ":"
				<< "type:" << ui::E_FIELD << ":id:" << &time[i] << ":parent:" << &cols[i] << ":value:" << time[i].val << ":";

	}
}

namespace language_selector {
	struct radio : ::radio {
		static void callback(radio&, uint32_t);
		radio() : ::radio((void *)callback) {};
	};
	
	box widget;
	radio language_radio[NUMBER_OF_SUPPORTED_LANGUAGES];
	
	void radio::callback(radio &id, uint32_t sender) {
		language new_language = (language)(&id - language_radio);
		if (current_language != new_language) {
			current_language = new_language;
			tab_navigation::selected = &tab_navigation::settings;
			rebuild_electrobox_ui();
			
			need_commit = true;
		}
	}

	void save()
	{
		EEPROM.put(FV_LANGUAGE, current_language);
	}

	void load()
	{
		EEPROM.get(FV_LANGUAGE, current_language);
	}
	
	void build(elbox_client *client)
	{
		client->messages << "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::settings << ":dir:v:h:content:text:" << l_str(LANGUAGE_SELECTOR_TITLE) << ":";
		for (int i = 0; i < NUMBER_OF_SUPPORTED_LANGUAGES; i++)
			client->messages
				<< "type:" << ui::E_BUTTON << ":id:" << &language_radio[i] << ":parent:" << &widget << ":"
				<< language_radio[i].attributes(i == (int)current_language ? true : false) << "text:" << languages[i][LANGUAGE_NAME] << ":";
	}	
}

//String dbg;
namespace charge_log {
	bool need_update; //if we have to update log in web ui
	
	struct log {
		time_t time_stamp;
		charge_status::status value = charge_status::C_UNKNOWN;
		
		double kwt;
	};
	
	bool log_peak = false;
	size_t max_log_len = 1024;
	size_t log_len = 0;
	
	size_t window_begin, window_len = 32;
	File data;
	
	box widget;
	text text;
	
	void init()
	{
		window_begin = 0;
		log_len = 0;
		log_peak = false;
	}
	
	void clear()
	{
		if (!data || !log_len) return;
		
		init();
		
		data.truncate(0);
		
		data.seek(0);
		data.write((uint8_t *)&log_len, sizeof(size_t));
		
		(ui::messages("id:") << &text << ":text::").flush(); //also clear browser log string
		
		need_update = true;
	}
	
	void load()
	{
		init();
		
		data = LittleFS.open("/log.bin", "r+");
		//dbg += String("") + (data ? "file exist" : "file not exist") + "\n";
		if (!data) {
			data = LittleFS.open("/log.bin", "w+");
			//dbg += String("") + "data size is zero" + "\n";
			data.write((uint8_t *)&log_len, sizeof(size_t));
		} else {
			//dbg += String("") + "data size: " + data.size() + "\n";
			data.read((uint8_t *)&log_len, sizeof(size_t));
		}
		
		if (!data) return;
		
		//dbg += String("") + "log_len: " + log_len + "\n";
		window_begin = log_len > window_len ? log_len - window_len : 0;
		//dbg += String("") + "window_begin: " + window_begin + "\n";
		
		need_update = false;
	}
	
	void put()
	{
		if (!data) return;
		
		log l;
		l.time_stamp = now();
		l.value = charge_status::value;
		l.kwt = time_and_kwt_for_session::kwt;
		
		if (log_len == max_log_len)
			log_peak = true;
			
		if (log_len >= window_begin + window_len)
			window_begin++;
			
		//Seek new place to insert
		data.seek((log_len++ % max_log_len) * sizeof(log) + sizeof(size_t));
		data.write((uint8_t *)&l, sizeof(log));
		
		//save new log len
		data.seek(0);
		data.write((uint8_t *)&log_len, sizeof(size_t));
		
		need_update = true;
	}
	
	String dump()
	{
		if (!data) return "";
		
		String dmp;
		log l;
		
		size_t end = window_begin + window_len;
		if (end >= log_len && !log_peak) end = log_len;
		for (int i = window_begin; i < end; i++) {
			data.seek((i % max_log_len) * sizeof(log) + sizeof(size_t));
			data.read((uint8_t *)&l, sizeof(log));
			dmp = ui::escape(
				String(day(l.time_stamp)) + "/" + month(l.time_stamp) + "/" + year(l.time_stamp) + 
				" (" + hour(l.time_stamp) + ":" + minute(l.time_stamp) + ":" + second(l.time_stamp) + "): "
				+ l_str(CHARGER_STATUS + l.value) + " (" + l_str(KWT_FOR_SESSION) + ": " + l.kwt + l_str(KWT) + ");\n\n"
			) + dmp;
		}
		
		return dmp;
	}
	
	void update()
	{	
		if (need_update) {
			data.flush(); //Write into flash memory
			runtime_messages
				<< "id:" << &text << ":text:" << dump() << ":";
			need_update = false;
		}
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::logs << ":fill:true:text:" << l_str(LOGS_TITLE) << ":"
			<< "type:" << ui::E_TEXT << ":id:" << &text << ":parent:" << &widget << ":wrap:true:text:" << dump() << ":";
	}
}

namespace clear_log {
	struct button : ::button {
		static void callback(button &, uint32_t);
		button() : ::button((void *)callback) {};
	};
	
	box widget;
	button button;
	
	void button::callback(button &id, uint32_t sender)
	{
		charge_log::clear();
	}

	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::logs << ":dir:h:text:" << l_str(CLEAR_LOG_TITLE) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &button << ":parent:" << &widget << ":backcolor:orange:";
	}
}

namespace factory_reset {
	struct button : ::button {
		bool enabled = false;
		static void callback(button &, uint32_t);
		button() : ::button((void *)callback) {};
	};
	
	box widget;
	button applier;
	
	dialog are_you_sure;
	box wrapper;
	
	void yes_callback(button &id, uint32_t sender)
	{
		if (!applier.enabled) return;	
			applier.enabled = false;
		((ui::messages)":id:" << &are_you_sure << ":display:false:").flush(sender);
		factory_init(true); //set reset
		
		tab_navigation::selected = &tab_navigation::settings;
		rebuild_electrobox_ui();
	}
	
	void no_callback(button &id, uint32_t sender)
	{
		//":id:" << &widget << ":parent:" << &tab_navigation::settings << ":dir:h:text:"
		((ui::messages)":id:" << &are_you_sure << ":display:false:").flush(sender);
	}
	
	::button yes((void *)yes_callback);
	::button no((void *)no_callback);
	
	void button::callback(button &id, uint32_t sender)
	{
		//elbox_client &cl = *find_elbox_client(sender);
		((ui::messages)":id:" << &are_you_sure << ":display:true:").flush(sender);
		
		//TODO we need to run dialog window here!!	
		/*if (!applier.enabled) return;	
		applier.enabled = false;
		
		factory_init(true); //set reset
		
		tab_navigation::selected = &tab_navigation::settings;
		rebuild_electrobox_ui();*/
	}
	
	void applier_enable()
	{
		if (!applier.enabled)
			applier.enable(applier.enabled).flush();
	}

	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::settings << ":dir:h:text:" << l_str(FACTORY_RESET_TITLE) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &applier << ":parent:" << &widget << ":backcolor:orange:disabled:" << (applier.enabled ? "false:" : "true:")
			
			<< "type:" << ui::E_DIALOG << ":id:" << &are_you_sure << ":parent:main:display:false:"
			<< "type:" << ui::E_BOX << ":id:" << &wrapper << ":parent:" << &are_you_sure << ":dir:h:text:" << l_str(ARE_YOU_SURE_RESET) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &yes << ":parent:" << &wrapper << ":text:" << l_str(RESET_YES) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &no << ":parent:" << &wrapper << ":text:" << l_str(RESET_NO) << ":";
	}
}

namespace charge_switch {
	bool enabled = false;
	
	struct switcher : ::switcher {
		static void callback(switcher&, uint32_t);
		switcher() : ::switcher((void *)callback) {};
	};
	
	box widget;
	switcher switcher;
	
	void enable(bool state)
	{
		if (state != enabled)
		{
			switcher.turn(enabled).flush();
		}
	}
	
	void switcher::callback(switcher &id, uint32_t sender)
	{
		id.turn(enabled).flush();
		
		need_commit = true;
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::home << ":fill:true:dir:h:text:" << l_str(CHARGE_SWITCH_TITLE) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &switcher << ":parent:" << &widget << ":" << switcher.attributes(enabled);
	}
}

namespace charge_status {
	status value = C_UNSET;
	
	box widget;
	text text;
	
	void update(int val)
	{
		value = (status)val;
		charge_log::put();
		runtime_messages
			<< text.update(l_str(CHARGER_STATUS + val));
		//Start/Stop manual switch automatically
		if (value == C_CHARGING) {
			//charge_switch::enable(true);
		} else {
			//save statistics if charging stopped
			statistics::update(now(), time_and_kwt_for_session::kwt - time_and_kwt_for_session::previous_kwt);
			time_and_kwt_for_session::previous_kwt = 0.0; //reset previous kwt as new session will start
			//charge_switch::enable(false);
		}
	}
	 
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::home << ":dir:h:text:" << l_str(CHARGER_STATUS_TITLE) << ":"
			<< "type:" << ui::E_TEXT << ":id:" << &text << ":parent:" << &widget << ":text:" << l_str(CHARGER_STATUS + (int)value) << ":";
	}
}

namespace weekly_schedule {
	bool enabled;
	bool blocking = false;
	
	struct tfield : ::tfield {
		static void callback(tfield&, String);
		tfield() : ::tfield((void *)callback) {};
	};

	struct switcher : ::switcher {
		bool enabled = false;
		
		static void callback(switcher&, uint32_t);
		switcher() : ::switcher((void *)callback) {};
	};
	
	box widget;
	box cols[4];

	text col_names[4];
	text day_names[7];

	//UNSAFE, order is not guaranteed
	tfield begin[7];
	tfield end[7];
	
	switcher switchers[7];
	
	void tfield::callback(tfield &id, String val)
	{
		id.update(val).flush();
		
		need_commit = true;
	}

	void switcher::callback(switcher &id, uint32_t sender)
	{
		id.turn(id.enabled).flush();
		
		need_commit = true;
	}
	
	void save()
	{
		EEPROM.put(FV_SCHEDULE_ENABLED, enabled);
		for (int i = 0; i < 7; i++) {
			EEPROM.put(FV_SCHEDULE_BEGINS + i * sizeof(int), begin[i].hours);
			EEPROM.put(FV_SCHEDULE_ENDS + i * sizeof(int), end[i].hours);
			EEPROM.put(FV_SCHEDULE_BEGINS + i * sizeof(int) + sizeof(int) * 7, begin[i].minutes);
			EEPROM.put(FV_SCHEDULE_ENDS + i * sizeof(int) + sizeof(int) * 7, end[i].minutes);
			EEPROM.put(FV_SCHEDULE_SWITCHERS + i * sizeof(bool), switchers[i].enabled);
		}
	}

	void load()
	{
		EEPROM.get(FV_SCHEDULE_ENABLED, enabled);
		for (int i = 0; i < 7; i++) {
			EEPROM.get(FV_SCHEDULE_BEGINS + i * sizeof(int), begin[i].hours);
			EEPROM.get(FV_SCHEDULE_ENDS + i * sizeof(int), end[i].hours);
			EEPROM.get(FV_SCHEDULE_BEGINS + i * sizeof(int) + sizeof(int) * 7, begin[i].minutes);
			EEPROM.get(FV_SCHEDULE_ENDS + i * sizeof(int) + sizeof(int) * 7, end[i].minutes);
			EEPROM.get(FV_SCHEDULE_SWITCHERS + i * sizeof(bool), switchers[i].enabled);
		}
	}
	
	void build(elbox_client *client)
	{
		client->messages << "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::timers << ":dir:h:text:" << l_str(WEEKLY_SCHEDULE_TITLE) << ":";
		for (int i = 0; i < 4; i++)
			client->messages
				<< "type:" << ui::E_BOX << ":id:" << &cols[i] << ":parent:" << &widget << ":"
				<< "type:" << ui::E_TEXT << ":id:" << &col_names[i] << ":parent:" << &cols[i] << ":text:" << l_str(WEEKLY_SCHEDULE_COL_NAMES + i) << ":";
					
		for (int row = 0; row < 7; row++)	
			client->messages 
				<< "type:" << day_names[row].element.type << ":id:" << &day_names[row] << ":parent:" << &cols[0] << ":text:" << l_str(WEEKLY_SCHEDULE_DAY_NAMES + row) << ":"
				<< "type:" << ui::E_TIME_FIELD << ":id:" << &begin[row] << ":parent:" << &cols[1] << ":" << begin[row].attributes()
				<< "type:" << ui::E_TIME_FIELD << ":id:" << &end[row] << ":parent:" << &cols[2] << ":" << end[row].attributes()
				<< "type:" << ui::E_BUTTON << ":id:" << &switchers[row] << ":parent:" << &cols[3] << ":" << switchers[row].attributes(switchers[row].enabled);
	}
	
	void trigger()
	{
		time_t t = now();	
		int cur_day = ((weekday(t) - 2) + 7) % 7;
	
		if (!enabled || !switchers[cur_day].enabled) {
			blocking = false;
			return;
		}
		
		int cur_time = hour(t) * 60 + minute(t);
		
		int cur_day_begin_time = begin[cur_day].hours * 60 + begin[cur_day].minutes;
		int cur_day_end_time = end[cur_day].hours * 60 + end[cur_day].minutes;
		
		int prev_day = ((cur_day - 1) + 7) % 7;
		int prev_day_begin_time = begin[prev_day].hours * 60 + begin[prev_day].minutes;
		int prev_day_end_time = end[prev_day].hours * 60 + end[prev_day].minutes;
			
		if (switchers[prev_day].enabled && prev_day_begin_time >= prev_day_end_time && cur_time < prev_day_end_time)
			blocking = false;
		else if (cur_day_begin_time < cur_day_end_time)
			blocking = (cur_time >= cur_day_begin_time && cur_time < cur_day_end_time) ? false : true;
		else
			blocking = (cur_time >= cur_day_begin_time) ? false : true;
			
		//send_log(String("cur_day: ") + cur_day + "\ncur_day_begin_time: " + cur_day_begin_time + "\ncur_day_end_time: " + cur_day_end_time + 
		//	"\nprev_day: " + prev_day + "\nprev_day_begin_time: " + prev_day_begin_time + "\nprev_day_end_time: " + prev_day_end_time + "\nblocking: " + blocking);
	}
}

namespace consumption {
	float voltage = 0;
	float current = 0;
	float kwt = 0;
	
	box widget;
	text text;
	
	String get()
	{
		return String(voltage) + "V  " + current + "A  " + kwt + "kWt";
	}
	
	void update(float v, float c, float k)
	{
		voltage = v, current = c, kwt = k;
		runtime_messages
			<< text.update(get());
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::home << ":dir:h:text:" << l_str(CONSUMPTION_TITLE) << ":"
			<< "type:" << ui::E_TEXT << ":id:" << &text << ":parent:" << &widget << ":text:" << get() << ":";
	}
}

namespace time_and_kwt_for_session {
	int hours = 0, minutes = 0;
	float kwt = 0;
	
	float previous_kwt = 0;
	
	box widget;
	text text;
	
	void save()
	{
		EEPROM.put(FV_HOURS_FOR_SESSION, hours);
		EEPROM.put(FV_MINUTES_FOR_SESSION, minutes);
		EEPROM.put(FV_KWT_FOR_SESSION, kwt);
	}

	void load()
	{
		EEPROM.get(FV_HOURS_FOR_SESSION, hours);
		EEPROM.get(FV_MINUTES_FOR_SESSION, minutes);
		EEPROM.get(FV_KWT_FOR_SESSION, kwt);
	}
	
	String get()
	{
		return hours_and_minutes_to_str(hours, minutes) + " - " + kwt + " " + l_str(KWT);
	}
	
	void update(int h, int m, float k)
	{
		hours = h, minutes = m, kwt = k;
		runtime_messages
			<< text.update(get());
		
		//need_commit = true; //oh noes, this was damaging for eeprom
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::home << ":dir:h:text:" << l_str(TIME_KWT_TITLE) << ":"
			<< "type:" << ui::E_TEXT << ":id:" << &text << ":parent:" << &widget << ":text:" << get() << ":";
	}
}

namespace current_regulator {
	int value;
	
	struct range : ::range {
		static void callback(range&, int val);
		range() : ::range((void *)callback) {};
	};
	
	box widget;
	range range;
	
	void range::callback(range &id, int val)
	{
		id.slide(value, val).flush();
		
		need_commit = true;
	}
	
	void save()
	{
		EEPROM.put(FV_CURRENT_REGULATOR, value);
	}

	void load()
	{
		EEPROM.get(FV_CURRENT_REGULATOR, value);
	}
	
	void update(int val)
	{
		runtime_messages
			<< range.slide(value, val);
		
		need_commit = true;
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::home << ":fill:true:dir:h:text:" << l_str(CURRENT_REGULATOR_TITLE) << ":"
			<< "type:" << ui::E_RANGE << ":id:" << &range << ":parent:" << &widget << ":min:6:max:48:value:" << value << ":";
	}
}

namespace ground_check {
	bool enabled = true;
	
	struct switcher : ::switcher {
		static void callback(switcher&, uint32_t);
		switcher() : ::switcher((void *)callback) {};
	};
	
	box widget;
	switcher switcher;
	
	void switcher::callback(switcher &id, uint32_t sender)
	{
		id.turn(enabled).flush();
		
		need_commit = true;
	}
	
	void save()
	{
		EEPROM.put(FV_GROUND_CHECK, enabled);
	}

	void load()
	{
		EEPROM.get(FV_GROUND_CHECK, enabled);
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::home << ":dir:h:text:" << l_str(GROUND_CHECK_TITLE) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &switcher << ":parent:" << &widget << ":" << switcher.attributes(enabled);
	}
}

namespace adaptive_mode {
	bool enabled;
		
	struct switcher : ::switcher {
		static void callback(switcher&, uint32_t);
		switcher() : ::switcher((void *)callback) {};
	};
	
	box widget;
	switcher switcher;
	
	void switcher::callback(switcher &id, uint32_t sender)
	{
		id.turn(enabled).flush();
		
		need_commit = true;
	}
	
	void save()
	{
		EEPROM.put(FV_ADAPTIVE_MODE, enabled);
	}

	void load()
	{
		EEPROM.get(FV_ADAPTIVE_MODE, enabled);
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::home << ":dir:h:text:" << l_str(ADAPTIVE_MODE_TITLE) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &switcher << ":parent:" << &widget << ":" << switcher.attributes(enabled);
	}
}

namespace limit_kwt_for_session {
	bool enabled;
	int value;
	
	struct switcher : ::switcher {
		static void callback(switcher&, uint32_t);
		switcher() : ::switcher((void *)callback) {};
	};
	
	struct range : ::range {
		static void callback(range&, int val);
		range() : ::range((void *)callback) {};
	};
	
	box widget;
	switcher switcher;
	range range;
	
	void switcher::callback(switcher &id, uint32_t sender)
	{
		id.turn(enabled).flush();
		
		need_commit = true;
	}
	
	void range::callback(range &id, int val)
	{
		id.slide(value, val).flush();
		
		need_commit = true;
	}
	
	void save()
	{
		EEPROM.put(FV_LIMIT_KWT_ENABLED, enabled);
		EEPROM.put(FV_LIMIT_KWT_VALUE, value);
	}

	void load()
	{
		EEPROM.get(FV_LIMIT_KWT_ENABLED, enabled);
		EEPROM.get(FV_LIMIT_KWT_VALUE, value);
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::home << ":fill:true:dir:h:text:" << l_str(LIMIT_KWT_TITLE) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &switcher << ":parent:" << &widget << ":w:5:" << switcher.attributes(enabled)
			<< "type:" << ui::E_RANGE << ":id:" << &range << ":parent:" << &widget << ":min:1:max:100:value:" << value << ":";
	}
}

namespace limit_by_time {
	bool enabled;
	int value;
	
	struct switcher : ::switcher {
		static void callback(switcher&, uint32_t);
		switcher() : ::switcher((void *)callback) {};
	};
	
	struct range : ::range {
		static void callback(range&, int val);
		range() : ::range((void *)callback) {};
	};
	
	box widget;
	switcher switcher;
	range range;
	
	void switcher::callback(switcher &id, uint32_t sender)
	{
		id.turn(enabled).flush();
		
		need_commit = true;
	}
	
	void range::callback(range &id, int val)
	{
		id.slide(value, val).flush();
		
		need_commit = true;
	}
	
	void save()
	{
		EEPROM.put(FV_LIMIT_BY_TIME_ENABLED, enabled);
		EEPROM.put(FV_LIMIT_BY_TIME_VALUE, value);
	}

	void load()
	{
		EEPROM.get(FV_LIMIT_BY_TIME_ENABLED, enabled);
		EEPROM.get(FV_LIMIT_BY_TIME_VALUE, value);
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::home << ":fill:true:dir:h:text:" << l_str(LIMIT_BY_TIME_TITLE) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &switcher << ":parent:" << &widget << ":w:5:" << switcher.attributes(enabled)
			<< "type:" << ui::E_RANGE << ":id:" << &range << ":parent:" << &widget << ":min:1:max:24:value:" << value << ":";
	}
}

namespace display_off_time {
	bool enabled;
	int value;
	
	struct switcher : ::switcher {
		static void callback(switcher&, uint32_t);
		switcher() : ::switcher((void *)callback) {};
	};
	
	struct range : ::range {
		static void callback(range&, int val);
		range() : ::range((void *)callback) {};
	};
	
	box widget;
	switcher switcher;
	range range;
	
	void switcher::callback(switcher &id, uint32_t sender)
	{
		id.turn(enabled).flush();
		
		need_commit = true;
	}
	
	void range::callback(range &id, int val)
	{
		id.slide(value, val).flush();
		
		need_commit = true;
	}
	
	void save()
	{
		EEPROM.put(FV_DISPLAY_OFF_ENABLED, enabled);
		EEPROM.put(FV_DISPLAY_OFF_VALUE, value);
	}

	void load()
	{
		EEPROM.get(FV_DISPLAY_OFF_ENABLED, enabled);
		EEPROM.get(FV_DISPLAY_OFF_VALUE, value);
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::settings << ":fill:true:dir:h:text:" << l_str(DISPLAY_OFF_TIME) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &switcher << ":parent:" << &widget << ":w:5:" << switcher.attributes(enabled)
			<< "type:" << ui::E_RANGE << ":id:" << &range << ":parent:" << &widget << ":min:1:max:60:value:" << value << ":";
	}
}

namespace schedule_enabler {
	struct switcher : ::switcher {
		static void callback(switcher&, uint32_t);
		switcher() : ::switcher((void *)callback) {};
	};
	
	box widget;
	switcher switcher;
	
	void switcher::callback(switcher &id, uint32_t sender)
	{
		id.turn(weekly_schedule::enabled).flush();
		
		need_commit = true;
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::timers << ":dir:h:text:" << l_str(SCHEDULE_ENABLER_TITLE) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &switcher << ":parent:" << &widget << ":" << switcher.attributes(weekly_schedule::enabled);
	}
}

namespace display_brightness_regulator {
	int value;
	
	struct range : ::range {
		static void callback(range&, int val);
		range() : ::range((void *)callback) {};
	};
	
	box widget;
	range range;
	
	void range::callback(range &id, int val)
	{
		id.slide(value, val).flush();
		
		need_commit = true;
	}
	
	void save()
	{
		EEPROM.put(FV_DISPLAY_BRIGHTNESS, value);
	}

	void load()
	{
		EEPROM.get(FV_DISPLAY_BRIGHTNESS, value);
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::settings << ":fill:true:h:content:dir:h:text:" << l_str(BRIGHTNESS_REGULATOR_TITLE) << ":"
			<< "type:" << ui::E_RANGE << ":id:" << &range << ":parent:" << &widget << ":min:0:max:100:value:" << value << ":";
	}
}

namespace network_settings {
	bool applied = false; //if we have to update network settings
	bool sta_enabled;
	
	struct field : ::field {
		static void callback(field&, String);
		field() : ::field((void *)callback) {};
	};
	
	struct switcher : ::switcher {
		static void callback(switcher&, uint32_t);
		switcher() : ::switcher((void *)callback) {};
	};
	
	struct button : ::button {
		bool enabled = false;
		static void callback(button&, uint32_t);
		button() : ::button((void *)callback) {};
	};
	
	//WiFiMode_t 
	
	void save()
	{
		if (!applied) return;
		//applied = false;
		
		EEPROM.put(FV_AP_SSID, network::ap_ssid);
		EEPROM.put(FV_AP_PASS, network::ap_pass);
		EEPROM.put(FV_AP_IP, uint32_t(network::ap_ip));
		EEPROM.put(FV_AP_GATEWAY, uint32_t(network::ap_gateway));
		EEPROM.put(FV_AP_SUBNET, uint32_t(network::ap_subnet));

		EEPROM.put(FV_STA_ENABLED, sta_enabled);
		EEPROM.put(FV_STA_SSID, network::sta_ssid);
		EEPROM.put(FV_STA_PASS, network::sta_pass);
		EEPROM.put(FV_STA_IP, uint32_t(network::sta_ip));
		EEPROM.put(FV_STA_GATEWAY, uint32_t(network::sta_gateway));
		EEPROM.put(FV_STA_SUBNET, uint32_t(network::sta_subnet));
	}

	void load()
	{
		uint32_t ap_ip, ap_gateway, ap_subnet, sta_ip, sta_gateway, sta_subnet;
		
		EEPROM.get(FV_AP_SSID, network::ap_ssid);
		EEPROM.get(FV_AP_PASS, network::ap_pass);
		EEPROM.get(FV_AP_IP, ap_ip);
		EEPROM.get(FV_AP_GATEWAY, ap_gateway);
		EEPROM.get(FV_AP_SUBNET, ap_subnet);

		EEPROM.get(FV_STA_ENABLED, sta_enabled);
		EEPROM.get(FV_STA_SSID, network::sta_ssid);
		EEPROM.get(FV_STA_PASS, network::sta_pass);
		EEPROM.get(FV_STA_IP, sta_ip);
		EEPROM.get(FV_STA_GATEWAY, sta_gateway);
		EEPROM.get(FV_STA_SUBNET, sta_subnet);
		
		network::ap_ip = IPAddress(ap_ip);
		network::ap_gateway = IPAddress(ap_gateway);
		network::ap_subnet = IPAddress(ap_subnet);
		network::sta_ip = IPAddress(sta_ip);
		network::sta_gateway = IPAddress(sta_gateway);
		network::sta_subnet = IPAddress(sta_subnet);
	}
	
	box widget;
	tab ap_tab;
	tab sta_tab;
	
	text row_name[11];
	field row_field[10];
	
	button applier;
	switcher sta;
	
	box ap_text_col, ap_field_col, sta_text_col, sta_field_col;
	
	void applier_enable()
	{
		if (!applier.enabled)
			applier.enable(applier.enabled).flush();
	}
	
	bool passphrase_is_valid(String val)
	{
		if (val.length() && (val.length() > 40 || val.length() < 8))
			return false;
		else
			return true;
	}
	
	void field::callback(field &id, String val)
	{
		int idx = (int)(&id - &row_field[0]);
		applier_enable();
		
		switch (idx) {
			case 0:
				val.toCharArray(network::ap_ssid, 40);
				break;
			case 1:
				if (passphrase_is_valid(val)) {
					val.toCharArray(network::ap_pass, 40);
					break;
				} else {
					return;
				}
			case 2:
				network::ap_ip.fromString(val);
				break;
			case 3:
				network::ap_gateway.fromString(val);
				break;
			case 4:
				network::ap_subnet.fromString(val);
				break;
				
			case 5:
				val.toCharArray(network::sta_ssid, 40);
				break;
			case 6:
				if (passphrase_is_valid(val)) {
					val.toCharArray(network::sta_pass, 40);
					break;
				} else {
					return;
				}
			case 7:
				network::sta_ip.fromString(val);
				break;
			case 8:
				network::sta_gateway.fromString(val);
				break;
			case 9:
				network::sta_subnet.fromString(val);
				break;
		}
		
		id.update(val).flush();
	}
	
	String get_value(int idx)
	{	
		switch (idx) {
			case 0:
				return network::ap_ssid;
			case 1:
				return network::ap_pass;
			case 2:
				return network::ap_ip.toString();
			case 3:
				return network::ap_gateway.toString();
			case 4:
				return network::ap_subnet.toString();
			case 5:
				return network::sta_ssid;
			case 6:
				return network::sta_pass;
			case 7:
				return network::sta_ip.toString();
			case 8:
				return network::sta_gateway.toString();
			case 9:
				return network::sta_subnet.toString();
		}
		return "";
	}
	
	void button::callback(button &id, uint32_t sender)
	{
		if (!applier.enabled) return;
		applier.enabled = false;
		
		applied = need_commit = true;
		network::end(); //no need to flush since network reloads
	}
	
	void switcher::callback(switcher &id, uint32_t sender)
	{
		applier_enable();
		id.turn(sta_enabled).flush();
	}
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::settings << ":fill:true:text:" << l_str(NETWORK_SETTINGS_TITLE) << ":"
			<< "type:" << ui::E_TAB << ":id:" << &ap_tab << ":parent:" << &widget << ":panel:net:dir:h:selected:true:wrap:true:text:" << l_str(AP_TAB) << ":"
			<< "type:" << ui::E_TAB << ":id:" << &sta_tab << ":parent:" << &widget << ":panel:net:dir:h:wrap:true:text:" << l_str(STA_TAB) << ":"
			<< "type:" << ui::E_BOX << ":id:" << &ap_text_col << ":parent:" << &ap_tab << ":dir:v:w:content:"
			<< "type:" << ui::E_BOX << ":id:" << &ap_field_col << ":parent:" << &ap_tab << ":dir:v:"
			<< "type:" << ui::E_BOX << ":id:" << &sta_text_col << ":parent:" << &sta_tab << ":dir:v:w:content:"
			<< "type:" << ui::E_BOX << ":id:" << &sta_field_col << ":parent:" << &sta_tab << ":dir:v:"
			<< "type:" << ui::E_BUTTON << ":id:" << &applier << ":parent:" << &widget << ":backcolor:orange:disabled:" << (applier.enabled ? "false" : "true") << ":text:" << l_str(SAVE_NETWORK_SETTINGS) << ":"
			<< "type:" << ui::E_TEXT << ":id:" << &row_name[5] << ":parent:" << &sta_text_col << ":align:right:text:" << l_str(NETWORK_SETTINGS + 5) << ":"
			<< "type:" << ui::E_BUTTON << ":id:" << &sta << ":parent:" << &sta_field_col << ":" << sta.attributes(sta_enabled);
			
		for(int i = 0; i < 5; i++)
			client->messages
				<< "type:" << ui::E_TEXT << ":id:" << &row_name[i] << ":parent:" << &ap_text_col << ":align:right:text:" << l_str(NETWORK_SETTINGS + i) << ":"
				<< "type:" << ui::E_FIELD << ":id:" << &row_field[i] << ":parent:" << &ap_field_col << ":value:" << get_value(i) << ":";
		
		for(int i = 5; i < 10; i++)
			client->messages
				<< "type:" << ui::E_TEXT << ":id:" << &row_name[i + 1] << ":parent:" << &sta_text_col << ":align:right:text:" << l_str(NETWORK_SETTINGS + i + 1) << ":"
				<< "type:" << ui::E_FIELD << ":id:" << &row_field[i] << ":parent:" << &sta_field_col << ":value:" << get_value(i) << ":";
	}
}

namespace statistics {
	double total_kwt;

	kwt_range ranges[5] = {
		{0, 60 * 60, 0, 0}, //hour
		{0, 60 * 60 * 24, 0, 0}, //day
		{0, 60 * 60 * 24 * 7, 0, 0}, //week
		{0, 60 * 60 * 24 * 7 * 4, 0, 0}, //month
		{0, 60 * 60 * 24 * 7 * 4 * 12, 0, 0}, //year
	};

	box widget;
	
	text total_kwt_text;
	//box stats_wrapper;
	tab prev_stats_tab, cur_stats_tab;
	//text prev_title, cur_title;
	text prev_text[5], cur_text[5];

	//plz use macros to reduce copypaste
	void save()
	{
		time_and_kwt_for_session::save();
		EEPROM.put(FV_STATS_TOTAL, total_kwt);
		for (int i = 0; i < 5; i++)
			EEPROM.put(FV_STATS_RANGES + i * sizeof(kwt_range), ranges[i]);
	}
	
	void load()
	{
		time_and_kwt_for_session::load();
		EEPROM.get(FV_STATS_TOTAL, total_kwt);
		for (int i = 0; i < 5; i++)
			EEPROM.get(FV_STATS_RANGES + i * sizeof(kwt_range), ranges[i]);
	}


	void update_message(int i)
	{
		runtime_messages
			<< "id:" << &prev_text[i] << ":text:" << l_str(STATS_TEXT + i) << " " << String(ranges[i].delta) << l_str(KWT) << ":"
			<< "id:" << &cur_text[i] << ":text:" << l_str(STATS_TEXT + i) << " " << String(ranges[i].kwt) << l_str(KWT) << ":";
	}
	
	//TODO make statistics carrier protection
	void update(time_t now, double kwt)
	{
		total_kwt += kwt;
		
		for (int i = 0; i < 5; i++) {
			DEBUG_MSG("Statistics for %i: carry: %s, now: %lu, from: %lu, offset: %lu.\n", i, ((now > ranges[i].from + ranges[i].offset) ? "true" : "false"), now, ranges[i].from, ranges[i].offset);
			if (now > ranges[i].from + ranges[i].offset) {
				ranges[i].from = now;
				ranges[i].delta = ranges[i].kwt + kwt;
				ranges[i].kwt = 0;
			} else {
				ranges[i].kwt += kwt;
			}
			update_message(i);
		}
		
		runtime_messages
			<< ":id:" << &total_kwt_text << ":text:" << l_str(STATS_ALL_TIME) << " " << String(total_kwt) << l_str(KWT) << ":";
		
		need_commit = true;
	}
	
	void reset_callback(button& id, uint32_t sender)
	{
		time_t t = now();
		for (int i = 0; i < 5; i++) {
			ranges[i].from = t;
			ranges[i].kwt = ranges[i].delta = 0;
			update_message(i);
		} //total kwt not changed
		
		need_commit = true;
	}
	button reset((void*)reset_callback);
	
	void build(elbox_client *client)
	{
		client->messages
			<< "type:" << ui::E_BOX << ":id:" << &widget << ":parent:" << &tab_navigation::logs << ":fill:true:text:" << l_str(STATS_TITLE) << ":"
			<< "type:" << ui::E_TEXT << ":id:" << &total_kwt_text << ":parent:" << &widget << ":text:" << l_str(STATS_ALL_TIME) << " " << String(total_kwt) << l_str(KWT) << ":"
			//<< "type:" << ui::E_BOX << ":id:" << &stats_wrapper << ":parent:" << &widget << ":dir:h:"
			<< "type:" << ui::E_TAB << ":id:" << &prev_stats_tab << ":parent:" << &widget << ":panel:stats:selected:true:text:" << l_str(STATS_PREV_TITLE) << ":"
			<< "type:" << ui::E_TAB << ":id:" << &cur_stats_tab << ":parent:" << &widget << ":panel:stats:text:" << l_str(STATS_CUR_TITLE) << ":";
			//<< "id:" << &prev_stats << ":text:" << l_str(STATS_PREV_TITLE) << ":" //lil hack
			//<< "id:" << &cur_stats << ":text:" << l_str(STATS_CUR_TITLE) << ":";

			for (int i = 0; i < 5; i++) {
				client->messages
					<< "type:" << ui::E_TEXT << ":id:" << &prev_text[i] << ":parent:" << &prev_stats_tab << ":align:left:text:" << l_str(STATS_TEXT + i) << " " << String(ranges[i].delta) << l_str(KWT) << ":"
					<< "type:" << ui::E_TEXT << ":id:" << &cur_text[i] << ":parent:" << &cur_stats_tab << ":align:left:text:" << l_str(STATS_TEXT + i) << " " << String(ranges[i].kwt) << l_str(KWT) << ":";
			}
			
		client->messages
			<< "type:" << ui::E_BUTTON << ":id:" << &reset << ":parent:" << &widget << ":text:" << l_str(STATS_RESET) << ":";
	}
}

void elbox_client::confirm_frame(void *id)
{
	elbox_client *sender = (elbox_client *)((char *)id - offsetof(elbox_client, current_frame));
	sender->frame_number++; //goto next frame and send it
	sender->waiting_for_frame_confirmation = false;
}

bool queue_is_not_empty(uint32_t id) //TODO fix crash
{
	AsyncWebSocketClient *client = ui::ws->client(id); //maybeh it becomes null?
	if (!client) {
		//DEBUG_MSG("can't get client queue size, because it does not exist, id: %i\n", id);
		return false;
	}
	if((client->_messageQueue.length()) && (client->_status == WS_CONNECTED) ) return true;
	return false;
}

int number_of_interface_loaders = 0;

void elbox_client::build_interface()
{
	if (waiting_for_frame_confirmation || queue_is_not_empty(id)) //!ui::ws->availableForWrite(id)
		return;
	
	switch (frame_number) {
	case 0: tab_navigation::build(this); break;
	case 1: charge_status::build(this); break;
	case 2: time_and_kwt_for_session::build(this); break;
	case 3: consumption::build(this); break;
	case 4: current_regulator::build(this); break;
	case 5: ground_check::build(this); break;
	case 6: adaptive_mode::build(this); break;
	
	case 7: display_brightness_regulator::build(this); break;
	case 8: display_brightness_regulator::build(this); break;
	case 9: display_off_time::build(this); break;
	case 10: date_time::build(this); break;
	case 11: sync_time::build(this); break;
	case 12: language_selector::build(this); break;
	case 13: factory_reset::build(this); break;
	case 14: network_settings::build(this); break;
	
	case 15: schedule_enabler::build(this); break;
	case 16: weekly_schedule::build(this); break;
	
	case 17: statistics::build(this); break;
	case 18: charge_log::build(this); break;
	case 19: clear_log::build(this); break;
	
	default:
		interface_loaded = true;
		tab_navigation::selected = &tab_navigation::home;
		number_of_interface_loaders--;
		messages.buffer.~String(); //invalidate buffer
		break;
	}
	
	if (!interface_loaded) {
		messages << "type:" << ui::FRAME << ":id:" << &current_frame << ":sender:" << this << ":";
		messages.flush(id); //flush messages to this client
	}
	
	waiting_for_frame_confirmation = true;
}

void elbox_client::cleanup()
{
	//present = false;
	interface_loaded = false;
	waiting_for_frame_confirmation = false;
	frame_number = 0;
}

elbox_client::elbox_client() : current_frame((void *)elbox_client::confirm_frame)
{
	cleanup();
};

elbox_client elbox_clients[MAX_ELBOX_CLIENTS];

int number_of_clients = 0;

void add_elbox_client(uint32_t id)
{
	for (int i = 0; i < MAX_ELBOX_CLIENTS; i++) {
		elbox_client &cl = elbox_clients[i];
		if (!cl.present) {
			cl.present = true;
			cl.id = id;
			number_of_interface_loaders++;
			number_of_clients++;
			
			DEBUG_MSG("client connected, id: %i\n\tinterface loader №: %i\n\ttotal number of clients: %i\n", cl.id, number_of_interface_loaders, number_of_clients);
			break;
		}
	}
}

void del_elbox_client(uint32_t id)
{
	for (int i = 0; i < MAX_ELBOX_CLIENTS; i++) {
		elbox_client &cl = elbox_clients[i];
		
		if (cl.id == id) {
			if (cl.messages.buffer)
				cl.messages.buffer.~String(); //invalidate buffer
			cl.present = false;
			if (!cl.interface_loaded)
				number_of_interface_loaders--;
			number_of_clients--;
			
			DEBUG_MSG("client disconnected, id: %i\n\tlast interface status: %s\n\ttotal number of clients: %i\n", cl.id, (cl.interface_loaded ? "loaded" : "not loaded"), number_of_clients);
			cl.cleanup();
			break;
		}
	}
}

elbox_client *find_elbox_client(uint32_t id)
{
	for (int i = 0; i < MAX_ELBOX_CLIENTS; i++) {
		elbox_client &cl = elbox_clients[i];
			
		if (cl.present && cl.id == id)
			return &cl;
	}
	return nullptr;
}

ui::messages electrobox_ui(uint32_t client)
{
	//add_elbox_client(client);
	/*
	client->messages.clear();
	
	//Widgets may not be built fully. Mostly parts that meant to be dynamic
	tab_navigation::build();
	
	//charge_switch::build(&tab_navigation::home);
	charge_status::build(&tab_navigation::home);
	time_and_kwt_for_session::build(&tab_navigation::home);
	consumption::build(&tab_navigation::home);
	current_regulator::build(&tab_navigation::home);
	ground_check::build(&tab_navigation::home);
	adaptive_mode::build(&tab_navigation::home);
	//limit_kwt_for_session::build(&tab_navigation::home);
	//limit_by_time::build(&tab_navigation::home);
	
	display_brightness_regulator::build(&tab_navigation::settings);
	display_off_time::build(&tab_navigation::settings);
	date_time::build(&tab_navigation::settings);
	sync_time::build(&tab_navigation::settings);
	language_selector::build(&tab_navigation::settings);
	factory_reset::build(&tab_navigation::settings);
	network_settings::build(&tab_navigation::settings);
	
	schedule_enabler::build(&tab_navigation::timers);
	weekly_schedule::build(&tab_navigation::timers);
	
	//statistics::build(&tab_navigation::logs);
	//charge_log::build(&tab_navigation::logs);
	//clear_log::build(&tab_navigation::logs);
	
	tab_navigation::selected = &tab_navigation::home;
	
	return client->messages;*/
}

//Full rebuild, if core changes were made. (Interface language switch for example)
void rebuild_electrobox_ui()
{
	for (int i = 0; i < MAX_ELBOX_CLIENTS; i++) {
		number_of_interface_loaders++;
		elbox_clients[i].cleanup();
	}
	//electrobox_ui().send(); TODO new version
}

void ui::on_connect(uint32_t client)
{
	//send_log(dbg);
	add_elbox_client(client);
	//return electrobox_ui();
};

void ui::on_disconnect(uint32_t client)
{
	//send_log(dbg);
	del_elbox_client(client);
	//return electrobox_ui();
};

void factory_init(bool reset = false)
{
	//Set all values to default
	current_language = LANG_UA;
	
	{
		using namespace weekly_schedule;
		enabled = false;
		
		for (int row = 0; row < 7; row++) {
			begin[row].hours = 23;
			begin[row].minutes = 0;
			end[row].hours = 7;
			end[row].minutes = 0;
			
			switchers[row].enabled = false;
		}
	}
	
	current_regulator::value = 6;
	//ground_check::enabled = true;
	adaptive_mode::enabled = false;
	/*limit_kwt_for_session::enabled = false;
	limit_kwt_for_session::value = 1;
	limit_by_time::enabled = false;
	limit_by_time::value = 1;*/
	display_off_time::enabled = false;
	display_off_time::value = 1;
	display_brightness_regulator::value = 100;
	network_settings::sta_enabled = false;
	
	date_time::changed = true;	
	setTime(0);

	//network defaults
	String("ElektroBOX").toCharArray(network::ap_ssid, 40);
	network::ap_pass[0] = '\0';
	network::ap_ip = IPAddress(7, 7, 7, 7);
	network::ap_gateway = IPAddress(7, 7, 7, 7);
	network::ap_subnet = IPAddress(255, 255, 255, 0);
	network::sta_ssid[0] = '\0';
	network::sta_pass[0] = '\0';
	network::sta_ip = IPAddress(192, 168, 1, 227);
	network::sta_gateway = IPAddress(192, 168, 1, 1);
	network::sta_subnet = IPAddress(255, 255, 255, 0);

	LittleFS.format();
	if (!reset) {
		EEPROM.put(FV_HEADER_MAGIC, flash_header_magic);
	} else {
		network_settings::applied = true;
		save_settings();
	}
	
	charge_log::load();
}

void save_settings()
{
	DEBUG_MSG("Saving electrobox settings...\n");
	
	language_selector::save();
	weekly_schedule::save();
	time_and_kwt_for_session::save();
	current_regulator::save();
	//ground_check::save();
	adaptive_mode::save();
	//limit_kwt_for_session::save();
	//limit_by_time::save();
	display_off_time::save();
	display_brightness_regulator::save();
	network_settings::save();
	statistics::save();
	sync_time::save();
	
	EEPROM.commit();
	need_commit = false;
	
	if (network_settings::applied) {
		DEBUG_MSG("Network settings applied, restarting...\n");
		delay(500);
		ESP.restart();
	}
}

void load_settings()
{
	language_selector::load();
	weekly_schedule::load();
	time_and_kwt_for_session::load();
	current_regulator::load();
	//ground_check::load();
	adaptive_mode::load();
	//limit_kwt_for_session::load();
	//limit_by_time::load();
	display_off_time::load();
	display_brightness_regulator::load();
	network_settings::load();
	charge_log::load();
	statistics::load();
	sync_time::load();
}

void electrobox_ui_setup()
{
	uint64_t magic;
	
	LittleFS.begin();
	EEPROM.begin(FV_SIZE);
	EEPROM.get(FV_HEADER_MAGIC, magic);
	
	if (flash_header_magic == magic) {
		load_settings();
	} else {
		factory_init();
	}
		
	//Initialize values once
	//WiFi.mode(WIFI_AP);
	network::begin(network_settings::sta_enabled);
	
	/*ArduinoOTA.onStart([]() {
		Serial.println("Start");
	});
	
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
  
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
  
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	  });
	
	ArduinoOTA.begin();*/
	
	ui::init_server();
}

void electrobox_ui_update()
{
	static unsigned long old_millis = 0;
	
	static int statistics_update_interval = 60 * 5; //every 5 minutes
	
	if (millis() >= old_millis + 1000) { //send messages every second
		ui::ws->cleanupClients();
				
		//if (step++ >= 15) { //run every 15 seconds
		charge_log::update(); //WARNING, flash write cycles warning
		if (charge_status::value == charge_status::C_CHARGING) //save statistics every 5 minutes during charging
			if (!statistics_update_interval--) {
				DEBUG_MSG("Updating statistics...\n");
				statistics::update(now(), time_and_kwt_for_session::kwt - time_and_kwt_for_session::previous_kwt);
				time_and_kwt_for_session::previous_kwt = time_and_kwt_for_session::kwt;
				
				statistics_update_interval = 60 * 5;
			}
			
			//charge_status::update((charge_status::value + 1) % 8); //to test log
		//	step = 0;
		//};
		
		if (need_commit) {
			factory_reset::applier_enable();
			save_settings();
		}
		
		weekly_schedule::trigger();
		date_time::update();
		runtime_messages.flush();
		
		old_millis += 1000;
	}
	
	//static unsigned long connect_millis = 0;
	if (number_of_interface_loaders > 0 /*&& (millis() >= connect_millis + 250)*/) {
		for (int i = 0; i < MAX_ELBOX_CLIENTS; i++) {
			elbox_client &cl = elbox_clients[i];
			if (cl.present && !cl.interface_loaded)
				cl.build_interface();
		}
		//connect_millis += 250;
	}
	
	ui::dns_server.processNextRequest();
	//ArduinoOTA.handle();
}
