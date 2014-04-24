import socket
import json

#Python doc: https://docs.python.org/3/library/socket.html


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #different options, but AF_INET and SOCK_STREAM most common, TCP so streaming data
sock.connect(('localhost', 9999)) #alternative host and port numbers could be ('127.0.0.1', 13373)

# orig_data = {'text':'hello world', 'words':2}
# get_and_send(orig_data)


def get_and_send(data):
    #send the data and receive response
    print "about to send data"
    sock.send(json.dumps(data))
    result = json.loads(sock.recv(1024))
    print "On client side, received response:"
    print result

    #ELIDE if don't want each data
    #parse the json and print back each item
    for term in result:
        print term, ":", result[term]

def end():
    sock.shutdown(socket.SHUT_RDWR) #terminates connection immediately, further sends and receives are disallowed
    sock.close() #mark it as closed

while True:
    user_info=raw_input("Enter some text:")
    if user_info=="end": #end out of the program
        end()
    else:
        new_data={'text':user_info,'words':len(user_info.split())}
        print "new data is", new_data
        get_and_send(new_data)




#Main difference between Python and C:
#buffer allocation is automatic and buffer length is implicit on send