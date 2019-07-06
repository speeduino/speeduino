#clean previous build files
[ -d build ] && rm -rf build
mkdir build

#compile libraries
g++ -c include/Arduino.cpp -o build/Arduino.o -I./include
g++ -c -DARDUINO=100 ../speeduino/src/PID_v1/PID_v1.cpp -o build/PID.o -I./include -I./include/avr
g++ -c ../speeduino/src/FastCRC/FastCRCsw.cpp -o build/FastCRC.o -I./include -I./include/avr

#compile speeduino
g++ -fpermissive -DF_CPU=8000000 -D__AVR_ATmega2560__ -I../speeduino -I./include -c -o build/speeduino.o speeduino.cpp

#compile main
g++ -D__AVR_ATmega2560__ -I../speeduino -I./include -c -o build/main_tests.o main_tests.cpp

#link all that together into a working program
g++ -o test build/Arduino.o build/FastCRC.o build/speeduino.o build/main_tests.o build/PID.o lib/libgtest.a -lpthread
