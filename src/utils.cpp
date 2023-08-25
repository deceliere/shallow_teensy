#include "shallow.h"

void led_blink(int time_on, int time_off)
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(time_on);
    digitalWrite(LED_BUILTIN, LOW);
    delay(time_off);
}

/**
 * @brief LSB binary serial print
 *
 * @param num
 */
void print_binary(int num)
{
    for (int i = 0; i < 8; i++)
    {
        if (num & 1)
            DPRINT("1");
        else
            DPRINT("0");
        num >>= 1;
    }
    DPRINTLN();
}

void check_binary_operation(void)
{
    u_int8_t num1;

    num1 = 230;
    num1 &= 0;
    DPRINTLN(num1);
    while (1)
        ;
}

int bytes_count(int num)
{
    int byte_count = 0;

    if (!num)
        return (0);
    for (int i = 0; i < 8; i++)
    {
        if (num & 1)
            byte_count++;
        num >>= 1;
    }
    return (byte_count);
}