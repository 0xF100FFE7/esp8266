from http.server import BaseHTTPRequestHandler, HTTPServer

class MyServer(BaseHTTPRequestHandler):
	def do_GET(self):
		self.send_response(200)
		self.send_header('Content-type', 'application/octet-stream')
		self.send_header('Content-Disposition', 'attachment; filename="firmware.zip"')
		self.end_headers()

		with open('firmware.zip', 'rb') as file: 
			self.wfile.write(file.read())


myServer = HTTPServer(('localhost', 80), MyServer)
myServer.serve_forever()
myServer.server_close()
