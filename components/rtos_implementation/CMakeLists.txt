idf_component_register(
    SRCS "rtos_implementation.c" "/home/samuel/Documents/RS-485_Telemetry_Hub/components/max_chip_config/max_chip_config.c"
    INCLUDE_DIRS "."
    REQUIRES driver oled_config max_chip_config crc_check lookup_table
)