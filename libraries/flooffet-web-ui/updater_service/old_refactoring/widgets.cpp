#include "widgets.h"

enum element_type : uint8_t {
	//generic element types compatible with web elements
	E_HEADER,
	E_TAB,
	E_BOX,
	E_TEXT,
	E_BUTTON,
	E_SWITCHER,
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

//root is not an element, it's just a root lfmao
struct root {};

struct alignas(sizeof(size_t)) element {
	static size_t first_element_address_in_memory;
	
	element_type type;

	void set_offset()
	{
		if (first_element_address_in_memory > (size_t)this)
			first_element_address_in_memory = (size_t)this;
	}

	template<typename T>
	string get_id(T *el) {
		return to_string(((size_t)el - first_element_address_in_memory) / sizeof(size_t));
	}
	
	template<typename T>
	packet pack(T &parent, attributes att)
	{
		return packet("type:") + to_string(this->type) + ":id:" + get_id(this) + ":parent:" + get_id(&parent) + ":" + att.buffer;
	}
	
	packet pack(struct root, attributes att)
	{
		return packet("type:") + to_string(this->type) + ":id:" + get_id(this) + ":parent:main:" + att.buffer;
	}
	
	packet pack(attributes att)
	{
		return packet("id:") + get_id(this) + ":" + att.buffer;
	}
	
	element()
	{
		set_offset();
	}
};

struct interactive {
	union {
		void (*field)(struct field &id, string value, uint32_t sender);
		void (*ifield)(struct ifield &id, string value, uint32_t sender);
		void (*tfield)(struct tfield &id, string value, uint32_t sender);
		void (*radio)(struct radio &id, string value, uint32_t sender);
		void (*button)(struct button &id, uint32_t sender);
		void (*switcher)(struct switcher &id, uint32_t sender);
		void (*range)(struct range &id, int value, uint32_t sender);
		
		void (*time)(time_t t);
		void (*frame)(uint32_t sender);
			
		//void *callback;
	} callback;
};

struct interactive_element : element, interactive {
	static interactive_element* address_from_id(string id)
	{
		cout << id << endl;
		return (interactive_element*)(stoull(id) * sizeof(size_t) + first_element_address_in_memory);
	}	
};

size_t element::first_element_address_in_memory = (size_t)-1;

struct tab : element {
	tab() {type = E_TAB;}
};

struct text : element {
	text() {type = E_TEXT;}
};

struct box : element {
	box() {type = E_BOX;};
};
struct field : interactive_element {};
struct ifield : interactive_element {};
struct tfield : interactive_element {};
struct radio : interactive_element {};
struct button : interactive_element {};
struct switcher : interactive_element {
	bool enabled = false;
	
	attributes get()
	{
		return enabled ? (attributes(background = "green"), attr::text = "&#10003;") : (attributes(background = "red"), attr::text = "&#10008;");
	}
	
	packet turn()
	{
		enabled = !enabled;
		return packet(get().buffer);
	}
	
	switcher(void (*callback)(struct switcher &id, uint32_t sender))
	{
		type = E_SWITCHER;
		enabled = false;
		this->callback.switcher = callback;
	}
};

struct range : interactive_element {};
struct time : interactive_element {};
struct dialog : element {};
struct frame : interactive_element {};
