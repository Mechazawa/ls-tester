
/*
/  Tester for CMOS and TTl IC.
/  IC will be tested according to a test file.
/  Syntax of File
/

$40161
16
0C10110G10XXXXXV
0C10110G11LLLLLV
1C10110G00HHLHLV
1000001G11HHLHLV
1C00001G11HHHLLV
1C00001G11HHHHHV
1C00001G11LLLLLV
1C00001G11LLLHLV
1100001G11LLHLLV
1000001G11LLHLLV
0000001G11LLLLLV
1C00001G11XXXXXV

/  $  Start character before the part name (max 16 charachters after $)
/  Amount of pins of the IC
/  V = supply voltage
/  G = GND (fix by hardware)
/  0/1 = logical 0 or 1 as input to the IC
/  L/H = logical 0 or 1 as output from the IC
/  X = IC output not relevant (signal wil be ignored)
/  C = Clock -> Trigger
/
/  general test routine:
/    1. seach file until a $ is found. Store the adress (this adress is used later on to print the name of the part)
/    2. set the IC as beeing "OK"
/    3. if part is set OK "OK"
/    4. set all input signals
/    5. trigger clock, if required
/    6. read outputs and compare to expected signals -> if different: IC -> NOK
/    7. if IC still OK, increment "where to find the description-adress"
/    8. find next $
/    9. if & -> end
*/

/*
/   Resistors
/   works well with 680 Ohms -> keeps the load to the Arduino low BUT way out of specifications of CMOS and TTL (still works well)
*/

/* Version 5
/  cleanup and translate all comments to english
*/

/* PINS
IC   IC    Pin
14   16    Arduino

8    9  =  5
9    10 =  4
10   11 =  3
11   12 =  2
12   13 =  13
13   14 =  A0
14   15 =  A1
     16 =  A2
     1  =  6
1    2  =  7
2    3  =  8
3    4  =  9
4    5  =  10
5    6  =  11
6    7  =  12
7    8  =  GND
*/

#include <Arduino.h>
#include "F5KPGEUIK2UZ059.h"
#include "optional.h"

#define MAX_PINS 16
#define DELAY_CLOCK 10
#define DELAY_STAGE 2

int PinMap[16] = {6, 7, 8, 9, 10, 11, 12, A7, 5, 4, 3, 2, 13, A0, A1, A2}; // contains the correct allocation for a 16 Pins IC

char dat_read(unsigned int eeaddress);
String dat_read_line(unsigned int addr);

void setup()
{
  Serial.begin(9600);  
  
  while (!Serial.available());
}

struct TestSuite {
  String name;
  char pinCount;
  unsigned int offset;
};

struct TestResult { 
  bool pass;
  char state[MAX_PINS];
};

Optional<TestSuite> next(unsigned int offset) 
{
  for (char c = dat_read(offset); c != '$'; offset++, c = dat_read(offset)) {
    if (c == 0) {
      return {};
    }
  }

  offset++;

  String name = dat_read_line(offset);
  offset += name.length() + 1;

  String pinCount = dat_read_line(offset);
  offset += pinCount.length() + 1;

  name.trim();

  return TestSuite{
      name,
      (char) pinCount.toInt(),
      offset,
  };
}

int* getPinDef(char pins)
{
  return PinMap + (8 - (pins / 2));
}

TestResult test(char pins, const char* definition)
{
  int* pinDef = getPinDef(pins);

  // Stage 1 setup pins and signals
  for(char pin = 0; pin < pins; pin++) {
    int p = pinDef[pin];
    char def = definition[pin];

    switch(def) {
      case 'L':
      case 'H':
        pinMode(p, INPUT_PULLUP);
        break;
      
      default:
        pinMode(p, OUTPUT);
        digitalWrite(p, def == 'V' || def == '1');
        break;
    }
  }

  delay(DELAY_STAGE);

  // Stage 2 clock
  for(char pin = 0; pin < pins; pin++) {
    if(definition[pin] == 'C') {
        digitalWrite(pinDef[pin], HIGH);
    }
  }

  delay(DELAY_CLOCK);

  for(char pin = 0; pin < pins; pin++) {
    if(definition[pin] == 'C') {
        digitalWrite(pinDef[pin], LOW);
    }
  }

  delay(DELAY_STAGE);

  // Stage 3 read back
  TestResult result{true};
  memset(result.state, 0, MAX_PINS);

  for (int pin = 0; pin < pins; pin++) {
    int p = pinDef[pin];
    char def = definition[pin];

    switch(def) {
      case 'L':
      case 'H':
        result.state[pin] = digitalRead(p) ? 'H' : 'L';
        result.pass &= result.state[pin] == def;
        break;

      default:
        result.state[pin] = def;
        break;
    }
  }

  return result;
}

char dat_read(unsigned int eeaddress)
{
  if (eeaddress > F5KPGEUIK2UZ059_dat_len)
  {
    return 0x00;
  }

  return pgm_read_byte(&F5KPGEUIK2UZ059_dat[eeaddress]);
}

String dat_read_line(unsigned int addr) 
{
  unsigned short count;
  String output;
  char c;

  while((c = dat_read(addr + count)) != '\n') {
    output += c;

    count++;
  }

  return output;
}

void listChips() {
  int count = 1;

  for(Optional<TestSuite> suite = next(0); suite.has_value(); suite = next(suite.take().offset + 1)) {
    Serial.print(" - ");
    Serial.println(suite.take().name);    
    count++;
  }

  Serial.println();
  Serial.print("Found ");
  Serial.print(count, DEC);
  Serial.print(" test suites");
  Serial.println();
}

Optional<TestSuite> find(String name) 
{
  for(Optional<TestSuite> suite = next(0); suite.has_value(); suite = next(suite.take().offset + 1)) {
    if (suite.take().name == name) {
      return suite;
    }
  }

  return {};
}

String readline() {
  // shit todo rewrite
  char c;
  String line = "";

  while(Serial.available() == 0);

  while ((c = Serial.read()) != '\n') {
    if (c == '\r') {
      if (line.length() > 0) {
        line.remove(line.length() - 1);
        Serial.print("\r \r");
      }
    } else {
      Serial.print(c); 
      line += c;
    }    

    while(Serial.available() == 0);
  }

  Serial.println();
  line.trim();

  return line;
}

void manualMode() {
  Serial.print("Pin count: ");
  while (Serial.available() == 0);
  int pinCount = Serial.parseInt();

  Serial.println("Manual mode, blank line to abort");

  while (true) {
    Serial.print("> ");
    while (Serial.available() == 0);
    String line = Serial.readString();
    line.trim();

    if (line.length() == 0) {
      return;
    }

    if (line.length() != pinCount) {
      Serial.print("ERR, expected definition of size ");
      Serial.print(pinCount, DEC);
      Serial.print(" got ");
      Serial.print(line.length(), DEC);
      Serial.println();
      continue;
    }

    TestResult result = test(pinCount, line.c_str());

    Serial.print("> ");
    Serial.print(result.state);
    Serial.println(result.pass ? " √" : " X");
  }
}

void runSuite(TestSuite suite) {
  Serial.print("Running suite: ");
  Serial.println(suite.name);

  
} 

void testMode() {
  Serial.print("Chip: ");
  String name = readline();

  Optional<TestSuite> suite = find(name);
  if (suite.has_value()) {
    runSuite(suite.take()); 
  } else {
    Serial.print("ERR not found: '");
    Serial.print(name);
    Serial.println("'");
  }
}

void loop() 
{
  Serial.println("Chip tester");
  Serial.println("[m] Manual mode");
  Serial.println("[t] Test chip");
  Serial.println("[l] List definitions");
  Serial.print("> ");
  while (Serial.available() == 0);
  Serial.println();

  switch(Serial.read()) {
    case 'm':
    case 'M':
      manualMode();
      break;
      
    case 't':
    case 'T':
      testMode();
      break;

    case 'l':
    case 'L':
      listChips();
      break;

    default:
      Serial.println("Uknown command");
      break;
  }
}