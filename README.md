Arduino Chip Tester
-------------------

I made this to test the LS chips on my Apple II using an Arduino UNO.
The tests are based on [the work done by JorBi](https://www.instructables.com/Arduino-IC-Tester/).


![image](https://github.com/user-attachments/assets/8de85816-8417-4187-92cd-55f8e2543c07)

Flash the firmware, wire up the UNO and run `test.py`

*Example:*
Below is an example of a chip that fails the test

```sh
$ python3 test.py /dev/ttyACM0 data/16-74139.dat
RST
-> 101HHHHGHHHH011V
<- O:101HHHHGHHHH011V
-> 110HHHHGHHHH101V
<- X:110LHHHGHHHH101V
-> 000LHHHGHHHL000V
<- O:000LHHHGHHHL000V
-> 010HLHHGHHLH010V
<- X:010LLHHGHHLH010V
-> 001HHLHGHLHH100V
<- X:001HHLHGHHHL100V
-> 011HHHLGLHHH110V
<- X:011HHHLGHHLH110V
```
