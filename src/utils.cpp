#include "shallow.h"


void    led_blink(int time_on, int time_off)
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(time_on);
    digitalWrite(LED_BUILTIN, LOW);
    delay(time_off);
}