#include "attributes.h"

namespace ui {
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
		dir_t		dir;
		wrap_t		wrap;
		align_t		align;
		width_t		width;
		height_t	height;
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
		attributes dir_t::operator = (e_dir b) {		return String("dir:") + (b == DIR_H ? "h:" : "v:");							}
		attributes wrap_t::operator = (bool b) {		return String("wrap:") + (b ? "true:" : "false:");							}
		attributes align_t::operator = (e_align b) {		return String("align:") + (b == ALIGN_CENTER ? "center:" : (b == ALIGN_LEFT ? "left:" : "right:"));	}		
		attributes width_t::operator = (String b) {		return String("w:") + b + ":";										}
		attributes width_t::operator = (unsigned int b) {	return String("w:") + b + ":";										}
		attributes height_t::operator = (String b) {		return String("h:") + b + ":";										}
		attributes height_t::operator = (unsigned int b) {	return String("h:") + b + ":";										}
		attributes text_t::operator = (String b) {		return String("text:") + b + ":";									}
		attributes esize_t::operator = (unsigned int b) {	return String("size:") + b + ":";									}
		attributes value_t::operator = (String b) {		return String("value:") + b + ":";									}
		attributes background_t::operator = (String b) {	return String("backcolor:") + b + ":";									}
		attributes textcolor_t::operator = (String b) {		return String("textcolor:") + b + ":";									}
		attributes display_t::operator = (bool b) {		return String("display:") + (b ? "true:" : "false:");							}
		attributes disabled_t::operator = (bool b) {		return String("disabled:") + (b ? "true:" : "false:");							}
	}
}
