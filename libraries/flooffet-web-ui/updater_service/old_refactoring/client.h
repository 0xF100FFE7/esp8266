#ifndef CLIENT_UI_GUARD
#define CLIENT_UI_GUARD

#include <ESPAsyncWebServer.h>

//serverside client;
namespace ui {
	struct frame {
		ui::element element;
		
		frame(void *callback);
	};

	#define MAX_ELBOX_CLIENTS DEFAULT_MAX_WS_CLIENTS

	struct client {
		bool present; //if client exists
		uint32_t id;
		
		bool interface_loaded;
		bool waiting_for_frame_confirmation;
		int frame_number;
		
		ui::messages messages;
		frame current_frame;
		
		static void confirm_frame(void *);
		void build_interface();
		void cleanup();
		
		elbox_client();
	} extern elbox_clients[MAX_ELBOX_CLIENTS];

	elbox_client *find_elbox_client(uint32_t id);
}

#endif
*/
