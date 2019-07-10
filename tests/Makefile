CXX=g++
INOSOURCES=$(wildcard ../speeduino/*.ino) $(wildcard ../speeduino/*.h)

all: main test

main: build/main.o build/Arduino.o build/FastCRC.o build/speeduino.o build/PID.o
	$(CXX) -o $@ $^

test: build/main_tests.o build/Arduino.o build/FastCRC.o build/speeduino.o build/PID.o
	$(CXX) -o $@ $^ lib/libgtest.a -lpthread

build/main.o: main.cpp include/mock_globals.h
	$(CXX) -D__AVR_ATmega2560__ -I../speeduino -I./include -o $@ -c $<

build/main_tests.o: main_tests.cpp include/mock_globals.h
	$(CXX) -D__AVR_ATmega2560__ -I../speeduino -I./include -o $@ -c $<

build/Arduino.o: include/Arduino.cpp
	$(CXX) -I./include -o $@ -c $<

build/FastCRC.o: ../speeduino/src/FastCRC/FastCRCsw.cpp
	$(CXX) -I./include -I./include/avr -o $@ -c $<

build/speeduino.o: speeduino.cpp
	$(CXX) -fpermissive -DF_CPU=8000000 -D__AVR_ATmega2560__ -I../speeduino -I./include -o $@ -c $<

build/PID.o: ../speeduino/src/PID_v1/PID_v1.cpp
	$(CXX) -DARDUINO=100 -I./include -I./include/avr -o $@ -c $<

include/mock_globals.h: speeduino.cpp
	cat include/my_globals.h > $@
	echo "" >> $@
	sed -e 's/^int\|^uint\|^bool\|^volatile\|^unsigned\|^byte/extern \0/' -e '/{/!s/^struct*/extern \0/' \
	    -e '/static_assert/d' -e '/const/!s/=.*;/;/g' \
	    ../speeduino/globals.h >> $@
	sed -e '/#line/q' -e '/^#/d' -e '/^\/\/#/d' speeduino.cpp >> $@

speeduino.cpp: $(INOSOURCES)
	./get_cpp.sh

.PHONY: clean
clean:
	rm -f build/* test main
