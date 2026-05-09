#include "crc_check.h"
#include <stdbool.h>
#include <stdint.h>
unsigned int crc32(uint8_t message);
bool is_crc_passed(uint8_t message, unsigned int crc_to_check);

unsigned int crc32(uint8_t message) {
    int j;
    unsigned int byte, crc, mask;
    crc = 0xFFFFFFFF;
    byte = message;
    crc = crc ^ byte;
    for (j = 7; j >= 0; j--) {
        mask = -(crc & 1);
        crc = (crc >> 1) ^ (0xEDB88320 & mask);
    }    
    return ~crc;
}
bool is_crc_passed(uint8_t message, unsigned int crc_to_check) {
    if (crc32(message) == crc_to_check) return true;
    else return false;
}