import paho.mqtt.client as mqtt # Import the MQTT client library from paho-mqtt
import time # Import the time module (though not extensively used in this receiver-only script)

# --- Monitoring Configuration ---
MONITOR_DEV_BOARD = False # Set to True to monitor the development board, False for the production board
#---------------------------------

# --- MQTT Broker Configuration ---
# You can uncomment and switch between brokers as needed during development.
# mqtt_broker="192.168.1.204" # FreeNAS broker
mqtt_broker="192.168.1.45" # Raspberry Pi broker - Current target
# mqtt_broker="iot.eclipse.org" # Public test broker
# mqtt_broker="mqtt.eclipse.org" # Another public test broker

mqtt_port = 1883 # Default MQTT port
mqtt_keepalive = 60 # Maximum period in seconds between communications with the broker

# --- Authentication (Uncomment if your broker requires username/password) ---
# mqtt_user = 'ofiriot'
# mqtt_password = 'ofiriot'

# --- Topics to Subscribe To ---
# Define production topics
mqtt_topic_smart_garden_prod = 'smart-garden/#'
mqtt_topic_test_prod = 'test_topic'

# Define development topics
mqtt_topic_smart_garden_dev = 'smart-garden-dev/#'
mqtt_topic_test_dev = 'test_topic_dev'

if MONITOR_DEV_BOARD:
    mqtt_topic_smart_garden = mqtt_topic_smart_garden_dev
    mqtt_topic_test = mqtt_topic_test_dev
    print("Monitoring Development Board Topics")
else:
    mqtt_topic_smart_garden = mqtt_topic_smart_garden_prod
    mqtt_topic_test = mqtt_topic_test_prod
    print("Monitoring Production Board Topics")

# Using wildcards for broader subscription, which is efficient for receiving related messages.

# --- Callback Functions ---
# These functions are called by the paho-mqtt client library in response to MQTT events.

def on_log(client, userdata, level, buf):
    """
    Callback function for logging.
    Prints verbose logs from the MQTT client, useful for debugging connection issues.
    Uncomment `mqtt_client.on_log = on_log` in main() to enable.
    """
    print(f"LOG: {buf}")

def on_connect(client, userdata, flags, rc):
    """
    Callback function executed when the client connects to the MQTT broker.
    It checks the connection result code (rc).
    """
    if rc == 0:
        print("Connected to MQTT Broker successfully!")
        # Re-subscribe to topics upon successful reconnection.
        # This is crucial for persistent sessions (clean_session=False) or if the broker
        # does not store subscriptions for clean_session=True.
        # Although client.subscribe() is called in main(), putting it here ensures
        # subscriptions are renewed if the connection drops and then reconnects.
        # The PubSubClient library (used on ESP32) often handles this differently,
        # but for paho-mqtt, re-subscribing in on_connect is a robust practice.
        print(f"Re-subscribing to: {mqtt_topic_smart_garden}")
        client.subscribe(mqtt_topic_smart_garden)
        print(f"Re-subscribing to: {mqtt_topic_test}")
        client.subscribe(mqtt_topic_test)
    else:
        print(f"Failed to connect to MQTT Broker. Return code: {rc}")
        # Refer to paho.mqtt.client.CONNACK_RC for common return codes:
        # 1: Incorrect protocol version
        # 2: Invalid client ID
        # 3: Server unavailable
        # 4: Bad username or password
        # 5: Not authorized

def on_message(client, userdata, message):
    """
    Callback function executed when a PUBLISH message is received from the broker.
    It decodes and prints the topic, payload, QoS, and retain flag of the message.
    """
    try:
        # Decode the payload from bytes to a UTF-8 string
        payload_decoded = message.payload.decode("utf-8")
        print(f"\n--- Message Received ---")
        print(f"Topic: {message.topic}")
        print(f"Payload: {payload_decoded}")
        print(f"QoS: {message.qos}")
        print(f"Retain Flag: {bool(message.retain)}") # Convert 0/1 to boolean for clarity
        print(f"------------------------")
    except Exception as e:
        print(f"Error decoding message payload: {e}")
        print(f"Raw payload: {message.payload}")

def on_subscribe(client, userdata, mid, granted_qos):
    """
    Callback function executed when the broker acknowledges a subscription request.
    'mid' is the message ID of the subscribe request.
    'granted_qos' is a list of QoS levels granted for the subscription topics.
    """
    print(f"Subscription acknowledged. Message ID: {mid}, Granted QoS: {granted_qos}")

def on_unsubscribe(client, userdata, mid):
    """
    Callback function executed when the broker acknowledges an unsubscribe request.
    'mid' is the message ID of the unsubscribe request.
    """
    print(f"Unsubscription acknowledged. Message ID: {mid}")

def on_disconnect(client, userdata, rc):
    """
    Callback function executed when the client disconnects from the broker.
    If 'rc' is non-zero, it indicates an unexpected disconnection.
    """
    if rc != 0:
        print(f"WARNING: Unexpected disconnection from MQTT Broker. Return code: {rc}")
    else:
        print("Disconnected from MQTT Broker gracefully.")

def main():
    """
    Main function to set up and run the MQTT client.
    """
    print("Creating new MQTT client instance...")
    # Create an MQTT client instance.
    # The client ID "test-garden-receiver" should be unique per client.
    # clean_session=True (default) means the broker will not store subscriptions
    # or undelivered QoS 1/2 messages for this client across sessions.
    mqtt_client = mqtt.Client("test-garden-receiver")

    # --- Assign Callback Functions ---
    mqtt_client.on_message = on_message
    mqtt_client.on_connect = on_connect
    mqtt_client.on_subscribe = on_subscribe
    mqtt_client.on_unsubscribe = on_unsubscribe
    mqtt_client.on_disconnect = on_disconnect
    # mqtt_client.on_log = on_log # Uncomment to enable verbose logging

    # --- Set Authentication Credentials (if required by broker) ---
    # if mqtt_user and mqtt_password:
    #     mqtt_client.username_pw_set(mqtt_user, mqtt_password)
    #     print("Username/password set for MQTT client.")

    print(f"Attempting to connect to MQTT broker at: {mqtt_broker}:{mqtt_port}")
    try:
        mqtt_client.connect(mqtt_broker, mqtt_port, mqtt_keepalive)
    except Exception as e:
        print(f"Failed to establish initial connection to broker: {e}")
        print("Please check broker address, port, and network connectivity.")
        return # Exit if initial connection fails

    # --- Initial Subscriptions ---
    # Subscribe to topics. QoS 0 is "at most once" delivery.
    # The subscribe method returns a tuple (result, mid).
    print(f"Subscribing to topic: '{mqtt_topic_smart_garden}'")
    mqtt_client.subscribe(mqtt_topic_smart_garden, qos=0)

    print(f"Subscribing to topic: '{mqtt_topic_test}'")
    mqtt_client.subscribe(mqtt_topic_test, qos=0)

    # --- Start the MQTT Loop ---
    # This method blocks the main thread and handles network traffic,
    # dispatches callbacks, and processes reconnections automatically.
    print("\nMQTT client loop started. Waiting for messages...")
    mqtt_client.loop_forever() # Keeps the client running indefinitely

if __name__ == '__main__':
    print('Starting MQTT Receiver Program for Smart Garden Controller')
    main()