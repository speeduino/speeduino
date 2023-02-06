pio run -t clean
pio run

rm speeduino.S
~/.platformio/packages/toolchain-atmelavr/bin/avr-objdump -d .pio/build/megaatmega2560/firmware.elf > speeduino.S

rm size.txt
~/.platformio/packages/toolchain-atmelavr/bin/avr-nm -S --size-sort .pio/build/megaatmega2560/firmware.elf > size.txt
