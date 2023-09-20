#include <Arduino.h>
#include "Entropy.h"

#define DEBUG
#define WAIT_SERIAL
#define BLINK_TEST_MODE

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
#define RELAY 41 // relay pour alimentation 12v pin

#define LATCH_DELAY 100 // temps min d'activation du latch pour un signal correct (micros)
#define CYCLES_MULTIPLY 3 // facteur de duree ON des signaux data + clock pour un signal correct

#define RJ_TOT 19 // quantite de sorties RJ actives
#define MODULE_SHIFT_REG 2 // quantite de shift reg par module
#define MODULE_SERIE_Q 2 // quantite de modules en serie, au bout de chaque rj12
#define SHIFT_REG_OUTPUT_Q 8 // quantite de sorties par module 
#define MODULE_OUTPUT_Q 16 // quantite de sorties par module 

 /* pour run test sur chaque sortie */
#define RJ_START 5 // premiere sortie a jouer
#define RJ_END 5 // premiere sortie a jouer

#define MIN_ON_TIME 100 // temps min leaf active (microsecondes)
#define MAX_ON_TIME 8000 // temps max leaf active (microsecondes)

#define MIN_OFF_TIME 1000 // temps min leaf inactive (millis) * 10000 2023.8.25
#define MAX_OFF_TIME 10000 // temps max leaf inactive (millis) * 100000 2023.8.25

#define TEST_TIME_ON 10000 // temps ON pour test (micros)
#define TEST_TIME_OFF 1000 // temps OFF pour test (millis)

#define TOT_LEAVES 601 // nombre total de feuilles // TBC



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
  t_leaf    leaf[MODULE_SERIE_Q * MODULE_OUTPUT_Q];
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



/* utils */
void  led_blink(int time_on, int time_off);
void  print_binary(int num);
int   bytes_count(int num);



// void  u_int8_to_binary(uint8_t num); // nul, WIP

