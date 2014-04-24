import socket
import json

#Python doc: https://docs.python.org/3/library/socket.html

data = {'text':'hello world', 'words':2}

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #different options, but AF_INET and SOCK_STREAM most common, TCP so streaming data
sock.connect(('localhost', 9999)) #alternative host and port numbers could be ('127.0.0.1', 13373)

#send the data and receive response
sock.send(json.dumps(data))
result = json.loads(sock.recv(1024))
print "On client side, received response:"
print result

#ELIDE if don't want each data
#parse the json and print back each item
for term in result:
  print term, ":", result[term]

sock.shutdown(socket.SHUT_RDWR) #terminates connection immediately, further sends and receives are disallowed
sock.close() #mark it as closed






#Main difference between Python and C:
#buffer allocation is automatic and buffer length is implicit on send