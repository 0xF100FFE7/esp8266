#include "interface.h"

namespace ui
{
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
	
	String add_leading_zero(int val)
	{
		return val < 10 ? String(0) + val : String(val);
	}
	
	String hm_to_str(hm_t t)
	{
		hm_t hours = t / 100, minutes = t % 60;
		return (hours < 10 ? String("0") + hours : String(hours)) + "\\:" + (minutes < 10 ? String("0") + minutes : String(minutes));
	}
	
	hm_t convert_to_hm(int h, int m)
	{
		return (h * 100 + m % 60);
	}
	
	packet packet::operator + (const packet &b)
	{
		return this->buffer + b.buffer;
	}
	
	packet& packet::operator += (const packet &b)
	{
		this->buffer += b.buffer;
		return *this;
	}

	packet packet::operator + (const String &b)
	{
		return this->buffer + b;
	}
	
	packet& packet::operator += (const String &b)
	{
		this->buffer += b;
		return *this;
	}
	
	void packet::send(client_id_t id)
	{
		ws->text(id, buffer);
		buffer = "";
	}
	
	void packet::send_all()
	{
		ws->textAll(buffer);
		buffer = "";
	}
	
	String packet::next_value(size_t &begin)
	{
		size_t old_begin = begin, end = begin;
		for (; end < buffer.length(); end++)
			if (buffer[end] == ':' && buffer[end-1] != '\\') break;
		
		begin = end+1;
		return buffer.substring(old_begin, end);
	}

	void packet::get(client_id_t sender)
	{
		size_t begin = 0;
		String msg;
		
		#define NEXT_MSG (msg = next_value(begin))
		while (NEXT_MSG != "") {
			interactive_element &e = *interactive_element::address_from_id(msg);
			switch (e.type) {
			case E_RADIO:
				e.callback.radio(static_cast<radio&>(e), sender);
				break;
				
			case E_BUTTON:
				e.callback.button(static_cast<button&>(e), sender);
				break;

			case E_SWITCHER:
				e.callback.switcher(static_cast<switcher&>(e), sender);
				break;
				
			case E_APPLIER:
				e.callback.applier(static_cast<applier&>(e), sender);
				break;

			case E_FIELD:
				e.callback.field(static_cast<field&>(e), unescape(NEXT_MSG), sender);
				break;
			
			case E_DATE_FIELD:
				e.callback.date_field(static_cast<date_field&>(e), unescape(NEXT_MSG), sender);
				break;
			
			case E_TIME_FIELD:
				e.callback.time_field(static_cast<time_field&>(e), unescape(NEXT_MSG), sender);
				break;

			case E_RANGE:
				e.callback.range(static_cast<range&>(e), NEXT_MSG.toInt(), sender);
				break;

			case GET_TIME:
				e.callback.time(static_cast<time&>(e), NEXT_MSG.toInt());
				break;

			case FRAME_CONFIRMED:
				e.callback.frame(sender);
				break;

			default:
				NEXT_MSG;
				//maybe insert some debug messages here? TODO
				break;
			}
		}
	}
	
	packet::packet(String buf)
	{
		buffer = buf;
	}
	
	//attributes implementation
	attributes none;
	
	attributes& attributes::operator , (attributes b)
	{
		this->buffer += b.buffer;
		return *this;
	}
		
	attributes::attributes(String buf)
	{
		buffer = buf;
	}
	
	namespace attr {
		panel_t		panel;
		selected_t	selected;
		tab_align_t	tab_align;
		fill_t		fill;
		dir_t		direction;
		wrap_t		wrap;
		align_t		align;
		width_t		width;
		height_t	height;
		min_t		min;
		max_t		max;
		text_t		text;
		esize_t		size;
		value_t		value;
		background_t	background;
		textcolor_t	textcolor;
		display_t	display;
		disabled_t	disabled;
		
		attributes panel_t::operator = (String b) {		return String("panel:") + b + ":";									}
		attributes selected_t::operator = (bool b) {		return String("selected:") + (b ? "true:" : "false:");							}
		attributes tab_align_t::operator = (e_align b) {	return String("tab_align:") + (b == ALIGN_CENTER ? "center:" : (b == ALIGN_LEFT ? "left:" : "right:"));	}
		attributes fill_t::operator = (bool b) {		return String("fill:") + (b ? "true:" : "false:");							}
		attributes dir_t::operator = (e_dir b) {		return String("dir:") + (b == DIR_H ? "h:" : "v:");							}
		attributes wrap_t::operator = (bool b) {		return String("wrap:") + (b ? "true:" : "false:");							}
		attributes align_t::operator = (e_align b) {		return String("align:") + (b == ALIGN_CENTER ? "center:" : (b == ALIGN_LEFT ? "left:" : "right:"));	}		
		attributes width_t::operator = (String b) {		return String("w:") + b + ":";										}
		attributes width_t::operator = (unsigned int b) {	return String("w:") + b + ":";										}
		attributes height_t::operator = (String b) {		return String("h:") + b + ":";										}
		attributes height_t::operator = (unsigned int b) {	return String("h:") + b + ":";										}
		attributes min_t::operator = (int b) {			return String("min:") + b + ":";									}
		attributes max_t::operator = (int b) {			return String("max:") + b + ":";									}
		attributes text_t::operator = (String b) {		return String("text:") + b + ":";									}
		attributes esize_t::operator = (unsigned int b) {	return String("size:") + b + ":";									}
		attributes value_t::operator = (String b) {		return String("value:") + b + ":";									}
		attributes value_t::operator = (int b) {		return String("value:") + b + ":";									}
		attributes background_t::operator = (String b) {	return String("backcolor:") + b + ":";									}
		attributes textcolor_t::operator = (String b) {		return String("textcolor:") + b + ":";									}
		attributes display_t::operator = (bool b) {		return String("display:") + (b ? "true:" : "false:");							}
		attributes disabled_t::operator = (bool b) {		return String("disabled:") + (b ? "true:" : "false:");							}
	}
	
	//element implementation
	struct root root;
	
	size_t element::first_element_address_in_memory = (size_t)-1;
	
	void element::set_offset()
	{
		if (first_element_address_in_memory > (size_t)this)
			first_element_address_in_memory = (size_t)this;
	}
		
	packet element::pack(struct root, attributes att)
	{
		return packet("type:") + String(this->type) + ":id:" + get_id(this) + ":parent:main:" + att.buffer;
	}
	
	packet element::pack()
	{
		return packet("type:") + String(this->type) + ":id:" + get_id(this) + ":";
	}
		
	packet element::pack(attributes att)
	{
		return packet("id:") + get_id(this) + ":" + att.buffer;
	}
		
	element::element()
	{
		set_offset();
	}

	interactive_element* interactive_element::address_from_id(String id)
	{
		return (interactive_element*)(id.toInt() * sizeof(size_t) + first_element_address_in_memory);
	}	

	//widgets implementation (Not ready yet)
	tab::tab() {type = E_TAB;}
	text::text() {type = E_TEXT;}
	box::box() {type = E_BOX;}
	dialog::dialog() {type = E_DIALOG;}
	
	radio::radio(void (*callback)(struct radio &id, client_id_t sender))
	{
		type = E_RADIO;
		this->callback.radio = callback;
	}
	
	attributes radio::get(bool value)
	{
		return attr::background = value ? "green" : "#666";
	}
	
	button::button(void (*callback)(struct button &id, client_id_t sender))
	{
		type = E_BUTTON;
		this->callback.button = callback;
	}
	
	attributes switcher::get(bool value)
	{
		return value ? (attr::background = "green", attr::text = "&#10003;") : (attr::background = "red", attr::text = "&#10008;");
	}
		
	packet switcher::turn(bool &value)
	{
		value = !value;
		return pack(get(value));
	}
		
	switcher::switcher(void (*callback)(struct switcher &id, uint32_t sender))
	{
		type = E_SWITCHER;
		this->callback.switcher = callback;
	}
	
	attributes applier::get()
	{
		return attr::disabled = avail ? false : true;
	}
		
	packet applier::turn(bool value)
	{
		avail = value;
		return pack(get());
	}
		
	applier::applier(void (*callback)(struct applier &id, uint32_t sender))
	{
		type = E_APPLIER;
		this->callback.applier = callback;
	}
	
	field::field(void (*callback)(struct field &id, String value, client_id_t sender))
	{
		type = E_FIELD;
		this->callback.field = callback;
	}
	
	time_t date_field::parse(String value)
	{
		tmElements_t tm;
		size_t res_time;
		
		tm.Year = CalendarYrToTm(value.substring(0, 4).toInt());
		tm.Month = value.substring(5, 7).toInt();
		tm.Day = value.substring(8, 10).toInt();
		
		res_time = now();
		tm.Hour = hour(res_time);
		tm.Minute = minute(res_time);
		tm.Second = second(res_time);
		
		res_time = makeTime(tm);
		
		return res_time;
	}
	
	String date_field::formatted(time_t t)
	{
		return String(year(t)) + "-" + add_leading_zero(month(t)) + "-" + add_leading_zero(day(t));
	}
	
	date_field::date_field(void (*callback)(struct date_field &id, String value, client_id_t sender))
	{
		type = E_DATE_FIELD;
		this->callback.date_field = callback;
	}
	
	time_t time_field::parse(String value)
	{
		return previousMidnight(now()) + value.substring(0, 2).toInt() * SECS_PER_HOUR + value.substring(3, 5).toInt() * SECS_PER_MIN;
	}
	
	String time_field::formatted(time_t t)
	{
		return String() + add_leading_zero(hour(t)) + "\\:" + add_leading_zero(minute(t));
	}
	
	time_field::time_field(void (*callback)(struct time_field &id, String value, client_id_t sender))
	{
		type = E_TIME_FIELD;
		this->callback.time_field = callback;
	}
	
	packet range::slide(int &value, int new_value)
	{
		value = new_value;
		return this->pack((attr::value = value));
	}
		
	range::range(void (*callback)(struct range &id, int value, client_id_t sender))
	{
		type = E_RANGE;
		this->callback.range = callback;
	}

	time::time(void (*callback)(struct time &id, time_t t))
	{
		type = GET_TIME;
		this->callback.time = callback;	
	}
	
	frame::frame(void (*callback)(client_id_t sender))
	{
		type = FRAME;
		this->callback.frame = callback;
	}

	//client implementation
	int number_of_interface_loaders = 0;
	int number_of_clients = 0;
	client clients[MAX_UI_CLIENTS];

	void client::confirm_frame(client_id_t sender)
	{
		client &cl = *find_client(sender);
		cl.frame_number++; //goto next frame and send it
		cl.waiting_for_frame_confirmation = false;
	}

	void client::build_interface()
	{
		if (!waiting_for_frame_confirmation)
			waiting_for_frame_confirmation = true;
		else
			return;
			
		if (interface(*this, frame_number)) {
			interface_loaded = true;
			number_of_interface_loaders--;
			packet.buffer.~String(); //invalidate buffer
		} else if (!interface_loaded) {
			(packet += current_frame.pack()).send(id); //flush messages to this client
		}
	}

	void client::cleanup()
	{
		interface_loaded = false;
		waiting_for_frame_confirmation = false;
		frame_number = 0;
	}

	client::client() : current_frame(client::confirm_frame)
	{
		cleanup();
	};

	void add_client(client_id_t id)
	{
		for (int i = 0; i < MAX_UI_CLIENTS; i++) {
			client &cl = clients[i];
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

	void del_client(client_id_t id)
	{
		for (int i = 0; i < MAX_UI_CLIENTS; i++) {
			client &cl = clients[i];
			
			if (cl.id == id) {
				if (cl.packet.buffer)
					cl.packet.buffer.~String(); //invalidate buffer
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

	client *find_client(client_id_t id)
	{
		for (int i = 0; i < MAX_UI_CLIENTS; i++) {
			client &cl = clients[i];
				
			if (cl.present && cl.id == id)
				return &cl;
		}
		return nullptr;
	}

	//web server implementation
	const char* basicAuthUsername;
	const char* basicAuthPassword;
	bool basicAuth;
	
	DNSServer dns_server;
	AsyncWebServer* server;
	AsyncWebSocket* ws;

	void web_event(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
	{
		switch (type) {
		case WS_EVT_CONNECT:
			add_client(client->id());
			break;
			
		case WS_EVT_DISCONNECT:
			del_client(client->id());
			break;
			
		case WS_EVT_DATA: {
			packet in;
			in.buffer.reserve(len + 1);
			for (size_t i = 0; i < len; i++)
				in.buffer += (char)data[i];
			in.get(client->id());
			break;
		}
		
		default:
			break;
		}
	}
	
	void start_web_server(const char *username, const char *password)
	{
		dns_server.setErrorReplyCode(DNSReplyCode::NoError);
		dns_server.start(53, "*", WiFi.softAPIP());

		basicAuthUsername = username;
		basicAuthPassword = password;

		basicAuth = (username != nullptr && password != nullptr) ? true : false;

		server = new AsyncWebServer(80);
		ws = new AsyncWebSocket("/ws");

		ws->onEvent(web_event);
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
	
	void interface_setup()
	{
		LittleFS.begin();
		network::begin();
		start_web_server();
	}
	
	void interface_loop()
	{
		if (number_of_interface_loaders > 0) {
			for (int i = 0; i < MAX_UI_CLIENTS; i++) {
				client &cl = clients[i];
				if (cl.present && !cl.interface_loaded)
					cl.build_interface();
			}
		}
		
		dns_server.processNextRequest();
		network::loop();
	}
}
