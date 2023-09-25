#include "shallow.h"

t_rj rj_out[20];
elapsedMicros latch_on = 0;
int RJ_test = RJ_TEST_OUT;
int RJ_leaf_test = RJ_LEAF_TEST_OUT;

void setup()
{
  Entropy.Initialize();
  Serial.begin(115200);
  SERIAL_WAIT;
  for (int i = 0; i < 42; i++)
    pinMode(i, OUTPUT);
  rj_out[0].data = 0;
  rj_out[0].clock = 0;
  for (u_int8_t i = 1; i <= 19; i++)
  {
    rj_out[i].data = i * 2;
    rj_out[i].clock = (i * 2) + 1;
  }
#ifdef DEBUG
  for (u_int8_t i = 1; i <= 19; i++) // debug le numero de chaque RJ output data + clock
  {
    DPRINT("rj.out");
    DPRINT(i);
    DPRINT(" | data=");
    DPRINT(rj_out[i].data);
    DPRINT(" | clock=");
    DPRINTLN(rj_out[i].clock);
  }
#endif
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
  // rj_out[2].leaf[0].testMode = 1;
}

// int test_RJ_loop = 1;
// int rj_testing = 1;
// char serial_num = 0;

void loop()
{
#ifdef TEST_MODE
#if TEST_MODE == 1
  leaf_test_mode();
#elif TEST_MODE == 2
  for (int i = RJ_START; i <= RJ_END; i++)
    test_module(rj_out[i], 2, 2000);
#else
#endif
#else
  leaf_status_update1();
#endif

  // leaf_status_update(); // old

  /**
   * @brief test les leds module par module, en incrmentant et decrementant avec 1 et 2
   *
  if (Serial.available())
  {
    Serial.println("hello");
    serial_num = Serial.read();
    if (serial_num == '1')
    {
      if (rj_testing == 1)
        rj_testing = 19;
      else
        rj_testing--;
      Serial.println("1");
      Serial.print("current RJ.");
      Serial.println(rj_testing);
    }
    if (serial_num == '2')
    {
      if (rj_testing == 19)
        rj_testing = 1;
      else
        rj_testing++;
      Serial.println("2");
      Serial.print("current RJ.");
      Serial.println(rj_testing);
    }
  }

  // test_module(rj_out[rj_testing], 2, 200000);
  */
}

void leaf_init(void)
{
  for (int i = 1; i <= RJ_TOT; i++)
  {
    for (int rj_leaf = (SHIFT_REG_OUTPUT_Q * MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1; rj_leaf >= 0; rj_leaf--)
    {
      rj_out[i].leaf[rj_leaf].isActive = 0;
      rj_out[i].leaf[rj_leaf].leaf_byte = 1;
      rj_out[i].leaf[rj_leaf].leaf_byte <<= rj_leaf % 8; // on prepare le bit a la bonne place, qui n'en changera plus
#ifdef TEST_MODE
#if TEST_MODE == 1
      if (i == RJ_test && rj_leaf == RJ_leaf_test)
        rj_out[i].leaf[rj_leaf].testMode = 1;
      else
        rj_out[i].leaf[rj_leaf].testMode = 0;
      rj_out[i].leaf[rj_leaf].timeOn = TEST_TIME_ON;
      rj_out[i].leaf[rj_leaf].timeOff = TEST_TIME_OFF;
#else
#endif
#else
      rj_out[i].leaf[rj_leaf].timeOn = Entropy.random(MIN_ON_TIME, MAX_ON_TIME);
      rj_out[i].leaf[rj_leaf].timeOff = Entropy.random(MIN_OFF_TIME, MAX_OFF_TIME);
#endif
      rj_out[i].leaf[rj_leaf].elapsed_off = 0;
      rj_out[i].leaf[rj_leaf].elapsed_on = 0;
      DPRINT("rj.");
      DPRINT(i);
      DPRINT(" leaf.");
      DPRINT(rj_leaf);
      DPRINT(": time on(micros)=");
      DPRINT(rj_out[i].leaf[rj_leaf].timeOn);
      DPRINT(", time off(millis)=");
      DPRINTLN(rj_out[i].leaf[rj_leaf].timeOff);
      print_binary(rj_out[i].leaf[rj_leaf].leaf_byte);
      if (rj_out[i].leaf[rj_leaf].testMode)
      {
        DPRINT("test_mode.");
        DPRINTLN(rj_out[i].leaf[rj_leaf].testMode);
      }
    }
    for (int j = 0; j < MODULE_SHIFT_REG * MODULE_SERIE_Q; j++)
    {
      rj_out[i].shift_register[j] = 0;      // mise a zero de tous les shift registers
      rj_out[i].prev_shift_register[j] = 0; // mise a zero de tous les shift registers
    }
  }
}

void leaf_status_update1(void)
{
#ifdef TEST_MODE
#if TEST_MODE == 1
  return;
#else
#endif
#else
  if (latch_on >= LATCH_DELAY)
    digitalWrite(LATCH, LOW);
  else
    return;
  // uint32_t t  = ARM_DWT_CYCCNT; ///
  for (int i = 1; i <= RJ_TOT; i++)
  {
    for (int rj_leaf = (SHIFT_REG_OUTPUT_Q * MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1; rj_leaf >= 0; rj_leaf--)
    {
      if (rj_out[i].leaf[rj_leaf].elapsed_off >= rj_out[i].leaf[rj_leaf].timeOff && !rj_out[i].leaf[rj_leaf].isActive)
      {
        rj_out[i].leaf[rj_leaf].isActive = 1;
        // rj_out[i].leaf[rj_leaf].leaf_byte = 1; // mettre dans init puisque jamais modifie?
        // rj_out[i].leaf[rj_leaf].leaf_byte <<= (rj_leaf + 1) % 8; // mettre dans init puisque jamais modifie?
        rj_out[i].shift_register[rj_leaf / SHIFT_REG_OUTPUT_Q] |= rj_out[i].leaf[rj_leaf].leaf_byte;
        rj_out[i].leaf[rj_leaf].timeOff = Entropy.random(MIN_OFF_TIME, MAX_OFF_TIME);
        rj_out[i].leaf[rj_leaf].elapsed_on = 0;
        DPRINT("rj.");
        DPRINT(i);
        DPRINT(" shift_reg.");
        DPRINT(rj_leaf / SHIFT_REG_OUTPUT_Q);
        DPRINT(" reg_leaf.");
        DPRINT(rj_leaf % 8);
        DPRINT(" rj_leaf.");
        DPRINT(rj_leaf);
        DPRINTLN(" ON");
        DPRINT("result ON=");
        DPRINT_BINARY(rj_out[i].shift_register[rj_leaf / SHIFT_REG_OUTPUT_Q]);
        DPRINTLN();
      }
      else if (rj_out[i].leaf[rj_leaf].elapsed_on >= rj_out[i].leaf[rj_leaf].timeOn && rj_out[i].leaf[rj_leaf].isActive)
      {
        rj_out[i].leaf[rj_leaf].isActive = 0;
        rj_out[i].shift_register[rj_leaf / SHIFT_REG_OUTPUT_Q] &= ~rj_out[i].leaf[rj_leaf].leaf_byte;
        rj_out[i].leaf[rj_leaf].timeOn = Entropy.random(MIN_ON_TIME, MAX_ON_TIME);
        rj_out[i].leaf[rj_leaf].elapsed_off = 0;
        DPRINT("rj.");
        DPRINT(i);
        DPRINT(" shift_reg.");
        DPRINT(rj_leaf / SHIFT_REG_OUTPUT_Q);
        DPRINT(" reg_leaf.");
        DPRINT(rj_leaf % 8);
        DPRINT(" rj_leaf.");
        DPRINT(rj_leaf);
        DPRINTLN(" OFF");
        DPRINT("result OFF=");
        DPRINT_BINARY(rj_out[i].shift_register[rj_leaf / SHIFT_REG_OUTPUT_Q]);
        DPRINTLN();
      }
    }
    bool push = 0;
    for (int current_shift_reg = (MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1; current_shift_reg >= 0; current_shift_reg--)
      if (rj_out[i].shift_register[current_shift_reg] != rj_out[i].prev_shift_register[current_shift_reg]) // pour tenter d'envoyer uniquement les changement d'etat du shift register
      {
        push = 1;
        break;
      }
    if (push)
    {
#ifdef HIDE_17_32
      shiftOut_msbFirst(rj_out[i].data, rj_out[i].clock, 0);
      shiftOut_msbFirst(rj_out[i].data, rj_out[i].clock, 0);
#else
#endif
      for (int current_shift_reg = (MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1; current_shift_reg >= 0; current_shift_reg--)
      {
        shiftOut_msbFirst_rd(rj_out[i].data, rj_out[i].clock, rj_out[i].shift_register[current_shift_reg]);
        rj_out[i].prev_shift_register[current_shift_reg] = rj_out[i].shift_register[current_shift_reg];
      }
    }
  }
  digitalWrite(LATCH, HIGH);
  latch_on = 0;
  // DPRINT("cycles=");
  // DPRINTLN(ARM_DWT_CYCCNT - t);
  // while(1);
#endif
}

void leaf_test_mode(void) // blink each led until increment through Serial
{
  if (latch_on >= LATCH_DELAY)
    digitalWrite(LATCH, LOW);
  else
    return;
  // uint32_t t  = ARM_DWT_CYCCNT; ///
  for (int i = 1; i <= RJ_TOT; i++)
  {
    for (int rj_leaf = (SHIFT_REG_OUTPUT_Q * MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1; rj_leaf >= 0; rj_leaf--)
    {
      if (rj_out[i].leaf[rj_leaf].elapsed_off >= rj_out[i].leaf[rj_leaf].timeOff && !rj_out[i].leaf[rj_leaf].isActive && rj_out[i].leaf[rj_leaf].testMode)
      {
        rj_out[i].leaf[rj_leaf].isActive = 1;
        rj_out[i].shift_register[rj_leaf / SHIFT_REG_OUTPUT_Q] |= rj_out[i].leaf[rj_leaf].leaf_byte;
        rj_out[i].leaf[rj_leaf].timeOff = TEST_TIME_OFF;
        rj_out[i].leaf[rj_leaf].elapsed_on = 0;
        // DPRINT("rj.");
        // DPRINT(i);
        // DPRINT(" shift_reg.");
        // DPRINT(rj_leaf / MODULE_SHIFT_REG * MODULE_SERIE_Q);
        // DPRINT(" reg_leaf.");
        // DPRINT(rj_leaf % 8);
        // DPRINT(" rj_leaf.");
        // DPRINT(rj_leaf);
        // DPRINTLN(" ON");
        // DPRINT("result ON=");
        // DPRINT_BINARY(rj_out[i].shift_register[rj_leaf / SHIFT_REG_OUTPUT_Q]);
        // DPRINTLN();
      }
      else if (rj_out[i].leaf[rj_leaf].elapsed_on >= rj_out[i].leaf[rj_leaf].timeOn && rj_out[i].leaf[rj_leaf].isActive)
      {
        rj_out[i].leaf[rj_leaf].isActive = 0;
        rj_out[i].shift_register[rj_leaf / SHIFT_REG_OUTPUT_Q] &= ~rj_out[i].leaf[rj_leaf].leaf_byte;
        rj_out[i].leaf[rj_leaf].timeOn = TEST_TIME_ON;
        rj_out[i].leaf[rj_leaf].elapsed_off = 0;
        // DPRINT("rj.");
        // DPRINT(i);
        // DPRINT(" shift_reg.");
        // DPRINT(rj_leaf / MODULE_SHIFT_REG * MODULE_SERIE_Q);
        // DPRINT(" reg_leaf.");
        // DPRINT(rj_leaf % 8);
        // DPRINT(" rj_leaf.");
        // DPRINT(rj_leaf);
        // DPRINTLN(" OFF");
        // DPRINT("result OFF=");
        // DPRINT_BINARY(rj_out[i].shift_register[rj_leaf / SHIFT_REG_OUTPUT_Q]);
        // DPRINTLN();
      }
    }
    bool push = 0;
    for (int current_shift_reg = (MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1; current_shift_reg >= 0; current_shift_reg--)
      if (rj_out[i].shift_register[current_shift_reg] != rj_out[i].prev_shift_register[current_shift_reg]) // pour tenter d'envoyer uniquement les changement d'etat du shift register
      {
        push = 1;
        break;
      }
    if (push)
    {
#ifdef HIDE_17_32
      shiftOut_msbFirst(rj_out[i].data, rj_out[i].clock, 0);
      shiftOut_msbFirst(rj_out[i].data, rj_out[i].clock, 0);
#else
#endif
      for (int current_shift_reg = (MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1; current_shift_reg >= 0; current_shift_reg--)
      {
        shiftOut_msbFirst_rd(rj_out[i].data, rj_out[i].clock, rj_out[i].shift_register[current_shift_reg]);
        rj_out[i].prev_shift_register[current_shift_reg] = rj_out[i].shift_register[current_shift_reg];
      }
    }
  }
  // DPRINT("cycles=");
  // DPRINTLN(ARM_DWT_CYCCNT - t);
  // while(1);
  digitalWrite(LATCH, HIGH);
  latch_on = 0;
  test_mode_increment();
}

void test_mode_increment(void)
{
  char n;

  if (Serial.available())
  {
    rj_out[RJ_test].leaf[RJ_leaf_test].testMode = 0;
    n = Serial.read();
    switch (n)
    {
    case '1':
      if (RJ_leaf_test > 0)
        RJ_leaf_test--;
      else if (!RJ_leaf_test)
      {
        RJ_leaf_test = (SHIFT_REG_OUTPUT_Q * MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1;
        if (RJ_test > 1)
          RJ_test--;
        else
          RJ_test = RJ_TOT;
      }
      DPRINT("test leaf:");
      DPRINT(" RJ=");
      DPRINT(RJ_test);
      DPRINT(" leaf=");
      DPRINTLN(RJ_leaf_test);
      break;

    case '2':
      if (RJ_leaf_test < (SHIFT_REG_OUTPUT_Q * MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1)
        RJ_leaf_test++;
      else if (RJ_leaf_test == (SHIFT_REG_OUTPUT_Q * MODULE_SHIFT_REG * MODULE_SERIE_Q) - 1)
      {
        RJ_leaf_test = 0;
        if (RJ_test < RJ_TOT)
          RJ_test++;
        else
          RJ_test = 1;
      }
      DPRINT("test leaf:");
      DPRINT("RJ=");
      DPRINT(RJ_test);
      DPRINT(" leaf=");
      DPRINTLN(RJ_leaf_test);
      break;

    case '3':
      if (RJ_test == 1)
        RJ_test = RJ_TOT;
      else
        RJ_test--;
      break;

    case '4':
      if (RJ_test == RJ_TOT)
        RJ_test = 1;
      else
        RJ_test++;
      break;

    default:
      break;
    }
    rj_out[RJ_test].leaf[RJ_leaf_test].testMode = 1;
  }
}

/**
 * @brief test reset modules (write 0 and latch)
 *
 * @param rj_out which RJ
 * @param module_nbr n modules in serie (min = 1)
 */
void reset_module(t_rj rj_out, int module_nbr)
{
  int totalShiftreg; 
  
  if (module_nbr < 1)
    module_nbr = 1;
  totalShiftreg = module_nbr * 2;

  for (int i = 0; i < 8 * totalShiftreg; i++)
  {
    digitalWrite(LATCH, LOW);
    for (int y = totalShiftreg; y > 0; y--)
      shiftOut_msbFirst_rd(rj_out.data, rj_out.clock, 0);
    digitalWrite(LATCH, HIGH);
    delayMicroseconds(LATCH_DELAY);
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
