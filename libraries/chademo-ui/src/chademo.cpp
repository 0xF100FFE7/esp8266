#include "chademo.h"

struct chademo_status {
	float voltage, current, amphours, power, kwh;
} status;

struct chademo_main {
	struct box box;
	struct text text;
	
	attributes status_attr()
	{
		return attr::text =
			String("Voltage: ") + status.voltage + "\n" +
			"Current: " + status.current + "\n" +
			"A/h: " + status.amphours + "\n" +
			"KWt/h: " + status.kwh; 
	}
	
	void update_status()
	{
		text.pack(status_attr()).send_all();
	}
	
	packet build()
	{
		return
		box.pack(root, (attr::text = "Status", attr::fill = true)) +
		text.pack(box, (status_attr(), attr::wrap = true));
	}
} chademo_main;

bool ui::interface(client &cl, int idx) //implementation of interface builder is user specified
{
	cl.packet += chademo_main.build();
}

void chademo_setup()
{
	interface_setup();
}

void chademo_loop()
{
	interface_loop();
}
