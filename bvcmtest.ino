//#include <MsTimer2.h>
#include <SevenSeg.h>
#define resetbtn 2
#define clearbtn 3
#define coinmech 4
const int numOfDigits = 4;
unsigned long flicker_delay = 500;
unsigned long delaytime = 0;
unsigned long delaytime2 = 0;
SevenSeg disp(36, 28, 22, 42, 40, 34, 24);
SevenSeg disp2(37, 29, 23, 43, 41, 35, 25);
int binledpins[] = {53, 52, 51, 50, 49, 48, 47, 46};
int dispdigitpins[] = {38, 32, 30, 26};
int disp2digitpins[] = {39, 33, 31, 27};
unsigned long poll_interval = 100;
int esc_count = 0;
unsigned long ta = 0;
byte reset[] = { 0x02, 0x00, 0x20, 0xff, 0xd4, 0x8a, 0x03 };
byte standby[] = { 0x02, 0x00, 0x20, 0x58, 0xa7, 0x40, 0x1D, 0x03 };
byte standby_cm[] = { 0x02, 0x00, 0x20, 0x60, 0xA6, 0xDC, 0x03 };
byte inreq[] = { 0x02, 0x00, 0x20, 0x5a, 0xa5, 0x06, 0x3d, 0x03 };
byte inreq_cm[] = { 0x02, 0x00, 0x20, 0x62, 0x86, 0x9E, 0x03 };
byte start_bv[] = { 0x02, 0x00, 0x20, 0x5B, 0x7D, 0xE3, 0x10, 0x31, 0x00, 0x22, 0xEF, 0x3E, 0x03 };
byte start_three_bv[] = {0x02, 0x00, 0x20 , 0x5B , 0x7D , 0xE3 , 0x10 , 0xE1 , 0x00 , 0xF2 , 0x41 , 0xD7 , 0x03 };
byte start_cm[] = { 0x02, 0x00, 0x20, 0x63, 0x7D, 0xE2, 0x00, 0x01, 0x7D, 0xE3, 0xDD, 0xF2, 0x03 };
byte insertclear_bv[] = { 0x02, 0x00, 0x20, 0x5b, 0x7d, 0xe3, 0x10, 0x22, 0x00, 0x31, 0xd7, 0x5f, 0x03 };
byte insertclear_cm[] = { 0x02, 0x00, 0x20, 0x63, 0x7D, 0xE2, 0x00, 0x7D, 0xE2, 0x00, 0xB8, 0xC2, 0x03 };
byte alldata_bv[] = { 0x02, 0x00, 0x20, 0x59, 0x01, 0xA6, 0x03 };
byte alldata_cm[] = { 0x02, 0x00, 0x20, 0x61, 0xB6, 0xFD, 0x03 };
byte inhibit_bv[] = { 0x02, 0x00, 0x20, 0x5B, 0x7D, 0xE3, 0x10, 0x00, 0x00, 0x13, 0x3B, 0xD9, 0x03 };
byte inhibit_cm[] = { 0x02, 0x00, 0x20, 0x63, 0x7D, 0xE2, 0x00, 0x00, 0x7D, 0xE2, 0xFE, 0xE2, 0x03 };
byte bill_return_1000[] = { 0x02, 0x00, 0x20, 0x5B, 0x05, 0x11, 0x00, 0x10, 0x00, 0x00, 0x04, 0x26, 0xAE, 0x03 };
byte bill_return_10000[] = {0x02, 0x00, 0x20, 0x5B, 0x05, 0x11, 0x00, 0x00, 0x01, 0x00, 0x15, 0x08, 0x29, 0x03 };
byte recieve[256] ;
byte buff_escaped[256] ;
int leng = 0;
int leng_escaped = 0;
long credit_bill = 0;
long credit_coin = 0;
long credit = 0;
byte bv_status_code[2] = {0};
byte cm_status_code = 0;
byte bv_error_code = 0;
bool escrow = false;
bool escrow_5000 = false;
bool escrow_10000 = false;
byte bv_version = 0x00;
byte bv_maker = 0x00;
byte cm_version = 0x00;
byte cm_maker = 0x00;
int j = 0;
byte esc_data_temp = 0x00;
char cmd;

void setup() {
  Serial.begin(38400);
  Serial1.begin(38400);
  disp2.setDigitPins(numOfDigits, disp2digitpins);
  disp.setDigitPins(numOfDigits, dispdigitpins);
  disp2.setDPPin(45);
  disp.setDPPin(44);
  disp2.setCommonAnode();
  disp.setCommonAnode();
  disp.setDigitDelay(1667);
  disp2.setDigitDelay(1667);
  for (int i = 0; i < (sizeof(binledpins) / sizeof(binledpins[0])); i++) {
    pinMode(binledpins[i], OUTPUT);
  }
  pinMode(resetbtn, INPUT_PULLUP);
  pinMode(clearbtn, INPUT_PULLUP);
  pinMode(coinmech, OUTPUT);
  digitalWrite(coinmech,HIGH);
  //MsTimer2::set(poll_interval, polling);
  //MsTimer2::start();
  ta=millis();
  if (digitalRead(resetbtn))
    bvcm_reset();
  else {
    while (!digitalRead(resetbtn)) disp2.write("rst.");
    data_refresh();
  }
  digitalWrite(coinmech,HIGH);
}

void loop() {
  digit_refresh();
  bv_status_write_binled();
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    cmd = Serial.read();
    if (cmd == 'c') {
      credit_clear();
    }
    else if (cmd == 'r') {
      bvcm_reset();
    }
  }
  if (digitalRead(resetbtn) == LOW) {
    unsigned long t = millis();
    while (digitalRead(resetbtn) == LOW) {
      if ((millis() - t) > 4000) bvcm_reset();
    };

  }
  else if (digitalRead(clearbtn) == LOW) {
    unsigned long t = 0;
    while (digitalRead(clearbtn) == LOW)t = millis();
    while (1) {
      disp2.write("CLR");
      if ((millis() - t) > 500) break;
    }
    credit_clear();
  }
  /*
  if(credit >= 500){
    while(1){
      Serial1.write(inhibit_bv,sizeof(inhibit_bv));
      Serial1.write(inhibit_cm,sizeof(inhibit_cm));
      credit_bill = 0;
      credit_coin = 0;
      for(int i=0; i <= (int)credit / 500; i++){
        digitalWrite(coinmech,LOW);
        delay(100);
        digitalWrite(coinmech,HIGH);
        delay(500);
        credit = credit - 500;
        digit_refresh(); 
      }
      credit_clear();
      break;
    }
  }*/
  if((millis() - ta) > poll_interval){ polling(); ta=millis();}
}
void credit_clear() {
  escrow = false;
  escrow_5000 = false;
  escrow_10000 = false;
  Serial1.write(inhibit_bv, sizeof(inhibit_bv));
  Serial1.write(inhibit_cm, sizeof(inhibit_cm));
  delay(100);
  Serial.println("Clear");
  Serial1.write(insertclear_bv, sizeof(insertclear_bv));
  Serial1.write(insertclear_cm, sizeof(insertclear_cm));
  credit_coin = 0;
  credit_bill = 0;
  credit = 0;
  delay(100);
  Serial1.write(start_three_bv, sizeof(start_three_bv));
  Serial1.write(start_cm, sizeof(start_cm));
}
void data_refresh() {
  Serial1.write(alldata_bv, sizeof(alldata_bv));
  Serial1.write(alldata_cm, sizeof(alldata_cm));
}
void serialEvent1() {
  j = 0;
  if (Serial1.available()) {
    leng = Serial1.readBytesUntil(0x03, recieve, 1024);
    leng_escaped = leng - 1;
    esc_count = 0;
  }
  if (leng > 4) {
    if (recieve[3] == 0xFD) {
      Serial.println("Data recieve timeout");
      disconnect_retry();
    }
    else if (recieve[3] == 0x44) {
      stand_by();
    }
    for (int i = 1; i < leng; i++) {

      if (recieve[i] == 0x7D && recieve[i + 1] == 0xE3 )
      {
        esc_data_temp = 0x03;
        i++;
        leng_escaped--;
      }
      else if (recieve[i] == 0x7D && recieve[i + 1] == 0xE2)
      {
        esc_data_temp = 0x02;
        i++;
        leng_escaped--;
      }
      else {
        esc_data_temp = recieve[i];
      }
      buff_escaped[j] = esc_data_temp;
      j++;
    }
    for (int i = 0; i < leng_escaped; i++) {
      if (buff_escaped[i] == 0x18 && escrow == false) {
        if (buff_escaped[i - 1] == 0x05)
        {
          if (buff_escaped[i + 1] > 0)
          {
            credit_bill += buff_escaped[i + 1] * 1000;
            escrow = true;
          }
          if (buff_escaped[i + 2] > 0)
          {
            credit_bill += buff_escaped[i + 2] * 5000;
            escrow_5000 = true;
          }
          if (buff_escaped[i + 3] > 0)
          {
            credit_bill += buff_escaped[i + 3] * 10000;
            escrow_10000 = true;
          }
          credit = credit_bill + credit_coin;
          Serial.println(credit);
        }
      }
      if (buff_escaped[i] == 0x08 && buff_escaped[i - 1] == 0x05)
      {
        credit_coin = (BcdToDec(buff_escaped[i + 1]) * 10) + (BcdToDec(buff_escaped[i + 2]) * 50) + (BcdToDec(buff_escaped[i + 3]) * 100) + (BcdToDec(buff_escaped[i + 4]) * 500);
        credit = credit_bill + credit_coin;
        Serial.println(credit);
        break;
      }
      if (buff_escaped[i] == 0x0B && buff_escaped[i - 1] == 0x02) {
        cm_status_code = buff_escaped[i + 1];
        Serial.print("CM Status Code = ");
        printHex(cm_status_code, 2);
        Serial.print(" (");
        print_binary(cm_status_code, 8);
        Serial.println(")");
        if (GetBit(cm_status_code, 3)) {
          return_price(credit_bill + credit_coin);
        }
      }
      if (buff_escaped[i] == 0x03 && buff_escaped[i + 1] == 0x1C)
      {
        bv_error_code = buff_escaped[i + 2];
        Serial.print("BV Error Code = ");
        printHex(bv_error_code, 2);
        Serial.print(" (");
        print_binary(bv_error_code, 8);
        Serial.println(")");
        if (GetBit(buff_escaped[i + 2], 4))
        {
          Serial.println("Bill Jammed");
        }
      }
      if (buff_escaped[i] == 0x04 && buff_escaped[i + 1] == 0x1B)
      {
        bv_status_code[0] = buff_escaped[i + 2];
        bv_status_code[1] = buff_escaped[i + 3];
        Serial.print("BV Status Code (2byte) = ");
        printHex(bv_status_code[0], 2);
        Serial.print(", ");
        printHex(bv_status_code[1], 2);
        Serial.print(" (");
        print_binary(bv_status_code[0], 8);
        Serial.print(", ");
        print_binary(bv_status_code[1], 8);
        Serial.println(")");
        if (!GetBit(buff_escaped[i + 2], 0))
        {
          Serial1.write(start_three_bv, sizeof(start_three_bv));
        }
        if (GetBit(buff_escaped[i + 2], 1))
        {
          Serial.println("Stacker is opened.");
        }
        if (GetBit(buff_escaped[i + 2], 5))
        {
          Serial.println("Identifying...");
        }
        if (GetBit(buff_escaped[i + 2], 7))
        {
          Serial.println("Returning..");
        }
        if (escrow == true || escrow_5000 == true || escrow_10000 == true)
        {
          //Serial.write("Clear");
          if (GetBit(buff_escaped[i + 2], 5))
          {
            escrow = false;
            escrow_5000 = false;
            escrow_10000 = false;
            Serial1.write(insertclear_bv, sizeof(insertclear_bv));
            Serial1.write(start_three_bv, sizeof(start_three_bv));
          }
        }
      }

      if (buff_escaped[i] == 0x03 && buff_escaped[i + 1] == 0x1D)
      {
        bv_version = buff_escaped[i + 2];
        bv_maker = buff_escaped[i + 3];
        Serial.print("BV Software Ver = ");
        printHex(bv_version, 2);
        Serial.print(", ");
        Serial.print("BV Maker code = ");
        printHex(bv_maker, 2);
        Serial.println();
      }
      if (buff_escaped[i] == 0x03 && buff_escaped[i + 1] == 0x0D)
      {
        cm_version = buff_escaped[i + 2];
        cm_maker = buff_escaped[i + 3];
        Serial.print("CM Software Ver = ");
        printHex(cm_version, 2);
        Serial.print(", ");
        Serial.print("CM Maker code = ");
        printHex(cm_maker, 2);
        Serial.println();
      }

    }
    //    for (int i = 0; i < leng_escaped; i++) {
    //      printHex(buff_escaped[i], 2);
    //      Serial.print(" ");
    //    }

    //Serial.println();
    /*if (recieve[3] == 0x11) {
      Serial.println("ACK1");
      }
      else if (recieve[3] == 0x22) {
      Serial.println("ACK2");

      }
      else if (recieve[3] == 0x33) {
      Serial.println("ACK3");
      }
      else if (recieve[3] == 0x44) {
      Serial.println("ACK4");
      }
      else if (recieve[3] == 0x55) {
      Serial.println("ACK5");
      }
      else if (recieve[3] == 0xEE) {
      Serial.println("NAK");
      }*/
  }
}
void polling() {
  Serial1.write(inreq, sizeof(inreq));
  Serial1.write(inreq_cm, sizeof(inreq_cm));
}
void printHex(int num, int precision) {
  char tmp[16];
  char format[128];

  sprintf(format, "0x%%.%dX", precision);

  sprintf(tmp, format, num);
  Serial.print(tmp);
}
bool GetBit(byte b, int bitNumber)
{
  return (b & (1 << bitNumber)) != 0;
}
int BcdToDec(byte bcd)
{
  return ( ((bcd >> 4) * 10) + (bcd & 0xF) );
}

void return_price(int returnprice) {
  Serial.println("Returning...");
  Serial1.write(inhibit_bv, sizeof(inhibit_bv));
  Serial1.write(inhibit_cm, sizeof(inhibit_cm));
  delay(100);
  int returned = 0;
  int won10 = 0;
  int won50 = 0;
  int won100 = 0;
  int won500 = 0;
  int won1000 = 0;
  int won5000 = 0;
  int won10000 = 0;
  if (returnprice >= 10000) {
    won10000 = 1;
    returnprice = returnprice - 10000;
    returned = returned + 10000;
    Serial1.write(bill_return_10000, sizeof(bill_return_10000));
  }
  else if (returnprice >= 5000 && returnprice < 10000) {
    won5000 = 1;
    returnprice = returnprice - 5000;
    returned = returned + 5000;
  }
  else if (returnprice >= 1000 && returnprice < 5000) {
    won1000 = 1;
    returnprice = returnprice - 1000;
    returned = returned + 1000;
    Serial1.write(bill_return_1000, sizeof(bill_return_1000));
  }
  won500 = returnprice;
  escrow = false;
  escrow_5000 = false;
  escrow_10000 = false;
  //credit_bill = 0;
  credit_clear();
}
void print_binary(int number, byte Length) {
  static int Bits;
  if (number) { //The remaining bits have a value greater than zero continue
    Bits++; // Count the number of bits so we know how many leading zeros to print first
    print_binary(number >> 1, Length); // Remove a bit and do a recursive call this function.
    if (Bits) for (byte x = (Length - Bits); x; x--)Serial.write('0'); // Add the leading zeros
    Bits = 0; // clear no need for this any more
    Serial.write((number & 1) ? '1' : '0'); // print the bits in reverse order as we depart the recursive function
  }

}
void bvcm_reset() {
  //MsTimer2::stop();
  credit_coin = 0;
  credit_bill = 0;
  escrow = false;
  escrow_5000 = false;
  escrow_10000 = false;
  byte rec[8] = {0, 0, 0, 0xFF, 0, 0, 0, 0};
  Serial1.write(reset, sizeof(reset));
  Serial.println("Resetting...");
  disp.write("8.8.8.8.");
  disp2.write("8.8.8.8.");

  while (1) {
     polling();
    //Serial.println(rec[3]);
    for (int i = 0; i < Serial1.readBytesUntil(0x03, rec, 8); i++) {
      
      // Serial.print(rec[i], HEX);
      //Serial.print(" ");
    }
    //Serial.println();
    if (rec[3] != 0xFF) break;
  };
  Serial1.write(standby, sizeof(standby));
  Serial1.write(standby_cm, sizeof(standby_cm));
  Serial1.write(alldata_bv, sizeof(alldata_bv));
  Serial1.write(start_three_bv, sizeof(start_three_bv));
  Serial1.write(start_cm, sizeof(start_cm));

}
void digit_refresh() {
  if (bv_error_code != 0) {
    disp.write("err");
    writedigit(credit);

  }
  else {
    writedigit(credit);
  }
}
int getCRC(byte data[], int leng)
{
  int crc = 0xFFFF;          // initial value
  int polynomial = 0x1021;   // 0001 0000 0010 0001  (0, 5, 12)
  byte b;
  for (int j = 0; j < leng; j++) {
    b = data[j];
    for (int i = 0; i < 8; i++)
    {
      bool bitA = ((b >> (7 - i) & 1) == 1);
      bool c15 = ((crc >> 15 & 1) == 1);
      crc <<= 1;
      if (c15 ^ bitA) crc ^= polynomial;
    }
  }
  crc &= 0xffff;
  return crc;
}
byte ToBcd(int value)
{
  if (value < 0 || value > 99999999)
    //throw new ArgumentOutOfRangeException("value");
    return -1;
  byte ret = new byte();

  ret = (byte)(value % 10);
  value /= 10;
  ret |= (byte)((value % 10) << 4);
  value /= 10;

  return ret;
}
byte getfcc(byte bc, byte dc, byte data[])
{
  byte fcc = 0;
  fcc = bc;
  fcc = fcc ^ dc;
  for (int i = 0; i < bc - 1; i++)
  {
    fcc = fcc ^ data[i];
  }
  return fcc;
}
void writedigit(long val) {
  long v1 = 0;
  long v2 = 0;
  v2 = val / 10000;
  v1 = (val % 10000);
  if (v2 != 0)
    disp.write(v2);
  else
    disp.clearDisp();
  if (v2 == 0)
    disp2.write(v1);
  else
    disp2.write(v1, 4);
}
void bv_status_write_binled() {
  if (bv_error_code == 0) {
    for (int i = 0; i < (sizeof(binledpins) / sizeof(binledpins[0])); i++) {
      digitalWrite(binledpins[i], GetBit(bv_status_code[0], i));
    }
  }
  else {
    for (int i = 0; i < (sizeof(binledpins) / sizeof(binledpins[0])); i++) {
      digitalWrite(binledpins[i], GetBit(bv_error_code, i));
    }
  }
}
void disconnect_retry() {
  byte rec[8] = {0, 0, 0, 0xFF, 0, 0, 0, 0};
  while (1) {
    polling();
    for (int i = 0; i < Serial1.readBytesUntil(0x03, rec, 8); i++) {
    }
    if (rec[3] != 0xFD) break;
  }


}
void stand_by() {
  Serial1.write(standby, sizeof(standby));
  Serial1.write(standby_cm, sizeof(standby_cm));
  Serial1.write(alldata_bv, sizeof(alldata_bv));
  Serial1.write(start_three_bv, sizeof(start_three_bv));
  Serial1.write(start_cm, sizeof(start_cm));
}
