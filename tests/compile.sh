rm -rf build
mkdir build

g++ -c include/Arduino.cpp -o build/Arduino.o -I./include
g++ -c -DARDUINO=100 ../speeduino/src/PID_v1/PID_v1.cpp -o build/PID.o -I./include -I./include/avr
g++ -c ../speeduino/src/FastCRC/FastCRCsw.cpp -o build/FastCRC.o -I./include -I./include/avr
g++ -fpermissive -DF_CPU=8000000 -D__AVR_ATmega2560__ -I../speeduino -I./include -c -o build/speeduino.o speeduino.cpp
g++ -fpermissive -c -D__AVR_ATmega2560__ -I../speeduino -I./include -o build/main.o main.cpp

g++ -o test build/Arduino.o build/FastCRC.o build/speeduino.o build/main.o build/PID.o
