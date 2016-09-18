#include <wiringPi.h>

// Pin assignment:
// --------------------
// gpio 0 = switch 1
// gpio 4 = switch 2
// gpio 2 = led 1 green
// gpio 3 = led 2 yellow

int main (void) {
  wiringPiSetup() ;
  pinMode (2, OUTPUT);
  for (;;) {
    digitalWrite(2, HIGH); delay(500);
    digitalWrite(2,  LOW); delay(500);
  }
  return 0 ;
}
