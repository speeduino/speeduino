#include <Serial.h>

#define vePage    1
#define ignPage   2

byte currentPage;

void command();
void sendValues();
void receiveValue(byte offset, byte newValue);
void saveConfig();
void sendPage();
void testComm();
