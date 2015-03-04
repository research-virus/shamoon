################################################################################
## Copyright 2015 Christian Roggia. All rights reserved.                      ##
## Use of this source code is governed by an Apache 2.0 license that can be   ##
## found in the LICENSE file.                                                 ##
################################################################################

from http.server import BaseHTTPRequestHandler, HTTPServer

PORT_NUMBER = 8000

class CCRequestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path[:27] == "/ajax_modal/modal/data.asp?":
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            self.wfile.write(bytes("ok", 'utf-8'))
        else:
            self.send_response(404)
            self.send_header('Content-type','text/html')
            self.end_headers()
            
            self.wfile.write(bytes("404: Not found.", 'utf-8'))

server = HTTPServer(('127.0.0.1', PORT_NUMBER), CCRequestHandler)
print('Started [Shamoon C&C] on port', PORT_NUMBER)

server.serve_forever()