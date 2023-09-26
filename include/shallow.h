#include <Arduino.h>
#include "Entropy.h"

/* MACROS DE TEST */

// #define DEBUG
// #define WAIT_SERIAL

// #define TEST_MODE 1 // blink each item, to be incremented through serial
/* 
1 - decrement led
2 - increment led
3 - decrement RJ
4 - increment RJ
*/
// #define TEST_MODE 2 // test all leds from RJ_START to RJ_END
// #define HIDE_17_32 // pour le test sur panneau led - met a zero les led de 17 a 32. pas necessaire sur l'install
// #define BYPASS_RELAY_SECURITY // ne checke pas que les deux teensy soient actifs

//////////////////////////////
//////////////////////////////

#ifdef DEBUG
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#define DPRINT_BINARY(...) print_binary(__VA_ARGS__)
#else 
#define DPRINT(...)
#define DPRINTLN(...)
#define DPRINT_BINARY(...)
#endif

#ifdef WAIT_SERIAL
#define SERIAL_WAIT while(!Serial)
#else 
#define SERIAL_WAIT
#endif

#define LATCH 40 // common latch pin
#define RELAY 41 // relay pour alimentation

#define LATCH_DELAY 100 // temps min d'activation du latch pour un signal correct (micros)
#define CYCLES_MULTIPLY 3 // facteur de duree ON des signaux data + clock pour un signal correct

#define RJ_TOT 19 // quantite de sorties RJ actives
#define MODULE_SHIFT_REG 2 // quantite de shift reg par module
#define MODULE_SERIE_Q 1 // quantite de modules en serie, au bout de chaque rj12
#define SHIFT_REG_OUTPUT_Q 8 // quantite de sorties par module 

 /* pour run test sur chaque sortie */
#define RJ_START 1 // premiere sortie a jouer
#define RJ_END 19 // premiere sortie a jouer

/* pour le blink test - premiere element a tester */
#define RJ_TEST_OUT 5
#define RJ_LEAF_TEST_OUT 0
#define TEST_TIME_ON 10000 // temps ON pour test (micros)
#define TEST_TIME_OFF 1000 // temps OFF pour test (millis)

#define MIN_ON_TIME 100 // temps min leaf active (microsecondes) * 100 2023.9.22
#define MAX_ON_TIME 8000 // temps max leaf active (microsecondes) * 8000 2023.9.22

#define MIN_OFF_TIME 1000 // temps min leaf inactive (millis) * 1000 2023.9.22
#define MAX_OFF_TIME 15000 // temps max leaf inactive (millis) * 15000 2023.9.26

#define INPUT_CHECK 0
#define OUTPUT_CHECK 1


// #define TOT_LEAVES 601 // nombre total de feuilles // TBC



typedef struct s_leaf {
  u_int32_t     timeOn;
  u_int32_t     timeOff;
  u_int8_t      leaf_byte; // TBC, en int ou en byte ?
  bool          isActive;
  bool          testMode;
  elapsedMicros elapsed_on;
  elapsedMillis elapsed_off;
}               t_leaf;

typedef struct s_rj {
  u_int8_t  data;
  u_int8_t  clock;
  t_leaf    leaf[SHIFT_REG_OUTPUT_Q * MODULE_SHIFT_REG * MODULE_SERIE_Q];
  u_int8_t  shift_register[MODULE_SHIFT_REG * MODULE_SERIE_Q];
  u_int8_t  prev_shift_register[MODULE_SHIFT_REG * MODULE_SERIE_Q]; // etat precedant du shift register
}             t_rj;


void  shiftOut_msbFirst_rd(uint8_t dataPin, uint8_t clockPin, uint8_t value);
void  shiftOut_lsbFirst_rd(uint8_t dataPin, uint8_t clockPin, uint8_t value);
void  test_module(t_rj rj_out, int module_nbr, int del);
void  reset_module(t_rj rj_out, int module_nbr);
void  leaf_init(void);
void  leaf_status_update(void); //obsolete
void  leaf_status_update1(void);
void  leaf_test_mode(void);
void  test_mode_increment(void);
void  relay_and_check(void);



/* utils */
void  led_blink(int time_on, int time_off);
void  print_binary(int num);
int   bytes_count(int num);



// void  u_int8_to_binary(uint8_t num); // nul, WIP

