#include <stdbool.h>
#include <stdint.h>

unsigned int crc32(uint16_t message);
bool is_crc_passed(uint16_t message, unsigned int crc_to_check);