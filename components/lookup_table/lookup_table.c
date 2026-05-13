#include "lookup_table.h"
#include <stdbool.h>
#include "oled_config.h"

static void function_to_handle_display(char* message);
void struct_for_display_init();
void to_hand_control_to_oled(StructForDisplay* chosen_struct);

StructForDisplay struct_for_display_1={0};
StructForDisplay struct_for_display_2={0};
StructForDisplay struct_for_display_3={0};
StructForDisplay struct_for_display_4={0};
StructForDisplay struct_for_display_5={0};
StructForDisplay struct_for_display_6={0};
StructForDisplay struct_for_display_7={0};
StructForDisplay struct_for_display_8={0};
StructForDisplay struct_for_display_9={0};
StructForDisplay struct_for_display_10={0};
StructForDisplay struct_for_display_11={0};
StructForDisplay struct_for_display_12={0};
StructForDisplay struct_for_display_13={0};
StructForDisplay struct_for_display_14={0};
StructForDisplay struct_for_display_15={0};

void struct_for_display_init() {
    //struct_1_display intialization
    struct_for_display_1.message="ACK: BASE-PWR Grid A switched to backup generator";
    struct_for_display_1.function=function_to_handle_display;
    lookup_table[0]=&struct_for_display_1;
    //struct_2_display initialization
    struct_for_display_2.message="ACK: PERIM-GATE1 Secured and Locked";
    struct_for_display_2.function=function_to_handle_display;
    lookup_table[1]=&struct_for_display_2;
    //struct_3_display initialization
    struct_for_display_3.message="ACK: COMM-NODE3 Link established, encryption active";
    struct_for_display_3.function=function_to_handle_display;
    lookup_table[2]=&struct_for_display_3;
    //struct_4_display initialization
    struct_for_display_4.message="ACK: HVAC-BUNKER2 Temp set to 297K,mode-auto";
    struct_for_display_4.function=function_to_handle_display;
    lookup_table[3]=&struct_for_display_4;
    //struct_5_display initialization
    struct_for_display_5.message="ACK: FUEL-TANK4 Transfer complete, valve closed";
    struct_for_display_5.function=function_to_handle_display;
    lookup_table[4]=&struct_for_display_5;
    //struct_6_display initialization
    struct_for_display_6.message="ACK: LIGHTS-EXT Perimeter lights set to low intensity";
    struct_for_display_6.function=function_to_handle_display;
    lookup_table[5]=&struct_for_display_6;
    //struct_7_display initialization
    struct_for_display_7.message="ACK: ALM-RESET All non-critical alarms cleared";
    struct_for_display_7.function=function_to_handle_display;
    lookup_table[6]=&struct_for_display_7;
    //struct_8_display initialization
    struct_for_display_8.message="ACK: WATER-PUMP1 Flow rate stabilized at 120L/min";
    struct_for_display_8.function=function_to_handle_display;
    lookup_table[7]=&struct_for_display_8;
    //struct_9_display initialization
    struct_for_display_9.message="ACK: RADAR-SITE2 System Online, self-test passed";
    struct_for_display_9.function=function_to_handle_display;
    lookup_table[8]=&struct_for_display_9;
    //struct_10_display initialization
    struct_for_display_10.message="ACK: DOOR-ARMORY Locked, access logged";
    struct_for_display_10.function=function_to_handle_display;
    lookup_table[9]=&struct_for_display_10;
    //struct_11_display initialization
    struct_for_display_11.message="ACK: SCADA-SYNC DB updated, version 6.03";
    struct_for_display_11.function=function_to_handle_display;
    lookup_table[10]=&struct_for_display_11;
    //struct_12_display initialization
    struct_for_display_12.message="ACK: UPS-RACK5 Switched to battery, load 42\%";
    struct_for_display_12.function=function_to_handle_display;
    lookup_table[11]=&struct_for_display_12;
    //struct_13_display initialization
    struct_for_display_13.message="ACK: GATE-VEHICLE gate cycled, clear";
    struct_for_display_13.function=function_to_handle_display;
    lookup_table[12]=&struct_for_display_13;
    //struct_14_display initialization
    struct_for_display_14.message="ACK: HVAC-COMMS Temp alarm cleared, nominal";
    struct_for_display_14.function=function_to_handle_display;
    lookup_table[13]=&struct_for_display_14;
    //struct_15_display initialization
    struct_for_display_15.message="ACK: NET-SWITCH Sector 7 VLAN isolated";
    struct_for_display_15.function=function_to_handle_display;
    lookup_table[14]=&struct_for_display_15;
}
static void function_to_handle_display(char* message) {
    oled_screen_print(message);
}
void to_hand_control_to_oled(StructForDisplay* chosen_struct) {
    if (!chosen_struct) {
        printf("Pointer for StructForDisplay doesn't exist!\n");
        fflush(stdout);
        return;
    }
    chosen_struct->function(chosen_struct->message);
}