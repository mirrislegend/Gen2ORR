import SocketServer
import json

#Python info: https://docs.python.org/2.7/library/socketserver.html

class MyTCPServer(SocketServer.ThreadingTCPServer): #wrote the own class to allow resue of address
    allow_reuse_address = True

class MyTCPServerHandler(SocketServer.BaseRequestHandler):
    #need to override the handle method to have communication with client
    def handle(self):
        try:
    	    while self.request.recv(1024)!=0:
    	    	data = json.loads(self.request.recv(1024).strip())
                print data
                	# send some 'ok' back
                self.request.sendall(json.dumps({'return':'ok'}))
        except Exception, e:
            print "Exception wile receiving message: ", e


if __name__ == "__main__":
    #server = MyTCPServer(('127.0.0.1', 13373), MyTCPServerHandler)
    server=MyTCPServer(('localhost', 9999), MyTCPServerHandler)
    server.serve_forever() #will keep running forever, until hit ctrl-c
