import paho.mqtt.client as mqtt
import ssl
import sys

broker_url = "iot.eclipse.org"
broker_port = 8883

def on_connect(client, userdata, flags, rc):
   print("Connected With Result Code " + str(rc))

def on_message(client, userdata, message):
   print("Message Recieved from Others: "+message.payload.decode())
   sys.exit(0)

client = mqtt.Client()
client.on_connect = on_connect
#To Process Every Other Message
client.on_message = on_message
client.tls_set(None,
                    None,
                    None, cert_reqs=ssl.CERT_NONE, tls_version=ssl.PROTOCOL_TLSv1, ciphers=None)

client.tls_insecure_set(True)
client.connect(broker_url, broker_port, 60)

client.subscribe("/topic/ota_update", qos=1)
client.publish(topic="/topic/ota_update", payload="poc_v2.bin", qos=1, retain=False)

client.loop_forever()
