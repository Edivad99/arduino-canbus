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
bool error = false;

void loop() {
  sendMessage(0x11, data);
  delay(10);
  if (error) {
    digitalWrite(3, HIGH);
    printErrorCounter();
  } else {
    digitalWrite(3, LOW);
  }
}

void sendMessage(unsigned long id, unsigned char* buf) {
  byte sndStat = CAN.sendMsgBuf(id, 0, 4, buf);
  if (sndStat != CAN_OK) {
    Serial.print("Error sending message ");
    Serial.print(id);
    Serial.println("!");
    error = true;
  }
}
 
void printErrorCounter() {
  byte tec = CAN.errorCountTX();
  byte rec = CAN.errorCountRX();
  Serial.print("TEC: ");
  Serial.print(tec);
  Serial.print(", REC: ");
  Serial.println(rec);
  if (tec == 0 && rec == 0) {
    error = false;
    Serial.println("NO ERROR");
  }
}
