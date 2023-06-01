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

topic = 'triplet/fromArduino/#'
#topic = 'triplet/#'    # all topics

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
    print(f"Data from mqtt: {data}")

while 1:
    time.sleep(3)

    ''' send test data '''
    mqtt.publish("triplet/simulator/base", 180)
    mqtt.publish("triplet/simulator/shoulder",-90)
    mqtt.publish("triplet/simulator/elbow", 90)
    mqtt.publish("triplet/simulator/wrist1", 180)
    mqtt.publish("triplet/simulator/wrist2", 90)
    mqtt.publish("triplet/simulator/wrist3", 90)
    print("Sent position")
    time.sleep(3)

    ''' rotate base '''
    mqtt.publish("triplet/simulator/base", -180)
    mqtt.publish("triplet/simulator/shoulder", -90)
    mqtt.publish("triplet/simulator/elbow", 90)
    mqtt.publish("triplet/simulator/wrist1", 180)
    mqtt.publish("triplet/simulator/wrist2", 90)
    mqtt.publish("triplet/simulator/wrist3", 90)
    
    print("Sent position")
