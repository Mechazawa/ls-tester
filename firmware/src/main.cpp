
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

#define MAX_PINS 16
#define DELAY_CLOCK 10
#define DELAY_STAGE 2

int PinMap[16] = {6, 7, 8, 9, 10, 11, 12, A7, 5, 4, 3, 2, 13, A0, A1, A2}; // contains the correct allocation for a 16 Pins IC
char cmd_reset[MAX_PINS];

void setup()
{
  memset(cmd_reset, 'G', MAX_PINS);

  Serial.begin(9600);  
}

struct TestResult { 
  bool pass;
  char state[MAX_PINS];
};


int* getPinDef(char pins)
{
  return PinMap + (8 - (pins / 2));
}

TestResult test(const char pins, const char* definition)
{
  int* pinDef = getPinDef(pins);

  // Stage 1 setup pins and signals
  for(int pin = 0; pin < pins; pin++) {
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
  for(int pin = 0; pin < pins; pin++) {
    if(definition[pin] == 'C') {
        digitalWrite(pinDef[pin], HIGH);
    }
  }

  delay(DELAY_CLOCK);

  for(int pin = 0; pin < pins; pin++) {
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


void loop() 
{
  while(!Serial.available());
  
  String cmd = Serial.readStringUntil('\n');

  cmd.trim();
  
  if (cmd.equals("R")) {
    TestResult result = test(MAX_PINS, cmd_reset);

    Serial.println(result.pass ? "RST" : "ERR");
  } else if (cmd.length() > MAX_PINS) {
    Serial.println("ERR");
  } else {
    TestResult result = test(cmd.length(), cmd.c_str());

    Serial.print(result.pass ? "O:" : "X:");
    Serial.println(result.state);
  }

  Serial.flush();
}