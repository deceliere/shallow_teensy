#include <Arduino.h>
#include "shallow.h"

t_rj    rj_out[20];

void setup() {
  Serial.begin(9600);
  SERIAL_WAIT;
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
  for(u_int8_t i = 1; i <= 19; i++) // debug le numero de chaque RJ output
  {
    DPRINT("rj.out");
    DPRINT(i);
    DPRINT(" | data=");
    DPRINT(rj_out[i].data);
    DPRINT(" | clock=");
    DPRINTLN(rj_out[i].clock);
  }
  for (int x = 0; x < 3; x++) { // on reset trois fois de suite toutes les sorties, par paranoia
    for(int i = 1; i <= RJ_TOT; i++)
      reset_module(rj_out[i], MODULE_SERIE_Q);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
  randomSeed(analogRead(A0));
  leaf_init();
  // while(1);
  /* 
  ici il faudra ajouter une fonction qui check que le courant qui passe est nul 
  s'il n'est pas nul: allume led 13 puis while(1)
  on pourrait aussi faire une boucle qui allume chaque élément, et check le courrant de chacun (et le printant par exemple)
  à voir si c'est possible
  */
  
  digitalWrite(RELAY, HIGH);
  DPRINTLN("relay HIGH");
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

void  leaf_init(void)
{
  for (int i = 1; i <= RJ_TOT; i++)
  {
    for(int module_leaf = 0; module_leaf < MODULE_SERIE_Q * MODULE_OUTPUT_Q; module_leaf++)
    {
      rj_out[i].leaf[module_leaf].isActive = 0;
      // rj_out[i].leaf[module_leaf].leaf_byte = 0;
      rj_out[i].leaf[module_leaf].timeOn = random(MIN_ON_TIME, MAX_ON_TIME);
      rj_out[i].leaf[module_leaf].timeOff = random(MIN_OFF_TIME, MAX_OFF_TIME);
      DPRINT("rj ");
      DPRINT(i);
      DPRINT(" leaf ");
      DPRINT(module_leaf + 1);
      DPRINT(": time on(micros)=");
      DPRINT(rj_out[i].leaf[module_leaf].timeOn);
      DPRINT(", time off(millis)=");
      DPRINTLN(rj_out[i].leaf[module_leaf].timeOff);
    }
    rj_out[i].module1 = 0;
    rj_out[i].module2 = 0;
  }
}

void  leaf_status_update (void)
{
  for (int i = 1; i <= TOT_LEAVES; i++)
  {

  }
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
    // DPRINT("writing to outputs ");
    // DPRINT(rj_out.data);
    // DPRINT(" and ");
    // DPRINTLN(rj_out.clock);
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

