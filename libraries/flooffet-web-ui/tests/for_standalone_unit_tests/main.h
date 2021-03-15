#ifndef FLAME_UI_GUARD
#define FLAME_UI_GUARD

#include <string>
using namespace std;

namespace flame_ui
{
	uintptr_t str_to_ptr(string);
	string ptr_to_str(uintptr_t);
	
	struct end_of_message_t {}; //eom

	enum element_type : uint8_t {
		E_HEADER,
		E_TAB,
		
		E_RESERVED,
		
		E_BOX,
		E_TEXT,
		E_BUTTON,
		E_CHECKBOX,
		E_FIELD,
		E_RANGE,
	};

	enum button_state : uint8_t {
		E_BUTTON_DOWN,
		E_BUTTON_UP,
	};

	enum checkbox_state : uint8_t {
		E_CHECKBOX_ENABLED,
		E_CHECKBOX_DISABLED,
	};

	/*struct button {
		void (*callback)(button_state state);
	};

	struct checkbox {
		enum checkbox_state state;
		void (*callback)(checkbox_state state);
	};

	struct field {
		string value;
		void (*callback)(string value);
	};*/
	
	struct messages {
		string buffer;

		messages& operator , (string);
		messages& operator , (element_type);
		messages& operator = (string);
		messages& operator + (element_type);
		messages& operator | (string);
		string get();
		void clear();
	};
	
	union element {
		void (*button)(int state);
		void (*field)(int state);
		
		void (*generic)();
	};
	
	string operator + (element&, string);
	string operator + (string, element&);
	
	struct web_server {

	};
}

#endif
