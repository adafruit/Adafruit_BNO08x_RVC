#if defined(__AVR__) || defined(ESP8266)
// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (WHITE wire)
// Set up the serial port to use softwareserial..
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, -1);
// void parseFrame(uint8_t * buffer, size_t len);
#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is white wire, data input
#define mySerial Serial1

#endif
#define MILLI_G_TO_MS2 0.0098067
#define DEGREE_SCALE 0.01
#define BUFFER_SIZE 64
int16_t raw_accel_x, raw_accel_y, raw_accel_z, raw_yaw, raw_roll,
    raw_pitch;               // The last measured distance
bool newData = false;        // Whether new data is available from the sensor
uint8_t buffer[BUFFER_SIZE]; // our buffer for storing data
uint8_t idx = 0;             // our idx into the storage buffer
float yaw, pitch, roll, x_accel, y_accel, z_accel;
bool started = false;
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    delay(
        10); // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Adafruit BNO08x UART-RVC Test");

  // set the data rate for the Serial port, 9600 for the sensor
  mySerial.begin(115200);
}

void loop() { // run over and over
  uint8_t available = mySerial.available();
  if (available < 2) {
    return;
  }
  buffer[0] = mySerial.read();

  if (buffer[0] == 0xAA) {
    buffer[1] = mySerial.read();

    if (buffer[1] == 0xAA) {
      if (parseFrame()) {
        printFrame();
      }
    }
  }
}

bool parseFrame() {
  uint8_t crc_calc = 0;
  uint8_t c;
  // read the next 17 bytes
  for (int i = 0; i < 17; i++) {
    while (!mySerial.available()) {
      delay(1);
    }
    buffer[i] = mySerial.read();
    if (i < 16) {
      crc_calc += buffer[i];
    }
  }

  // verify CRC
  if (crc_calc != buffer[16]) {
    Serial.println("********** BAD CRC ************");
    return false;
  }

  raw_accel_x = ((buffer[2] << 8) | buffer[1]);
  raw_accel_y = ((buffer[4] << 8) | buffer[3]);
  raw_accel_z = ((buffer[6] << 8) | buffer[5]);
  raw_yaw = ((buffer[8] << 8) | buffer[7]);
  raw_roll = ((buffer[10] << 8) | buffer[9]);
  raw_pitch = ((buffer[12] << 8) | buffer[11]);

  yaw = (float)raw_accel_x * DEGREE_SCALE;
  pitch = (float)raw_accel_y * DEGREE_SCALE;
  roll = (float)raw_accel_z * DEGREE_SCALE;
  x_accel = (float)raw_yaw * MILLI_G_TO_MS2;
  y_accel = (float)raw_roll * MILLI_G_TO_MS2;
  z_accel = (float)raw_pitch * MILLI_G_TO_MS2;
  // 13,14, 15=reserved, 16=
  return true;
}

void printFrame() {
  Serial.print("Yaw: ");
  Serial.print(yaw);
  Serial.print(" Pitch: ");
  Serial.print(pitch);
  Serial.print(" Roll: ");
  Serial.print(roll);
  Serial.println(" degrees");

  Serial.print("Acceleration:");
  Serial.print("X: ");
  Serial.print(x_accel);
  Serial.print(" Y: ");
  Serial.print(y_accel);
  Serial.print(" Z: ");
  Serial.print(z_accel);

  Serial.println(" m/s^2");
  Serial.println("");
}

