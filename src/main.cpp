#include <Arduino.h>
#include "shallow.h"

#define LATCH 40 // common latch
#define RELAY 41 // relay pour alimentation 12v
#define RJ_START 1 // premiere sortie a jouer
#define RJ_END 19 // premiere sortie a jouer
#define RJ_TOT 19 // quantite de sorties RJ actives
#define MODULE_SERIE_Q 2 // quantitee de modules en serie, au bout de chaque rj12

typedef struct s_rj {
  u_int8_t data;
  u_int8_t clock;
}             t_rj;

t_rj  rj_out[20];

void test_module(t_rj rj_out, int module_nbr, int del);
void reset_module(t_rj rj_out, int module_nbr);

void setup() {
  Serial.begin(9600);
  // while(!Serial);
  for (int i = 0; i < 42; i++)
  {
    pinMode(i, OUTPUT);
  }
  rj_out[0].data = 0;
  rj_out[0].clock = 0;
  for(u_int8_t i = 1; i <= 19; i++)
  {
    rj_out[i].data = i * 2;
    rj_out[i].clock = (i * 2) + 1 ;
  }
  for(u_int8_t i = 1; i <= 19; i++)
  {
    Serial.print("rj.out");
    Serial.print(i);
    Serial.print(" | data=");
    Serial.print(rj_out[i].data);
    Serial.print(" | clock=");
    Serial.println(rj_out[i].clock);
  }
  for(int i = 1; i <= RJ_TOT; i++)
    reset_module(rj_out[i], MODULE_SERIE_Q);
  digitalWrite(RELAY, HIGH);
  Serial.println("relay HIGH");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
}



void loop() {
  // digitalWrite(13, HIGH);
  // delay(1000);
  // digitalWrite(13, LOW);
  // delay(1000);
  for(int i = RJ_START; i <= RJ_END; i++)
    test_module(rj_out[i], 2, 500);

  
}


void reset_module(t_rj rj_out, int module_nbr)
{
  int totalShiftreg = module_nbr * 2;

  for (int i = 0; i < 8 * totalShiftreg; i++)
  {
    digitalWrite(LATCH, LOW);
    for (int y = totalShiftreg; y > 0; y--)
      shiftOut_msbFirst_rd(rj_out.data, rj_out.clock, 0);
    digitalWrite(LATCH, HIGH);
  }
}

/**
 * @brief test n modules (in serie) on RJ x output, blinking each led from 1 to 16
 *
 * @param delay in microseconds
 */
void test_module(t_rj rj_out, int module_nbr, int del)
{
  int totalShiftreg = module_nbr * 2;
  int currentShiftreg = 0;

  for (int i = 0; i < 8 * totalShiftreg; i++)
  {
    currentShiftreg = i / 8;
    digitalWrite(LATCH, LOW);
    for (int y = totalShiftreg - currentShiftreg; y > 0; y--)
      shiftOut_msbFirst_rd(rj_out.data, rj_out.clock, 0);
    byte data = 1;
    data <<= i % 8;
    shiftOut_msbFirst_rd(rj_out.data, rj_out.clock, data);
    for (int y = currentShiftreg; y > 0; y--)
      shiftOut_msbFirst_rd(rj_out.data, rj_out.clock, 0);
    digitalWrite(LATCH, HIGH);
    // Serial.print("writing to outputs ");
    // Serial.print(rj_out.data);
    // Serial.print(" and ");
    // Serial.println(rj_out.clock);
    delayMicroseconds(del);
    if (i == (8 * totalShiftreg) - 1)
    {
      shiftOut_msbFirst_rd(rj_out.data, rj_out.clock, 0);
      digitalWrite(LATCH, HIGH);
      // digitalWrite(LATCH, LOW);
    }


    // delayMicroseconds(del);
  }
}

