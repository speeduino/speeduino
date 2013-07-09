#include <Serial.h>
#include "globals.h"

#define vePage    1
#define ignPage   2

byte currentPage;

void command();
void sendValues();
void saveConfig();
void sendPage();
void testComm();
