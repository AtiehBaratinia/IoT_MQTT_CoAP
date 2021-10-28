from coapthon.client.helperclient import HelperClient
from coapthon import defines
from coapthon.messages.response import Response
import time
host = "192.168.1.5"
port = 5683
path ="tag"
if __name__=="__main__":
    while True:
        client = HelperClient(server=(host, port))

        response = client.get("tag")
        print(response.payload)
        if response.payload == "18218114943":
            print("80,80")
            client.put("result", "80,80")
        elif response.payload == "25037828943":
            print("20,20")
            client.put("result", "20,20")
        # time.sleep(500)
        client.stop()



