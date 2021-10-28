from coapthon.server.coap import CoAP
from exampleresources import BasicResource


class CoAPServer(CoAP):
    def __init__(self, host, port):
        CoAP.__init__(self, (host, port))
        self.add_resource('tag', BasicResource())
        self.add_resource('result', BasicResource())


if __name__ == '__main__':
    server = CoAPServer("0.0.0.0", 5683)
    try:
        server.listen(10)
    except KeyboardInterrupt:
        print("Server Shutdown")
        server.close()
        print("Exiting...")
