from flask import Flask
from flask_mqtt import Mqtt
import time


app = Flask(__name__)
app.config['MQTT_BROKER_URL'] = 'broker.hivemq.com'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = ''  #'ntnu'
app.config['MQTT_PASSWORD'] = ''  #'ntnuais2103'
app.config['MQTT_KEEPALIVE'] = 5
app.config['MQTT_TLS_ENABLED'] = False

topic = 'triplet/#'

mqtt = Mqtt(app)

@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe(topic)
    print("Subscribed to " + topic)

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    data = dict(
        topic=message.topic,
        payload=message.payload.decode()
    )
    print(f"Received data from mqtt: {data}")

while 1:
    time.sleep(3)
    mqtt.publish("triplet/controller/base", 1024)
    mqtt.publish("triplet/controller/shoulder", 768)
    mqtt.publish("triplet/controller/elbow", 1024)
    mqtt.publish("triplet/controller/wrist1", 768)
    mqtt.publish("triplet/controller/wrist2", 1024)
    mqtt.publish("triplet/controller/wrist3", 1024)
    mqtt.publish("NTNU_TEST2", 90)
    time.sleep(3)
    mqtt.publish("triplet/controller/wrist1", 1024)
    
    print("Sent Home position")
