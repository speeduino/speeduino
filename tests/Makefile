CXX=g++
inofiles=$(wildcard ../speeduino/*.ino)
headerfiles=$(wildcard ../speeduino/*.h)

all: main tests

main: build/main.o build/Arduino.o build/FastCRC.o build/PID.o
	$(CXX) -o $@ $^

tests: build/tests.o build/Arduino.o build/FastCRC.o build/PID.o
	$(CXX) -o $@ $^ lib/libgtest.a -lpthread

build/main.cpp: src/speeduino.cpp src/main.cpp
	cat src/speeduino.cpp > $@
	echo "" >> $@
	cat src/main.cpp >> $@

build/tests.cpp: src/speeduino.cpp src/tests.cpp
	cat src/speeduino.cpp > $@
	echo "" >> $@
	cat src/tests.cpp >> $@

build/main.o: build/main.cpp
	$(CXX) -fpermissive -DF_CPU=8000000 -D__AVR_ATmega2560__ -I../speeduino -I./include -o $@ -c $<

build/tests.o: build/tests.cpp
	$(CXX) -fpermissive -DF_CPU=8000000 -D__AVR_ATmega2560__ -I../speeduino -I./include -o $@ -c $<

build/Arduino.o: src/Arduino.cpp include/Arduino.h
	$(CXX) -I./include -o $@ -c $<

build/FastCRC.o: ../speeduino/src/FastCRC/FastCRCsw.cpp
	$(CXX) -I./include -I./include/avr -o $@ -c $<

build/PID.o: ../speeduino/src/PID_v1/PID_v1.cpp
	$(CXX) -DARDUINO=100 -I./include -I./include/avr -o $@ -c $<

src/speeduino.cpp: $(inofiles) $(headerfiles)
	./get_cpp.sh


clean:
	rm -f build/* tests main

cleanall: clean
	rm -f speeduino.cpp

.PHONY: all clean cleanall
