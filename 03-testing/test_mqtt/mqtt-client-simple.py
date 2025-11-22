import paho.mqtt.client as mqtt #import the client1
import time

#mqtt_broker="192.168.1.204" # FreeNAS broker
mqtt_broker="192.168.1.45" # RPI broker
#mqtt_broker="iot.eclipse.org"
#mqtt_broker="mqtt.eclipse.org"
mqtt_user = 'ofiriot'
mqtt_password = 'ofiriot'
mqtt_topic = 'esp32/test'

def on_log(client, userdata, level, buf):
    print("log: " + buf)

def on_connect(client, userdata, flags, rc): # called when the broker responds to our connection request
    if rc==0:
        print("connected OK")
    else:
        print("Bad connection Returned code=", rc)

def on_message(client, userdata, message):
    """The callback for when a PUBLISH message is received from the server."""
    print(message.topic + ' ' + str(message.payload.decode("utf-8")))
    #print("message received " ,str(message.payload.decode("utf-8")))
    #print("message topic=",message.topic)
    #print("message qos=",message.qos)
    #print("message retain flag=",message.retain)

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed. 
    client.subscribe(mqtt_topic)

def on_subscribe(client, userdata, mid, granted_qos):
    print("Subscribed:", str(mid),str(granted_qos))

def on_unsubscribe(client, userdata,mid):
    print('Unsubscribed:',str(mid))

def on_disconnect(client,userdata,rc): # called when the client disconnects from the broker
    if rc != 0:
        print("Unexpected Disconnection")

def main():
    print("creating new instance")
    mqtt_client = mqtt.Client("test-mqtt") #create new instance
    #mqtt_client.username_pw_set(mqtt_user, mqtt_password)
    mqtt_client.on_message=on_message #attach function to callback
    mqtt_client.on_connect = on_connect
    #mqtt_client.on_log = on_log # this is used to get log messages
    print("connecting to broker")
    mqtt_client.connect(mqtt_broker, 1883) #connect to broker
    print("Subscribing to topic",mqtt_topic)
    mqtt_client.subscribe(mqtt_topic)
    mqtt_client.publish(mqtt_topic,"hello from pc")
    mqtt_client.loop_forever()

# client.loop_start() #start the loop

# print("Publishing message to topic","house/bulbs/bulb1")
# client.publish("house/bulbs/bulb1","OFF")

# time.sleep(4) # wait
# client.loop_stop() #stop the loop

if __name__ == '__main__':
    print('Test MQTT to controller')
    main()
