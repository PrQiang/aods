import socketserver, threading
from RestfulApiHandler import RestfulApiHandler

if __name__ == "__main__":
    server = socketserver.ThreadingTCPServer(('0.0.0.0', 32002), RestfulApiHandler)
    t = threading.Thread(target= server.serve_forever, args = ())
    t.start()