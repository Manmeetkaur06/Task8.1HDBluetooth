import asyncio
from bleak import BleakClient
import RPi.GPIO as GPIO

# Define GPIO pin for LED
LED_PIN = 18  # Use a PWM-capable GPIO pin
GPIO.setmode(GPIO.BCM)
GPIO.setup(LED_PIN, GPIO.OUT)

# Set up PWM on the LED pin
pwm = GPIO.PWM(LED_PIN, 1000)  # Set PWM frequency to 1kHz
pwm.start(0)  # Initially, the LED is off

# Arduino Nano 33 IoT MAC address
device_address = "08:B6:1F:82:1B:FA"

# UUID of the characteristic that sends sensor data 
sensor_characteristic_uuid = "2A56"  

# Map sensor value to LED brightness (0-100%)
def map_sensor_to_brightness(sensor_value):
    return max(0, min(100, int((sensor_value / 1023) * 100)))  # Assuming sensor_value ranges from 0 to 1023

async def handle_notifications(sender, data):
    # Assuming the sensor data is transmitted as a short (2 bytes)
    sensor_value = int.from_bytes(data, byteorder='little')
    print(f"Received sensor value: {sensor_value}")

    # Map the sensor value to LED brightness
    brightness = map_sensor_to_brightness(sensor_value)
    print(f"Setting LED brightness to {brightness}%")

    # Set the LED brightness using PWM
    pwm.ChangeDutyCycle(brightness)

async def connect_and_receive_data():
    try:
        async with BleakClient(device_address) as client:
            print(f"Connected: {client.is_connected}")

            # Subscribe to notifications from the sensor characteristic
            await client.start_notify(sensor_characteristic_uuid, handle_notifications)
            print(f"Subscribed to {sensor_characteristic_uuid} notifications...")

            # Keep running to receive notifications
            while True:
                await asyncio.sleep(1)

    except Exception as e:
        print(f"An error occurred: {e}")
    finally:
        # Stop the PWM and clean up GPIO when the script is done
        pwm.stop()
        GPIO.cleanup()

# Run the BLE connection and notification handling
loop = asyncio.get_event_loop()
loop.run_until_complete(connect_and_receive_data())
