#include "shallow.h"

void u_int8_to_binary(uint8_t num) {
    char* binary_representation;
    for (int i = 7; i >= 0; --i) {
        uint8_t mask = 1 << i;
        binary_representation += ((num & mask) ? '1' : '0');
    }
    DPRINTLN(binary_representation);
    // return binary_representation;
}