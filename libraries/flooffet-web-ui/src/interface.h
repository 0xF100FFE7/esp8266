#ifndef UI_INTERFACE_HEADER
#define UI_INTERFACE_HEADER

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

#include "Arduino.h"
#include "network.h"

//#define private public
//#define protected public
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <TimeLib.h>

#include "web_javascript.h"
#include "web_styles.h"
#include "web_page.h"

#define WS_AUTHENTICATION false

namespace ui {
	typedef uint32_t client_id_t;

	typedef uint32_t hm_t;
	extern String escape(String);
	extern String unescape(String);

	extern String add_leading_zero(int);
	extern hm_t convert_to_hm(int, int);
	extern String hm_to_str(hm_t);

	
	//packet declaration
	struct packet {
		String buffer;
		
		packet operator + (const packet&);
		packet& operator += (const packet&);
		packet operator + (const String&);
		packet& operator += (const String&);
		
		//implement these somewhere else
		void send(client_id_t);
		void send_all();
		void get(client_id_t);
		
		packet(String buf = "");
		
	private:
		String next_value(size_t &);
	};
	
	//attributes declaration
	struct attributes {
		String buffer;
		
		attributes& operator , (attributes b);
		attributes(String buf = "");
	} extern none;
	
	enum e_dir {
		DIR_H, DIR_HORIZONTAL = 0,
		DIR_V, DIR_VERTICAL = 1
	};

	enum e_align {
		ALIGN_UP, ALIGN_LEFT = 0,
		ALIGN_DOWN, ALIGN_RIGHT = 1,
		ALIGN_CENTER
	};
		
	namespace attr {
		struct panel_t {attributes operator = (String b);} 		extern panel;
		struct selected_t {attributes operator = (bool b);} 		extern selected;
		struct tab_align_t {attributes operator = (e_align b);}		extern tab_align;
		struct fill_t {attributes operator = (bool b);}			extern fill;

		struct dir_t {attributes operator = (e_dir b);}			extern direction;
		struct wrap_t {attributes operator = (bool b);}			extern wrap;
		struct align_t {attributes operator = (e_align b);}		extern align;
	
		struct width_t {
			attributes operator = (String b);
			attributes operator = (unsigned int b);
		}								extern width;	
		struct height_t {
			attributes operator = (String b);
			attributes operator = (unsigned int b);
		}								extern height;
		
		struct min_t {attributes operator = (int b);}			extern min;
		struct max_t {attributes operator = (int b);}			extern max;
	
		struct text_t {attributes operator = (String b);}		extern text;
		struct esize_t {attributes operator = (unsigned int b);}	extern size;
		
		struct value_t {
			attributes operator = (String b);
			attributes operator = (int b);
		}								extern value;
		
		struct background_t {attributes operator = (String b);}		extern background;
		struct textcolor_t {attributes operator = (String b);}		extern textcolor;
		struct display_t {attributes operator = (bool b);}		extern display;
		struct disabled_t {attributes operator = (bool b);}		extern disabled;
	}
	
	//element declaration
	enum element_type : uint8_t {
		//generic element types compatible with web elements
		E_TAB,
		E_BOX,
		E_TEXT,
		E_RADIO,
		E_BUTTON,
		E_SWITCHER,
		E_APPLIER,
		E_FIELD,
		E_DATE_FIELD,
		E_TIME_FIELD,
		E_RANGE,
		E_DIALOG,
			
		GET_TIME,
		FRAME, FRAME_CONFIRMED = FRAME,
		
		//custom types only (not compatible with web elements).
		/*E_SWITCHER,
		E_RADIO,
		E_IFIELD,
		E_TFIELD,*/
			
		E_UNKNOWN
	};

	struct root {} extern root; //root element

	struct alignas(sizeof(size_t)) element {
		static size_t first_element_address_in_memory;
		element_type type;

		void set_offset();
		template<typename T> String get_id(T *el)
		{
			return String(((size_t)el - first_element_address_in_memory) / sizeof(size_t));
		}
		
		template<typename T> packet pack(T &parent)
		{
			return packet("type:") + String(this->type) + ":id:" + get_id(this) + ":parent:" + get_id(&parent) + ":";
		}
		
		template<typename T> packet pack(T &parent, attributes att)
		{
			return pack(parent) + att.buffer;
		}

		packet pack(struct root, attributes att);
		packet pack(attributes att);
		packet pack();
		element();
	};
	
	struct interactive {
		union {
			void (*field)(struct field &id, String value, client_id_t sender);
			//void (*ifield)(struct ifield &id, String value, client_id_t sender);
			void (*date_field)(struct date_field &id, String value, client_id_t sender);
			void (*time_field)(struct time_field &id, String value, client_id_t sender);
			void (*radio)(struct radio &id, client_id_t sender);
			void (*button)(struct button &id, client_id_t sender);
			void (*switcher)(struct switcher &id, client_id_t sender);
			void (*applier)(struct applier &id, client_id_t sender);
			void (*range)(struct range &id, int value, client_id_t sender);
			
			void (*time)(struct time &id, time_t t);
			void (*frame)(client_id_t sender);
		} callback;
	};
	
	struct interactive_element : element, interactive {
		static interactive_element* address_from_id(String id);	
	};
	
	//widgets declaration
	struct tab : element {tab();};
	struct text : element {text();};
	struct box : element {box();};
	struct dialog : element {dialog();};
	
	struct field : interactive_element {
		field(void (*callback)(struct field &id, String value, client_id_t sender));
	};

	struct date_field : interactive_element {
		time_t parse(String value);
		String formatted(time_t);
		date_field(void (*callback)(struct date_field &id, String value, client_id_t sender));
	};
	
	struct time_field : interactive_element {
		time_t parse(String value);
		String formatted(time_t);
		time_field(void (*callback)(struct time_field &id, String value, client_id_t sender));
	};
	
	struct radio : interactive_element {
		attributes get(bool);
		//packet turn(oldradio, newradio);		
		radio(void (*callback)(struct radio &id, client_id_t sender));
	};
	
	struct button : interactive_element {
		button(void (*callback)(struct button &id, client_id_t sender));
	};
	
	struct switcher : interactive_element {
		attributes get(bool);
		packet turn(bool &);		
		switcher(void (*callback)(struct switcher &id, client_id_t sender));
	};
	
	struct applier : interactive_element {
		bool avail;
		attributes get();
		packet turn(bool);
		applier(void (*callback)(struct applier &id, client_id_t sender));
	};
	
	struct range : interactive_element {
		packet slide(int &, int);		
		range(void (*callback)(struct range &id, int value, client_id_t sender));	
	};
	
	struct time : interactive_element {
		time(void (*callback)(struct time &id, time_t t));
	};
	
	struct frame : interactive_element {
		frame(void (*callback)(client_id_t sender));
	};
	
	//client declaration
	#define MAX_UI_CLIENTS DEFAULT_MAX_WS_CLIENTS
	
	extern int number_of_interface_loaders;
	extern int number_of_clients;

	struct client {
		bool present; //if client exists
		client_id_t id;
		
		bool interface_loaded;
		bool waiting_for_frame_confirmation;
		int frame_number;
		
		struct packet packet;
		frame current_frame;
		
		static void confirm_frame(client_id_t);
		void build_interface();
		void cleanup();
		
		client();
	} extern clients[MAX_UI_CLIENTS];

	extern void add_client(client_id_t id);
	extern void del_client(client_id_t id);
	extern client *find_client(client_id_t id);
	
	extern bool interface(client &, int); //This is completely for user implementation.
	extern void interface_setup();
	extern void interface_loop();
	
	//web server declaration
	extern const char* basicAuthUsername;
	extern const char* basicAuthPassword;
	extern bool basicAuth;
	
	extern DNSServer dns_server;
	extern AsyncWebServer* server;
	extern AsyncWebSocket* ws;
	
	extern void web_event(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType, void *, uint8_t *, size_t);
	extern void start_web_server(const char *username = nullptr, const char *password = nullptr);
}

using namespace ui;

#endif
