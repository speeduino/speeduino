/*
These are some utility functions and variables used through the main code
*/ 
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#define MS_IN_MINUTE 60000
#define US_IN_MINUTE 60000000

int freeRam ();
void setPinMapping(byte boardID);
unsigned int PW();
unsigned int PW_SD();
unsigned int PW_AN();

#endif // UTILS_H

#if defined (__MK20DX256__)
#ifndef TEENSYRAM_H
#define TEENSYRAM_H

	#include <malloc.h>
	#include <inttypes.h>

	extern int* __brkval;
	extern char _estack;

	class TeensyRam{
	private:
		typedef uint32_t MemMarker;
		typedef uint8_t MemState;

		static const uint16_t STACKALLOCATION = 1024;
		static const uint32_t HWADDRESS_RAMSTART = 0x1FFF8000;

		static const MemMarker MEMMARKER = 0x524D6D6D;
		static const uint16_t MARKER_STEP = STACKALLOCATION / sizeof(MemMarker);

		static const MemState msOk = 0;

		MemMarker* _mlastmarker;
		MemState _mstate;

		void _check_stack(){
			int32_t free;

			free = ((char*) &free) - ((char*) _mlastmarker);
			if (free < 0){
				int32_t steps;

				steps = free / STACKALLOCATION;
				if(free % STACKALLOCATION)
					--steps;
				_mlastmarker += MARKER_STEP * steps;
			};
			while((*_mlastmarker != MEMMARKER) && (_mlastmarker >= (MemMarker*) __brkval))
				_mlastmarker -= MARKER_STEP;
		};

	public:
		int32_t unallocated() const { char tos; return &tos - (char*) __brkval; };
		uint32_t heap_free() const { return mallinfo().fordblks; };
		
		int32_t adj_free() { return adj_unallocd() + heap_free() ; };

		int32_t adj_unallocd(){
			_check_stack();
			return ((char*) _mlastmarker) - (char*) __brkval;
		};

		void initialize(){
			MemMarker* marker = (MemMarker*) &_estack;
			int32_t size;
			int32_t steps;

			size = &_estack - (char*) &marker;
			steps = size / STACKALLOCATION;
			if(size % STACKALLOCATION)
				++steps;

			marker -= MARKER_STEP * steps;

			_mlastmarker = marker;
			_mstate = msOk;

			while(marker >= (MemMarker*) __brkval){
				*marker = MEMMARKER;
				marker -= MARKER_STEP;
			};
		};
	};  	
#endif
#endif