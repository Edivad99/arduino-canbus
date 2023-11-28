#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);

void setup() {
  pinMode(3, OUTPUT);
  Serial.begin(9600);
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
    Serial.println("MCP2515 Initialized Successfully!");
  } else {
    Serial.println("Error Initializing MCP2515...");
  }
  // Set operation mode to normal so the MCP2515 sends acks to received data.
  CAN.setMode(MCP_NORMAL);
}

long unsigned int rxId;
byte len = 0;
unsigned char buf[8];

void loop() {
  // Read data: len = data length, buf = data byte(s)
  CAN.readMsgBuf(&rxId, &len, buf);

  // Determine if message is a remote request frame.
  if ((rxId & 0x40000000) == 0x40000000) {
    Serial.println("REMOTE REQUEST FRAME");
  } else {
    for (byte i = 0; i < len; i++) {
      Serial.write(buf[i]);
      Serial.print("\t");
    }
    Serial.println();
    if (rxId == 1) {
      if (buf[0] == 'A') {
        digitalWrite(3, HIGH);
      } else if (buf[0] == 'S') {
        digitalWrite(3, LOW);
      }
    }
  }
}
