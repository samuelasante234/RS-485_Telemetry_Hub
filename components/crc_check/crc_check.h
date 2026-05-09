#include <stdbool.h>
#include <stdint.h>

unsigned int crc32(uint8_t message);
bool is_crc_passed(uint8_t message, unsigned int crc_to_check);