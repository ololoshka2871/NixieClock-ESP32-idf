#file NixieClock-ESP32-idf.elf
target remote :3333
monitor reset halt
monitor program_esp32 NixieClock-ESP32-idf.bin 0x10000 verify exit
quit

