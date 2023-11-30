#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 4;
MCP_CAN CAN(SPI_CS_PIN);

/*
 * NODE B
 * Send messages with ID=0x11 every 10ms
 */

void setup() {
  //pinMode(3, OUTPUT);
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

unsigned long rxId;
byte len = 0;
unsigned char buf[8];


long interval = 0, prev_interval = 0;
long sum_interval = 0;
int count = 0, max_count = 1000;

bool done = false;

void loop() {
  // Read data: len = data length, buf = data byte(s)
  if (!done) {
    analyzePacketFreq(0x11);
    done = true;
  }

  //packetSniff();
}

void analyzePacketFreq(unsigned long id) {
  count = 0;
  prev_interval = 0;
  sum_interval = 0;
  while (count < max_count) {
    if (CAN_MSGAVAIL == CAN.checkReceive() && CAN_OK == CAN.readMsgBuf(&rxId, &len, buf)) {
      if (rxId == id && (rxId & 0x40000000) != 0x40000000) {
        interval = millis();
        long delta = interval - prev_interval;
        //Serial.println(delta);
        prev_interval = interval;
        sum_interval += delta;
        count++;
      }
    }
  }
  long delta = sum_interval / max_count;
  Serial.print("Packet with id 0x");
  Serial.print(id, HEX);
  Serial.print(" sent every ");
  Serial.print(delta);
  Serial.println(" ms");
}

void packetSniff() {
  while (true) {
    if (CAN_MSGAVAIL == CAN.checkReceive() && CAN_OK == CAN.readMsgBuf(&rxId, &len, buf)) {
      if ((rxId & 0x40000000) == 0x40000000) {
        Serial.println("REMOTE REQUEST FRAME");
      } else {
        Serial.print("ID: ");
        Serial.print(rxId);
        Serial.print("\t");
        Serial.print("DLC: ");
        Serial.print(len);
        Serial.print("\t");
        for (byte i = 0; i < len; i++) {
          Serial.write(buf[i]);
          Serial.print("\t");
        }
        Serial.println();
      }
    }
  }
}


