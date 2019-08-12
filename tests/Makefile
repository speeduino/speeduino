CXX=g++
inofiles=$(wildcard ../speeduino/*.ino)
headerfiles=$(wildcard ../speeduino/*.h)

all: main tests

main: build/main.o build/Arduino.o build/FastCRC.o build/PID.o
	$(CXX) -o $@ $^

tests: build/tests.o build/Arduino.o build/FastCRC.o build/PID.o lib/libgtest.a
	$(CXX) -o $@ $^ -lpthread

build/main.o: src/main.cpp src/speeduino.cpp include/EEPROM.h
	mkdir -p build
	$(CXX) -fpermissive -DF_CPU=8000000 -D__AVR_ATmega2560__ -I../speeduino -I./include -o $@ -c $<

build/tests.o: src/tests.cpp src/speeduino.cpp include/EEPROM.h include/gtest
	mkdir -p build
	$(CXX) -fpermissive -DF_CPU=8000000 -D__AVR_ATmega2560__ -I../speeduino -I./include -o $@ -c $<

build/Arduino.o: src/Arduino.cpp include/Arduino.h
	mkdir -p build
	$(CXX) -I./include -o $@ -c $<

build/FastCRC.o: ../speeduino/src/FastCRC/FastCRCsw.cpp
	mkdir -p build
	$(CXX) -I./include -I./include/avr -o $@ -c $<

build/PID.o: ../speeduino/src/PID_v1/PID_v1.cpp
	mkdir -p build
	$(CXX) -DARDUINO=100 -I./include -I./include/avr -o $@ -c $<

src/speeduino.cpp: $(inofiles) $(headerfiles)
	./get_cpp.sh


lib/libgtest.a: gtest

include/gtest: gtest

gtest:
	./get_googletest.sh


clean:
	rm -f build/* tests main

cleanall: clean
	rm -f src/speeduino.cpp

.PHONY: all clean cleanall gtest
