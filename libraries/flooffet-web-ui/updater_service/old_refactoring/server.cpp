#include "server.h"

namespace ui
{
	client clients[8];
	void web_event(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
	{
		switch (type) {
		case WS_EVT_CONNECT:
			client.connect(client->id());
			break;
			
		case WS_EVT_DISCONNECT:
			on_disconnect(client->id());
			break;
			
		case WS_EVT_DATA: {
			messages in;
			in.buffer.reserve(len + 1);
			for (size_t i = 0; i < len; i++)
				in.buffer += (char)data[i];
			in.parse(client->id());
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

		ws->onEvent(ui_event);
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
}
