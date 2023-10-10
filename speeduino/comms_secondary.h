#ifndef COMMS_SECONDARY_H
#define COMMS_SECONDARY_H

#define NEW_CAN_PACKET_SIZE   123
#define CAN_PACKET_SIZE   75

#define SECONDARY_SERIAL_PROTO_GENERIC_FIXED  0
#define SECONDARY_SERIAL_PROTO_GENERIC_INI    1
#define SECONDARY_SERIAL_PROTO_CAN            2
#define SECONDARY_SERIAL_PROTO_MSDROID        3
#define SECONDARY_SERIAL_PROTO_REALDASH       4

#if ( defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) )
  #define secondarySerial_AVAILABLE
  extern HardwareSerial &secondarySerial;
#elif defined(CORE_STM32)
  #define secondarySerial_AVAILABLE
  #ifndef HAVE_HWSERIAL2 //Hack to get the code to compile on BlackPills
    #define Serial2 Serial1
  #endif
  #if defined(STM32GENERIC) // STM32GENERIC core
    extern SerialUART &secondarySerial;
  #else //libmaple core aka STM32DUINO
    extern HardwareSerial &secondarySerial;
  #endif
#elif defined(CORE_TEENSY)
  #define secondarySerial_AVAILABLE
  extern HardwareSerial &secondarySerial;
#endif

void secondserial_Command(void);//This is the heart of the Command Line Interpreter.  All that needed to be done was to make it human readable.
void can_Command(void);
void sendCancommand(uint8_t cmdtype , uint16_t canadddress, uint8_t candata1, uint8_t candata2, uint16_t sourcecanAddress);
void obd_response(uint8_t therequestedPID , uint8_t therequestedPIDlow, uint8_t therequestedPIDhigh);
void readAuxCanBus();

#endif // COMMS_SECONDARY_H
