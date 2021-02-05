#include "flame_ui.h"
/* General todo list:
 * Translate every comment to english
 * Create new message type responsible for storing and using
 * 	specific readable/writeable data by indexes, so we do not need to resend
 * 	huge amount of same data every time (like languages, texts, logs, styles, etc)
 * Create new message type responsible for deleting specific ui elements //not actual
 * Shorten message adresses. //done
 * Incapsulate JS functions into some class
 */

namespace ui
{
	String hours_and_minutes_to_str(int hours, int minutes) {
		return (hours < 10 ? String("0") + hours : String(hours)) + "\\:" + (minutes < 10 ? String("0") + minutes : String(minutes));
	}
	
	String escape(String str)
	{
		str.replace(":", "\\:");
		return str;
	}

	String unescape(String str)
	{
		str.replace("\\:", ":");
		return str;
	}
	
	/*Переводить строки з базою 16 у вказівники
	* Однак замість hex форматованих строк викоистовується чисто буквенна інтерпретація:
	* a[0x] b[0x1] c[0x2] d[0x3] e[0x4] f[0x5] g[0x6] h[0x7] i[0x8]
	* j[0x9] k[0xA] l[0xB] m[0xC] n[0xD] o[0xE] p[0xF]
	* всі букви тілки в нижньому регістрі
	*/
	uintptr_t str_to_ptr(String str)
	{
		size_t idx;
		uintptr_t ptr = 0;
		for(idx = 0; idx < str.length(); ++idx) {
			ptr <<= 4;
			ptr |= str[idx] - 'a';
		}
		return ptr;
	}

	//new approach
	/*uintptr_t str_to_ptr(String str)
	{
		uintptr_t ptr = 0;
		for(size_t idx = str.length(); idx > 0; --idx) {
			ptr <<= 4;
			ptr |= str[idx - 1] - 'a';
		}
		return ptr;
	}*/

	//Переводить вказівникі в строки з базою 16 в виключно буквенній інтерпретації.
	String ptr_to_str(uintptr_t ptr)
	{
		size_t idx = (sizeof(uintptr_t) * 2 - 1);
		String str((uintptr_t)1 << idx, BIN); //Хак для ініціалізації
		for(; idx != (size_t)-1; --idx) {
			str[idx] = 'a' + (ptr & 0xF);
			ptr >>= 4;
		}
		return "_" + str;
	}
	
	//new approach
	/*String ptr_to_str(uintptr_t ptr)
	{
		String str;
		do {
			str += 'a' + (ptr & 0xF);
			ptr >>= 4;
		} while (ptr > 0);
		return str;
	}*/
	
	element::element(element_type t, void *cb)
	{
		type = t;
		callback = cb;
	}

	messages messages::operator + (messages b)
	{
		return messages(this->buffer + b.buffer);
	}
	
	messages& messages::operator << (messages b)
	{
		this->buffer += b.buffer;
		return *this;
	}
	
	messages& messages::operator << (String b)
	{
		this->buffer += b;
		return *this;
	}

	messages& messages::operator << (void* b)
	{
		this->buffer += PTR_TO_STR(b);
		return *this;
	}
		
	messages& messages::operator << (int b)
	{
		this->buffer += b;
		return *this;
	}
		
	messages& messages::operator << (float b)
	{
		this->buffer += b;
		return *this;
	}
		
	messages::messages(String buf)
	{
		buffer = buf;
	}
	
	String messages::next()
	{
		while (true) {
			if (end >= buffer.length()) {
				return "";
			} else if (buffer[end] == ':') {
				end++;
				break;
			}
			break;
		}

		begin = end;
		
		for (; end < buffer.length(); end++)
			if (buffer[end] == ':' && buffer[end-1] != '\\') break;
		
		return buffer.substring(begin, end);
	}

	void messages::parse(uint32_t sender)
	{
		begin = end = 0;
		String msg;
		element *e;
		
		#define NEXT_MSG (msg = next())
		while (NEXT_MSG != "") {
			/*if (msg == "SET_TIME") {
				setTime(NEXT_MSG.toInt());
				break;
			}*/
			
			e = (element*)STR_TO_PTR(msg);
			switch (e->type) {
			case E_BUTTON:
				e->button(e, sender);
				break;
				
			case E_FIELD:
			case E_TIME_FIELD:
				NEXT_MSG;
				unescape(msg);
				e->field(e, msg);
				break;
				
			case E_RANGE:
				e->range(e, NEXT_MSG.toInt());
				break;
				
			case GET_TIME:
				e->time(NEXT_MSG.toInt());
				break;
				
			case FRAME_CONFIRMED:
				e->frame(e);
				break;
				
			default:
				NEXT_MSG;
				break;
			}
		}
	}
	
	void messages::send()
	{
		ws->textAll(buffer);
	}
	
	void messages::flush()
	{
		ws->textAll(buffer);
		buffer = "";
	}
	
	void messages::flush(uint32_t id)
	{
		ws->text(id, buffer);
		buffer = "";
	}
	
	void messages::clear()
	{
		buffer = "";
	}
	
	const char* basicAuthUsername;
	const char* basicAuthPassword;
	bool basicAuth;
	
	DNSServer dns_server;
	AsyncWebServer* server;
	AsyncWebSocket* ws;
	
	void ui_event(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
	{
		switch (type) {
		case WS_EVT_CONNECT:
			on_connect(client->id());
			break;
		case WS_EVT_DISCONNECT:
			on_disconnect(client->id());
			break;
		case WS_EVT_DATA: {
			messages in;
			in.buffer.reserve(len + 1);
			for (size_t i = 0; i < len; i++)
				in.buffer += (char)data[i];
			in.parse(client->id());
			break;
		}
		default:
			break;
		}
	}
	
	void init_server(const char *username, const char *password)
	{
		dns_server.setErrorReplyCode(DNSReplyCode::NoError);
		dns_server.start(53, "*", WiFi.softAPIP());

		basicAuthUsername = username;
		basicAuthPassword = password;

		basicAuth = (username != nullptr && password != nullptr) ? true : false;

		server = new AsyncWebServer(80);
		ws = new AsyncWebSocket("/ws");

		ws->onEvent(ui_event);
		server->addHandler(ws);

		if (basicAuth && WS_AUTHENTICATION)
			ws->setAuthentication(username, password);

		server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
			if (basicAuth && !request->authenticate(basicAuthUsername, basicAuthPassword))
				return request->requestAuthentication();

			AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML_INDEX);
			request->send(response);
		});


		server->on("/js/main.js", HTTP_GET, [](AsyncWebServerRequest *request) {
			if (basicAuth && !request->authenticate(basicAuthUsername, basicAuthPassword))
				return request->requestAuthentication();

			AsyncWebServerResponse *response = request->beginResponse_P(200, "application/javascript", JS_MAIN);
			request->send(response);
		});

		server->on("/css/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
			if (basicAuth && !request->authenticate(basicAuthUsername, basicAuthPassword))
				return request->requestAuthentication();

			AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", CSS_MAIN);
			request->send(response);
		});

		//capative portal
		server->onNotFound([](AsyncWebServerRequest *request) {
			AsyncResponseStream *response = request->beginResponseStream("text/plain");
			response->setCode(302);
			response->addHeader("Location", String("http://") + WiFi.softAPIP().toString());
			request->send(response);
		});
		//server->onNotFound([](AsyncWebServerRequest *request) { request->send(404); });

		server->begin();
	}
};


/*flame::ui::button btn;

void some_action(void *id)
{
  static int i = 0;
  flame::web_ui.stream << "type:" << flame::ui::E_TEXT << ",id:FUCKING_SUCCESS,parent:body,text:" << i++;
  flame::web_ui.flush_messages();
}

void setup()
{
  WiFi.mode(WIFI_AP);
  _esp8266WifiModuleApReconnect();
  btn.callback = some_action;
  flame::web_ui.connect << "type:" << flame::ui::E_BUTTON << ",id:" << &btn << ",parent:body,text:DIS_BUTTON";
  //F("type:0,id:_h4,parent:body,text:Electrobox UI\ntype:1,id:_t0,parent:body,panel:tabs,tab_dir:h,selected:true,display:true,text:Головна\ntype:1,id:_t1,parent:body,panel:tabs,text:Настройки\ntype:1,id:_t2,parent:body,panel:tabs,dir:h,text:Таймер\ntype:1,id:_t3,parent:body,panel:tabs,tab_align:right,text:Куча\ntype:3,id:_hds5,parent:_t2,dir:h,text:Тижневий розклад\ntype:3,id:_c1,parent:_hds5,dir:v\ntype:3,id:_c2,parent:_hds5,dir:v\ntype:3,id:_c3,parent:_hds5,dir:v\ntype:3,id:_c4,parent:_hds5,dir:v\ntype:4,id:_c1t0,parent:_c1,text:Дні тижнів\ntype:4,id:_c1t1,parent:_c1,text:Понеділок\ntype:4,id:_c1t2,parent:_c1,text:Вівторок\ntype:4,id:_c1t3,parent:_c1,text:Середа\ntype:4,id:_c1t4,parent:_c1,text:Четверг\ntype:4,id:_c1t5,parent:_c1,text:П'ятниця\ntype:4,id:_c1t6,parent:_c1,text:Суботта\ntype:4,id:_c1t7,parent:_c1,text:Неділя\ntype:4,id:_c2t0,parent:_c2,text:Старт зарядки\ntype:7,id:_c2t1,parent:_c2\ntype:7,id:_c2t2,parent:_c2\ntype:7,id:_c2t3,parent:_c2\ntype:7,id:_c2t4,parent:_c2\ntype:7,id:_c2t5,parent:_c2\ntype:7,id:_c2t6,parent:_c2\ntype:7,id:_c2t7,parent:_c2\ntype:4,id:_c3t0,parent:_c3,text:Стоп зарядки\ntype:7,id:_c3t1,parent:_c3\ntype:7,id:_c3t2,parent:_c3\ntype:7,id:_c3t3,parent:_c3\ntype:7,id:_c3t4,parent:_c3\ntype:7,id:_c3t5,parent:_c3\ntype:7,id:_c3t6,parent:_c3\ntype:7,id:_c3t7,parent:_c3\ntype:5,id:_c4t0,parent:_c4,text:Увімкнути\ntype:6,id:_c4t1,parent:_c4\ntype:6,id:_c4t2,parent:_c4\ntype:6,id:_c4t3,parent:_c4\ntype:6,id:_c4t4,parent:_c4\ntype:6,id:_c4t5,parent:_c4\ntype:6,id:_c4t6,parent:_c4\ntype:6,id:_c4t7,parent:_c4\ntype:3,id:_hds,parent:_t1,text:noob,dir:h\ntype:3,id:_hds2,parent:_hds,dir:h\ntype:5,id:_g33,parent:_hds2,text:Hello button\ntype:5,id:_g34,parent:_hds2,text:Hello button\ntype:3,id:_hds3,parent:_hds,dir:v\ntype:4,id:_h,parent:_t0,text:Hello world\ntype:5,id:_g,parent:_t0,text:Hello button\ntype:8,id:_r1,parent:_t0,dir:v\ntype:3,id:678,parent:_t1,text:Настройки WIFI\ntype:3,id:648,parent:678,dir:h\ntype:4,id:6484,parent:648,text:4343\ntype:4,id:6484,parent:648,text:43433434344334\ntype:7,id:64854,parent:648");
  flame::init_server();
}*/
