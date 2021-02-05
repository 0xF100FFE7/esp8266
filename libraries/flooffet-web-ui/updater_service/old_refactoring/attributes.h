#ifndef UI_ATTRIBUTES_GUARD
#define UI_ATTRIBUTES_GUARD

#include "Arduino.h"

namespace ui {
	struct attributes {
		String buffer;
		
		attributes& operator , (attributes b);
		attributes(String buf = "");
	} extern none;
	
	namespace attr {
		enum e_dir {
			DIR_H, DIR_HORIZONTAL = 0,
			DIR_V, DIR_VERTICAL = 1
		};

		enum e_align {
			ALIGN_UP, ALIGN_LEFT = 0,
			ALIGN_DOWN, ALIGN_RIGHT = 1,
			ALIGN_CENTER
		};
		
		struct panel_t {attributes operator = (String b);} 		extern panel;
		struct selected_t {attributes operator = (bool b);} 		extern selected;
		struct tab_align_t {attributes operator = (e_align b);}		extern tab_align;

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
	
		struct text_t {attributes operator = (String b);}		extern text;
		struct esize_t {attributes operator = (unsigned int b);}	extern size;
		struct value_t {attributes operator = (String b);}		extern value;
		struct background_t {attributes operator = (String b);}		extern background;
		struct textcolor_t {attributes operator = (String b);}		extern textcolor;
		struct display_t {attributes operator = (bool b);}		extern display;
		struct disabled_t {attributes operator = (bool b);}		extern disabled;
	}
}

#endif
