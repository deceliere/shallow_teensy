#define DEBUG

#ifdef DEBUG
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#else 
#define DPRINT(...)
#define DPRINTLN(...)
#endif


void shiftOut_msbFirst_rd(uint8_t dataPin, uint8_t clockPin, uint8_t value);
void shiftOut_lsbFirst_rd(uint8_t dataPin, uint8_t clockPin, uint8_t value);
