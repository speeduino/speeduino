ARDUINO_INCLUDE=~/.platformio/packages/framework-arduinoavr/cores/arduino/
AVR_INCLUDE=~/.platformio/packages/toolchain-atmelavr/avr/include/
MEGA_INCLUDE=~/.platformio/packages/framework-arduinoavr/variants/mega/
EEPROM_INCLUDE=~/.platformio/packages/framework-arduinoavr/libraries/__cores__/arduino/EEPROM/src/

g++ -fpermissive -DF_CPU=8000000 -D__AVR_ATmega2560__ -I../speeduino -I$ARDUINO_INCLUDE -I$AVR_INCLUDE -I$MEGA_INCLUDE -I$EEPROM_INCLUDE -o test speeduino.cpp
