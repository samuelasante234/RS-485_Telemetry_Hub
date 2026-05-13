#include <stdint.h>
#include <string.h>
#include <stdbool.h>


typedef struct {
    char* message;
    void (*function)(char*);
    bool is_long;
} StructForDisplay;
StructForDisplay* lookup_table[256];
void struct_for_display_init();
void to_hand_control_to_oled(StructForDisplay* chosen_struct);


extern StructForDisplay struct_for_display_1;
extern StructForDisplay struct_for_display_2;
extern StructForDisplay struct_for_display_3;
extern StructForDisplay struct_for_display_4;
extern StructForDisplay struct_for_display_5;
extern StructForDisplay struct_for_display_6;
extern StructForDisplay struct_for_display_7;
extern StructForDisplay struct_for_display_8;
extern StructForDisplay struct_for_display_9;
extern StructForDisplay struct_for_display_10;
extern StructForDisplay struct_for_display_11;
extern StructForDisplay struct_for_display_12;
extern StructForDisplay struct_for_display_13;
extern StructForDisplay struct_for_display_14;
extern StructForDisplay struct_for_display_15;