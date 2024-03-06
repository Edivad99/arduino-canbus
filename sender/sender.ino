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
int time = 10; //10

unsigned long startMillis;
unsigned long currentMillis;

void loop() {
  /*currentMillis = millis();
  if (currentMillis - startMillis >= 10) {
    sendMessage(0x07, 4, data1);
    sendMessage(0x09, 4, data2);
    startMillis = currentMillis;
  }*/


  sendMessage(0x07, 4, data1);
  sendMessage(0x09, 4, data2);
  delay(time);
  //packetSniff();
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


unsigned long rxId;
byte len = 0;
unsigned char buf[8];

void packetSniff() {
  if (CAN_MSGAVAIL == CAN.checkReceive() && CAN_OK == CAN.readMsgBuf(&rxId, &len, buf)) {
    if ((rxId & 0x40000000) == 0x40000000) {
      Serial.println("REMOTE REQUEST FRAME");
    } else {
      Serial.print("ID: 0x");
      Serial.print(rxId < 16 ? "0" : "");
      Serial.print(rxId, HEX);
      Serial.print("\tDLC: ");
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
