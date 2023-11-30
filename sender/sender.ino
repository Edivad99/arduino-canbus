#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);

/*
 * NODE A
 * Send 2 consecutive messagges with ID=0x07 and 0x09 every 10ms
 */

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

unsigned char data1[] = { 'A', 'B', 'C', 'D' };
unsigned char data2[] = { 'E', 'F', 'G', 'H' };
bool error = false;
int time = 10; //10

void loop() {
  sendMessage(0x07, data1);
  delay(time);
  sendMessage(0x09, data2);
  delay(time);
  if (error) {
    printErrorCounter();
  }
}

void sendMessage(unsigned long id, unsigned char* buf) {
  byte sndStat = CAN.sendMsgBuf(id, 0, 4, buf);
  if (sndStat != CAN_OK) {
    Serial.print("Error sending message ");
    Serial.print(id);
    Serial.println("!");
    printErrorCounter();
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
