#include <Wire.h>
#include <BH1750.h>
#include <ArduinoBLE.h>

// Create BH1750 light sensor object
BH1750 lightSensor;

// Define BLE service and characteristic UUIDs
BLEService sensingService("180C");  // Custom BLE service UUID for the sensor
BLEUnsignedShortCharacteristic lightCharacteristic("2A56", BLERead | BLENotify);  // Characteristic for sending sensor data

void setup() {
  Serial.begin(9600);
  while (!Serial);  // Wait for serial monitor to connect

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);  // Halt on failure
  }

  // Setup BLE properties
  BLE.setLocalName("ParkingSensor");  // Device name visible via Bluetooth
  BLE.setAdvertisedService(sensingService);  // Set service to advertise
  sensingService.addCharacteristic(lightCharacteristic);  // Add characteristic to the service
  BLE.addService(sensingService);  // Add the service to BLE
  BLE.advertise();  // Start BLE advertising

  Serial.println("BLE device is now advertising...");

  // Initialize BH1750 light sensor
  Wire.begin();
  if (!lightSensor.begin()) {
    Serial.println("BH1750 initialization failed");
    while (1);  // Halt on failure
  }

  Serial.println("BH1750 sensor initialized");
}

void loop() {
  // Wait for a central device to connect
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    // While the central is connected, read sensor data and send via BLE
    while (central.connected()) {
      float lux = lightSensor.readLightLevel();  // Read light level from BH1750
      int mappedValue = map(lux, 0, 65535, 0, 1023);  // Map lux value to a 0-1023 scale

      // Write sensor value to the BLE characteristic and notify the central
      lightCharacteristic.writeValue(mappedValue);
      Serial.print("Light level (lux): ");
      Serial.print(lux);
      Serial.print(" | Mapped value: ");
      Serial.println(mappedValue);

      delay(500);  // Delay between readings to avoid overloading the central device
    }

    // Notify when the central disconnects
    Serial.println("Disconnected from central");
  }
}
