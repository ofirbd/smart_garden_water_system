import paho.mqtt.client as mqtt
import time
import json

# MQTT Broker Details
BROKER_ADDRESS = "192.168.1.45"
PORT = 1883
STATUS_TOPIC = "smart-garden/controller/status"
SCHEDULE_SET_TOPIC = "schedule/set"
SCHEDULE_GET_TOPIC = "schedule/get"
SCHEDULE_STATE_TOPIC = "schedule/state"
TEST_TOPIC = "test_topic"

# Global variable to store the schedule payload
current_schedule_payload = ""

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")
    client.subscribe(STATUS_TOPIC)
    client.subscribe(SCHEDULE_GET_TOPIC) # Subscribe to schedule requests
    client.subscribe(SCHEDULE_SET_TOPIC) # Subscribe to schedule set topic to see what ESP32 receives

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global current_schedule_payload
    print(f"Message received on topic {msg.topic}: {msg.payload.decode()}")
    
    if msg.topic == SCHEDULE_GET_TOPIC:
        print(f"Python client received GET request on {msg.topic}. Responding with schedule.")
        client.publish(SCHEDULE_SET_TOPIC, current_schedule_payload, qos=1, retain=True)
        print(f"Python client published schedule to {SCHEDULE_SET_TOPIC}.")
    elif msg.topic == SCHEDULE_SET_TOPIC:
        print(f"Python client received SET message on {msg.topic}. Payload: {msg.payload.decode()}")


def main():
    global current_schedule_payload

    client = mqtt.Client("schedule-test-client")
    client.on_connect = on_connect
    client.on_message = on_message

    print(f"Connecting to broker at {BROKER_ADDRESS}...")
    try:
        client.connect(BROKER_ADDRESS, PORT, 60)
    except Exception as e:
        print(f"Could not connect to MQTT Broker: {e}")
        return

    client.loop_start()
    time.sleep(2) # Wait for connection and status message

    # Define a sample schedule
    schedule = {
        "enabled": True,
        "programs": [
            {
                "enabled": True,
                "days": ["Monday", "Wednesday", "Friday"],
                "actions": [
                    { "valve": 1, "time": "08:00", "duration_minutes": 15 },
                    { "valve": 2, "time": "08:20", "duration_minutes": 10 }
                ]
            },
            {
                "enabled": True,
                "days": ["Sunday"],
                "actions": [
                    { "valve": 3, "time": "19:00", "duration_minutes": 30 }
                ]
            }
        ]
    }

    # Convert the schedule to a JSON string and store globally
    current_schedule_payload = json.dumps(schedule)

    print(f"\nPublishing initial schedule to topic: {SCHEDULE_SET_TOPIC}")
    print(current_schedule_payload)
    
    # Publish the schedule with retain=True so it's always available
    client.publish(SCHEDULE_SET_TOPIC, current_schedule_payload, qos=1, retain=True)
    time.sleep(5) # Give ESP32 more time to process the schedule message

    print("\nSchedule published. The ESP32 should now have the new schedule.")

    # Publish a test message to the new test topic
    test_message = "Hello from Python test client!"
    print(f"\nPublishing test message to topic: {TEST_TOPIC}")
    client.publish(TEST_TOPIC, test_message)

    print("\nTest message published.")

    print("You can restart the ESP32 to see if it loads the schedule from its flash memory.")

    # Keep the client running for a longer period to respond to GET requests
    print("\nKeeping Python client alive for 30 seconds to respond to GET requests...")
    time.sleep(30)

    client.loop_stop()
    client.disconnect()
    print("\nDisconnected from broker.")

if __name__ == "__main__":
    main()
