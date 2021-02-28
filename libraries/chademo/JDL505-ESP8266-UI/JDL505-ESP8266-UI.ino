#include "interface.h"
#include "shared.h"

struct cmd cmd;

struct chademo_status {
	bool charging = false;
	float voltage, current, amphours, power, kwh, soc;
} status;

struct bms_status {
	float amperage, voltage, soc;
} bms;


int ah_to_kwh(int ah)
{
	return ah * 3.6 * 96 / 1000 + 1;
}

int kwh_to_ah(int kwh)
{
	return kwh * 1000 / 96 / 3.6;
}

//template class for regulator widgets TODOomb yet
/*template<typename type, enum command_type cmd, const char *desc, struct tab &placement, type initial_value, type min_value, type max_value, type step>
struct regulator_widget {
	struct box box;
	struct button l_button, r_button;
	struct field field;
	
	type value = initial_value;
	
	template <struct regulator_widget &object>
	static void l_button_callback(struct button &id, client_id_t sender)
	{
		object.value -= step;
		object.field.pack(attr::value = object.value).send_all();
		object.send(cmd, object.value);
	}
	
	template <struct regulator_widget &object>
	static void r_button_callback(struct button &id, client_id_t sender)
	{
		object.value += step;
		object.field.pack(attr::value = object.value).send_all();
		object.send(cmd, object.value);
	}
	
	template <struct regulator_widget &object>
	static void field_callback(struct field &id, String value, client_id_t sender)
	{
		id.pack(attr::value = object.value).send_all();
		object.send(cmd, object.value);
	}
}*/




//////////////////////////////////////////////////////////////////////////////////
/*				NAVIGATION PANEL WIDGETS			*/
//////////////////////////////////////////////////////////////////////////////////
struct tab_navigation_widget {
	tab home;
	tab additional;
	tab settings;
	
	tab *selected = &home; //User specified, does not change its state via websockets
	
	packet build()
	{
		return	
			home.pack(root, (attr::panel = "nav", (attr::text = "Home", selected == &home ? attr::selected = true : none), attr::direction = DIR_H, attr::wrap = true)) +
			additional.pack(root, (attr::panel = "nav", (attr::text = "Additional", selected == &additional ? attr::selected = true : none), attr::direction = DIR_H, attr::wrap = true)) +
			settings.pack(root, (attr::panel = "nav", (attr::text = "Settings", selected == &settings ? attr::selected = true : none), attr::direction = DIR_H, attr::tab_align = ALIGN_RIGHT, attr::wrap = true));
	}
} tab_navigation_widget;



struct regulator_widget<int, CMD_SET_MAX_ASKING_AMPS, constMessage, tab_navigation_widget.home, 10, 0, 40, 5> widgy;
//////////////////////////////////////////////////////////////////////////////////
/*				HOME PANEL WIDGETS				*/
//////////////////////////////////////////////////////////////////////////////////
struct home_status_widget {
	struct box box;
	struct text l_text;
	struct text r_text;
	int time_charging = 0;
		
	attributes l_attr()
	{
		return attr::text =
			String("Voltage: ") + status.voltage + "\n" +
			"Current: " + -status.current + "\n" +
			"Power: " + -status.power + "KWt\n" +
			"SOC: " + status.soc + "%\n" +
			"BMS Current: " + bms.amperage + "\n" +
			"BMS Voltage: " + bms.voltage + "\n" +
			"BMS SOC: " + bms.soc;
	}

	attributes r_attr()
	{
		return attr::text =
			String("A/h: ") + -status.amphours + "\n" +
			"KWt/h: " + -status.kwh + "\n" +
			"Charging time: " + (time_charging / 60 / 60) + ":" + ((time_charging / 60) % 60) + ":" + (time_charging % 60);
	}
	
	void update_status()
	{
		(l_text.pack(l_attr()) + r_text.pack(r_attr())).send_all();
	}
	
	packet build()
	{
		return
		box.pack(root, (attr::text = "Status", attr::fill = true, attr::direction = DIR_H)) +
		l_text.pack(box, (l_attr(), attr::wrap = true)) + r_text.pack(box, (r_attr(), attr::wrap = true));
	}
	
} home_status_widget;

extern struct set_max_asking_amps_widget set_max_asking_amps_widget;
struct set_max_asking_amps_widget {
	struct box box;
	struct button l_button, r_button;
	struct field field;
	
	int max_asking_amps = 150;
	
	static void l_button_callback(struct button &id, client_id_t sender)
	{
		int &val = ::set_max_asking_amps_widget.max_asking_amps;
		val -= 5;
		::set_max_asking_amps_widget.field.pack(attr::value = val).send_all();
		cmd.send(CMD_SET_MAX_ASKING_AMPS, val);
	}
	
	static void r_button_callback(struct button &id, client_id_t sender)
	{
		int &val = ::set_max_asking_amps_widget.max_asking_amps;
		val += 5;
		::set_max_asking_amps_widget.field.pack(attr::value = val).send_all();
		cmd.send(CMD_SET_MAX_ASKING_AMPS, val);
	}
	
	static void field_callback(struct field &id, String value, client_id_t sender)
	{
		int &val = ::set_max_asking_amps_widget.max_asking_amps;
		val = value.toInt();
		id.pack(attr::value = val).send_all();
		cmd.send(CMD_SET_MAX_ASKING_AMPS, val);
	}
	
	set_max_asking_amps_widget() : l_button(l_button_callback), r_button(r_button_callback), field(field_callback) {}
	packet build()
	{
		return
		box.pack(tab_navigation_widget.home, (attr::text = "Set max amperage", attr::direction = DIR_H)) +
		l_button.pack(box, (attr::text = "-", attr::width = "fill")) + field.pack(box, (attr::value = max_asking_amps, attr::width = "fill")) + r_button.pack(box, (attr::text = "+", attr::width = "fill"));
	}
} set_max_asking_amps_widget;

struct emergency_stop_widget {
	struct box box;
	struct button button;
	
	static void button_callback(struct button &id, client_id_t sender)
	{
		cmd.send(CMD_END_CHARGE);
	}
	
	emergency_stop_widget() : button(button_callback) {}
	
	packet build()
	{
		return
		box.pack(tab_navigation_widget.home, (attr::text = "Emergency stop", attr::background = "red", attr::fill = true)) +
		button.pack(box);
	}
} emergency_stop_widget;




//////////////////////////////////////////////////////////////////////////////////
/*				ADDITIONAL PANEL WIDGETS				*/
//////////////////////////////////////////////////////////////////////////////////
extern struct settings_status_widget settings_status_widget;
struct settings_status_widget {
	struct box box;
	struct text evse_status;
	String evse_status_str = "No evse params has been transmitted yet";
	struct button reset;
	
	struct dialog dialog;
	struct box wrapper;
	struct button yes, no;
	
	void update_evse_status(String str)
	{
		evse_status_str = str;
		evse_status.pack(attr::text = str).send_all();
	}
	
	static void yes_callback(struct button &id, client_id_t sender)
	{
		::settings_status_widget.dialog.pack(attr::display = false).send(sender);
		cmd.send(CMD_RESET);
	}
	
	static void no_callback(struct button &id, client_id_t sender)
	{
		::settings_status_widget.dialog.pack(attr::display = false).send(sender);
	}
	
	static void reset_callback(struct button &id, client_id_t sender)
	{
		::settings_status_widget.dialog.pack(attr::display = true).send(sender);
	}
	
	settings_status_widget() : reset(reset_callback), yes(yes_callback), no(no_callback) {}
	packet build()
	{
		return
		box.pack(tab_navigation_widget.additional, (attr::text = "Evse status", attr::fill = true)) +
		evse_status.pack(box, (attr::text = evse_status_str, attr::wrap = true)) +
		reset.pack(box, (attr::text = "Reset soc")) +
		dialog.pack(root, (attr::display = false)) + wrapper.pack(dialog, (attr::text = "Are you sure want to reset SOC?", attr::direction = DIR_H)) +
		yes.pack(wrapper, (attr::text = "Yes")) + no.pack(wrapper, (attr::text = "No"));
	}
	
} settings_status_widget;

extern struct set_target_voltage_widget set_target_voltage_widget; 
struct set_target_voltage_widget {
	struct box box;
	struct button l_button, r_button;
	struct field field;
	
	int target_voltage = 390;
	
	static void l_button_callback(struct button &id, client_id_t sender)
	{
		struct set_target_voltage_widget &self = ::set_target_voltage_widget;
		self.target_voltage--;
		self.field.pack(attr::value = self.target_voltage).send_all();
		cmd.send(CMD_SET_TARGET_VOLT, self.target_voltage);
	}
	
	static void r_button_callback(struct button &id, client_id_t sender)
	{
		struct set_target_voltage_widget &self = ::set_target_voltage_widget;
		self.target_voltage++;
		self.field.pack(attr::value = self.target_voltage).send_all();
		cmd.send(CMD_SET_TARGET_VOLT, self.target_voltage);
	}
	
	static void field_callback(struct field &id, String value, client_id_t sender)
	{
		struct set_target_voltage_widget &self = ::set_target_voltage_widget;
		self.target_voltage = value.toFloat();
		id.pack(attr::value = self.target_voltage).send_all();
		cmd.send(CMD_SET_TARGET_VOLT, self.target_voltage);
	}
	
	set_target_voltage_widget() : l_button(l_button_callback), r_button(r_button_callback), field(field_callback) {}
	packet build()
	{
		return
		box.pack(tab_navigation_widget.additional, (attr::text = "Set target voltage", attr::direction = DIR_H)) +
		l_button.pack(box, (attr::text = "-", attr::width = "fill")) + field.pack(box, (attr::value = target_voltage, attr::width = "fill")) + r_button.pack(box, (attr::text = "+", attr::width = "fill"));
	}
} set_target_voltage_widget;

extern struct calibrate_amps_widget calibrate_amps_widget; 
struct calibrate_amps_widget {
	struct box box;
	struct button l_button, r_button;
	struct field field;
	
	float current_calibration;
	
	static void l_button_callback(struct button &id, client_id_t sender)
	{
		float &val = ::calibrate_amps_widget.current_calibration;
		val -= 0.01;
		::calibrate_amps_widget.field.pack(attr::value = String(val)).send_all();
		cmd.send(CMD_CAL_AMPS, val);
	}
	
	static void r_button_callback(struct button &id, client_id_t sender)
	{
		float &val = ::calibrate_amps_widget.current_calibration;
		val += 0.01;
		::calibrate_amps_widget.field.pack(attr::value = String(val)).send_all();
		cmd.send(CMD_CAL_AMPS, val);
	}
	
	static void field_callback(struct field &id, String value, client_id_t sender)
	{
		float &val = ::calibrate_amps_widget.current_calibration;
		val = value.toFloat();
		id.pack(attr::value = String(val)).send_all();
		cmd.send(CMD_CAL_AMPS, val);
	}
	
	calibrate_amps_widget() : l_button(l_button_callback), r_button(r_button_callback), field(field_callback) {}
	packet build()
	{
		return
		box.pack(tab_navigation_widget.additional, (attr::text = "Calibrate ampers", attr::direction = DIR_H)) +
		l_button.pack(box, (attr::text = "-", attr::width = "fill")) + field.pack(box, (attr::value = String(current_calibration), attr::width = "fill")) + r_button.pack(box, (attr::text = "+", attr::width = "fill"));
	}
} calibrate_amps_widget;

extern struct calibrate_volt_widget calibrate_volt_widget;
struct calibrate_volt_widget {
	struct box box;
	struct button l_button, r_button;
	struct field field;
	
	float voltage_calibration;
	
	static void l_button_callback(struct button &id, client_id_t sender)
	{
		float &val = ::calibrate_volt_widget.voltage_calibration;
		val -= 0.01;
		::calibrate_volt_widget.field.pack(attr::value = String(val)).send_all();
		cmd.send(CMD_CAL_VOLT, val);
	}
	
	static void r_button_callback(struct button &id, client_id_t sender)
	{
		float &val = ::calibrate_volt_widget.voltage_calibration;
		val += 0.01;
		::calibrate_volt_widget.field.pack(attr::value = String(val)).send_all();
		cmd.send(CMD_CAL_VOLT, val);
	}
	
	static void field_callback(struct field &id, String value, client_id_t sender)
	{
		float &val = ::calibrate_volt_widget.voltage_calibration;
		val = value.toFloat();
		id.pack(attr::value = String(val)).send_all();
		cmd.send(CMD_CAL_VOLT, val);
	}
	
	calibrate_volt_widget() : l_button(l_button_callback), r_button(r_button_callback), field(field_callback) {}
	packet build()
	{
		return
		box.pack(tab_navigation_widget.additional, (attr::text = "Calibrate voltage", attr::direction = DIR_H)) +
		l_button.pack(box, (attr::text = "-", attr::width = "fill")) + field.pack(box, (attr::value = String(voltage_calibration), attr::width = "fill")) + r_button.pack(box, (attr::text = "+", attr::width = "fill"));
	}
} calibrate_volt_widget;

extern struct set_capacity_widget set_capacity_widget;
struct set_capacity_widget {
	struct box box;
	struct button l_button, r_button;
	struct field field;
	
	bool kwh_dim = true; //if false, then dimension is kwt
	struct button dimension; //either kwt or ah
	
	int capacity = 40;
	
	static void dimension_switch(struct button &id, client_id_t sender)
	{
		struct set_capacity_widget &self = ::set_capacity_widget;
		if (self.kwh_dim) {
			self.kwh_dim = false;
			self.capacity = kwh_to_ah(self.capacity);
		} else {
			self.kwh_dim = true;
			self.capacity = ah_to_kwh(self.capacity);
		}
		
		::set_capacity_widget.build().send_all();
	}
	
	int normal_capacity()
	{
		return kwh_dim ? capacity : ah_to_kwh(capacity);
	}
	
	static void l_button_callback(struct button &id, client_id_t sender)
	{
		struct set_capacity_widget &self = ::set_capacity_widget;
		self.capacity--;
		self.field.pack(attr::value = self.capacity).send_all();
		cmd.send(CMD_SET_CAPACITY, self.normal_capacity());
	}
	
	static void r_button_callback(struct button &id, client_id_t sender)
	{
		struct set_capacity_widget &self = ::set_capacity_widget;
		self.capacity++;
		self.field.pack(attr::value = self.capacity).send_all();
		cmd.send(CMD_SET_CAPACITY, self.normal_capacity());
	}
	
	static void field_callback(struct field &id, String value, client_id_t sender)
	{
		struct set_capacity_widget &self = ::set_capacity_widget;
		self.capacity = value.toInt();
		id.pack(attr::value = self.capacity).send_all();
		cmd.send(CMD_SET_CAPACITY, self.normal_capacity());
	}
	
	set_capacity_widget() : l_button(l_button_callback), r_button(r_button_callback), field(field_callback), dimension(dimension_switch) {}
	packet build()
	{
		return
		box.pack(tab_navigation_widget.additional, (attr::text = "Set capacity", attr::direction = DIR_H, attr::fill = true)) +
		l_button.pack(box, (attr::text = "-", attr::width = "fill")) +
		field.pack(box, (attr::value = capacity, attr::width = "fill")) + dimension.pack(box, (attr::text = kwh_dim ? "kWh" : "aH")) +
		r_button.pack(box, (attr::text = "+", attr::width = "fill"));
	}
} set_capacity_widget;

extern struct set_max_volt_widget set_max_volt_widget;
struct set_max_volt_widget {
	struct box box;
	struct button l_button, r_button;
	struct field field;
	
	int max_volt = 350;
	
	static void l_button_callback(struct button &id, client_id_t sender)
	{
		int &val = ::set_max_volt_widget.max_volt;
		val -= 10;
		::set_max_volt_widget.field.pack(attr::value = val).send_all();
		cmd.send(CMD_SET_MAX_VOLT, val);
	}
	
	static void r_button_callback(struct button &id, client_id_t sender)
	{
		int &val = ::set_max_volt_widget.max_volt;
		val += 10;
		::set_max_volt_widget.field.pack(attr::value = val).send_all();
		cmd.send(CMD_SET_MAX_VOLT, val);
	}
	
	static void field_callback(struct field &id, String value, client_id_t sender)
	{
		int &val = ::set_max_volt_widget.max_volt;
		val = value.toInt();
		id.pack(attr::value = val).send_all();
		cmd.send(CMD_SET_MAX_VOLT, val);
	}
	
	set_max_volt_widget() : l_button(l_button_callback), r_button(r_button_callback), field(field_callback) {}
	packet build()
	{
		return
		box.pack(tab_navigation_widget.additional, (attr::text = "Set max voltage", attr::direction = DIR_H)) +
		l_button.pack(box, (attr::text = "-", attr::width = "fill")) + field.pack(box, (attr::value = max_volt, attr::width = "fill")) + r_button.pack(box, (attr::text = "+", attr::width = "fill"));
	}
} set_max_volt_widget;

extern struct ignore_current_mismatch_widget ignore_current_mismatch_widget;
struct ignore_current_mismatch_widget {
	struct box box;
	struct switcher switcher;
	
	bool ignore = true;
	
	static void switcher_callback(struct switcher &id, client_id_t sender)
	{
		bool &val = ::ignore_current_mismatch_widget.ignore;
		id.turn(val).send_all();
		cmd.send(CMD_IGNORE_CURRENT_MISMATCH, val);
	}
	
	ignore_current_mismatch_widget() : switcher(switcher_callback) {};
	packet build()
	{
		return
			box.pack(tab_navigation_widget.additional, (attr::text = "Ignore current mismatch", attr::direction = DIR_H)) + 
			switcher.pack(box, (switcher.get(ignore)));
	}
} ignore_current_mismatch_widget;

extern struct ignore_voltage_mismatch_widget ignore_voltage_mismatch_widget;
struct ignore_voltage_mismatch_widget {
	struct box box;
	struct switcher switcher;
	
	bool ignore = true;
	
	static void switcher_callback(struct switcher &id, client_id_t sender)
	{
		bool &val = ::ignore_voltage_mismatch_widget.ignore;
		id.turn(val).send_all();
		cmd.send(CMD_IGNORE_VOLTAGE_MISMATCH, val);
	}
	
	ignore_voltage_mismatch_widget() : switcher(switcher_callback) {};
	packet build()
	{
		return
			box.pack(tab_navigation_widget.additional, (attr::text = "Ignore voltage mismatch", attr::direction = DIR_H)) + 
			switcher.pack(box, (switcher.get(ignore)));
	}
} ignore_voltage_mismatch_widget;




//////////////////////////////////////////////////////////////////////////////////
/*				SETTINGS PANEL WIDGETS				*/
//////////////////////////////////////////////////////////////////////////////////
extern struct ap_settings_widget ap_settings_widget;
struct ap_settings_widget {
	struct box box;
	struct box ap_text_col, ap_field_col;
	struct text ap_ssid_text, ap_pass_text, ap_ip_text;
	struct field ap_ssid_field, ap_pass_field, ap_ip_field;
	struct applier apply_settings;
	bool applier_enabled = false;
	
	struct dialog dialog;
	struct box wrapper; //TODO
	struct button yes, no;

	static void ap_ssid_field_callback(struct field &id, String value, client_id_t sender)
	{
		value.toCharArray(network::ap.changed().ssid, 80); //TODO check ssid maybeh?
		id.pack(attr::value = String(network::ap.ssid)).send_all();
	}
	
	static void ap_pass_field_callback(struct field &id, String value, client_id_t sender)
	{
		if (network::passphrase_is_valid(value))
			value.toCharArray(network::ap.changed().pass, 80);
		id.pack(attr::value = String(network::ap.pass)).send_all();
	}
	
	static void ap_ip_field_callback(struct field &id, String value, client_id_t sender)
	{
		IPAddress ip;
		ip.fromString(value);
		network::ap.changed().ip = network::ap.gateway = (uint32_t)ip;
		id.pack(attr::value = IPAddress(network::ap.ip).toString()).send_all();
	}

	static void applier_callback(struct applier &id, client_id_t sender)
	{
		::ap_settings_widget.dialog.pack(attr::display = true).send(sender);
	}

	static void yes_callback(struct button &id, client_id_t sender)
	{
		::ap_settings_widget.dialog.pack(attr::display = false).send(sender);
		network::end();
		network::ap.save();
		DEBUG_MSG("Ap settings applied, restarting...\n");
		delay(1000);
		ESP.restart();
	}
	
	static void no_callback(struct button &id, client_id_t sender)
	{
		::ap_settings_widget.dialog.pack(attr::display = false).send(sender);
	}

	ap_settings_widget() : ap_ssid_field(ap_ssid_field_callback), ap_pass_field(ap_pass_field_callback), ap_ip_field(ap_ip_field_callback), yes(yes_callback), no(no_callback), apply_settings(applier_callback) {}
	packet build()
	{
		return
		box.pack(tab_navigation_widget.settings, (attr::text = "Access point settings", attr::direction = DIR_H, attr::wrap = true)) +
		ap_text_col.pack(box, (attr::direction = DIR_V, attr::width = "content")) +
		ap_field_col.pack(box, (attr::direction = DIR_V)) +
		ap_ssid_text.pack(ap_text_col, (attr::text = "SSID: ")) + ap_pass_text.pack(ap_text_col, (attr::text = "Password: ")) + ap_ip_text.pack(ap_text_col, (attr::text = "Ip: ")) +
		ap_ssid_field.pack(ap_field_col, (attr::value = String(network::ap.ssid))) + ap_pass_field.pack(ap_field_col, (attr::value = String(network::ap.pass))) + 
			ap_ip_field.pack(ap_field_col, (attr::value = IPAddress(network::ap.ip).toString())) +
		apply_settings.pack(box, (attr::text = "Save network settings", attr::background = "orange", apply_settings.get(applier_enabled))) +
		
		dialog.pack(root, (attr::display = false)) + wrapper.pack(dialog, (attr::text = "Changing the access point settings will reboot the device. Do you really want to continue?", attr::direction = DIR_H)) +
		yes.pack(wrapper, (attr::text = "Yes")) + no.pack(wrapper, (attr::text = "No"));
	}
} ap_settings_widget;




//////////////////////////////////////////////////////////////////////////////////
/*				INTERFACE BUILDER				*/
//////////////////////////////////////////////////////////////////////////////////
#define BEGIN_ADD_TO_INTERFACE switch(idx)
#define ADD_TO_INTERFACE(idx, stuff) case idx: cl.packet += stuff.build(); return false
#define END_ADD_TO_INTERFACE default: return true
	
bool ui::interface(client &cl, int idx) //implementation of interface builder is user specified
{
	BEGIN_ADD_TO_INTERFACE {
		ADD_TO_INTERFACE(0, home_status_widget);
		ADD_TO_INTERFACE(1, tab_navigation_widget);
		ADD_TO_INTERFACE(2, set_max_asking_amps_widget);
		ADD_TO_INTERFACE(3, emergency_stop_widget);
		ADD_TO_INTERFACE(4, settings_status_widget);
		ADD_TO_INTERFACE(5, calibrate_amps_widget);
		ADD_TO_INTERFACE(6, calibrate_volt_widget);
		ADD_TO_INTERFACE(7, set_target_voltage_widget);
		ADD_TO_INTERFACE(8, set_max_volt_widget);
		ADD_TO_INTERFACE(9, set_capacity_widget);
		ADD_TO_INTERFACE(10, ignore_current_mismatch_widget);
		ADD_TO_INTERFACE(11, ignore_voltage_mismatch_widget);
		ADD_TO_INTERFACE(12, ap_settings_widget);
		END_ADD_TO_INTERFACE;
	}
}

void parse(struct cmd &cmd) {
	switch (cmd.type) {
		case CMD_VOLTAGE:
			status.voltage = cmd.buf.toFloat();
			break;
		
		case CMD_CURRENT:
			status.current = cmd.buf.toFloat();
			break;
			
		case CMD_AMPHOURS:
			status.amphours = cmd.buf.toFloat();
			break;
			
		case CMD_POWER:
			status.power = cmd.buf.toFloat();
			break;
			
		case CMD_KWH:
			status.kwh = cmd.buf.toFloat();
			break;
			
		case CMD_SOC:
			status.soc = cmd.buf.toInt();
			break;
			
		case CMD_UPDATE_STATUS:
			home_status_widget.update_status();
			break;
		
		//update widget status just once after start
		case CMD_CAL_AMPS:
			calibrate_amps_widget.field.pack(attr::value = String(calibrate_amps_widget.current_calibration = cmd.buf.toFloat()));
			break;
			
		case CMD_CAL_VOLT:
			calibrate_volt_widget.field.pack(attr::value = String(calibrate_volt_widget.voltage_calibration = cmd.buf.toFloat()));
			break;
			
		case CMD_SET_TARGET_VOLT:
			set_target_voltage_widget.field.pack(attr::value = String(set_target_voltage_widget.target_voltage = cmd.buf.toFloat()));
			break;
			
		case CMD_SET_CAPACITY:
			set_capacity_widget.capacity = cmd.buf.toInt();
			set_capacity_widget.capacity = set_capacity_widget.normal_capacity();
			set_capacity_widget.field.pack(attr::value = set_capacity_widget.capacity);
			break;
			
		case CMD_SET_MAX_VOLT:
			set_max_volt_widget.field.pack(attr::value = (set_max_volt_widget.max_volt = cmd.buf.toInt()));
			break;
			
		case CMD_SET_MAX_ASKING_AMPS:
			set_max_asking_amps_widget.field.pack(attr::value = (set_max_asking_amps_widget.max_asking_amps = cmd.buf.toInt()));
			break;
			
		case CMD_IGNORE_CURRENT_MISMATCH:
			ignore_current_mismatch_widget.switcher.pack(ignore_current_mismatch_widget.switcher.get(ignore_current_mismatch_widget.ignore = cmd.buf.toInt()));
			break;
			
		case CMD_IGNORE_VOLTAGE_MISMATCH:
			ignore_voltage_mismatch_widget.switcher.pack(ignore_voltage_mismatch_widget.switcher.get(ignore_voltage_mismatch_widget.ignore = cmd.buf.toInt()));
			break;
			
		case CMD_BEGIN_CHARGE:
			status.charging = true;
			home_status_widget.time_charging = 0;	
			break;
		
		case CMD_END_CHARGE:
			status.charging = false;
			
		case CMD_EVSE_STATUS:
			settings_status_widget.update_evse_status(cmd.buf);
			break;
		
		case CMD_BMS_AMPERAGE:
			bms.amperage = cmd.buf.toFloat();
			break;
		
		case CMD_BMS_VOLTAGE:
			bms.voltage = cmd.buf.toFloat();
			break;
		
		case CMD_BMS_SOC:
			bms.soc = cmd.buf.toFloat();
			break;
		
		default:
			break;
	}
}

void setup()
{
	Serial.begin(115200);
	interface_setup();
}

void loop()
{
	static unsigned long old_millis = 0;
	if (millis() > old_millis + 1000) {
		if (network::ap.need_commit) {
			if (!ap_settings_widget.applier_enabled)
				ap_settings_widget.apply_settings.pack(attr::disabled = false).send_all();
			//do not save settings here, better do this inside applier.
		}
		
		if (status.charging)
			home_status_widget.time_charging++;
		old_millis += 1000;
	}
	
	interface_loop();
	if (cmd.get())
		parse(cmd);
}
