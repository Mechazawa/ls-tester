
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

#define MAX_PINS 16

int Pin14[14] = {7, 8, 9, 10, 11, 12, A7, 5, 4, 3, 2, 13, A0, A1};        // contains the correct allocation for a 14 Pins IC
int Pin16[16] = {6, 7, 8, 9, 10, 11, 12, A7, 5, 4, 3, 2, 13, A0, A1, A2}; // contains the correct allocation for a 16 Pins IC
int PinMap[16] = {6, 7, 8, 9, 10, 11, 12, A7, 5, 4, 3, 2, 13, A0, A1, A2}; // contains the correct allocation for a 16 Pins IC

char dat_read(unsigned int eeaddress);

void setup()
{
  Serial.begin(9600);
}

typedef struct TestSuite {
  char name[8];
  char pinCount;
  unsigned int offset;
};

typedef struct TestResult { 
  bool pass;
  bool state[MAX_PINS];
};

int* getPinDef(char pins)
{
  return PinMap + (8 - (pins / 2));
}

TestResult test(char pins, char* definition)
{
  // Stage 1 set input signals
  
}

char dat_read(unsigned int eeaddress)
{
  if (eeaddress > F5KPGEUIK2UZ059_dat_len)
  {
    return 0x00;
  }

  return pgm_read_byte(&F5KPGEUIK2UZ059_dat[eeaddress]);
}