#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);

/*
 * NODE B
 * Send messages with ID=0x11 every 10ms
 */

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

unsigned char data[] = { 'L', 'M', 'N', 'O', 'P' };

void loop() {
  sendMessage(0x11, 5, data);
  if (isInError()) {
    digitalWrite(3, HIGH);
  } else {
    digitalWrite(3, LOW);
  }
  delay(10);
}

void sendMessage(unsigned long id, byte len, unsigned char* buf) {
  byte res = CAN.sendMsgBuf(id, len, buf);
  if (res != CAN_OK) {
    Serial.print("Error sending message 0x");
    Serial.print(id < 16 ? "0" : "");
    Serial.print(id, HEX);
  }
  if (res == CAN_GETTXBFTIMEOUT) {
    Serial.println(", get TX buff time out!");
  } else if (res == CAN_SENDMSGTIMEOUT) {
    Serial.println(", send msg timeout!");
  }
}
 
bool isInError() {
  byte tec = CAN.errorCountTX();
  byte rec = CAN.errorCountRX();
  if (tec != 0 || rec != 0) {
    Serial.print("TEC: ");
    Serial.print(tec);
    Serial.print(", REC: ");
    Serial.println(rec);
    return true;
  }
  return false;
}
