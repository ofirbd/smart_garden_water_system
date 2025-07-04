import paho.mqtt.client as mqtt
import time

# MQTT Broker Details
BROKER_ADDRESS = "192.168.1.45"
PORT = 1883

# List of topics to clear (the old ones)
TOPICS_TO_CLEAR = [
    "schedule/set",
    "schedule/get",
    "schedule/state",
    "test_topic"
]

def on_connect(client, userdata, flags, rc):
    """The callback for when the client receives a CONNACK response from the server."""
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print(f"Failed to connect, return code {rc}\n")

def clear_topics(client):
    """Publishes an empty, retained message to clear the topic."""
    for topic in TOPICS_TO_CLEAR:
        print(f"Clearing topic: {topic}...")
        # Publish an empty payload with the retain flag set to True
        result = client.publish(topic, "", qos=1, retain=True)
        result.wait_for_publish() # Wait for the publish to complete
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            print(f"Successfully cleared topic: {topic}")
        else:
            print(f"Failed to clear topic: {topic}")
    print("\nAll specified topics have been cleared from the broker.")

def main():
    """Main function to connect and clear topics."""
    client = mqtt.Client("clear-retained-client")
    client.on_connect = on_connect

    print(f"Connecting to broker at {BROKER_ADDRESS}...")
    try:
        client.connect(BROKER_ADDRESS, PORT, 60)
    except Exception as e:
        print(f"Could not connect to MQTT Broker: {e}")
        return

    # Start the loop
    client.loop_start()
    time.sleep(1) # Give a moment to connect

    if client.is_connected():
        clear_topics(client)
    else:
        print("Could not connect to the broker. Aborting.")

    # Stop the loop and disconnect
    client.loop_stop()
    client.disconnect()
    print("Disconnected from broker.")

if __name__ == "__main__":
    main()
