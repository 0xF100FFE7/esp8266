#ifndef FLAME_UI_GUARD
#define FLAME_UI_GUARD

#define WS_AUTHENTICATION false

#include "Arduino.h"

#include <DNSServer.h>
#include <TimeLib.h>
#include <ESPAsyncTCP.h>

#define private public        //This is the hack to access priviate variables of any class 
#define protected public     // This is the hack to access protected variables of any class  
#include <ESPAsyncWebServer.h>

#include "dataIndexHTML.h"
#include "dataStyleCSS.h"
#include "dataMainJS.h"

#define STR_TO_PTR(val) (void *)str_to_ptr(val)
#define PTR_TO_STR(ref) (String)ptr_to_str((uintptr_t)ref)

namespace ui {
	enum element_type : uint8_t {
		//generic element types compatible with web elements
		E_HEADER,
		E_TAB,
		E_BOX,
		E_TEXT,
		E_BUTTON,
		E_FIELD,
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

	struct element {
		element_type type;
		
		union {
			void (*button)(void *id, uint32_t sender);
			void (*field)(void *id, String value);
			void (*range)(void *id, int value);
			void (*time)(time_t t);
			void (*frame)(void *sender);
			
			void *callback;
		};

		element (element_type t = E_UNKNOWN, void *cb = nullptr);
	};
	
	struct messages {
		String buffer;
		size_t begin, end;
		
		messages operator + (messages);
		messages& operator << (messages);
		messages& operator << (String);
		messages& operator << (void *);
		messages& operator << (int);
		messages& operator << (float);
		
		String next();
		void parse(uint32_t);
		void send();
		void flush();
		void flush(uint32_t);
		void clear();
		
		messages(String buf = "");
	};
	
	//User specified ui helpers(wrappers)
	/*struct tab {
		element_type type = E_TAB;
	};

	struct text {
		element_type type = E_TEXT;
	};

	struct box {
		element_type type = E_BOX;
	};

	struct button {
		element_type type = E_BUTTON;
		void (*callback)(void *id);
	};

	struct field {
		element_type type = E_FIELD;
		void (*callback)(void *id, String value);
	};

	struct range {
		element_type type = E_RANGE;
		void (*range)(void *id, int value);
	};*/
	
	extern const char* basicAuthUsername;
	extern const char* basicAuthPassword;
	extern bool basicAuth;
	
	extern DNSServer dns_server;
	extern AsyncWebServer* server;
	extern AsyncWebSocket* ws;
	
	extern String hours_and_minutes_to_str(int hours, int minutes);
	extern String escape(String);
	extern String unescape(String);
	extern uintptr_t str_to_ptr(String);
	extern String ptr_to_str(uintptr_t);
	extern void on_connect(uint32_t);
	extern void on_disconnect(uint32_t);
	extern void web_event(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType, void *, uint8_t *, size_t);
	extern void init_server(const char *username = nullptr, const char *password = nullptr);
}
#endif
