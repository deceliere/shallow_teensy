#include "shallow.h"

t_rj rj_out[20];
elapsedMicros latch_on = 0;

void setup()
{
  Serial.begin(115200);
  SERIAL_WAIT;
  for (int i = 0; i < 42; i++)
  {
    pinMode(i, OUTPUT);
  }
  rj_out[0].data = 0;
  rj_out[0].clock = 0;
  for (u_int8_t i = 1; i <= 19; i++)
  {
    rj_out[i].data = i * 2;
    rj_out[i].clock = (i * 2) + 1;
  }
  for (u_int8_t i = 1; i <= 19; i++) // debug le numero de chaque RJ output data + clock
  {
    DPRINT("rj.out");
    DPRINT(i);
    DPRINT(" | data=");
    DPRINT(rj_out[i].data);
    DPRINT(" | clock=");
    DPRINTLN(rj_out[i].clock);
  }
  for (int x = 0; x < 3; x++) // reset 3x de suite toutes les sorties, par paranoia
  {
    for (int i = 1; i <= RJ_TOT; i++)
      reset_module(rj_out[i], MODULE_SERIE_Q);
    led_blink(200, 500);
  }
  // randomSeed(analogRead(A0));
  // while(1);
  /*
  ici il faudra ajouter une fonction qui check que le courant qui passe est nul
  s'il n'est pas nul: allume led 13 puis while(1)
  on pourrait aussi faire une boucle qui allume chaque élément, et check le courrant de chacun (et le printant par exemple)
  à voir si c'est possible
  */

  digitalWrite(RELAY, HIGH);
  DPRINTLN("relay HIGH");
  led_blink(500, 0);
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(500);
  // digitalWrite(LED_BUILTIN, LOW);
  leaf_init();
}

void loop()
{
  leaf_status_update1();

  // for(int i = RJ_START; i <= RJ_END; i++)
  // test_module(rj_out[i], 2, 100);
}

void leaf_init(void)
{
  for (int i = 1; i <= RJ_TOT; i++)
  {
    for (int module_leaf = 0; module_leaf < MODULE_SERIE_Q * MODULE_OUTPUT_Q; module_leaf++)
    {
      rj_out[i].leaf[module_leaf].isActive = 0;
      rj_out[i].leaf[module_leaf].leaf_byte = 0;
      rj_out[i].leaf[module_leaf].timeOn = random(MIN_ON_TIME, MAX_ON_TIME);
      rj_out[i].leaf[module_leaf].timeOff = random(MIN_OFF_TIME, MAX_OFF_TIME);
      rj_out[i].leaf[module_leaf].elapsed_off = 0;
      rj_out[i].leaf[module_leaf].elapsed_on = 0;
      DPRINT("rj.");
      DPRINT(i);
      DPRINT(" leaf.");
      DPRINT(module_leaf);
      DPRINT(": time on(micros)=");
      DPRINT(rj_out[i].leaf[module_leaf].timeOn);
      DPRINT(", time off(millis)=");
      DPRINTLN(rj_out[i].leaf[module_leaf].timeOff);
    }
    for (int j = 0; j < MODULE_SHIFT_REG * MODULE_SERIE_Q; j++)
      rj_out[i].shift_register[j] = 0;
  }
}


void leaf_status_update1(void)
{
  if (latch_on >= LATCH_DELAY)
    digitalWrite(LATCH, LOW);
  else
    return;
  for (int i = 1; i <= RJ_TOT; i++)
  {
    for (int rj_leaf = (SHIFT_REG_OUTPUT_Q * MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1; rj_leaf >= 0; rj_leaf--)
    {
      if (rj_out[i].leaf[rj_leaf].elapsed_off >= rj_out[i].leaf[rj_leaf].timeOff && !rj_out[i].leaf[rj_leaf].isActive)
      {
        rj_out[i].leaf[rj_leaf].isActive = 1;
        rj_out[i].leaf[rj_leaf].leaf_byte = 1; // mettre dans init puisque jamais modifie?
        rj_out[i].leaf[rj_leaf].leaf_byte <<= (rj_leaf + 1) % 8; // mettre dans init puisque jamais modifie?
        rj_out[i].shift_register[rj_leaf / SHIFT_REG_OUTPUT_Q] |= rj_out[i].leaf[rj_leaf].leaf_byte;
        rj_out[i].leaf[rj_leaf].timeOff = random(MIN_OFF_TIME, MAX_OFF_TIME);
        rj_out[i].leaf[rj_leaf].elapsed_on = 0;
        DPRINT("rj.");
        DPRINT(i);
        DPRINT(" shift_reg.");
        DPRINT(rj_leaf / MODULE_SHIFT_REG * MODULE_SERIE_Q);
        DPRINT(" reg_leaf.");
        DPRINT(rj_leaf % 8);
        DPRINT(" rj_leaf.");
        DPRINT(rj_leaf);
        DPRINTLN(" ON");
        DPRINT("result ON=");
        print_binary(rj_out[i].shift_register[rj_leaf / MODULE_SHIFT_REG * MODULE_SERIE_Q]);
        DPRINTLN();
      }
      else if (rj_out[i].leaf[rj_leaf].elapsed_on >= rj_out[i].leaf[rj_leaf].timeOn && rj_out[i].leaf[rj_leaf].isActive)
      {
        rj_out[i].leaf[rj_leaf].isActive = 0;
        rj_out[i].shift_register[rj_leaf / SHIFT_REG_OUTPUT_Q] &= ~rj_out[i].leaf[rj_leaf].leaf_byte;
        rj_out[i].leaf[rj_leaf].timeOn = random(MIN_ON_TIME, MAX_ON_TIME);
        rj_out[i].leaf[rj_leaf].elapsed_off = 0;
        DPRINT("rj.");
        DPRINT(i);
        DPRINT(" shift_reg.");
        DPRINT(rj_leaf / MODULE_SHIFT_REG * MODULE_SERIE_Q);
        DPRINT(" reg_leaf.");
        DPRINT(rj_leaf % 8);
        DPRINT(" rj_leaf.");
        DPRINT(rj_leaf);
        DPRINTLN(" OFF");
        DPRINT("result OFF=");
        print_binary(rj_out[i].shift_register[rj_leaf / MODULE_SHIFT_REG * MODULE_SERIE_Q]);
        DPRINTLN();
      }
    }
    for (int current_shift_reg = 3; current_shift_reg >= 0; current_shift_reg--)
      shiftOut_msbFirst_rd(rj_out[i].data, rj_out[i].clock, rj_out[i].shift_register[current_shift_reg]);
  }
  digitalWrite(LATCH, HIGH);
  latch_on = 0;
}

void leaf_status_update(void)
{
  if (latch_on >= LATCH_DELAY)
    digitalWrite(LATCH, LOW);
  else
    return;
  for (int i = 1; i <= RJ_TOT; i++)
  {
    // for (int j = 0; j < MODULE_SHIFT_REG * MODULE_SERIE_Q; j++)
    // rj_out[i].shift_register[j] = 0;
    for (int current_shift_reg = (MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1; current_shift_reg >= 0; current_shift_reg--)
    {
      for (int rj_leaf = 0; rj_leaf < SHIFT_REG_OUTPUT_Q; rj_leaf++)
      {
        if (rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].elapsed_off >= rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].timeOff && !rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].isActive) // changer pour est plus grand que time off && plus petit que time on ? et enlever !active ?
        {
          rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].isActive = 1;
          rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].leaf_byte = 1;
          rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].leaf_byte <<= rj_leaf;
          DPRINT("binary on=");
          print_binary(rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].leaf_byte);
          // DPRINT("ON byte=");
          // DPRINTLN(rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].leaf_byte);
          rj_out[i].shift_register[current_shift_reg] |= rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].leaf_byte;
          rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].timeOff = random(MIN_OFF_TIME, MAX_OFF_TIME);
          DPRINT("rj.");
          DPRINT(i);
          DPRINT(" shift_reg.");
          DPRINT(current_shift_reg);
          DPRINT(" leaf.");
          DPRINT(rj_leaf);
          DPRINT(" aka.");
          DPRINT(rj_leaf * (current_shift_reg + 1));
          DPRINTLN(" ON");
          DPRINT("result ON=");
          print_binary(rj_out[i].shift_register[current_shift_reg]);
          DPRINTLN();
          rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].elapsed_on = 0;
        }
        else if (rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].elapsed_on >= rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].timeOn && rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].isActive)
        {
          rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].isActive = 0;
          rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].leaf_byte = 1;
          rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].leaf_byte <<= rj_leaf;
          DPRINT("binary off=");
          print_binary(rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].leaf_byte);
          rj_out[i].shift_register[current_shift_reg] &= ~rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].leaf_byte;
          rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].timeOn = random(MIN_ON_TIME, MAX_ON_TIME);
          DPRINT("rj.");
          DPRINT(i);
          DPRINT(" shift_reg.");
          DPRINT(current_shift_reg);
          DPRINT(" leaf.");
          DPRINT(rj_leaf * (current_shift_reg + 1));
          DPRINT(" aka.");
          DPRINT(rj_leaf);
          DPRINTLN(" OFF");
          DPRINT("result OFF=");
          print_binary(rj_out[i].shift_register[current_shift_reg]);
          DPRINTLN();
          rj_out[i].leaf[rj_leaf * (current_shift_reg + 1)].elapsed_off = 0;
        }
      }
      // if (bytes_count(rj_out[i].shift_register[current_shift_reg]) > 1) // test pour voir si deux bits peuvent bien s'allumer
      // print_binary(rj_out[i].shift_register[current_shift_reg]);
      shiftOut_msbFirst_rd(rj_out[i].data, rj_out[i].clock, rj_out[i].shift_register[current_shift_reg]);
    }
  }
  digitalWrite(LATCH, HIGH);
  latch_on = 0;
  // delayMicroseconds(LATCH_DELAY); // a enlever si elapsed fonctionne
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
