#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);

void setup() {
  Serial.begin(9600);
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
    Serial.println("MCP2515 Initialized Successfully!");
  } else {
    Serial.println("Error Initializing MCP2515...");
  }
  // Change to normal mode to allow messages to be transmitted
  CAN.setMode(MCP_NORMAL);
}

unsigned char data[] = { 'A' };
bool status = false;

void loop() {
  // send data:  ID = 0x01, Standard CAN Frame, Data length = 1 bytes, 'data' = array of data bytes to send
  data[0] = status ? 'A' : 'S';
  byte sndStat = CAN.sendMsgBuf(0x01, 0, sizeof(data), data);
  if(sndStat != CAN_OK){
    Serial.println("Error Sending Message...");
  }
  status = !status;
  Serial.print("TEC: ");
  Serial.print(CAN.errorCountTX());
  Serial.print("\tREC: ");
  Serial.println(CAN.errorCountRX());
  delay(1000);
}
